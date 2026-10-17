// Vérifie les transitions USB et les relâchements avant toute bascule de couches.
#include <splitergoll/usb_auto_state.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

// Aucun OS n’est choisi sur alimentation seule ou sur une signature incertaine.
static void test_initial_selection(void) {
    struct usb_auto_state state = {0};
    assert(!usb_auto_can_select(&state, USB_OS_UNKNOWN, true, 5000));
    assert(!usb_auto_can_select(&state, USB_OS_AMBIGUOUS, true, 5000));
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, false, 5000));
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, USB_AUTO_IDLE_MS - 1));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, USB_AUTO_IDLE_MS));
    assert(usb_auto_can_select(&state, USB_OS_WINDOWS, true, USB_AUTO_IDLE_MS));
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
    assert(!usb_auto_can_select(&state, USB_OS_MACOS, true, 10749));
    assert(usb_auto_can_select(&state, USB_OS_MACOS, true, 10750));
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
    assert(!usb_auto_can_select(&state, USB_OS_WINDOWS, true, 5752));
    assert(usb_auto_can_select(&state, USB_OS_WINDOWS, true, 5753));
    state.selected = USB_OS_WINDOWS;
    usb_auto_sync(&state, 6, 6000);
    assert(state.selected == USB_OS_WINDOWS);
    usb_auto_sync(&state, 7, 6001);
    assert(state.selected == USB_OS_UNKNOWN);
    assert(!usb_auto_can_select(&state, USB_OS_UNKNOWN, true, 10000));
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
    test_initial_selection();
    test_waiting_keys();
    test_reconnection_while_held();
    test_duplicate_and_invalid_positions();
    test_counter_wrap();
    puts("USB auto state: OK");
    return 0;
}
