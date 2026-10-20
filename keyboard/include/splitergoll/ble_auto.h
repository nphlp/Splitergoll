// Synchronise les touches et les commandes explicites de diagnostic avec le sélecteur BLE.
#ifndef SPLITERGOLL_BLE_AUTO_H
#define SPLITERGOLL_BLE_AUTO_H
#include <stdbool.h>

// Partage la sérialisation des entrées avec les commandes USB optionnelles.
void splitergoll_ble_auto_lock(void);
void splitergoll_ble_auto_unlock(void);

// Refuse une mutation pendant un maintien, un rapport HID actif ou un comportement différé.
bool splitergoll_ble_auto_busy(void);

// Demande le prochain hôte connecté, sans mutation tant qu’un appui reste actif.
int splitergoll_ble_auto_next_device(void);
#endif
