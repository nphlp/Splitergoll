// Backend NVS du prototype BLE, appelé hors des callbacks radio.
#ifndef SPLITERGOLL_BLE_OS_SETTINGS_H
#define SPLITERGOLL_BLE_OS_SETTINGS_H
#include <splitergoll/ble_os_memory.h>

// Attend la restauration complète avant de traiter les premières connexions du démarrage.
void ble_os_settings_wait_ready(void);

// Vérifie le bond courant via les réglages publics, puis invalide une ancienne association.
bool ble_os_settings_bind(unsigned profile, bool new_pairing);

// Sauvegarde une décision initiale sans changer de keymap ni de profil Bluetooth.
void ble_os_settings_learn(unsigned profile, enum ble_memory_os os, uint16_t evidence);

// Invalide uniquement notre association d’OS après notification native de suppression du bond.
void ble_os_settings_forget(unsigned profile);

// Copie un état sans identifiant exportable et sans déclencher de sauvegarde.
struct ble_os_memory ble_os_settings_snapshot(unsigned profile);

// Copie l’empreinte vérifiée pour une autre mémoire interne, jamais pour un diagnostic.
bool ble_os_settings_identity(unsigned profile, uint8_t identity[BLE_OS_MEMORY_ID_SIZE]);

#endif
