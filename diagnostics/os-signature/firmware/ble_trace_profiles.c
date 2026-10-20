// Contrôle explicite d’emplacements ZMK depuis le diagnostic, sans effacement de bonds.
#include "ble_profile_control.h"
#include "trace_console.h"
#include <stdio.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/kernel.h>
#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/sensor_event.h>
#include <zmk/hid.h>
#include <zmk/matrix.h>

BUILD_ASSERT(ZMK_BLE_PROFILE_COUNT <= BLE_PROFILE_LIMIT);
BUILD_ASSERT(ZMK_KEYMAP_LEN <= 64);
static struct ble_profile_activity activity;
K_MUTEX_DEFINE(selection_mutex);

int __real_zmk_event_manager_raise(zmk_event_t *event);

// Sérialise les événements de frappe avec la sélection, sans consommer ni modifier les événements.
int __wrap_zmk_event_manager_raise(zmk_event_t *event) {
    const struct zmk_position_state_changed *position = as_zmk_position_state_changed(event);
    bool input = position != NULL || as_zmk_keycode_state_changed(event) != NULL ||
                 as_zmk_sensor_event(event) != NULL;
    if (!input) return __real_zmk_event_manager_raise(event);
    k_mutex_lock(&selection_mutex, K_FOREVER);
    uint32_t now_ms = k_uptime_get_32();
    if (position != NULL) {
        ble_profile_position(&activity, position->position, position->state, now_ms);
    } else {
        ble_profile_touch(&activity, now_ms);
    }
    int result = __real_zmk_event_manager_raise(event);
    k_mutex_unlock(&selection_mutex);
    return result;
}

// Ne lit que les états HID en RAM, jamais les frappes dans les traces.
static bool report_nonempty(const void *body, size_t length) {
    const uint8_t *bytes = body;
    for (size_t i = 0; i < length; i++) if (bytes[i] != 0) return true;
    return false;
}

// Le seul appel de mutation est le sélecteur natif ZMK, aucune clé n’est supprimée ou copiée.
void splitergoll_ble_profile_command(const struct device *uart, char command) {
    k_mutex_lock(&selection_mutex, K_FOREVER);
    uint16_t open_mask = 0;
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        if (zmk_ble_profile_is_open(i)) {
            open_mask |= 1u << i;
        }
    }
    bool hid_busy = zmk_hid_get_explicit_mods() != 0 ||
                    report_nonempty(&zmk_hid_get_keyboard_report()->body,
                                    sizeof(zmk_hid_get_keyboard_report()->body)) ||
                    report_nonempty(&zmk_hid_get_consumer_report()->body,
                                    sizeof(zmk_hid_get_consumer_report()->body));
    bool input_busy = ble_profile_input_busy(&activity, hid_busy, k_uptime_get_32());
    unsigned active = zmk_ble_active_profile_index();
    struct ble_profile_choice choice = ble_profile_choose(command, ZMK_BLE_PROFILE_COUNT,
                                                          active, open_mask, input_busy);
    const char *status;
    switch (choice.result) {
        case BLE_PROFILE_OK: status = "ok"; break;
        case BLE_PROFILE_BUSY: status = "busy"; break;
        case BLE_PROFILE_FULL: status = "full"; break;
        default: status = "invalid"; break;
    }
    int error = 0;
    bool changed = false;
    if (choice.result == BLE_PROFILE_OK && choice.change) {
        error = zmk_ble_prof_select(choice.target);
        if (error) {
            status = "failed";
        } else {
            changed = true;
        }
    }
    k_mutex_unlock(&selection_mutex);
    char line[170];
    snprintf(line, sizeof(line), "BLEPROFILE BEGIN version=2 count=%u active=%d save_delay_ms=%u\n",
             ZMK_BLE_PROFILE_COUNT, zmk_ble_active_profile_index(), CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE);
    if (!trace_send_line(uart, line)) {
        return;
    }
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        snprintf(line, sizeof(line), "BLEPROFILE SLOT index=%u paired=%u connected=%u\n",
                 i, !zmk_ble_profile_is_open(i), zmk_ble_profile_is_connected(i));
        if (!trace_send_line(uart, line)) {
            return;
        }
    }
    snprintf(line, sizeof(line), "BLEPROFILE RESULT command=%c status=%s selected=%d changed=%u error=%d\n",
             command, status, zmk_ble_active_profile_index(), changed, error);
    if (trace_send_line(uart, line)) {
        trace_send_line(uart, "BLEPROFILE END\n");
    }
}
