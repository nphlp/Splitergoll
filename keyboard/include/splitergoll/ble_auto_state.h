// Choix du groupe de couches depuis une association BLE vérifiée, sans OS implicite.
#ifndef SPLITERGOLL_BLE_AUTO_STATE_H
#define SPLITERGOLL_BLE_AUTO_STATE_H
#include <splitergoll/ble_os_memory.h>
#include <splitergoll/usb_auto_state.h>

struct ble_auto_target {
    int profile;
    uint32_t session;
    enum ble_memory_os os;
    bool ready;
};

struct ble_auto_state {
    struct usb_auto_state input;
    struct ble_auto_target target;
};

struct ble_auto_link {
    uint32_t observed_epoch;
    uint32_t processed_epoch;
    bool connected;
    bool secure;
    bool reliable;
};

// Exige une mémoire utilisable et le même état de connexion dans la radio et le worker.
bool ble_auto_memory_ready(const struct ble_os_memory *memory, struct ble_auto_link link);

// Invalide le groupe lors d’un changement d’appareil, de session, d’OS ou de disponibilité.
void ble_auto_sync(struct ble_auto_state *state, struct ble_auto_target target, uint32_t now_ms);

// Traduit explicitement les deux OS pris en charge, sans convertir Linux ou inconnu en Windows.
enum usb_os_guess ble_auto_guess(struct ble_auto_target target);

// Réutilise la garde d’appuis indépendante du transport, avec vérification du rapport HID.
bool ble_auto_can_select(const struct ble_auto_state *state, bool hid_busy, uint32_t now_ms);
#endif
