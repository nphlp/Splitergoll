// Le worker apprend seulement après pairing_complete, jamais sur une simple reconnexion.
#include <splitergoll/ble_acquisition.h>
#include <splitergoll/ble_os_classifier.h>
#include <splitergoll/ble_observer.h>
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
    uint32_t epoch;
};
struct acquisition {
    struct ble_os_classifier classifier;
    bool fresh;
    bool bound;
    bool existing_at_open;
    int profile;
    uint32_t epoch;
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
void splitergoll_ble_acquisition_event(struct bt_conn *conn, enum ble_trace_kind kind,
                             const uint32_t data[5], uint16_t uuid, bool reliable) {
    if (kind != BLE_TRACE_CONNECTED && kind != BLE_TRACE_DISCONNECTED &&
        kind != BLE_TRACE_SECURITY && kind != BLE_TRACE_ATT) return;
    struct message message = {.conn = conn, .uuid = uuid, .reliable = reliable,
                               .epoch = splitergoll_ble_connection_epoch(conn),
                               .event = {.kind = kind}};
    memcpy(message.event.data, data, sizeof(message.event.data));
    enqueue(&message);
}

// Un appairage neuf arme l’acquisition, la connexion seule ne suffit jamais.
#if defined(CONFIG_SPLITERGOLL_BLE_RECONNECT)
void splitergoll_ble_acquisition_paired(struct bt_conn *conn, bool bonded) {
#else
static void paired(struct bt_conn *conn, bool bonded) {
#endif
    struct bt_conn_info info;
    if (!bonded || bt_conn_get_info(conn, &info) != 0 ||
        info.type != BT_CONN_TYPE_LE || info.role != BT_CONN_ROLE_PERIPHERAL) return;
    splitergoll_ble_connection_changed(conn);
    struct message message = {.conn = conn, .kind = PAIRED,
                               .epoch = splitergoll_ble_connection_epoch(conn)};
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
    static struct bt_conn_auth_info_cb callbacks = {
#if !defined(CONFIG_SPLITERGOLL_BLE_RECONNECT)
        .pairing_complete = paired,
#endif
        .bond_deleted = forgotten,
    };
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
    session->epoch = message->epoch;
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
#if defined(CONFIG_SPLITERGOLL_BLE_AUTO)
        splitergoll_ble_auto_notify();
#endif
    }
}
K_THREAD_DEFINE(ble_memory_thread, 4096, memory_worker, NULL, NULL, NULL, 10, 0, 0);

// La sélection lit un instantané cohérent, sans déclencher de détection ou de sauvegarde.
struct ble_acquisition_snapshot splitergoll_ble_acquisition_snapshot(unsigned profile) {
    struct ble_acquisition_snapshot snapshot = {0};
    if (profile >= ZMK_BLE_PROFILE_COUNT) return snapshot;
    k_mutex_lock(&acquisition_mutex, K_FOREVER);
    snapshot.memory = ble_os_settings_snapshot(profile);
    snapshot.measured = measured[profile];
    snapshot.learning = learning[profile];
    snapshot.reliable = !atomic_get(&lost);
    for (unsigned i = 0; i < BLE_TRACE_CONNECTION_LIMIT; i++) {
        const struct acquisition *session = &sessions[i];
        if (session->profile != (int)profile || !session->classifier.connected) continue;
        snapshot.session = session->classifier.session;
        snapshot.epoch = session->epoch;
        snapshot.connected = true;
        snapshot.secure = session->classifier.encrypted && !session->classifier.security_failed &&
                          !session->classifier.invalid;
    }
    k_mutex_unlock(&acquisition_mutex);
    return snapshot;
}

// Copie le diagnostic sous son verrou, sans exposer les identités privées.
void splitergoll_ble_acquisition_status(struct ble_os_memory *snapshots, enum ble_os_guess *observations,
                                       bool *acquisitions, unsigned *dropped) {
    k_mutex_lock(&acquisition_mutex, K_FOREVER);
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        snapshots[i] = ble_os_settings_snapshot(i);
        observations[i] = measured[i];
        acquisitions[i] = learning[i];
    }
    *dropped = (unsigned)atomic_get(&lost);
    k_mutex_unlock(&acquisition_mutex);
}
