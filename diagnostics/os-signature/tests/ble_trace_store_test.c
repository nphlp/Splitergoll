// Vérifie les sessions, les bornes et l’exclusion des valeurs privées des paquets BLE.
#include "ble_trace_store.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Fournit des paquets ATT complets avec le même en-tête L2CAP que le pilote figé.
static void observe(struct ble_trace_store *store, const uint8_t *att, size_t length) {
    uint8_t packet[64] = {(uint8_t)length, 0, 4, 0};
    assert(length <= sizeof(packet) - 4);
    memcpy(packet + 4, att, length);
    ble_trace_l2cap(store, 40, packet, length + 4, true);
}

// Les connexions successives ne partagent ni leur numéro ni leur état actif.
static void sessions(void) {
    struct ble_trace_store store = {0};
    const uint32_t data[5] = {2, 0, 0, 0, 0};
    assert(!ble_trace_append(&store, 1, BLE_TRACE_SECURITY, data));
    assert(store.rejected == 1);
    ble_trace_connect(&store, 2, 12, 0, 200, 1);
    assert(store.connected && store.session == 1 && store.count == 1);
    assert(store.records[0].data[3] == 1);
    ble_trace_connect(&store, 3, 12, 0, 200, 0);
    assert(store.rejected == 2 && store.session == 1);
    assert(ble_trace_append(&store, 4, BLE_TRACE_SECURITY, data));
    ble_trace_disconnect(&store, 5, 0x13);
    assert(!store.connected && store.records[2].session == 1);
    ble_trace_connect(&store, 6, 24, 4, 72, 0);
    assert(store.session == 2 && store.records[3].session == 2);
    assert(store.records[3].data[0] == 24 && store.records[3].data[2] == 72);
    struct ble_trace_record copy;
    assert(ble_trace_get(&store, 0, &copy));
    copy.data[0] = 999;
    assert(store.records[0].data[0] == 12);
    assert(!ble_trace_get(&store, store.count, &copy));
    assert(!ble_trace_get(&store, 0, NULL));
    assert(!ble_trace_append(&store, 7, BLE_TRACE_KIND_COUNT, data));
    assert(!ble_trace_append(&store, 7, BLE_TRACE_SECURITY, NULL));
    assert(strcmp(ble_trace_kind_name(BLE_TRACE_ATT), "ATT") == 0);
    assert(strcmp(ble_trace_kind_name((enum ble_trace_kind)-1), "INVALID") == 0);
}

// Seuls les en-têtes utiles sont extraits, jamais les valeurs écrites, notifications ou clés SMP.
static void privacy(void) {
    struct ble_trace_store first = {0}, second = {0};
    ble_trace_connect(&first, 0, 12, 0, 200, 0);
    ble_trace_connect(&second, 0, 12, 0, 200, 0);
    uint8_t write[] = {0x12, 0x34, 0x12, 0xde, 0xad, 0xbe, 0xef};
    observe(&first, write, sizeof(write));
    memset(write + 3, 0x55, sizeof(write) - 3);
    observe(&second, write, sizeof(write));
    assert(memcmp(first.records[1].data, second.records[1].data, sizeof(first.records[1].data)) == 0);
    assert(first.records[1].data[2] == 0x1234);
    assert(first.records[1].data[3] == 0 && first.records[1].data[4] == 0);
    uint8_t notify[] = {0x1b, 1, 0, 0xaa, 0xbb};
    observe(&first, notify, sizeof(notify));
    assert(first.count == 2);
    uint8_t smp[] = {3, 0, 6, 0, 1, 2, 3};
    ble_trace_l2cap(&first, 10, smp, sizeof(smp), true);
    assert(first.count == 2 && first.rejected == 0);
    uint8_t eatt[] = {3, 0, 0x40, 0, 0x0a, 1, 0};
    ble_trace_l2cap(&first, 10, eatt, sizeof(eatt), true);
    assert(first.count == 2 && first.rejected == 0);
}

