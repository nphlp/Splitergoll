// Observe uniquement les connexions où la centrale clavier est périphérique de l’ordinateur.
// La droite, les adresses Bluetooth, SMP et les valeurs HID ne sont jamais enregistrés.
#include "ble_trace_store.h"
#include "trace_console.h"

#include <stdio.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/net_buf.h>
#include <zephyr/sys/byteorder.h>

static struct ble_trace_store store;
static struct k_spinlock store_lock;
static struct bt_conn *active_host;

// Vérifie le rôle local, sans supposer qu’un index de connexion désigne toujours le Mac.
static bool is_host(struct bt_conn *conn) {
    struct bt_conn_info info;
    return bt_conn_get_info(conn, &info) == 0 && info.type == BT_CONN_TYPE_LE &&
           info.role == BT_CONN_ROLE_PERIPHERAL;
}

// Copie des métadonnées brèves sous verrou, sans accès série dans le callback Bluetooth.
static void record(struct bt_conn *conn, enum ble_trace_kind kind,
                   uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    if (!is_host(conn)) {
        return;
    }
    const uint32_t data[5] = {a, b, c, d, e};
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    if (conn == active_host) {
        ble_trace_append(&store, k_uptime_get_32(), kind, data);
    }
    k_spin_unlock(&store_lock, key);
}

// Ouvre une session numérotée, en ne conservant que le type d’adresse publique ou aléatoire.
static void connected(struct bt_conn *conn, uint8_t error) {
    struct bt_conn_info info;
    if (error || !is_host(conn) || bt_conn_get_info(conn, &info) != 0) {
        return;
    }
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    ble_trace_connect(&store, k_uptime_get_32(), info.le.interval, info.le.latency,
                      info.le.timeout, info.le.dst->type);
    if (active_host == NULL) {
        active_host = conn;
    }
    k_spin_unlock(&store_lock, key);
}

// Garde la déconnexion pour distinguer premier appairage et reconnexions dans un même dump.
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    if (!is_host(conn)) {
        return;
    }
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    if (conn == active_host) {
        ble_trace_disconnect(&store, k_uptime_get_32(), reason);
        active_host = NULL;
    }
    k_spin_unlock(&store_lock, key);
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

// Lit sans consommer le tampon, puis délègue exactement une fois au traitement natif.
void __wrap_bt_l2cap_recv(struct bt_conn *conn, struct net_buf *buffer, bool complete) {
    if (is_host(conn)) {
        k_spinlock_key_t key = k_spin_lock(&store_lock);
        if (conn == active_host) {
            ble_trace_l2cap(&store, k_uptime_get_32(), buffer->data, buffer->len, complete);
        }
        k_spin_unlock(&store_lock, key);
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

// Restitue les événements depuis le démarrage, indépendamment des branchements USB du lecteur.
void splitergoll_trace_dump(const struct device *uart) {
    char line[220];
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    size_t count = store.count;
    uint32_t dropped = store.dropped;
    uint32_t rejected = store.rejected;
    uint32_t session = store.session;
    bool active = store.connected;
    uint32_t uptime = k_uptime_get_32();
    k_spin_unlock(&store_lock, key);
    snprintf(line, sizeof(line), "BLETRACE BEGIN version=1 side=left count=%u dropped=%u rejected=%u "
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
    if (dump.success) {
        snprintf(line, sizeof(line), "BLETRACE END attrs=%u\n", dump.count);
        trace_send_line(uart, line);
    }
}
