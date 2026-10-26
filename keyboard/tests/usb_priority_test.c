// Politique d’autorisation USB, sans radio ni pilote et sans attente réelle.
#include <assert.h>
#include <stdio.h>
#include <splitergoll/usb_priority.h>

// Les contextes synthétiques représentent des générations de liens, pas des identifiants d’appareil.
int main(void) {
    struct usb_priority_state state = {0};
    struct usb_priority_evidence e = {.profile = 0, .usb_context = 1, .ble_context = 1};
    assert(!usb_priority_step(&state, e, false) && !state.enabled);
    e.context_valid = true;
    assert(!usb_priority_step(&state, e, false) && !state.enabled);
    e.matched = true;
    assert(!usb_priority_step(&state, e, true) && state.pending && !state.enabled);
    assert(usb_priority_step(&state, e, false) && !state.pending && state.enabled);
    assert(usb_priority_allowed(&state, e));
    assert(!usb_priority_step(&state, e, false));
    e.matched = false;
    assert(!usb_priority_step(&state, e, true) && state.enabled && state.pending);
    assert(usb_priority_allowed(&state, e));
    assert(usb_priority_step(&state, e, false) && !state.enabled);
    e.matched = true;
    assert(usb_priority_step(&state, e, false));
    e.profile = 1;
    assert(!usb_priority_allowed(&state, e));
    assert(usb_priority_step(&state, e, true) && !state.enabled && state.pending);
    assert(usb_priority_step(&state, e, false) && state.enabled);
    e.usb_context++;
    assert(!usb_priority_allowed(&state, e));
    assert(usb_priority_step(&state, e, false) && state.enabled);
    e.ble_context++;
    assert(!usb_priority_allowed(&state, e));
    assert(usb_priority_step(&state, e, true) && !state.enabled);
    e.context_valid = false;
    assert(!usb_priority_step(&state, e, false));
    e.context_valid = true;
    assert(usb_priority_step(&state, e, false));
    e.context_valid = false;
    assert(usb_priority_step(&state, e, true) && !state.enabled && !state.pending);
    puts("usb_priority: preuve requise, relâchements, expiration et contextes remplacés passent");
    return 0;
}
