// Réconcilie les annonces de reconnexion avec une liste de slots connus, sans sélection ni NVS.
#ifndef SPLITERGOLL_BLE_RECONNECT_STATE_H
#define SPLITERGOLL_BLE_RECONNECT_STATE_H
#include <stdbool.h>
#include <stdint.h>

struct ble_reconnect_view {
    unsigned count;
    uint16_t known;
    uint16_t connected;
    uint32_t revision;
    bool window_open;
};

struct ble_reconnect_state {
    uint16_t allowed;
    uint32_t revision;
    bool advertising;
    int error;
};

struct ble_reconnect_ops {
    void *context;
    int (*stop)(void *context);
    int (*filter)(void *context, uint16_t allowed);
    int (*start)(void *context);
    bool (*current)(void *context);
};

// Arrête avant de modifier le filtre et refuse tout démarrage incomplet ou périmé.
int ble_reconnect_apply(struct ble_reconnect_state *state, struct ble_reconnect_view view,
                         const struct ble_reconnect_ops *ops);

// Une connexion nouvelle doit correspondre à un bond connu et arriver dans la fenêtre.
bool ble_reconnect_accept(bool window_open, bool profile_known, bool bonded);
#endif
