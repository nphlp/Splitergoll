// Politique pure : aucun pilote, timer, événement ou écriture en flash.
#include <splitergoll/usb_priority.h>

// Une connexion, une association ou un profil remplacé ne conserve jamais l’ancienne autorisation USB.
bool usb_priority_allowed(const struct usb_priority_state *state, struct usb_priority_evidence e) {
    return state->enabled && e.context_valid && state->accepted.profile == e.profile &&
        state->accepted.usb_context == e.usb_context && state->accepted.ble_context == e.ble_context;
}

// Une expiration sur le même ordinateur peut attendre la fin des frappes, jamais un changement de contexte.
bool usb_priority_step(struct usb_priority_state *state, struct usb_priority_evidence e, bool busy) {
    bool before = state->enabled;
    bool revoked = before && !usb_priority_allowed(state, e);
    if (revoked) state->enabled = false;
    bool wanted = e.context_valid && e.matched;
    state->pending = wanted != state->enabled;
    if (state->pending && !busy) {
        state->enabled = wanted;
        state->accepted = e;
        state->pending = false;
    }
    return revoked || before != state->enabled;
}
