// Politique pure de changement d’hôte, distincte de l’observation radio et de NVS.
#include <splitergoll/ble_auto_state.h>

// Un cache vérifié pour une ancienne session ne rend pas la nouvelle connexion utilisable.
bool ble_auto_memory_ready(const struct ble_os_memory *memory, struct ble_auto_link link) {
    bool supported = memory->os == BLE_MEMORY_MACOS || memory->os == BLE_MEMORY_WINDOWS;
    return supported && memory->present && memory->verified && !memory->dirty && !memory->error &&
           link.connected && link.secure && link.reliable && link.observed_epoch != 0 &&
           link.observed_epoch == link.processed_epoch;
}

// Une signature connue ne suffit pas sans session courante et connexion utilisable.
enum usb_os_guess ble_auto_guess(struct ble_auto_target target) {
    if (!target.ready || target.profile < 0 || target.session == 0) return USB_OS_UNKNOWN;
    if (target.os == BLE_MEMORY_MACOS) return USB_OS_MACOS;
    if (target.os == BLE_MEMORY_WINDOWS) return USB_OS_WINDOWS;
    return USB_OS_UNKNOWN;
}

// Les frappes déjà admises gardent leur relâchement lorsque la destination change.
void ble_auto_sync(struct ble_auto_state *state, struct ble_auto_target target, uint32_t now_ms) {
    bool changed = state->target.profile != target.profile || state->target.session != target.session ||
                   state->target.os != target.os || state->target.ready != target.ready;
    if (!changed) return;
    state->target = target;
    usb_auto_sync(&state->input, state->input.generation + 1, now_ms);
}

// À froid, aucune attente fixe, seulement les anciens comportements s’il y en a.
bool ble_auto_can_select(const struct ble_auto_state *state, bool hid_busy, uint32_t now_ms) {
    return !hid_busy && usb_auto_can_select(&state->input, ble_auto_guess(state->target),
                                           state->target.ready, now_ms);
}
