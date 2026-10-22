// Politique pure d’appairage, sans adresse, clé, API radio ni écriture NVS.
#include <splitergoll/ble_pairing_state.h>
#include <errno.h>

// Une connexion inconnue ne prend jamais un slot occupé ni celui d’une autre transaction.
int ble_pairing_reserve(struct ble_pairing_state *state, unsigned count, uint16_t free_slots,
                        int selected, uint32_t connection, bool window_open) {
    if (count == 0 || count > 16 || ((uint32_t)free_slots >> count) != 0 ||
        selected < -1 || selected >= (int)count || connection == 0) return -EINVAL;
    if (!window_open) return -EACCES;
    if (state->connection != 0) return -EBUSY;
    if (free_slots == 0) return -ENOSPC;
    int slot = selected;
    if (slot < 0 || !(free_slots & (UINT32_C(1) << slot))) {
        for (slot = 0; slot < (int)count; slot++) {
            if (free_slots & (UINT32_C(1) << slot)) break;
        }
    }
    *state = (struct ble_pairing_state){.connection = connection, .slot = slot, .origin = selected};
    return slot;
}

// Un doublon SMP ne réarme ni les dix secondes ni les trente secondes.
bool ble_pairing_begin(struct ble_pairing_state *state, uint32_t connection, uint32_t now_ms,
                       bool window_open) {
    if (!window_open || connection == 0 || state->connection != connection ||
        state->phase != BLE_PAIRING_WAITING) return false;
    state->started_at_ms = now_ms;
    state->phase = BLE_PAIRING_STARTED;
    return true;
}

// Les soustractions non signées gèrent aussi le passage à zéro de l’horloge 32 bits.
uint32_t ble_pairing_remaining(const struct ble_pairing_state *state, uint32_t now_ms,
                               uint32_t window_remaining_ms) {
    if (state->connection == 0) return 0;
    if (state->phase == BLE_PAIRING_WAITING) return window_remaining_ms;
    if (state->phase != BLE_PAIRING_STARTED) return 0;
    uint32_t elapsed = now_ms - state->started_at_ms;
    return elapsed < BLE_PAIRING_TIMEOUT_MS ? BLE_PAIRING_TIMEOUT_MS - elapsed : 0;
}

// Une réussite déjà admise n’est pas annulée par un retard de planification du worker.
bool ble_pairing_expired(const struct ble_pairing_state *state, uint32_t now_ms,
                         uint32_t window_remaining_ms) {
    return state->connection != 0 && state->phase != BLE_PAIRING_COMPLETE &&
           ble_pairing_remaining(state, now_ms, window_remaining_ms) == 0;
}

// La réussite exige un bond et un début autorisé, jamais une simple connexion ACL.
bool ble_pairing_complete(struct ble_pairing_state *state, uint32_t connection, uint32_t now_ms,
                          bool bonded) {
    if (!bonded || connection == 0 || state->connection != connection ||
        state->phase != BLE_PAIRING_STARTED || ble_pairing_remaining(state, now_ms, 0) == 0) return false;
    state->phase = BLE_PAIRING_COMPLETE;
    return true;
}

// Oublie seulement la réservation en RAM, pas les appairages enregistrés.
void ble_pairing_release(struct ble_pairing_state *state) {
    *state = (struct ble_pairing_state){.slot = -1, .origin = -1};
}
