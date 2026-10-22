// Réserve un slot d’accueil sans changer la destination des frappes.
#ifndef SPLITERGOLL_BLE_PAIRING_STATE_H
#define SPLITERGOLL_BLE_PAIRING_STATE_H
#include <stdbool.h>
#include <stdint.h>

#define BLE_PAIRING_TIMEOUT_MS UINT32_C(30000)
enum ble_pairing_phase { BLE_PAIRING_WAITING, BLE_PAIRING_STARTED, BLE_PAIRING_COMPLETE, BLE_PAIRING_ABORTED };

struct ble_pairing_state {
    uint32_t connection;
    uint32_t started_at_ms;
    int slot;
    int origin;
    enum ble_pairing_phase phase;
};

// Réserve une seule place libre, de préférence celle déjà sélectionnée si elle est vide.
int ble_pairing_reserve(struct ble_pairing_state *state, unsigned count, uint16_t free_slots,
                        int selected, uint32_t connection, bool window_open);

// Le début SMP doit encore appartenir aux trente secondes, sans repousser sa propre limite.
bool ble_pairing_begin(struct ble_pairing_state *state, uint32_t connection, uint32_t now_ms,
                       bool window_open);

// Une réussite reçue avant la limite peut être enregistrée ensuite par le worker.
bool ble_pairing_complete(struct ble_pairing_state *state, uint32_t connection, uint32_t now_ms,
                          bool bonded);

// Une connexion sans début SMP expire avec la fenêtre, SMP dispose de sa propre borne.
uint32_t ble_pairing_remaining(const struct ble_pairing_state *state, uint32_t now_ms,
                               uint32_t window_remaining_ms);

// Une transaction réussie n’expire plus pendant l’attente de sa sauvegarde.
bool ble_pairing_expired(const struct ble_pairing_state *state, uint32_t now_ms,
                         uint32_t window_remaining_ms);

// La place ne redevient libre qu’après traitement de la connexion qui la détenait.
void ble_pairing_release(struct ble_pairing_state *state);
#endif
