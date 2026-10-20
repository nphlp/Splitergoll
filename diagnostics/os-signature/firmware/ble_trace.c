// Historique optionnel des observations du clavier, sans collecte en fonctionnement normal.
#include "ble_trace_store.h"
#include <splitergoll/ble_os_classifier.h>
#include <splitergoll/ble_observer.h>
#include "trace_console.h"
#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zmk/ble.h>

static struct ble_trace_store store;
static struct k_spinlock store_lock;

// Le moteur de production transmet une copie de ses métadonnées, jamais une valeur GATT.
void splitergoll_ble_trace_event(struct bt_conn *conn, enum ble_trace_kind kind,
                                const uint32_t data[5], uint16_t uuid, bool reliable) {
    ARG_UNUSED(uuid);
    unsigned connection = bt_conn_index(conn);
    int profile = zmk_ble_profile_index(bt_conn_get_dst(conn));
    if (profile < 0) profile = -1;
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    if (kind == BLE_TRACE_CONNECTED) {
        ble_trace_connect(&store, connection, profile, k_uptime_get_32(),
                           data[0], data[1], data[2], data[3]);
    } else if (connection < BLE_TRACE_CONNECTION_LIMIT && store.connections[connection].session != 0) {
        ble_trace_profile(&store, connection, profile);
        if (kind == BLE_TRACE_DISCONNECTED) {
            ble_trace_disconnect(&store, connection, k_uptime_get_32(), data[0]);
        } else if (!reliable) {
            if (store.rejected != UINT32_MAX) store.rejected++;
        } else {
            ble_trace_append(&store, connection, k_uptime_get_32(), kind, data);
        }
    }
    k_spin_unlock(&store_lock, key);
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
            uint16_t uuid = splitergoll_ble_attribute_uuid(&event, &session_reliable);
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
