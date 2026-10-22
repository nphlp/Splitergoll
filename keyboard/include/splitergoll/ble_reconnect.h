// Fenêtre commune de reconnexion et d’accueil BLE, distincte de la destination des frappes.
#ifndef SPLITERGOLL_BLE_RECONNECT_H
#define SPLITERGOLL_BLE_RECONNECT_H
#include <stdbool.h>
#include <stdint.h>

struct ble_reconnect_status {
    uint32_t remaining_ms;
    uint16_t known;
    uint16_t connected;
    bool ready;
    bool advertising;
    int error;
    uint16_t free_slots;
    int pairing_slot;
    uint32_t pairing_remaining_ms;
    bool accepting_new;
};

// Réarme dix secondes depuis le raccourci, sans changer de slot ou effacer de bond.
void splitergoll_ble_reconnect_open(void);

// Copie uniquement des compteurs et états, sans adresse Bluetooth ni donnée de frappe.
struct ble_reconnect_status splitergoll_ble_reconnect_status(void);
#endif
