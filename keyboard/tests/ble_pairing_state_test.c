// Vérifie l’accueil de nouveaux appareils sans radio, clés ni modification des slots réels.
#include <splitergoll/ble_pairing_state.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

// Tous les masques libres et les sélections choisissent une place réellement vide.
static void reservations(void) {
    for (unsigned count = 1; count <= 8; count++) {
        for (unsigned mask = 0; mask < (1u << count); mask++) {
            for (int selected = -1; selected < (int)count; selected++) {
                struct ble_pairing_state state = {0};
                int expected = -ENOSPC;
                for (unsigned i = 0; i < count; i++) {
                    if (mask & (1u << i)) { expected = (int)i; break; }
                }
                if (selected >= 0 && (mask & (1u << selected))) expected = selected;
                assert(ble_pairing_reserve(&state, count, (uint16_t)mask, selected, 1, true) == expected);
                if (expected < 0) { assert(!state.connection); continue; }
                assert(state.slot == expected && state.origin == selected && state.connection == 1);
                assert(ble_pairing_reserve(&state, count, (uint16_t)mask, selected, 2, true) == -EBUSY);
                assert(state.slot == expected && state.connection == 1);
                ble_pairing_release(&state);
                assert(!state.connection && state.slot == -1);
            }
        }
    }
    struct ble_pairing_state state = {0};
    assert(ble_pairing_reserve(&state, 16, 0x8000, 0, 1, true) == 15);
    ble_pairing_release(&state);
    assert(ble_pairing_reserve(&state, 0, 0, 0, 1, true) == -EINVAL);
    assert(ble_pairing_reserve(&state, 17, 1, 0, 1, true) == -EINVAL);
    assert(ble_pairing_reserve(&state, 5, 32, 0, 1, true) == -EINVAL);
    assert(ble_pairing_reserve(&state, 5, 1, 5, 1, true) == -EINVAL);
    assert(ble_pairing_reserve(&state, 5, 1, -2, 1, true) == -EINVAL);
    assert(ble_pairing_reserve(&state, 5, 1, 0, 0, true) == -EINVAL);
    assert(ble_pairing_reserve(&state, 5, 1, 0, 1, false) == -EACCES);
    assert(!state.connection);
}

// Une connexion seule ne prolonge pas l’accueil et ne vaut pas un appairage réussi.
static void waiting(void) {
    struct ble_pairing_state state = {0};
    assert(ble_pairing_reserve(&state, 5, 3, 0, 1, true) == 0);
    assert(ble_pairing_remaining(&state, 9999, 1) == 1);
    assert(ble_pairing_expired(&state, 10000, 0));
    assert(!ble_pairing_begin(&state, 1, 10000, false));
    assert(!ble_pairing_complete(&state, 1, 10000, true));
    assert(!ble_pairing_begin(&state, 2, 9000, true));
    assert(state.phase == BLE_PAIRING_WAITING);
}

// La transaction dispose de trente secondes depuis SMP, même après fermeture des annonces.
static void transaction_deadlines(void) {
    const uint32_t starts[] = {9999, UINT32_MAX - 100};
    for (unsigned i = 0; i < sizeof(starts) / sizeof(starts[0]); i++) {
        struct ble_pairing_state state = {0};
        assert(ble_pairing_reserve(&state, 5, 2, 0, 7, true) == 1);
        assert(ble_pairing_begin(&state, 7, starts[i], true));
        assert(!ble_pairing_begin(&state, 7, starts[i] + 1000, true));
        assert(ble_pairing_remaining(&state, starts[i] + 1000, 0) == 29000);
        assert(!ble_pairing_expired(&state, starts[i] + 29999, 0));
        assert(!ble_pairing_complete(&state, 8, starts[i] + 29999, true));
        assert(!ble_pairing_complete(&state, 7, starts[i] + 29999, false));
        struct ble_pairing_state late = state;
        assert(ble_pairing_expired(&late, starts[i] + 30000, 10000));
        assert(!ble_pairing_complete(&late, 7, starts[i] + 30000, true));
        assert(ble_pairing_complete(&state, 7, starts[i] + 29999, true));
        assert(!ble_pairing_expired(&state, starts[i] + 40000, 0));
        assert(!ble_pairing_complete(&state, 7, starts[i] + 29999, true));
        ble_pairing_release(&state);
        assert(!ble_pairing_remaining(&state, 0, 10000));
        assert(!ble_pairing_expired(&state, 0, 0));
        assert(ble_pairing_reserve(&state, 5, 4, 0, 8, true) == 2);
        state.phase = BLE_PAIRING_ABORTED;
        assert(ble_pairing_expired(&state, 0, 10000));
        assert(!ble_pairing_begin(&state, 8, 0, true));
    }
}

// Exécute les scénarios de réservation, échéances exactes, concurrence et limites de capacité.
int main(void) {
    reservations();
    waiting();
    transaction_deadlines();
    puts("ble_pairing_state: ok");
    return 0;
}
