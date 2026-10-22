// Demande explicite de bascule vers un hôte déjà connecté, indépendante de Zephyr.
#ifndef SPLITERGOLL_BLE_DEVICE_STATE_H
#define SPLITERGOLL_BLE_DEVICE_STATE_H
#include <splitergoll/usb_auto_state.h>

struct ble_device_request {
    unsigned origin;
    unsigned target;
    bool pending;
};

// Parcourt les connexions en sautant les cases vides, sans cible implicite si aucune ne convient.
bool ble_device_next(struct ble_device_request *request, unsigned count, unsigned active,
                      uint16_t connected);

// Adopte le seul nouvel hôte sans remplacer une destination connectée ou un choix explicite.
bool ble_device_adopt(struct ble_device_request *request, unsigned count, unsigned active,
                      uint16_t connected, unsigned target, unsigned origin);

// Annule une demande périmée, ou la conserve tant que les touches ne sont pas au repos.
int ble_device_take(struct ble_device_request *request, unsigned count, unsigned active,
                     uint16_t connected, bool busy);

// Reconnaît le raccourci de secours quand aucune keymap ne peut encore être appliquée.
bool ble_device_fallback(const struct usb_auto_state *input, unsigned modifier, unsigned button,
                         unsigned position, bool pressed);

// Termine le drainage une seule fois avant de basculer, sans ajouter une seconde attente.
bool ble_device_quiesce(struct usb_auto_state *input, bool hid_busy, uint32_t now_ms);
#endif
