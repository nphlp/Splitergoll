// Le worker apprend seulement après pairing_complete, jamais sur une simple reconnexion.
#include "ble_trace_memory.h"
#include "ble_os_classifier.h"
#include "trace_console.h"
#include <splitergoll/ble_os_settings.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zmk/ble.h>

enum message_kind { OBSERVATION, PAIRED, FORGOTTEN };
struct message {
    struct bt_conn *conn;
    struct ble_trace_record event;
    enum message_kind kind;
    uint16_t uuid;
    int profile;
    bool reliable;
};
struct acquisition {
    struct ble_os_classifier classifier;
    bool fresh;
    bool bound;
    bool existing_at_open;
    int profile;
};

static struct acquisition sessions[BLE_TRACE_CONNECTION_LIMIT];
static enum ble_os_guess measured[ZMK_BLE_PROFILE_COUNT];
static bool learning[ZMK_BLE_PROFILE_COUNT];
static uint32_t next_session;
static atomic_t lost;
static bool overflow_handled;
K_MUTEX_DEFINE(acquisition_mutex);
K_MSGQ_DEFINE(memory_messages, sizeof(struct message), 64, 4);

// Libère toujours la référence, même si une surcharge invalide l’acquisition de ce démarrage.
static void enqueue(struct message *message) {
    if (message->conn != NULL) bt_conn_ref(message->conn);
    if (k_msgq_put(&memory_messages, message, K_NO_WAIT) != 0) {
        atomic_set(&lost, 1);
        if (message->conn != NULL) bt_conn_unref(message->conn);
    }
}

// Les paramètres radio génériques ne sont pas utiles à la mémoire et ne remplissent pas sa file.
void ble_trace_memory_event(struct bt_conn *conn, enum ble_trace_kind kind,
                             const uint32_t data[5], uint16_t uuid, bool reliable) {
    if (kind != BLE_TRACE_CONNECTED && kind != BLE_TRACE_DISCONNECTED &&
        kind != BLE_TRACE_SECURITY && kind != BLE_TRACE_ATT) return;
    struct message message = {.conn = conn, .uuid = uuid, .reliable = reliable,
                               .event = {.kind = kind}};
    memcpy(message.event.data, data, sizeof(message.event.data));
    enqueue(&message);
}

// Un appairage neuf arme l’acquisition, la connexion seule ne suffit jamais.
static void paired(struct bt_conn *conn, bool bonded) {
    struct bt_conn_info info;
    if (!bonded || bt_conn_get_info(conn, &info) != 0 ||
        info.type != BT_CONN_TYPE_LE || info.role != BT_CONN_ROLE_PERIPHERAL) return;
    struct message message = {.conn = conn, .kind = PAIRED};
    enqueue(&message);
}

// L’effacement natif d’un bond invalide son OS sans effacer les autres liens du clavier.
static void forgotten(uint8_t id, const bt_addr_le_t *peer) {
    if (id != BT_ID_DEFAULT) return;
    int profile = zmk_ble_profile_index(peer);
    if (profile < 0) return;
    struct message message = {.kind = FORGOTTEN, .profile = profile};
    enqueue(&message);
}

// Enregistre les notifications avant le démarrage Bluetooth natif de ZMK.
static int memory_init(void) {
    static struct bt_conn_auth_info_cb callbacks = {.pairing_complete = paired, .bond_deleted = forgotten};
    return bt_conn_auth_info_cb_register(&callbacks);
}
SYS_INIT(memory_init, APPLICATION, 0);

// Résout l’emplacement après les callbacks ZMK, sans faire du numéro une signature d’OS.
static void acquire(struct acquisition *session, struct bt_conn *conn) {
    int profile = zmk_ble_profile_index(bt_conn_get_dst(conn));
    if (profile < 0 || profile >= ZMK_BLE_PROFILE_COUNT) return;
    if (session->profile >= 0 && session->profile != profile) {
        session->classifier.invalid = true;
        return;
    }
    session->profile = profile;
    if (!session->bound) session->bound = ble_os_settings_bind(profile, session->fresh);
    if (!session->bound || !session->fresh) return;
    learning[profile] = session->classifier.connected;
    enum ble_os_guess guess = ble_os_result(&session->classifier, !atomic_get(&lost));
    measured[profile] = guess;
    enum ble_memory_os os = guess == BLE_OS_MACOS ? BLE_MEMORY_MACOS :
                            guess == BLE_OS_WINDOWS ? BLE_MEMORY_WINDOWS : BLE_MEMORY_UNKNOWN;
    ble_os_settings_learn(profile, os, session->classifier.evidence);
}

