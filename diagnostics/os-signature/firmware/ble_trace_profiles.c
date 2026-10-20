// Contrôle explicite d’emplacements ZMK depuis le diagnostic, sans effacement de bonds.
#include "ble_profile_control.h"
#include "trace_console.h"
#include <stdio.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/kernel.h>
#include <zmk/ble.h>

BUILD_ASSERT(ZMK_BLE_PROFILE_COUNT <= BLE_PROFILE_LIMIT);

// Repère tout hôte encore connecté, même avant la fin de son appairage.
static void find_host(struct bt_conn *conn, void *context) {
    struct bt_conn_info info;
    if (bt_conn_get_info(conn, &info) == 0 && info.role == BT_CONN_ROLE_PERIPHERAL &&
        info.state == BT_CONN_STATE_CONNECTED) {
        *(bool *)context = true;
    }
}

// Le seul appel de mutation est le sélecteur natif ZMK, aucune clé n’est supprimée ou copiée.
void splitergoll_ble_profile_command(const struct device *uart, char command) {
    bool host_connected = false;
    uint16_t open_mask = 0;
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        if (zmk_ble_profile_is_open(i)) {
            open_mask |= 1u << i;
        }
    }
    bt_conn_foreach(BT_CONN_TYPE_LE, find_host, &host_connected);
    unsigned active = zmk_ble_active_profile_index();
    struct ble_profile_choice choice = ble_profile_choose(command, ZMK_BLE_PROFILE_COUNT,
                                                          active, open_mask, host_connected);
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
    char line[170];
    snprintf(line, sizeof(line), "BLEPROFILE BEGIN version=1 count=%u active=%d save_delay_ms=%u\n",
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
