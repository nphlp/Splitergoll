// Vérifie les destinations connectées et les gardes du raccourci, sans radio ni écriture NVS.
#include <splitergoll/ble_device_state.h>
#include <splitergoll/ble_auto_state.h>
#include <assert.h>
#include <stdio.h>

// Toutes les combinaisons de huit hôtes doivent aboutir au premier connecté dans l’ordre circulaire.
static void connected_only(void) {
    for (unsigned count = 1; count <= 8; count++) {
        for (unsigned active = 0; active < count; active++) {
            for (unsigned mask = 0; mask < (1u << count); mask++) {
                struct ble_device_request request = {0};
                int expected = -1;
                for (unsigned step = 1; step <= count; step++) {
                    unsigned candidate = (active + step) % count;
                    if (mask & (1u << candidate)) {
                        if (candidate != active) expected = (int)candidate;
                        break;
                    }
                }
                assert(ble_device_next(&request, count, active, mask) == (expected >= 0));
                assert(ble_device_take(&request, count, active, mask, false) == expected);
                assert(!request.pending);
                assert(ble_device_take(&request, count, active, mask, false) == -1);
            }
        }
    }
    struct ble_device_request request = {0};
    assert(ble_device_next(&request, 16, 14, 0x8000));
    assert(ble_device_take(&request, 16, 14, 0x8000, false) == 15);
}

// Les invalidités sont refusées et annulent une ancienne demande au lieu de sélectionner un autre hôte.
static void invalid_inputs(void) {
    const unsigned counts[] = {0, 17, 5, 5};
    const unsigned active[] = {0, 0, 5, 0};
    const uint16_t masks[] = {0, 1, 1, 32};
    for (unsigned i = 0; i < sizeof(counts) / sizeof(counts[0]); i++) {
        struct ble_device_request request = {.origin = 0, .target = 1, .pending = true};
        assert(!ble_device_next(&request, counts[i], active[i], masks[i]));
        assert(!request.pending);
        request = (struct ble_device_request){.origin = 0, .target = 1, .pending = true};
        assert(ble_device_take(&request, counts[i], active[i], masks[i], true) == -1);
        assert(!request.pending);
    }
    struct ble_device_request request = {.origin = 0, .target = 99, .pending = true};
    assert(ble_device_take(&request, 5, 0, 3, false) == -1);
}

// Une demande attend le repos mais ne survit ni à une déconnexion cible ni à une sélection externe.
static void pending_requests(void) {
    struct ble_device_request request = {0};
    assert(ble_device_next(&request, 5, 0, 3));
    assert(ble_device_take(&request, 5, 0, 3, true) == -1);
    assert(request.pending);
    assert(ble_device_take(&request, 5, 0, 3, false) == 1);
    assert(ble_device_next(&request, 5, 0, 3));
    assert(ble_device_take(&request, 5, 0, 1, true) == -1);
    assert(!request.pending);
    assert(ble_device_next(&request, 5, 0, 3));
    assert(ble_device_take(&request, 5, 2, 3, false) == -1);
    assert(!request.pending);
    assert(ble_device_next(&request, 5, 0, 3));
    assert(!ble_device_next(&request, 5, 0, 3));
    assert(!request.pending);
    assert(ble_device_next(&request, 5, 0, 0x15));
    assert(request.target == 2);
    assert(ble_device_next(&request, 5, 0, 0x15));
    assert(request.target == 4);
    assert(!ble_device_next(&request, 5, 0, 0x15));
}

// Le geste de secours reste accessible sans OS, sans laisser passer de touche ordinaire.
static void unavailable_keymap(void) {
    struct usb_auto_state input = {0};
    assert(!ble_device_fallback(&input, 40, 43, 43, true));
    assert(!usb_auto_position(&input, 40, true, 0));
    assert(ble_device_fallback(&input, 40, 43, 43, true));
    assert(!ble_device_fallback(&input, 40, 43, 43, false));
    assert(!ble_device_fallback(&input, 40, 43, 42, true));
    assert(!ble_device_fallback(&input, 44, 43, 43, true));
    assert(!ble_device_fallback(&input, 40, 44, 44, true));
    assert(!ble_device_fallback(&input, 40, 40, 40, true));
    input.selected = USB_OS_MACOS;
    assert(!ble_device_fallback(&input, 40, 43, 43, true));
    input.selected = USB_OS_UNKNOWN;
    assert(!usb_auto_position(&input, 43, true, 1));
    assert(!ble_device_fallback(&input, 40, 43, 43, true));
    assert(!usb_auto_position(&input, 43, false, 2));
    assert(!usb_auto_position(&input, 40, false, 3));
    assert(input.held == 0 && input.admitted == 0);
}

// Le changement de profil ne doit ni sacrifier une release ni ajouter deux fois les 750 ms.
static void complete_drain(void) {
    struct ble_auto_state state = {0};
    ble_auto_sync(&state, (struct ble_auto_target){0, 1, BLE_MEMORY_MACOS, true}, 0);
    state.input.selected = USB_OS_MACOS;
    assert(usb_auto_position(&state.input, 40, true, UINT32_MAX - 100));
    assert(!ble_device_quiesce(&state.input, false, 1000));
    assert(usb_auto_position(&state.input, 40, false, UINT32_MAX - 10));
    assert(!ble_device_quiesce(&state.input, false, 738));
    assert(state.input.selected == USB_OS_MACOS);
    assert(!ble_device_quiesce(&state.input, true, 739));
    state.input.admitted = 1;
    assert(!ble_device_quiesce(&state.input, false, 739));
    state.input.admitted = 0;
    assert(ble_device_quiesce(&state.input, false, 739));
    assert(state.input.selected == USB_OS_UNKNOWN && !state.input.wait_for_behaviors);
    ble_auto_sync(&state, (struct ble_auto_target){1, 2, BLE_MEMORY_WINDOWS, true}, 739);
    assert(ble_auto_can_select(&state, false, 739));
}

