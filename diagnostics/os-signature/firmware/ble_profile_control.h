// Politique de sélection des emplacements, sans accès aux clés Bluetooth.
#ifndef SPLITERGOLL_BLE_PROFILE_CONTROL_H
#define SPLITERGOLL_BLE_PROFILE_CONTROL_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define BLE_PROFILE_LIMIT 10
#define BLE_PROFILE_DRAIN_MS 750

struct ble_profile_activity {
    uint64_t held;
    uint32_t last_activity_ms;
    bool seen;
    bool invalid;
};

// Retient les positions physiques, y compris les touches de couche sans rapport HID.
void ble_profile_position(struct ble_profile_activity *activity, unsigned position, bool pressed,
                           uint32_t now_ms);

// Prolonge le drainage pour les macros, tap-dances et encodeurs encore en traitement.
void ble_profile_touch(struct ble_profile_activity *activity, uint32_t now_ms);

// Refuse une bascule pendant un maintien, un rapport actif ou un comportement différé.
bool ble_profile_input_busy(const struct ble_profile_activity *activity, bool hid_busy, uint32_t now_ms);

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
                                             uint16_t open_mask, bool input_busy);

enum ble_frame_result { BLE_FRAME_PASS, BLE_FRAME_CONSUMED, BLE_FRAME_READY };
struct ble_profile_frame {
    char bytes[16];
    size_t length;
    bool active;
    bool invalid;
};

// Retient toute trame commençant par !, sans laisser ses chiffres sélectionner un slot.
enum ble_frame_result ble_profile_frame_feed(struct ble_profile_frame *frame, uint8_t byte);

#endif
