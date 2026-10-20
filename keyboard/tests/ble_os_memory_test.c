// Vérifie identité, persistance et priorité manuelle sans Bluetooth ni accès à un appareil.
#include <splitergoll/ble_os_memory.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

// Un bond existant sans cache ne devient pas Windows lors d’une reconnexion silencieuse.
static void legacy_and_initial(void) {
    struct ble_os_memory memory = {0};
    uint8_t identity[BLE_OS_MEMORY_ID_SIZE] = {1};
    ble_memory_bind(&memory, identity, false);
    assert(memory.verified && !memory.present && !memory.dirty);
    assert(!ble_memory_assign(&memory, BLE_MEMORY_WINDOWS, 0x3c, false, false));
    assert(memory.os == BLE_MEMORY_UNKNOWN);
    ble_memory_bind(&memory, identity, true);
    assert(memory.present && memory.dirty && memory.origin == BLE_MEMORY_INITIAL);
    ble_memory_saved(&memory, 0);
    assert(ble_memory_assign(&memory, BLE_MEMORY_WINDOWS, 0x3c, true, false));
    assert(memory.dirty && memory.os == BLE_MEMORY_WINDOWS && memory.evidence == 0x3c);
    assert(!ble_memory_assign(&memory, BLE_MEMORY_WINDOWS, 0x3c, true, false));
}

// Une erreur de NVS n’est jamais présentée comme une sauvegarde réussie.
static void persistence(void) {
    struct ble_os_memory memory = {0}, restored;
    uint8_t identity[BLE_OS_MEMORY_ID_SIZE] = {2};
    uint8_t bytes[BLE_OS_MEMORY_SIZE];
    ble_memory_bind(&memory, identity, true);
    assert(ble_memory_assign(&memory, BLE_MEMORY_MACOS, 3, true, false));
    ble_memory_saved(&memory, -ENOSPC);
    assert(memory.dirty && memory.error == -ENOSPC);
    ble_memory_saved(&memory, 0);
    assert(!memory.dirty && !memory.error);
    ble_memory_encode(&memory, bytes);
    assert(ble_memory_decode(&restored, bytes, sizeof(bytes)));
    assert(restored.os == BLE_MEMORY_MACOS && !restored.verified && !restored.dirty);
    ble_memory_bind(&restored, identity, false);
    assert(restored.verified && restored.os == BLE_MEMORY_MACOS);
    assert(!ble_memory_assign(&restored, BLE_MEMORY_WINDOWS, 0x3c, false, false));
    assert(restored.os == BLE_MEMORY_MACOS && !restored.dirty);
    // Une contradiction sur l’acquisition initiale retire le choix, sans OS par défaut.
    assert(ble_memory_assign(&restored, BLE_MEMORY_UNKNOWN, 0x3f, true, false));
    assert(restored.os == BLE_MEMORY_UNKNOWN && restored.dirty);
}

// L’adresse identique ne suffit pas si le bond ou sa génération change.
static void replaced_bonds(void) {
    uint8_t identity[BLE_OS_MEMORY_ID_SIZE] = {3};
    for (size_t changed = 0; changed < sizeof(identity); changed++) {
        struct ble_os_memory memory = {0};
        ble_memory_bind(&memory, identity, true);
        assert(ble_memory_assign(&memory, BLE_MEMORY_MACOS, 3, true, false));
        ble_memory_saved(&memory, 0);
        uint8_t replacement[BLE_OS_MEMORY_ID_SIZE];
        memcpy(replacement, identity, sizeof(replacement));
        replacement[changed] ^= 0x80;
        ble_memory_bind(&memory, replacement, false);
        assert(memory.os == BLE_MEMORY_UNKNOWN && !memory.present && !memory.dirty);
    }
    struct ble_os_memory memory = {0};
    ble_memory_bind(&memory, identity, true);
    assert(ble_memory_assign(&memory, BLE_MEMORY_WINDOWS, 0x3c, true, false));
    ble_memory_saved(&memory, 0);
    ble_memory_bind(&memory, identity, true);
    assert(memory.os == BLE_MEMORY_UNKNOWN && memory.dirty);
}

// Une correction est sauvegardable et n’est pas remplacée par l’apprentissage automatique.
static void manual_priority(void) {
    struct ble_os_memory memory = {0}, restored;
    uint8_t identity[BLE_OS_MEMORY_ID_SIZE] = {4}, bytes[BLE_OS_MEMORY_SIZE];
    assert(!ble_memory_assign(&memory, BLE_MEMORY_LINUX, 0, false, true));
    ble_memory_bind(&memory, identity, false);
    assert(ble_memory_assign(&memory, BLE_MEMORY_LINUX, 0x3f, false, true));
    assert(memory.origin == BLE_MEMORY_MANUAL && memory.evidence == 0);
    ble_memory_saved(&memory, 0);
    ble_memory_encode(&memory, bytes);
    assert(ble_memory_decode(&restored, bytes, sizeof(bytes)));
    ble_memory_bind(&restored, identity, false);
    assert(!ble_memory_assign(&restored, BLE_MEMORY_WINDOWS, 0x3c, true, false));
    assert(!ble_memory_assign(&restored, BLE_MEMORY_UNKNOWN, 0, true, false));
    assert(restored.os == BLE_MEMORY_LINUX && !restored.dirty);
    assert(ble_memory_assign(&restored, BLE_MEMORY_MACOS, 0, false, true));
    assert(!ble_memory_assign(&restored, (enum ble_memory_os)-1, 0, false, true));
    assert(!ble_memory_assign(&restored, (enum ble_memory_os)4, 0, false, true));
}

// Refuse les versions futures, formats tronqués et valeurs réservées au chargement.
static void invalid_records(void) {
    struct ble_os_memory memory = {0}, restored;
    uint8_t identity[BLE_OS_MEMORY_ID_SIZE] = {5}, bytes[BLE_OS_MEMORY_SIZE];
    ble_memory_bind(&memory, identity, true);
    ble_memory_encode(&memory, bytes);
    assert(!ble_memory_decode(&restored, NULL, sizeof(bytes)));
    for (size_t size = 0; size < sizeof(bytes); size++) assert(!ble_memory_decode(&restored, bytes, size));
    assert(!ble_memory_decode(&restored, bytes, sizeof(bytes) + 1));
    const unsigned fields[] = {0, 1, 2, 3, 6, 7};
    for (size_t i = 0; i < sizeof(fields) / sizeof(fields[0]); i++) {
        uint8_t invalid[BLE_OS_MEMORY_SIZE];
        memcpy(invalid, bytes, sizeof(invalid));
        invalid[fields[i]] = 255;
        assert(!ble_memory_decode(&restored, invalid, sizeof(invalid)));
        assert(!restored.present && !restored.verified);
    }
    bytes[3] = BLE_MEMORY_NONE;
    assert(!ble_memory_decode(&restored, bytes, sizeof(bytes)));
}

// Exécute les scénarios métier avec ASan/UBSan dans la suite native du clavier.
int main(void) {
    legacy_and_initial();
    persistence();
    replaced_bonds();
    manual_priority();
    invalid_records();
    puts("ble_os_memory: ok");
    return 0;
}