// Un premier hôte peut devenir la destination, un ajout ne vole jamais une connexion existante.
static void initial_pairing(void) {
    struct ble_device_request request = {0};
    assert(ble_device_adopt(&request, 5, 4, 1, 0, 4));
    assert(request.target == 0 && request.origin == 4);
    assert(ble_device_take(&request, 5, 4, 1, true) == -1 && request.pending);
    assert(ble_device_take(&request, 5, 4, 1, false) == 0);
    assert(!ble_device_adopt(&request, 5, 0, 1, 0, 0));
    assert(!ble_device_adopt(&request, 5, 0, 3, 1, 0));
    assert(!ble_device_adopt(&request, 5, 3, 2, 1, 0));
    assert(!ble_device_adopt(&request, 5, 0, 0, 1, 0));
    assert(!ble_device_adopt(&request, 5, 0, 2, 16, 0));
    assert(!ble_device_adopt(&request, 0, 0, 2, 1, 0));
    assert(!ble_device_adopt(&request, 17, 0, 2, 1, 0));
    request = (struct ble_device_request){.origin = 0, .target = 2, .pending = true};
    assert(!ble_device_adopt(&request, 5, 0, 2, 1, 0));
    assert(request.pending && request.target == 2);
}

// La saisie pendant le drainage conserve son ordre, sa source et ses timestamps sans attendre un silence.
static void input_buffer(void) {
    struct ble_device_buffer buffer = {0};
    struct ble_device_position a = {.position = 1, .source = 0, .timestamp = 120, .pressed = true};
    assert(ble_device_buffer_forward(&buffer, a, false));
    ble_device_buffer_begin(&buffer);
    assert(!ble_device_buffer_forward(&buffer, a, false));
    assert(buffer.count == 1 && buffer.held == 2);
    assert(!ble_device_buffer_forward(&buffer, a, false));
    assert(buffer.count == 1);
    assert(ble_device_buffer_forward(&buffer, (struct ble_device_position){.position = 40}, true));
    assert(buffer.count == 1);
    a.timestamp = 145;
    a.pressed = false;
    assert(!ble_device_buffer_forward(&buffer, a, false));
    assert(buffer.count == 2 && buffer.held == 0);
    assert(buffer.events[0].pressed && !buffer.events[1].pressed);
    assert(buffer.events[0].timestamp == 120 && buffer.events[1].timestamp == 145);
    assert(buffer.events[0].source == 0 && buffer.events[1].position == 1);
    ble_device_buffer_begin(&buffer);
    assert(buffer.count == 2);
    a.position = 44;
    assert(!ble_device_buffer_forward(&buffer, a, true));
    assert(buffer.count == 2);
}

// Un abandon absorbe les relâchements du lot, sans laisser une release se promener vers un autre hôte.
static void cancelled_buffer(void) {
    struct ble_device_buffer buffer = {0};
    struct ble_device_position a = {.position = 1, .pressed = true};
    ble_device_buffer_begin(&buffer);
    assert(!ble_device_buffer_forward(&buffer, a, false));
    ble_device_buffer_discard(&buffer);
    assert(!buffer.active && buffer.count == 0 && buffer.suppressed == 2);
    assert(!ble_device_buffer_forward(&buffer, a, false));
    a.pressed = false;
    assert(!ble_device_buffer_forward(&buffer, a, false));
    assert(buffer.suppressed == 0);
    a.pressed = true;
    assert(ble_device_buffer_forward(&buffer, a, false));
}

// Un dépassement abandonne tout le lot, jamais une pression isolée dont la release aurait disparu.
static void bounded_buffer(void) {
    struct ble_device_buffer buffer = {0};
    ble_device_buffer_begin(&buffer);
    for (unsigned i = 0; i < BLE_DEVICE_BUFFER_CAPACITY; i++) {
        assert(!ble_device_buffer_forward(&buffer,
            (struct ble_device_position){.position = 1, .pressed = (i % 2 == 0)}, false));
    }
    assert(buffer.count == BLE_DEVICE_BUFFER_CAPACITY && !buffer.overflow);
    assert(!ble_device_buffer_forward(&buffer,
        (struct ble_device_position){.position = 2, .pressed = true}, false));
    assert(buffer.overflow && buffer.count == 0 && buffer.held == 4);
    ble_device_buffer_discard(&buffer);
    assert(!ble_device_buffer_forward(&buffer, (struct ble_device_position){.position = 2}, false));
    assert(!buffer.overflow && !buffer.suppressed);
}

// Exécute la politique utilisée par le comportement ZMK et le chemin de secours.
int main(void) {
    connected_only();
    invalid_inputs();
    pending_requests();
    unavailable_keymap();
    complete_drain();
    initial_pairing();
    input_buffer();
    cancelled_buffer();
    bounded_buffer();
    puts("ble_device_state: ok");
    return 0;
}
