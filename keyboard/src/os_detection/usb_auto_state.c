// État borné de l’auto-détection, indépendant des API du clavier.
#include <splitergoll/usb_auto_state.h>

// Un changement de bus bloque les nouveaux appuis mais laisse finir les anciens.
void usb_auto_sync(struct usb_auto_state *state, uint32_t generation, uint32_t now_ms) {
    if (state->generation != generation) {
        state->generation = generation;
        state->selected = USB_OS_UNKNOWN;
        state->last_activity_ms = now_ms;
    }
}

// Les appuis ignorés pendant l’attente ne produisent jamais de relâchement orphelin.
bool usb_auto_position(struct usb_auto_state *state, uint32_t position, bool pressed,
                       uint32_t now_ms) {
    if (position >= USB_AUTO_POSITION_COUNT) {
        return false;
    }
    state->last_activity_ms = now_ms;
    uint64_t bit = UINT64_C(1) << position;
    if (pressed) {
        if ((state->held & bit) != 0) {
            return false;
        }
        state->held |= bit;
        bool selected = state->selected == USB_OS_MACOS || state->selected == USB_OS_WINDOWS;
        if (selected) {
            state->admitted |= bit;
        }
        return selected;
    }
    bool admitted = (state->admitted & bit) != 0;
    state->held &= ~bit;
    state->admitted &= ~bit;
    return admitted;
}

// La décision reste figée pendant le cycle, même si un autre motif arrive ensuite.
bool usb_auto_can_select(const struct usb_auto_state *state, enum usb_os_guess guess,
                         bool hid_ready, uint32_t now_ms) {
    bool known = guess == USB_OS_MACOS || guess == USB_OS_WINDOWS;
    return state->selected == USB_OS_UNKNOWN && known && hid_ready && state->held == 0 &&
           (uint32_t)(now_ms - state->last_activity_ms) >= USB_AUTO_IDLE_MS;
}
