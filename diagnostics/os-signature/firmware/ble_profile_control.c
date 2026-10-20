// Ne remplace jamais un appairage pour choisir un emplacement libre.
#include "ble_profile_control.h"

// Valide toute la décision avant que l’adaptateur n’appelle le sélecteur ZMK.
struct ble_profile_choice ble_profile_choose(char command, unsigned count, unsigned active,
                                             uint16_t open_mask, bool host_connected) {
    struct ble_profile_choice choice = {.result = BLE_PROFILE_INVALID, .target = -1};
    if (count == 0 || count > BLE_PROFILE_LIMIT || active >= count) {
        return choice;
    }
    if (command == 'S') {
        choice.result = BLE_PROFILE_OK;
        choice.target = (int)active;
        return choice;
    }
    if (command == 'F') {
        for (unsigned index = 0; index < count; index++) {
            if (open_mask & (1u << index)) {
                choice.target = (int)index;
                break;
            }
        }
        if (choice.target < 0) {
            choice.result = BLE_PROFILE_FULL;
            return choice;
        }
    } else if (command >= '0' && command <= '9' && (unsigned)(command - '0') < count) {
        choice.target = command - '0';
    } else {
        return choice;
    }
    if (host_connected && choice.target != (int)active) {
        choice.result = BLE_PROFILE_BUSY;
        return choice;
    }
    choice.result = BLE_PROFILE_OK;
    choice.change = choice.target != (int)active;
    return choice;
}
