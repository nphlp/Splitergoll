// Vérifie la sélection par hôte et les appuis qui traversent un changement de destination.
#include <splitergoll/ble_auto_state.h>
#include <assert.h>
#include <stdio.h>

// Chaque condition protège un cas différent, notamment un worker en retard sur la radio.
static void verified_memory(void) {
    struct ble_os_memory good = {.os = BLE_MEMORY_MACOS, .present = true, .verified = true};
    struct ble_auto_link link = {.observed_epoch = 7, .processed_epoch = 7,
                                 .connected = true, .secure = true, .reliable = true};
    assert(ble_auto_memory_ready(&good, link));
    struct ble_os_memory bad = good;
    bad.present = false; assert(!ble_auto_memory_ready(&bad, link));
    bad = good; bad.verified = false; assert(!ble_auto_memory_ready(&bad, link));
    bad = good; bad.dirty = true; assert(!ble_auto_memory_ready(&bad, link));
    bad = good; bad.error = -1; assert(!ble_auto_memory_ready(&bad, link));
    bad = good; bad.os = BLE_MEMORY_UNKNOWN; assert(!ble_auto_memory_ready(&bad, link));
    bad = good; bad.os = BLE_MEMORY_LINUX; assert(!ble_auto_memory_ready(&bad, link));
    bad = good; bad.os = BLE_MEMORY_WINDOWS; assert(ble_auto_memory_ready(&bad, link));
    struct ble_auto_link unavailable = link;
    unavailable.connected = false; assert(!ble_auto_memory_ready(&good, unavailable));
    unavailable = link; unavailable.secure = false; assert(!ble_auto_memory_ready(&good, unavailable));
    unavailable = link; unavailable.reliable = false; assert(!ble_auto_memory_ready(&good, unavailable));
    unavailable = link; unavailable.observed_epoch++; assert(!ble_auto_memory_ready(&good, unavailable));
    unavailable.processed_epoch++; assert(ble_auto_memory_ready(&good, unavailable));
    unavailable.observed_epoch = unavailable.processed_epoch = 0;
    assert(!ble_auto_memory_ready(&good, unavailable));
}

// Simule uniquement la confirmation du groupe après les gardes du moteur pur.
static void select_target(struct ble_auto_state *state, uint32_t now_ms) {
    assert(ble_auto_can_select(state, false, now_ms));
    state->input.selected = ble_auto_guess(state->target);
}

// La première connexion utilisable peut fonctionner immédiatement, sans dix secondes de pause.
static void cold_start(void) {
    struct ble_auto_state state = {0};
    struct ble_auto_target target = {.profile = 3, .session = 1, .os = BLE_MEMORY_MACOS, .ready = true};
    ble_auto_sync(&state, target, 42);
    assert(ble_auto_can_select(&state, false, 42));
    assert(!ble_auto_can_select(&state, true, 42));
    select_target(&state, 42);
    assert(state.input.selected == USB_OS_MACOS);
    assert(!ble_auto_can_select(&state, false, 42));
    uint32_t generation = state.input.generation;
    ble_auto_sync(&state, target, 43);
    assert(state.input.generation == generation);
    assert(state.input.selected == USB_OS_MACOS);
}

// Aucun résultat connu n’est inventé depuis un slot, une session manquante ou Linux.
static void unavailable_targets(void) {
    for (unsigned ready = 0; ready < 2; ready++) {
        for (int profile = -1; profile < 5; profile++) {
            for (unsigned session = 0; session < 2; session++) {
                for (enum ble_memory_os os = BLE_MEMORY_UNKNOWN; os <= BLE_MEMORY_LINUX; os++) {
                    struct ble_auto_state state = {0};
                    struct ble_auto_target target = {.profile = profile, .session = session,
                                                     .os = os, .ready = ready};
                    ble_auto_sync(&state, target, 0);
                    bool expected = ready && profile >= 0 && session != 0 &&
                                    (os == BLE_MEMORY_MACOS || os == BLE_MEMORY_WINDOWS);
                    assert(ble_auto_can_select(&state, false, 0) == expected);
                }
            }
        }
    }
}

// Un appui commencé sur l’ancien hôte doit se terminer avant d’armer le nouveau groupe.
static void switch_with_held_key(void) {
    struct ble_auto_state state = {0};
    ble_auto_sync(&state, (struct ble_auto_target){0, 1, BLE_MEMORY_MACOS, true}, 0);
    select_target(&state, 0);
    assert(usb_auto_position(&state.input, 7, true, 10));
    ble_auto_sync(&state, (struct ble_auto_target){1, 2, BLE_MEMORY_WINDOWS, true}, 20);
    assert(state.input.selected == USB_OS_UNKNOWN);
    assert(!ble_auto_can_select(&state, false, 10000));
    assert(!usb_auto_position(&state.input, 8, true, 10001));
    assert(usb_auto_position(&state.input, 7, false, 10002));
    assert(!usb_auto_position(&state.input, 8, false, 10003));
    assert(!ble_auto_can_select(&state, false, 10751));
    assert(ble_auto_can_select(&state, false, 10752));
    select_target(&state, 10752);
    assert(state.input.selected == USB_OS_WINDOWS);
    assert(usb_auto_position(&state.input, 8, true, 10753));
    assert(usb_auto_position(&state.input, 8, false, 10754));
}

// Perdre la sécurité, se reconnecter ou invalider le cache retire immédiatement la sélection.
static void invalidate_target(void) {
    const struct ble_auto_target changed[] = {
        {0, 1, BLE_MEMORY_MACOS, false},
        {0, 2, BLE_MEMORY_MACOS, true},
        {1, 1, BLE_MEMORY_MACOS, true},
        {0, 1, BLE_MEMORY_UNKNOWN, true},
        {0, 1, BLE_MEMORY_WINDOWS, true},
    };
    for (unsigned i = 0; i < sizeof(changed) / sizeof(changed[0]); i++) {
        struct ble_auto_state state = {0};
        ble_auto_sync(&state, (struct ble_auto_target){0, 1, BLE_MEMORY_MACOS, true}, UINT32_MAX - 20);
        select_target(&state, UINT32_MAX - 20);
        ble_auto_sync(&state, changed[i], UINT32_MAX - 10);
        assert(state.input.selected == USB_OS_UNKNOWN);
        assert(!usb_auto_sensor(&state.input, 0));
        assert(!usb_auto_keycode(&state.input, true, 1));
        assert(usb_auto_keycode(&state.input, false, 2));
        assert(!ble_auto_can_select(&state, false, 751));
        bool known = changed[i].ready && changed[i].os != BLE_MEMORY_UNKNOWN;
        assert(ble_auto_can_select(&state, false, 752) == known);
    }
}

// Un appui ignoré à froid n’ajoute pas de drainage, son maintien doit néanmoins finir.
static void ignored_cold_key(void) {
    struct ble_auto_state state = {0};
    assert(!usb_auto_position(&state.input, 9, true, 0));
    ble_auto_sync(&state, (struct ble_auto_target){1, 4, BLE_MEMORY_WINDOWS, true}, 1);
    assert(!ble_auto_can_select(&state, false, 1));
    assert(!usb_auto_position(&state.input, 9, false, 2));
    assert(ble_auto_can_select(&state, false, 2));
}

// Exécute la même politique que le firmware, indépendamment des callbacks Zephyr.
int main(void) {
    verified_memory();
    cold_start();
    unavailable_targets();
    switch_with_held_key();
    invalidate_target();
    ignored_cold_key();
    puts("ble_auto_state: ok");
    return 0;
}