// Vérifie les offsets, plages, UUID 16 bits et MTU avec des requêtes concrètes.
static void requests(void) {
    struct ble_trace_store store = {0};
    ble_trace_connect(&store, 0, 12, 0, 200, 0);
    const uint8_t mtu[] = {2, 0xf7, 0};
    observe(&store, mtu, sizeof(mtu));
    assert(store.records[1].data[2] == 247);
    const uint8_t type[] = {8, 1, 0, 0xff, 0xff, 0x4b, 0x2a};
    observe(&store, type, sizeof(type));
    assert(store.records[2].data[2] == 1 && store.records[2].data[3] == 0xffff);
    assert(store.records[2].data[4] == 0x2a4b);
    const uint8_t blob[] = {0x0c, 0x34, 0x12, 0x10, 0};
    observe(&store, blob, sizeof(blob));
    assert(store.records[3].data[2] == 0x1234 && store.records[3].data[3] == 16);
    const uint8_t multiple[] = {0x0e, 1, 0, 2, 0, 3, 0};
    observe(&store, multiple, sizeof(multiple));
    assert(store.records[4].data[2] == 1 && store.records[4].data[3] == 3);
    uint8_t long_uuid[21] = {8, 1, 0, 0xff, 0xff};
    memset(long_uuid + 5, 0xcc, 16);
    observe(&store, long_uuid, sizeof(long_uuid));
    assert(store.records[5].data[4] == 0 && store.records[5].data[1] == 21);
    const uint8_t execute[] = {0x18, 0x55};
    observe(&store, execute, sizeof(execute));
    assert(store.records[6].data[2] == 0);
}

// Refuse les tailles incohérentes avant toute lecture hors du paquet.
static void malformed(void) {
    struct ble_trace_store store = {0};
    ble_trace_connect(&store, 0, 12, 0, 200, 0);
    ble_trace_l2cap(&store, 1, NULL, 0, true);
    uint8_t packet[] = {3, 0, 4, 0, 0x0a, 1, 0};
    ble_trace_l2cap(&store, 1, packet, 3, true);
    ble_trace_l2cap(&store, 1, packet, sizeof(packet), false);
    packet[0] = 4;
    ble_trace_l2cap(&store, 1, packet, sizeof(packet), true);
    packet[0] = 0;
    ble_trace_l2cap(&store, 1, packet, 4, true);
    const uint8_t short_read[] = {0x0a, 1};
    observe(&store, short_read, sizeof(short_read));
    const uint8_t long_read[] = {0x0a, 1, 0, 0};
    observe(&store, long_read, sizeof(long_read));
    const uint8_t invalid_uuid[] = {8, 1, 0, 2, 0, 0, 0, 0};
    observe(&store, invalid_uuid, sizeof(invalid_uuid));
    const uint8_t invalid_multiple[] = {0x0e, 1, 0, 2, 0, 0};
    observe(&store, invalid_multiple, sizeof(invalid_multiple));
    assert(store.rejected == 9 && store.count == 1);
    // Toutes les petites tailles et tous les opcodes passent aussi sous ASan/UBSan.
    for (unsigned opcode = 0; opcode < 256; opcode++) {
        for (size_t length = 1; length < 24; length++) {
            uint8_t att[24] = {0};
            att[0] = opcode;
            observe(&store, att, length);
        }
    }
}

// Le plein n’écrase pas le début et les compteurs de perte ne reviennent jamais à zéro.
static void capacity(void) {
    struct ble_trace_store store = {0};
    ble_trace_connect(&store, 0, 12, 0, 200, 0);
    const uint32_t data[5] = {1, 2, 3, 4, 5};
    for (size_t i = 1; i < BLE_TRACE_CAPACITY; i++) {
        assert(ble_trace_append(&store, (uint32_t)i, BLE_TRACE_MTU, data));
    }
    assert(!ble_trace_append(&store, 300, BLE_TRACE_MTU, data));
    assert(store.dropped == 1 && store.records[0].kind == BLE_TRACE_CONNECTED);
    ble_trace_disconnect(&store, 301, 0x13);
    assert(!store.connected && store.dropped == 2);
    store.dropped = UINT32_MAX;
    ble_trace_connect(&store, 302, 12, 0, 200, 0);
    assert(store.dropped == UINT32_MAX);
    store.rejected = UINT32_MAX;
    ble_trace_append(&store, 303, BLE_TRACE_KIND_COUNT, data);
    assert(store.rejected == UINT32_MAX);
    store.connected = false;
    store.session = UINT32_MAX;
    ble_trace_connect(&store, 304, 12, 0, 200, 0);
    assert(!store.connected && store.session == UINT32_MAX);
}

// Exécute les comportements indépendants du matériel et des files Bluetooth.
int main(void) {
    sessions();
    privacy();
    requests();
    malformed();
    capacity();
    puts("BLE trace store: OK");
    return 0;
}
