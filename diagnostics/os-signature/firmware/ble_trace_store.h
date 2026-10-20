// Historique BLE borné, sans adresse, clé de connexion ni contenu de rapport HID.
#ifndef SPLITERGOLL_BLE_TRACE_STORE_H
#define SPLITERGOLL_BLE_TRACE_STORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BLE_TRACE_CAPACITY 256
#define BLE_TRACE_CONNECTION_LIMIT 6

struct ble_trace_connection {
    uint32_t session;
    int profile;
};

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

struct ble_trace_store {
    struct ble_trace_record records[BLE_TRACE_CAPACITY];
    size_t count;
    uint32_t dropped;
    uint32_t rejected;
    uint32_t session;
    unsigned connected;
    struct ble_trace_connection connections[BLE_TRACE_CONNECTION_LIMIT];
};

// Ouvre une session hôte, sans effacer les connexions précédentes du même démarrage.
void ble_trace_connect(struct ble_trace_store *store, unsigned connection, int profile,
                       uint32_t timestamp_ms, uint16_t interval, uint16_t latency,
                       uint16_t timeout, uint8_t address_type);

// Complète un emplacement inconnu après appairage, sans réécrire les événements passés.
void ble_trace_profile(struct ble_trace_store *store, unsigned connection, int profile);

// Ferme la session active et conserve sa raison de déconnexion.
void ble_trace_disconnect(struct ble_trace_store *store, unsigned connection,
                          uint32_t timestamp_ms, uint8_t reason);

// Ajoute des métadonnées à la session active, sans allocation ni écrasement.
bool ble_trace_append(struct ble_trace_store *store, unsigned connection, uint32_t timestamp_ms,
                      enum ble_trace_kind kind, const uint32_t data[5]);

// Décode les métadonnées ATT : 1 événement, 0 ignoré, -1 paquet invalide.
int ble_trace_decode(const uint8_t *packet, size_t length, bool complete, uint32_t data[5]);

// Extrait seulement les champs publics des requêtes ATT entrantes sur le canal fixe 0x0004.
void ble_trace_l2cap(struct ble_trace_store *store, unsigned connection, uint32_t timestamp_ms,
                     const uint8_t *packet, size_t length, bool complete);

// Rend une copie d’un enregistrement existant.
bool ble_trace_get(const struct ble_trace_store *store, size_t index, struct ble_trace_record *record);

// Fournit les noms stables du protocole série.
const char *ble_trace_kind_name(enum ble_trace_kind kind);

#endif