// Traite les événements dans l’ordre, avec un classifieur distinct par connexion simultanée.
static void process(struct message *message) {
    if (atomic_get(&lost) && !overflow_handled) {
        overflow_handled = true;
        for (unsigned i = 0; i < BLE_TRACE_CONNECTION_LIMIT; i++) {
            struct acquisition *session = &sessions[i];
            session->classifier.invalid = true;
            if (session->fresh && session->bound && session->profile >= 0) {
                measured[session->profile] = BLE_OS_UNKNOWN;
                ble_os_settings_learn(session->profile, BLE_MEMORY_UNKNOWN, 0);
            }
        }
    }
    if (message->kind == FORGOTTEN) {
        ble_os_settings_forget(message->profile);
        measured[message->profile] = BLE_OS_UNKNOWN;
        learning[message->profile] = false;
        return;
    }
    unsigned index = bt_conn_index(message->conn);
    if (index >= BLE_TRACE_CONNECTION_LIMIT) {
        atomic_set(&lost, 1);
        return;
    }
    struct acquisition *session = &sessions[index];
    if (message->kind == PAIRED) {
        // Une seconde association sur la même connexion ne réutilise pas les anciennes preuves.
        if (session->fresh || session->existing_at_open) {
            session->classifier = (struct ble_os_classifier){
                .session = session->classifier.session,
                .connected = session->classifier.connected,
                .encrypted = session->classifier.encrypted,
            };
        }
        session->fresh = true;
        session->bound = false;
    } else {
        if (message->event.kind == BLE_TRACE_CONNECTED) {
            *session = (struct acquisition){.profile = -1,
                .existing_at_open = message->event.data[4] != 0};
            if (next_session == UINT32_MAX) atomic_set(&lost, 1);
            else next_session++;
            session->classifier.session = 0;
            message->event.session = next_session;
        } else {
            message->event.session = session->classifier.session;
        }
        ble_os_observe(&session->classifier, &message->event, message->uuid);
        if (!message->reliable) session->classifier.invalid = true;
    }
    acquire(session, message->conn);
    if (!session->classifier.connected && session->profile >= 0) learning[session->profile] = false;
}

// Les accès NVS se font dans ce thread dédié, jamais dans le traitement des touches ou du BLE.
static void memory_worker(void *unused1, void *unused2, void *unused3) {
    ARG_UNUSED(unused1);
    ARG_UNUSED(unused2);
    ARG_UNUSED(unused3);
    struct message message;
    ble_os_settings_wait_ready();
    while (true) {
        k_msgq_get(&memory_messages, &message, K_FOREVER);
        k_mutex_lock(&acquisition_mutex, K_FOREVER);
        process(&message);
        k_mutex_unlock(&acquisition_mutex);
        if (message.conn != NULL) bt_conn_unref(message.conn);
    }
}
K_THREAD_DEFINE(ble_memory_thread, 4096, memory_worker, NULL, NULL, NULL, 10, 0, 0);

// Ne restitue ni empreinte, ni adresse, ni clé. La lecture ne déclenche aucune sauvegarde.
void ble_trace_memory_dump(const struct device *uart) {
    struct ble_os_memory snapshots[ZMK_BLE_PROFILE_COUNT];
    enum ble_os_guess observations[ZMK_BLE_PROFILE_COUNT];
    bool acquisitions[ZMK_BLE_PROFILE_COUNT];
    k_mutex_lock(&acquisition_mutex, K_FOREVER);
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        snapshots[i] = ble_os_settings_snapshot(i);
        observations[i] = measured[i];
        acquisitions[i] = learning[i];
    }
    k_mutex_unlock(&acquisition_mutex);
    char line[240];
    snprintf(line, sizeof(line), "BLEMEM BEGIN version=1 count=%u dropped=%u\n",
             ZMK_BLE_PROFILE_COUNT, (unsigned)atomic_get(&lost));
    if (!trace_send_line(uart, line)) return;
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        const struct ble_os_memory *memory = &snapshots[i];
        const char *state = memory->error ? "error" : !memory->present ? "empty" :
                            !memory->verified ? "unverified" : memory->dirty ? "pending" : "saved";
        const char *origin = memory->origin == BLE_MEMORY_MANUAL ? "manual" :
                             memory->origin == BLE_MEMORY_INITIAL ? "initial" : "none";
        enum ble_memory_os os = memory->verified && !memory->dirty && !memory->error ?
                                memory->os : BLE_MEMORY_UNKNOWN;
        snprintf(line, sizeof(line), "BLEMEM SLOT index=%u measured=%s stored=%s origin=%s "
                 "state=%s learning=%u evidence=%04x error=%d\n", i,
                 ble_os_name(observations[i]), ble_memory_os_name(os), origin, state,
                 acquisitions[i], memory->evidence, memory->error);
        if (!trace_send_line(uart, line)) return;
    }
    trace_send_line(uart, "BLEMEM END\n");
}
