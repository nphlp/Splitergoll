// Observe uniquement les connexions où la centrale clavier est périphérique de l’ordinateur.
// La droite, les adresses Bluetooth, SMP et les valeurs HID ne sont jamais enregistrés.
#include "ble_trace_store.h"
#include "ble_os_classifier.h"
#include "ble_trace_memory.h"
#include "trace_console.h"

#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/net_buf.h>
#include <zephyr/sys/byteorder.h>
#include <zmk/ble.h>

static struct ble_trace_store store;
static struct k_spinlock store_lock;
BUILD_ASSERT(CONFIG_BT_MAX_CONN <= BLE_TRACE_CONNECTION_LIMIT);

// Vérifie le rôle local, sans supposer qu’un index de connexion désigne toujours le Mac.
static bool is_host(struct bt_conn *conn) {
    struct bt_conn_info info;
    return bt_conn_get_info(conn, &info) == 0 && info.type == BT_CONN_TYPE_LE &&
           info.role == BT_CONN_ROLE_PERIPHERAL;
}

// Normalise les erreurs ZMK en emplacement inconnu, notamment avant le premier appairage.
static int host_profile(struct bt_conn *conn) {
    int profile = zmk_ble_profile_index(bt_conn_get_dst(conn));
    return profile < 0 ? -1 : profile;
}

// Copie des métadonnées brèves sous verrou, sans accès série dans le callback Bluetooth.
static void record(struct bt_conn *conn, enum ble_trace_kind kind,
                   uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    if (!is_host(conn)) {
        return;
    }
    unsigned connection = bt_conn_index(conn);
    int profile = host_profile(conn);
    const uint32_t data[5] = {a, b, c, d, e};
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    if (connection < BLE_TRACE_CONNECTION_LIMIT && store.connections[connection].session != 0) {
        ble_trace_profile(&store, connection, profile);
        ble_trace_append(&store, connection, k_uptime_get_32(), kind, data);
    }
    k_spin_unlock(&store_lock, key);
    ble_trace_memory_event(conn, kind, data, 0, true);
}

// Ouvre une session numérotée, en ne conservant que le type d’adresse publique ou aléatoire.
static void connected(struct bt_conn *conn, uint8_t error) {
    struct bt_conn_info info;
    if (error || !is_host(conn) || bt_conn_get_info(conn, &info) != 0) {
        return;
    }
    int profile = host_profile(conn);
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    ble_trace_connect(&store, bt_conn_index(conn), profile, k_uptime_get_32(), info.le.interval, info.le.latency,
                      info.le.timeout, info.le.dst->type);
    k_spin_unlock(&store_lock, key);
    const uint32_t data[5] = {0, 0, 0, 0, (uint32_t)(profile + 1)};
    ble_trace_memory_event(conn, BLE_TRACE_CONNECTED, data, 0, true);
}

// Garde la déconnexion pour distinguer premier appairage et reconnexions dans un même dump.
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    if (!is_host(conn)) {
        return;
    }
    unsigned connection = bt_conn_index(conn);
    int profile = host_profile(conn);
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    if (connection < BLE_TRACE_CONNECTION_LIMIT && store.connections[connection].session != 0) {
        ble_trace_profile(&store, connection, profile);
        ble_trace_disconnect(&store, connection, k_uptime_get_32(), reason);
    }
    k_spin_unlock(&store_lock, key);
    const uint32_t data[5] = {reason, 0, 0, 0, 0};
    ble_trace_memory_event(conn, BLE_TRACE_DISCONNECTED, data, 0, true);
}

// Observe les paramètres proposés, puis laisse les callbacks ZMK décider normalement.
static bool parameter_request(struct bt_conn *conn, struct bt_le_conn_param *parameters) {
    record(conn, BLE_TRACE_PARAM_REQUEST, parameters->interval_min, parameters->interval_max,
           parameters->latency, parameters->timeout, 0);
    return true;
}

// Conserve aussi les paramètres effectivement négociés, distincts de la proposition.
static void parameter_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency,
                               uint16_t timeout) {
    record(conn, BLE_TRACE_PARAM_UPDATED, interval, latency, timeout, 0, 0);
}

// Note le niveau de sécurité et l’erreur, sans lire les clés ou modifier l’appairage.
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err error) {
    record(conn, BLE_TRACE_SECURITY, level, error, 0, 0, 0);
}

