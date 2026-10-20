// Ne conserve que la forme des échanges de découverte, jamais leur contenu utilisateur.
#include "ble_trace_store.h"

// Lit un entier Bluetooth sans hypothèse d’alignement du tampon.
static uint16_t little16(const uint8_t *data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

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

// Parse une liste blanche de requêtes, sans copier les valeurs lues ou écrites.
int ble_trace_decode(const uint8_t *packet, size_t length, bool complete, uint32_t data[5]) {
    if (packet == NULL || length < 4) {
        return -1;
    }
    if (little16(packet + 2) != 0x0004) {
        return 0;
    }
    size_t payload_length = little16(packet);
    if (!complete || payload_length == 0 || payload_length != length - 4) {
        return -1;
    }
    const uint8_t *att = packet + 4;
    data[0] = att[0];
    data[1] = (uint32_t)payload_length;
    data[2] = data[3] = data[4] = 0;
    size_t minimum;
    switch (att[0]) {
        case 0x02: case 0x0a: minimum = 3; break;
        case 0x04: case 0x0c: minimum = 5; break;
        case 0x06: case 0x08: case 0x10: minimum = 7; break;
        case 0x0e: case 0x20: minimum = 5; break;
        case 0x12: case 0x52: minimum = 3; break;
        case 0x16: minimum = 5; break;
        case 0x18: minimum = 2; break;
        default: return 0;
    }
    if (payload_length < minimum) {
        return -1;
    }
    bool fixed_length = att[0] == 0x02 || att[0] == 0x0a || att[0] == 0x04 ||
                        att[0] == 0x0c || att[0] == 0x18;
    bool typed_read = att[0] == 0x08 || att[0] == 0x10;
    if ((fixed_length && payload_length != minimum) ||
        (typed_read && payload_length != 7 && payload_length != 21)) {
        return -1;
    }
    if (att[0] == 0x18) {
        // La valeur du drapeau Execute Write n’est pas nécessaire à cette étude.
    } else {
        data[2] = little16(att + 1);
        switch (att[0]) {
            case 0x04: case 0x06: case 0x08: case 0x10: case 0x0c: case 0x16:
                data[3] = little16(att + 3);
                break;
            default: break;
        }
        if (att[0] == 0x06 || ((att[0] == 0x08 || att[0] == 0x10) && payload_length == 7)) {
            data[4] = little16(att + 5);
        }
        // Read Multiple garde seulement le premier handle et le nombre demandé.
        if (att[0] == 0x0e || att[0] == 0x20) {
            if ((payload_length - 1) % 2 != 0) {
                return -1;
            }
            data[3] = (uint32_t)(payload_length - 1) / 2;
        }
    }
    return 1;
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
