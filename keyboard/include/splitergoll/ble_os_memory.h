// Association d’un OS à une identité opaque d’appairage, sans adresse ni clé exportable.
#ifndef SPLITERGOLL_BLE_OS_MEMORY_H
#define SPLITERGOLL_BLE_OS_MEMORY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BLE_OS_MEMORY_ID_SIZE 32
#define BLE_OS_MEMORY_SIZE 40
#define BLE_OS_MEMORY_RULES 1

enum ble_memory_os { BLE_MEMORY_UNKNOWN, BLE_MEMORY_MACOS, BLE_MEMORY_WINDOWS, BLE_MEMORY_LINUX };
enum ble_memory_origin { BLE_MEMORY_NONE, BLE_MEMORY_INITIAL, BLE_MEMORY_MANUAL };

struct ble_os_memory {
    uint8_t identity[BLE_OS_MEMORY_ID_SIZE];
    enum ble_memory_os os;
    enum ble_memory_origin origin;
    uint16_t evidence;
    bool present;
    bool verified;
    bool dirty;
    int error;
};

// Charge un enregistrement versionné, sans lui faire confiance avant vérification d’identité.
bool ble_memory_decode(struct ble_os_memory *memory, const uint8_t *bytes, size_t size);

// Sérialise explicitement les octets, sans dépendre du padding ou de l’endianness C.
void ble_memory_encode(const struct ble_os_memory *memory, uint8_t bytes[BLE_OS_MEMORY_SIZE]);

// Invalide une ancienne association et ouvre l’acquisition seulement pour un nouvel appairage.
void ble_memory_bind(struct ble_os_memory *memory, const uint8_t identity[BLE_OS_MEMORY_ID_SIZE],
                     bool new_pairing);

// Modifie l’OS pendant l’acquisition initiale ou sur correction explicite, jamais en reconnexion.
bool ble_memory_assign(struct ble_os_memory *memory, enum ble_memory_os os, uint16_t evidence,
                       bool initial_connection, bool manual);

// Ne présente une sauvegarde comme terminée qu’après le retour du backend persistant.
void ble_memory_saved(struct ble_os_memory *memory, int error);

// Fournit les noms stables du diagnostic, sans valeur implicite pour inconnu.
const char *ble_memory_os_name(enum ble_memory_os os);

#endif
