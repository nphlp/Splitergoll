// Vérifie le vrai point d’observation BLE sans radio, payload HID ni écriture NVS.
#include <observer_stub.h>
#include <splitergoll/ble_acquisition.h>
#include <splitergoll/ble_observer.h>
#include <string.h>

extern struct bt_conn_cb splitergoll_ble_callbacks;
extern int (*test_observer_init)(void);
void __wrap_bt_l2cap_recv(struct bt_conn *, struct net_buf *, bool);
static struct bt_gatt_cb *gatt;
static struct peer address = {.type = 1};
static unsigned acquisition_count, trace_count, native_count;
static bool last_reliable, trace_reliable, attribute_exists = true;
static enum ble_trace_kind last_kind;
static uint16_t last_uuid;
static uint32_t last_data[5];
static struct net_buf *expected_buffer;
static bool expected_complete;
static struct bt_uuid_16 attribute_uuid = {{BT_UUID_TYPE_16}, 0x2a4b};

// Les frontières Zephyr ne lisent ni identité ni valeur d’attribut.
int bt_conn_get_info(struct bt_conn *conn, struct bt_conn_info *info) {
    if (conn->error)
        return conn->error;
    *info = (struct bt_conn_info){
        .type = conn->type,
        .role = conn->role,
        .le = {.interval = 12, .latency = 0, .timeout = 200, .dst = &address}};
    return 0;
}
unsigned bt_conn_index(struct bt_conn *conn) {
    return conn->index;
}
const struct peer *bt_conn_get_dst(struct bt_conn *conn) {
    (void)conn;
    return &address;
}
int zmk_ble_profile_index(const struct peer *peer) {
    assert(peer == &address);
    return 1;
}
void bt_gatt_cb_register(struct bt_gatt_cb *callback) {
    gatt = callback;
}
void bt_gatt_foreach_attr(uint16_t first, uint16_t last,
                          uint8_t (*callback)(const struct bt_gatt_attr *, uint16_t, void *),
                          void *context) {
    assert(first == last);
    if (attribute_exists) {
        struct bt_gatt_attr attr = {.uuid = &attribute_uuid.uuid};
        assert(callback(&attr, first, context) == BT_GATT_ITER_STOP);
    }
}

// L’acquisition reçoit la fiabilité résolue, distincte de celle de l’en-tête brut.
void splitergoll_ble_acquisition_event(struct bt_conn *conn, enum ble_trace_kind kind,
                                       const uint32_t data[5], uint16_t uuid, bool reliable) {
    (void)conn;
    acquisition_count++;
    last_kind = kind;
    memcpy(last_data, data, sizeof(last_data));
    last_uuid = uuid;
    last_reliable = reliable;
}
void splitergoll_ble_trace_event(struct bt_conn *conn, enum ble_trace_kind kind,
                                 const uint32_t data[5], uint16_t uuid, bool reliable) {
    (void)conn;
    (void)kind;
    (void)data;
    (void)uuid;
    trace_count++;
    trace_reliable = reliable;
}

// Le wrapper doit déléguer exactement une fois avec les mêmes pointeurs et drapeau.
void __real_bt_l2cap_recv(struct bt_conn *conn, struct net_buf *buffer, bool complete) {
    (void)conn;
    assert(buffer == expected_buffer && complete == expected_complete);
    native_count++;
}

// Compare le paquet entier avant/après, y compris les octets que le diagnostic ignore.
static void receive(struct bt_conn *conn, uint8_t *packet, size_t length, bool complete) {
    uint8_t before[64];
    assert(length <= sizeof(before));
    memcpy(before, packet, length);
    struct net_buf buffer = {.data = packet, .len = length};
    expected_buffer = &buffer;
    expected_complete = complete;
    unsigned calls = native_count;
    __wrap_bt_l2cap_recv(conn, &buffer, complete);
    assert(native_count == calls + 1 && buffer.data == packet && buffer.len == length);
    assert(!memcmp(before, packet, length));
}

