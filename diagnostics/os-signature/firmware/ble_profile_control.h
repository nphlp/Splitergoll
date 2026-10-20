// Politique de sélection des emplacements, sans accès aux clés Bluetooth.
#ifndef SPLITERGOLL_BLE_PROFILE_CONTROL_H
#define SPLITERGOLL_BLE_PROFILE_CONTROL_H
#include <stdbool.h>
#include <stdint.h>

#define BLE_PROFILE_LIMIT 10

enum ble_profile_result {
    BLE_PROFILE_OK,
    BLE_PROFILE_INVALID,
    BLE_PROFILE_BUSY,
    BLE_PROFILE_FULL,
};

struct ble_profile_choice {
    enum ble_profile_result result;
    int target;
    bool change;
};

// S lit, F choisit un emplacement libre, un chiffre sélectionne explicitement cet index.
struct ble_profile_choice ble_profile_choose(char command, unsigned count, unsigned active,
                                             uint16_t open_mask, bool host_connected);

#endif
