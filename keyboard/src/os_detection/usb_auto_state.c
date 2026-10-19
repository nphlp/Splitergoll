// État borné de l’auto-détection, indépendant des API du clavier.
#include <splitergoll/usb_auto_state.h>

// Seuls les deux OS reconnus autorisent la saisie et peuvent laisser un comportement différé.
static bool has_selected_os(const struct usb_auto_state *state) {
    return state->selected == USB_OS_MACOS || state->selected == USB_OS_WINDOWS;
}

// Conserve une marge après les événements susceptibles de produire un comportement différé.
static void note_behavior_activity(struct usb_auto_state *state, uint32_t now_ms) {
    state->last_activity_ms = now_ms;
    state->wait_for_behaviors = true;
}

// Un changement de bus bloque les nouveaux appuis mais laisse finir les anciens.
void usb_auto_sync(struct usb_auto_state *state, uint32_t generation, uint32_t now_ms) {
    if (state->generation != generation) {
        if (has_selected_os(state) || state->wait_for_behaviors || state->admitted != 0) {
            note_behavior_activity(state, now_ms);
        }
        state->generation = generation;
        state->selected = USB_OS_UNKNOWN;
    }
}

// Les appuis ignorés pendant l’attente ne produisent jamais de relâchement orphelin.
bool usb_auto_position(struct usb_auto_state *state, uint32_t position, bool pressed,
                       uint32_t now_ms) {
    if (position >= USB_AUTO_POSITION_COUNT) {
        return false;
    }
    uint64_t bit = UINT64_C(1) << position;
    if (pressed) {
        if ((state->held & bit) != 0) {
            return false;
        }
        state->held |= bit;
        bool selected = has_selected_os(state);
        if (selected) {
            state->admitted |= bit;
            note_behavior_activity(state, now_ms);
        }
        return selected;
    }
    bool admitted = (state->admitted & bit) != 0;
    state->held &= ~bit;
    state->admitted &= ~bit;
    if (admitted) {
        note_behavior_activity(state, now_ms);
    }
    return admitted;
}

// Une rotation ignorée n’exécute aucun comportement et n’a rien à laisser se terminer.
bool usb_auto_sensor(struct usb_auto_state *state, uint32_t now_ms) {
    bool forward = has_selected_os(state);
    if (forward) {
        note_behavior_activity(state, now_ms);
    }
    return forward;
}

// Les keycodes peuvent venir d’une macro ou d’un tap-dance lancé avant le changement de bus.
bool usb_auto_keycode(struct usb_auto_state *state, bool pressed, uint32_t now_ms) {
    note_behavior_activity(state, now_ms);
    return has_selected_os(state) || !pressed;
}

// La décision reste figée pendant le cycle, même si un autre motif arrive ensuite.
bool usb_auto_can_select(const struct usb_auto_state *state, enum usb_os_guess guess,
                         bool hid_ready, uint32_t now_ms) {
    bool known = guess == USB_OS_MACOS || guess == USB_OS_WINDOWS;
    return state->selected == USB_OS_UNKNOWN && known && hid_ready && state->held == 0 &&
           usb_auto_drain_remaining(state, now_ms) == 0;
}

// Ne programme aucun délai à froid, et borne la soustraction lors du débordement de l’horloge.
uint32_t usb_auto_drain_remaining(const struct usb_auto_state *state, uint32_t now_ms) {
    uint32_t elapsed = now_ms - state->last_activity_ms;
    if (!state->wait_for_behaviors || elapsed >= USB_AUTO_DRAIN_MS) {
        return 0;
    }
    return USB_AUTO_DRAIN_MS - elapsed;
}
