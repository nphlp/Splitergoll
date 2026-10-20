// Ne conserve que la forme des échanges de découverte, jamais leur contenu utilisateur.
#include "ble_trace_store.h"

// Garde un compteur fiable même après un très grand nombre d’événements.
static void increment_saturated(uint32_t *value) {
    if (*value != UINT32_MAX) {
        (*value)++;
    }
}

// Refuse les événements hors connexion et garde les premières observations si le tampon est plein.
bool ble_trace_append(struct ble_trace_store *store, unsigned connection, uint32_t timestamp_ms,
                      enum ble_trace_kind kind, const uint32_t data[5]) {
    if (connection >= BLE_TRACE_CONNECTION_LIMIT || store->connections[connection].session == 0 ||
        (unsigned)kind >= BLE_TRACE_KIND_COUNT || data == NULL) {
        increment_saturated(&store->rejected);
        return false;
    }
    if (store->count == BLE_TRACE_CAPACITY) {
        increment_saturated(&store->dropped);
        return false;
    }
    struct ble_trace_record *record = &store->records[store->count++];
    *record = (struct ble_trace_record){.timestamp_ms = timestamp_ms,
                                        .session = store->connections[connection].session, .kind = kind};
    for (size_t i = 0; i < 5; i++) {
        record->data[i] = data[i];
    }
    return true;
}

// Numérote les connexions sans utiliser l’adresse de l’ordinateur comme identifiant.
void ble_trace_connect(struct ble_trace_store *store, unsigned connection, int profile,
                       uint32_t timestamp_ms, uint16_t interval, uint16_t latency,
                       uint16_t timeout, uint8_t address_type) {
    if (connection >= BLE_TRACE_CONNECTION_LIMIT || store->connections[connection].session != 0 ||
        store->session == UINT32_MAX || profile < -1 || profile > 9) {
        increment_saturated(&store->rejected);
        return;
    }
    store->session++;
    store->connected++;
    store->connections[connection] = (struct ble_trace_connection){store->session, profile};
    const uint32_t data[5] = {interval, latency, timeout, address_type, (uint32_t)(profile + 1)};
    ble_trace_append(store, connection, timestamp_ms, BLE_TRACE_CONNECTED, data);
}

// Un changement d’identité inattendu rend la capture non fiable au lieu de réattribuer ses preuves.
void ble_trace_profile(struct ble_trace_store *store, unsigned connection, int profile) {
    if (connection >= BLE_TRACE_CONNECTION_LIMIT || store->connections[connection].session == 0 ||
        profile < -1 || profile > 9) {
        increment_saturated(&store->rejected);
        return;
    }
    if (profile < 0) {
        return;
    }
    int previous = store->connections[connection].profile;
    if (previous >= 0 && previous != profile) {
        increment_saturated(&store->rejected);
        return;
    }
    store->connections[connection].profile = profile;
}

// Termine une connexion même lorsque le tampon ne peut plus recevoir sa dernière ligne.
void ble_trace_disconnect(struct ble_trace_store *store, unsigned connection,
                          uint32_t timestamp_ms, uint8_t reason) {
    if (connection >= BLE_TRACE_CONNECTION_LIMIT || store->connections[connection].session == 0) {
        increment_saturated(&store->rejected);
        return;
    }
    const uint32_t data[5] = {reason, (uint32_t)(store->connections[connection].profile + 1), 0, 0, 0};
    ble_trace_append(store, connection, timestamp_ms, BLE_TRACE_DISCONNECTED, data);
    store->connections[connection] = (struct ble_trace_connection){0};
    store->connected--;
}

// Partage le parseur avec l’acquisition, sans faire dépendre sa mémoire de ce tampon fini.
void ble_trace_l2cap(struct ble_trace_store *store, unsigned connection, uint32_t timestamp_ms,
                     const uint8_t *packet, size_t length, bool complete) {
    uint32_t data[5];
    int result = ble_trace_decode(packet, length, complete, data);
    if (result < 0) increment_saturated(&store->rejected);
    else if (result > 0) ble_trace_append(store, connection, timestamp_ms, BLE_TRACE_ATT, data);
}

// Les copies ne dépendent pas des ajouts ultérieurs, le stockage ne réutilise aucun index.
bool ble_trace_get(const struct ble_trace_store *store, size_t index, struct ble_trace_record *record) {
    if (record == NULL || index >= store->count) {
        return false;
    }
    *record = store->records[index];
    return true;
}

// Un type inconnu est explicite et ne peut pas être confondu avec une requête ATT.
const char *ble_trace_kind_name(enum ble_trace_kind kind) {
    static const char *const names[] = {"CONNECTED", "DISCONNECTED", "SECURITY", "PARAM_REQUEST",
                                        "PARAM_UPDATED", "MTU", "FEATURES", "ATT"};
    return (unsigned)kind < BLE_TRACE_KIND_COUNT ? names[kind] : "INVALID";
}
