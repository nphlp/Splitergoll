// Métadonnées publiques partagées par la détection BLE et son diagnostic optionnel.
#ifndef SPLITERGOLL_BLE_EVENTS_H
#define SPLITERGOLL_BLE_EVENTS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BLE_TRACE_CONNECTION_LIMIT 6

enum ble_trace_kind {
    BLE_TRACE_CONNECTED,
    BLE_TRACE_DISCONNECTED,
    BLE_TRACE_SECURITY,
    BLE_TRACE_PARAM_REQUEST,
    BLE_TRACE_PARAM_UPDATED,
    BLE_TRACE_MTU,
    BLE_TRACE_FEATURES,
    BLE_TRACE_ATT,
    BLE_TRACE_KIND_COUNT,
};

struct ble_trace_record {
    uint32_t timestamp_ms;
    uint32_t session;
    enum ble_trace_kind kind;
    uint32_t data[5];
};

// Extrait seulement les en-têtes ATT admis, sans conserver les valeurs ou les clés.
int ble_trace_decode(const uint8_t *packet, size_t length, bool complete, uint32_t data[5]);
#endif