// La moitié droite, les connexions non LE et les échecs ne polluent jamais la signature hôte.
static void connection_filters(void) {
    struct bt_conn host = {.index = 0, .role = BT_CONN_ROLE_PERIPHERAL, .type = BT_CONN_TYPE_LE};
    struct bt_conn split = {.index = 1, .role = 0, .type = BT_CONN_TYPE_LE};
    struct bt_conn invalid = {.index = BLE_TRACE_CONNECTION_LIMIT};
    splitergoll_ble_callbacks.connected(&host, 1);
    splitergoll_ble_callbacks.connected(&split, 0);
    host.error = -1;
    splitergoll_ble_callbacks.connected(&host, 0);
    assert(acquisition_count == 0);
    host.error = 0;
    splitergoll_ble_callbacks.connected(&host, 0);
    assert(acquisition_count == 1 && last_kind == BLE_TRACE_CONNECTED && last_data[4] == 2);
    assert(splitergoll_ble_connection_epoch(&host) == 1);
    splitergoll_ble_callbacks.security_changed(&host, 2, SECURITY_SUCCESS);
    assert(splitergoll_ble_connection_epoch(&host) == 2);
    struct bt_le_conn_param params = {6, 12, 0, 200}, before = params;
    assert(splitergoll_ble_callbacks.le_param_req(&host, &params));
    assert(!memcmp(&params, &before, sizeof(params)) && last_kind == BLE_TRACE_PARAM_REQUEST);
    splitergoll_ble_callbacks.le_param_updated(&host, 24, 1, 300);
    assert(last_kind == BLE_TRACE_PARAM_UPDATED && last_data[0] == 24);
    gatt->att_mtu_updated(&host, 247, 185);
    assert(last_kind == BLE_TRACE_MTU && last_data[1] == 185);
    uint8_t features[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    struct bt_conn_remote_info remote = {.type = BT_CONN_TYPE_LE, .le.features = features};
    splitergoll_ble_callbacks.remote_info_available(&host, &remote);
    assert(last_kind == BLE_TRACE_FEATURES && last_data[0] == 0x04030201);
    splitergoll_ble_callbacks.disconnected(&host, 0x13);
    assert(last_kind == BLE_TRACE_DISCONNECTED && splitergoll_ble_connection_epoch(&host) == 3);
    splitergoll_ble_connection_changed(&invalid);
    assert(splitergoll_ble_connection_epoch(&invalid) == 0);
    unsigned observed = acquisition_count;
    host.type = 0;
    gatt->att_mtu_updated(&host, 23, 23);
    assert(acquisition_count == observed && trace_count == acquisition_count);
}

// Le canal SMP et les notifications HID sont délégués sans enregistrement de contenu.
static void packets_and_privacy(void) {
    struct bt_conn host = {.index = 0, .role = BT_CONN_ROLE_PERIPHERAL, .type = BT_CONN_TYPE_LE};
    struct bt_conn split = {.index = 1, .role = 0, .type = BT_CONN_TYPE_LE};
    uint8_t write[] = {7, 0, 4, 0, 0x12, 1, 0, 0xde, 0xad, 0xbe, 0xef};
    unsigned observed = acquisition_count;
    receive(&split, write, sizeof(write), true);
    assert(acquisition_count == observed);
    receive(&host, write, sizeof(write), true);
    assert(last_kind == BLE_TRACE_ATT && last_uuid == 0x2a4b && last_reliable);
    uint32_t metadata[5];
    memcpy(metadata, last_data, sizeof(metadata));
    memset(write + 7, 0x55, sizeof(write) - 7);
    receive(&host, write, sizeof(write), true);
    assert(!memcmp(metadata, last_data, sizeof(metadata)));
    attribute_exists = false;
    receive(&host, write, sizeof(write), true);
    assert(!last_reliable && trace_reliable && last_uuid == 0);
    attribute_exists = true;
    attribute_uuid.uuid.type = BT_UUID_TYPE_128;
    receive(&host, write, sizeof(write), true);
    assert(last_reliable && last_uuid == 0);
    receive(&host, write, sizeof(write), false);
    assert(!last_reliable && !trace_reliable);
    observed = acquisition_count;
    uint8_t smp[] = {3, 0, 6, 0, 1, 2, 3}, notify[] = {4, 0, 4, 0, 0x1b, 1, 0, 0xaa};
    receive(&host, smp, sizeof(smp), true);
    receive(&host, notify, sizeof(notify), true);
    assert(acquisition_count == observed);
    bool reliable = true;
    struct ble_trace_record event = {.kind = BLE_TRACE_ATT, .data = {0x0a, 3, 0}};
    assert(splitergoll_ble_attribute_uuid(&event, &reliable) == 0 && !reliable);
    reliable = true;
    event.data[2] = 0x10000;
    assert(splitergoll_ble_attribute_uuid(&event, &reliable) == 0 && !reliable);
}

// Exécute les callbacks enregistrés par la vraie initialisation du module.
int main(void) {
    assert(test_observer_init() == 0 && gatt);
    connection_filters();
    packets_and_privacy();
    puts("real BLE observer: host/split filtering, epochs, callbacks, intact packets and privacy "
         "OK");
    return 0;
}
