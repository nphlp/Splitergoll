// Ne remplace jamais un appairage pour choisir un emplacement libre.
#include "ble_profile_control.h"

// Les trames tronquées, binaires ou trop longues ne deviennent jamais des commandes simples.
enum ble_frame_result ble_profile_frame_feed(struct ble_profile_frame *frame, uint8_t byte) {
    if (!frame->active) {
        if (byte != '!') return BLE_FRAME_PASS;
        *frame = (struct ble_profile_frame){.length = 1, .active = true, .bytes = "!"};
        return BLE_FRAME_CONSUMED;
    }
    if (byte == '\n') {
        frame->bytes[frame->length] = '\0';
        if (frame->invalid) frame->bytes[0] = '\0';
        frame->active = false;
        return BLE_FRAME_READY;
    }
    if (byte < 0x20 || byte > 0x7e || frame->length == sizeof(frame->bytes) - 1) {
        frame->invalid = true;
    } else {
        frame->bytes[frame->length++] = (char)byte;
    }
    return BLE_FRAME_CONSUMED;
}

// Tous les événements de saisie repoussent le drainage, sans temporisation du Bluetooth.
void ble_profile_touch(struct ble_profile_activity *activity, uint32_t now_ms) {
    activity->seen = true;
    activity->last_activity_ms = now_ms;
}

// Une position hors capacité rend la garde non fiable et bloque la bascule.
void ble_profile_position(struct ble_profile_activity *activity, unsigned position, bool pressed,
                           uint32_t now_ms) {
    ble_profile_touch(activity, now_ms);
    if (position >= 64) {
        activity->invalid = true;
        return;
    }
    uint64_t bit = UINT64_C(1) << position;
    if (pressed) activity->held |= bit;
    else activity->held &= ~bit;
}

// La soustraction non signée garde la même échéance au rebouclage de l’horloge.
bool ble_profile_input_busy(const struct ble_profile_activity *activity, bool hid_busy, uint32_t now_ms) {
    return activity->invalid || activity->held != 0 || hid_busy ||
           (activity->seen && (uint32_t)(now_ms - activity->last_activity_ms) < BLE_PROFILE_DRAIN_MS);
}

// Valide toute la décision avant que l’adaptateur n’appelle le sélecteur ZMK.
struct ble_profile_choice ble_profile_choose(char command, unsigned count, unsigned active,
                                             uint16_t open_mask, bool input_busy) {
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
    if (input_busy && choice.target != (int)active) {
        choice.result = BLE_PROFILE_BUSY;
        return choice;
    }
    choice.result = BLE_PROFILE_OK;
    choice.change = choice.target != (int)active;
    return choice;
}
