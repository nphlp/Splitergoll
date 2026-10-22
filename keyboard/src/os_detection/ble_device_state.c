// Les raccourcis changent de destination seulement après la fin des appuis et macros.
#include <splitergoll/ble_device_state.h>

// Une attente déjà armée conserve ses événements et sa borne mémoire.
void ble_device_buffer_begin(struct ble_device_buffer *buffer) {
    if (!buffer->active) {
        buffer->active = true;
        buffer->overflow = false;
        buffer->count = 0;
        buffer->held = 0;
    }
}

// Un ancien appui garde sa release, les nouveaux ne produisent pas encore de comportement ZMK.
bool ble_device_buffer_forward(struct ble_device_buffer *buffer, struct ble_device_position event,
                                bool passthrough) {
    if (event.position >= USB_AUTO_POSITION_COUNT) return false;
    uint64_t bit = UINT64_C(1) << event.position;
    if (buffer->suppressed & bit) {
        if (!event.pressed) buffer->suppressed &= ~bit;
        return false;
    }
    if (!buffer->active || passthrough) return true;
    bool held = (buffer->held & bit) != 0;
    if (event.pressed == held) return false;
    if (event.pressed) buffer->held |= bit;
    else buffer->held &= ~bit;
    if (buffer->count == BLE_DEVICE_BUFFER_CAPACITY) {
        buffer->overflow = true;
        buffer->count = 0;
    }
    if (!buffer->overflow) buffer->events[buffer->count++] = event;
    return false;
}

// Aucun lot incomplet n’est rejoué si sa cible disparaît ou si la capacité est dépassée.
void ble_device_buffer_discard(struct ble_device_buffer *buffer) {
    buffer->suppressed |= buffer->held;
    buffer->held = 0;
    buffer->count = 0;
    buffer->active = false;
    buffer->overflow = false;
}

// Les masques invalides sont refusés, pas tronqués en une autre sélection valide.
static bool valid_mask(unsigned count, unsigned active, uint16_t connected) {
    return count > 0 && count <= 16 && active < count &&
           ((uint32_t)connected >> count) == 0;
}

// Une connexion nouvelle ne prend pas la main sur un hôte existant ni une demande en cours.
bool ble_device_adopt(struct ble_device_request *request, unsigned count, unsigned active,
                      uint16_t connected, unsigned target, unsigned origin) {
    if (!valid_mask(count, active, connected) || target >= count || active != origin ||
        request->pending || connected != (UINT32_C(1) << target) || active == target) return false;
    *request = (struct ble_device_request){.origin = origin, .target = target, .pending = true};
    return true;
}

// Deux demandes rapides avancent deux fois, y compris si cela annule le premier changement.
bool ble_device_next(struct ble_device_request *request, unsigned count, unsigned active,
                      uint16_t connected) {
    if (!valid_mask(count, active, connected)) {
        request->pending = false;
        return false;
    }
    unsigned start = request->pending && request->origin == active ? request->target : active;
    if (start >= count) start = active;
    request->pending = false;
    for (unsigned step = 1; step <= count; step++) {
        unsigned candidate = (start + step) % count;
        if (!(connected & (UINT16_C(1) << candidate))) continue;
        request->origin = active;
        request->target = candidate;
        request->pending = candidate != active;
        return request->pending;
    }
    return false;
}

// Une déconnexion ou une autre sélection explicite ne doit pas déclencher une bascule tardive.
int ble_device_take(struct ble_device_request *request, unsigned count, unsigned active,
                     uint16_t connected, bool busy) {
    if (!request->pending) return -1;
    if (!valid_mask(count, active, connected) || request->origin != active ||
        request->target >= count || !(connected & (UINT16_C(1) << request->target))) {
        request->pending = false;
        return -1;
    }
    if (busy) return -1;
    request->pending = false;
    return (int)request->target;
}

// Le secours ne rejoue ni une touche tenue ni une frappe de la keymap précédente.
bool ble_device_fallback(const struct usb_auto_state *input, unsigned modifier, unsigned button,
                         unsigned position, bool pressed) {
    return input->selected == USB_OS_UNKNOWN && modifier < USB_AUTO_POSITION_COUNT &&
           button < USB_AUTO_POSITION_COUNT && modifier != button && pressed && position == button &&
           (input->held & (UINT64_C(1) << modifier)) &&
           !(input->held & (UINT64_C(1) << button));
}

// La bascule native videra un rapport déjà vide, aucune release n’est sacrifiée ici.
bool ble_device_quiesce(struct usb_auto_state *input, bool hid_busy, uint32_t now_ms) {
    if (input->held || input->admitted || hid_busy || usb_auto_drain_remaining(input, now_ms)) return false;
    input->selected = USB_OS_UNKNOWN;
    input->wait_for_behaviors = false;
    return true;
}