// Observe les fonctionnalités déjà échangées par Zephyr, sans activer de requête de version.
static void remote_info(struct bt_conn *conn, struct bt_conn_remote_info *info) {
    if (info->type == BT_CONN_TYPE_LE && info->le.features != NULL) {
        record(conn, BLE_TRACE_FEATURES, sys_get_le32(info->le.features),
               sys_get_le32(info->le.features + 4), 0, 0, 0);
    }
}

BT_CONN_CB_DEFINE(ble_trace_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .le_param_req = parameter_request,
    .le_param_updated = parameter_updated,
    .security_changed = security_changed,
    .remote_info_available = remote_info,
};

// Enregistre le MTU négocié dans les deux directions, sans lancer d’échange supplémentaire.
static void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx) {
    record(conn, BLE_TRACE_MTU, tx, rx, 0, 0, 0);
}

// Déclare le callback GATT avant l’initialisation Bluetooth de ZMK.
static int trace_init(void) {
    static struct bt_gatt_cb callbacks = {.att_mtu_updated = mtu_updated};
    bt_gatt_cb_register(&callbacks);
    return 0;
}

SYS_INIT(trace_init, APPLICATION, 0);

// Signature interne figée de Zephyr 4.1, appelée entre conn.c et l2cap.c sans LTO.
void __real_bt_l2cap_recv(struct bt_conn *conn, struct net_buf *buffer, bool complete);

// Résout plus bas les handles nécessaires au classifieur en ligne, sans lire leur valeur.
static uint16_t attribute_uuid(const struct ble_trace_record *event, bool *reliable);

// Lit sans consommer le tampon, puis délègue exactement une fois au traitement natif.
void __wrap_bt_l2cap_recv(struct bt_conn *conn, struct net_buf *buffer, bool complete) {
    if (is_host(conn)) {
        unsigned connection = bt_conn_index(conn);
        k_spinlock_key_t key = k_spin_lock(&store_lock);
        if (connection < BLE_TRACE_CONNECTION_LIMIT && store.connections[connection].session != 0) {
            ble_trace_l2cap(&store, connection, k_uptime_get_32(), buffer->data, buffer->len, complete);
        }
        k_spin_unlock(&store_lock, key);
        struct ble_trace_record event = {.kind = BLE_TRACE_ATT};
        int decoded = ble_trace_decode(buffer->data, buffer->len, complete, event.data);
        if (decoded != 0) {
            bool reliable = decoded > 0;
            uint16_t uuid = reliable ? attribute_uuid(&event, &reliable) : 0;
            ble_trace_memory_event(conn, BLE_TRACE_ATT, event.data, uuid, reliable);
        }
    }
    __real_bt_l2cap_recv(conn, buffer, complete);
}

struct attribute_dump {
    const struct device *uart;
    unsigned count;
    bool success;
};

// Associe les handles aux UUID publics du clavier, sans valeur, nom d’hôte ou adresse.
static uint8_t dump_attribute(const struct bt_gatt_attr *attribute, uint16_t handle, void *context) {
    struct attribute_dump *dump = context;
    char uuid[BT_UUID_STR_LEN];
    char line[100];
    bt_uuid_to_str(attribute->uuid, uuid, sizeof(uuid));
    snprintf(line, sizeof(line), "BLETRACE ATTR %04x %s\n", handle, uuid);
    dump->success = trace_send_line(dump->uart, line);
    if (!dump->success) {
        return BT_GATT_ITER_STOP;
    }
    dump->count++;
    return BT_GATT_ITER_CONTINUE;
}

struct attribute_lookup {
    uint16_t uuid;
    bool found;
};

// Lit le type de l’attribut local sans demander sa valeur au pilote GATT.
static uint8_t lookup_attribute(const struct bt_gatt_attr *attribute, uint16_t handle, void *context) {
    ARG_UNUSED(handle);
    struct attribute_lookup *lookup = context;
    if (attribute->uuid != NULL) {
        lookup->found = true;
        if (attribute->uuid->type == BT_UUID_TYPE_16) {
            lookup->uuid = BT_UUID_16(attribute->uuid)->val;
        }
    }
    return BT_GATT_ITER_STOP;
}

// Résout uniquement les UUID publics des lectures/écritures directes du clavier.
static uint16_t attribute_uuid(const struct ble_trace_record *event, bool *reliable) {
    if (event->kind != BLE_TRACE_ATT || (event->data[0] != 0x0a && event->data[0] != 0x12)) {
        return 0;
    }
    uint32_t handle = event->data[2];
    if (handle == 0 || handle > UINT16_MAX) {
        *reliable = false;
        return 0;
    }
    struct attribute_lookup lookup = {0};
    bt_gatt_foreach_attr((uint16_t)handle, (uint16_t)handle, lookup_attribute, &lookup);
    if (!lookup.found) {
        *reliable = false;
    }
    return lookup.uuid;
}

