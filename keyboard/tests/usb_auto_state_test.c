// Vérifie les transitions USB et les relâchements avant toute bascule de couches.
#include <splitergoll/usb_auto_state.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

// Les frappes ignorées ne prolongent pas le démarrage une fois leur touche relâchée.
static void test_ignored_inputs_do_not_delay_startup(void) {
    struct usb_auto_state state = {0};
    usb_auto_sync(&state, 1, 1000);
    for (unsigned i = 0; i < 20; i++) {
        uint32_t time = 1100 + i * 50;
        assert(!usb_auto_position(&state, 0, true, time));
        assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, time));
        assert(!usb_auto_position(&state, 0, false, time + 10));
        assert(usb_auto_can_select(&state, USB_OS_MACOS, true, time + 10));
    }
}

// Aucun OS n’est choisi sur alimentation seule ou sur une signature incertaine.
static void test_initial_selection(void) {
    struct usb_auto_state state = {0};
    assert(!usb_auto_can_select(&state, USB_OS_UNKNOWN, true, 5000));
    assert(!usb_auto_can_select(&state, USB_OS_AMBIGUOUS, true, 5000));
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, false, 5000));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 0));
    assert(usb_auto_can_select(&state, USB_OS_WINDOWS, true, 0));
    usb_auto_sync(&state, 1, 6000);
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 6000));
    assert(!state.wait_for_behaviors);
    state.selected = USB_OS_MACOS;
    assert(!usb_auto_can_select(&state, USB_OS_WINDOWS, true, 5000));
    assert(!usb_auto_can_select(&state, USB_OS_AMBIGUOUS, true, 5000));
}

// Maintenir une touche pendant l’énumération ne produit pas de frappe tardive.
static void test_waiting_keys(void) {
    struct usb_auto_state state = {0};
    assert(!usb_auto_position(&state, 0, true, 1000));
    assert(!usb_auto_position(&state, 43, true, 1100));
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, 5000));
    assert(!usb_auto_position(&state, 0, false, 5000));
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, 10000));
    assert(!usb_auto_position(&state, 43, false, 10000));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 10000));
    state.selected = USB_OS_MACOS;
    assert(!usb_auto_position(&state, 43, false, 11000));
    assert(usb_auto_position(&state, 43, true, 11001));
    assert(usb_auto_position(&state, 43, false, 11002));
}

// Un reset du bus au milieu d’un raccourci bloque les nouveaux appuis et garde les releases.
static void test_reconnection_while_held(void) {
    struct usb_auto_state state = {.selected = USB_OS_MACOS, .generation = 5};
    assert(usb_auto_position(&state, 18, true, 1000));
    assert(usb_auto_position(&state, 1, true, 1001));
    usb_auto_sync(&state, 6, 1002);
    assert(state.selected == USB_OS_UNKNOWN);
    assert(!usb_auto_position(&state, 30, true, 1003));
    assert(!usb_auto_can_select(&state, USB_OS_WINDOWS, true, 5000));
    assert(usb_auto_position(&state, 18, false, 5001));
    assert(usb_auto_position(&state, 1, false, 5002));
    assert(!usb_auto_position(&state, 30, false, 5003));
    assert(!usb_auto_can_select(&state, USB_OS_WINDOWS, true, 5751));
    assert(usb_auto_can_select(&state, USB_OS_WINDOWS, true, 5752));
    state.selected = USB_OS_WINDOWS;
    usb_auto_sync(&state, 6, 6000);
    assert(state.selected == USB_OS_WINDOWS);
    usb_auto_sync(&state, 7, 6001);
    assert(state.selected == USB_OS_UNKNOWN);
    assert(!usb_auto_can_select(&state, USB_OS_UNKNOWN, true, 10000));
}

// Les rotations et positions ignorées ne repoussent pas la marge après un cycle actif.
static void test_recovery_ignores_unadmitted_activity(void) {
    struct usb_auto_state state = {.selected = USB_OS_WINDOWS};
    usb_auto_sync(&state, 1, 1000);
    assert(state.wait_for_behaviors);
    assert(!usb_auto_position(&state, 43, true, 1500));
    assert(!usb_auto_sensor(&state, 1600));
    assert(!usb_auto_position(&state, 43, false, 1700));
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, 1749));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 1750));

    usb_auto_sync(&state, 2, 1800);
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, 2549));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 2550));
}

// Une sortie différée réarme le garde, même si son appui est bloqué dans le nouveau cycle.
static void test_delayed_keycodes_and_sensors(void) {
    struct usb_auto_state state = {0};
    assert(!usb_auto_sensor(&state, 1000));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 1000));

    assert(!usb_auto_keycode(&state, true, 1100));
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, 1849));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 1850));
    assert(usb_auto_keycode(&state, false, 1900));
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, 2649));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 2650));

    state.selected = USB_OS_MACOS;
    assert(usb_auto_sensor(&state, 3000));
    assert(usb_auto_keycode(&state, true, 3100));
    assert(usb_auto_keycode(&state, false, 3200));
    usb_auto_sync(&state, 1, 3300);
    assert(!usb_auto_keycode(&state, true, 3900));
    assert(usb_auto_keycode(&state, false, 4000));
    assert(!usb_auto_can_select(&state, USB_OS_WINDOWS, true, 4749));
    assert(usb_auto_can_select(&state, USB_OS_WINDOWS, true, 4750));
}

// Les répétitions et positions hors matrice ne provoquent ni release doublée ni décalage invalide.
static void test_duplicate_and_invalid_positions(void) {
    struct usb_auto_state state = {.selected = USB_OS_WINDOWS};
    assert(!usb_auto_position(&state, 44, true, 100));
    assert(!usb_auto_position(&state, UINT32_MAX, false, 100));
    assert(usb_auto_position(&state, 43, true, 101));
    assert(!usb_auto_position(&state, 43, true, 102));
    assert(usb_auto_position(&state, 43, false, 103));
    assert(!usb_auto_position(&state, 43, false, 104));
    assert(state.held == 0 && state.admitted == 0);
}

// Le délai reste correct lorsque l’horloge et le numéro de cycle débordent.
static void test_counter_wrap(void) {
    struct usb_auto_state state = {.generation = UINT32_MAX, .selected = USB_OS_MACOS};
    usb_auto_sync(&state, 0, UINT32_MAX - 200);
    assert(state.selected == USB_OS_UNKNOWN);
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, 548));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 549));
}

// Lance les scénarios indépendants, avec assertions et sanitizers activés par la commande de test.
int main(void) {
    test_ignored_inputs_do_not_delay_startup();
    test_initial_selection();
    test_waiting_keys();
    test_reconnection_while_held();
    test_recovery_ignores_unadmitted_activity();
    test_delayed_keycodes_and_sensors();
    test_duplicate_and_invalid_positions();
    test_counter_wrap();
    puts("USB auto state: OK");
    return 0;
}