// Envoie une hypothèse explicable, sans changer la keymap ni l’appareil sélectionné.
static bool dump_os(const struct device *uart, const struct ble_os_classifier *classifier,
                     bool reliable) {
    char line[100];
    snprintf(line, sizeof(line), "BLETRACE OS session=%u result=%s evidence=%04x\n",
             classifier->session, ble_os_name(ble_os_result(classifier, reliable)),
             (unsigned)classifier->evidence);
    return trace_send_line(uart, line);
}

// Rejoue le même instantané dans le thread USB, hors des callbacks Bluetooth.
static bool dump_classifications(const struct device *uart, size_t count, bool reliable,
                                  const struct ble_trace_connection connections[BLE_TRACE_CONNECTION_LIMIT]) {
    for (size_t start = 0; start < count; start++) {
        struct ble_trace_record first;
        k_spinlock_key_t key = k_spin_lock(&store_lock);
        bool found = ble_trace_get(&store, start, &first);
        k_spin_unlock(&store_lock, key);
        if (!found) return false;
        if (first.kind != BLE_TRACE_CONNECTED) continue;

        struct ble_os_classifier classifier = {0};
        bool session_reliable = reliable;
        int profile = (int)first.data[4] - 1;
        for (size_t i = start; i < count; i++) {
            struct ble_trace_record event;
            key = k_spin_lock(&store_lock);
            found = ble_trace_get(&store, i, &event);
            k_spin_unlock(&store_lock, key);
            if (!found) return false;
            if (event.session != first.session) continue;
            if (event.kind == BLE_TRACE_DISCONNECTED) profile = (int)event.data[1] - 1;
            uint16_t uuid = attribute_uuid(&event, &session_reliable);
            ble_os_observe(&classifier, &event, uuid);
        }
        for (unsigned i = 0; i < BLE_TRACE_CONNECTION_LIMIT; i++) {
            if (connections[i].session == first.session) profile = connections[i].profile;
        }
        char line[100];
        snprintf(line, sizeof(line), "BLETRACE HOST session=%u profile=%d connected=%u\n",
                 first.session, profile, classifier.connected);
        if (!trace_send_line(uart, line) || !dump_os(uart, &classifier, session_reliable)) return false;
    }
    return true;
}

// Restitue les événements depuis le démarrage, indépendamment des branchements USB du lecteur.
void splitergoll_trace_dump(const struct device *uart) {
    char line[220];
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    size_t count = store.count;
    uint32_t dropped = store.dropped;
    uint32_t rejected = store.rejected;
    uint32_t session = store.session;
    unsigned active = store.connected;
    struct ble_trace_connection connections[BLE_TRACE_CONNECTION_LIMIT];
    memcpy(connections, store.connections, sizeof(connections));
    uint32_t uptime = k_uptime_get_32();
    k_spin_unlock(&store_lock, key);
    snprintf(line, sizeof(line), "BLETRACE BEGIN version=2 side=left count=%u dropped=%u rejected=%u "
             "uptime_ms=%u session=%u connected=%u\n", (unsigned)count, dropped, rejected,
             uptime, session, active);
    if (!trace_send_line(uart, line)) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        struct ble_trace_record event;
        key = k_spin_lock(&store_lock);
        bool found = ble_trace_get(&store, i, &event);
        k_spin_unlock(&store_lock, key);
        if (!found) {
            return;
        }
        snprintf(line, sizeof(line), "BLETRACE EVENT %u %u %u %s %08x %08x %08x %08x %08x\n",
                 (unsigned)i, event.timestamp_ms, event.session, ble_trace_kind_name(event.kind),
                 event.data[0], event.data[1], event.data[2], event.data[3], event.data[4]);
        if (!trace_send_line(uart, line)) {
            return;
        }
    }
    struct attribute_dump dump = {.uart = uart, .success = true};
    bt_gatt_foreach_attr(1, UINT16_MAX, dump_attribute, &dump);
    if (dump.success && dump_classifications(uart, count, dropped == 0 && rejected == 0, connections)) {
        snprintf(line, sizeof(line), "BLETRACE END attrs=%u\n", dump.count);
        trace_send_line(uart, line);
    }
}
