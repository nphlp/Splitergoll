// Politique pure de mémoire BLE. Le backend Zephyr reste séparé de cette logique.
#include <splitergoll/ble_os_memory.h>
#include <string.h>

// Refuse versions, réserves, OS ou origines inconnus plutôt que réinterpréter un ancien format.
bool ble_memory_decode(struct ble_os_memory *memory, const uint8_t *bytes, size_t size) {
    memset(memory, 0, sizeof(*memory));
    if (bytes == NULL || size != BLE_OS_MEMORY_SIZE || bytes[0] != 1 ||
        bytes[1] != BLE_OS_MEMORY_RULES || bytes[2] > BLE_MEMORY_LINUX ||
        bytes[3] < BLE_MEMORY_INITIAL || bytes[3] > BLE_MEMORY_MANUAL ||
        bytes[6] != 0 || bytes[7] != 0) return false;
    memory->os = bytes[2];
    memory->origin = bytes[3];
    memory->evidence = (uint16_t)bytes[4] | (uint16_t)bytes[5] << 8;
    memcpy(memory->identity, bytes + 8, BLE_OS_MEMORY_ID_SIZE);
    memory->present = true;
    return true;
}

// La signature opaque reste interne au clavier, le protocole USB ne la restitue jamais.
void ble_memory_encode(const struct ble_os_memory *memory, uint8_t bytes[BLE_OS_MEMORY_SIZE]) {
    memset(bytes, 0, BLE_OS_MEMORY_SIZE);
    bytes[0] = 1;
    bytes[1] = BLE_OS_MEMORY_RULES;
    bytes[2] = (uint8_t)memory->os;
    bytes[3] = (uint8_t)memory->origin;
    bytes[4] = (uint8_t)memory->evidence;
    bytes[5] = (uint8_t)(memory->evidence >> 8);
    memcpy(bytes + 8, memory->identity, BLE_OS_MEMORY_ID_SIZE);
}

// Un slot réutilisé, même avec la même adresse, ne reprend pas l’OS de l’ancien bond.
void ble_memory_bind(struct ble_os_memory *memory, const uint8_t identity[BLE_OS_MEMORY_ID_SIZE],
                     bool new_pairing) {
    if (new_pairing || !memory->present || memcmp(memory->identity, identity, BLE_OS_MEMORY_ID_SIZE)) {
        *memory = (struct ble_os_memory){.present = new_pairing, .dirty = new_pairing,
                                         .origin = new_pairing ? BLE_MEMORY_INITIAL : BLE_MEMORY_NONE};
        memcpy(memory->identity, identity, BLE_OS_MEMORY_ID_SIZE);
    }
    memory->verified = true;
}

// Une correction explicite est prioritaire, y compris si le diagnostic courant se contredit.
bool ble_memory_assign(struct ble_os_memory *memory, enum ble_memory_os os, uint16_t evidence,
                       bool initial_connection, bool manual) {
    if (!memory->verified || (unsigned)os > BLE_MEMORY_LINUX ||
        (!manual && (!initial_connection || memory->origin == BLE_MEMORY_MANUAL))) return false;
    enum ble_memory_origin origin = manual ? BLE_MEMORY_MANUAL : BLE_MEMORY_INITIAL;
    if (memory->present && memory->os == os && memory->origin == origin) return false;
    memory->present = true;
    memory->os = os;
    memory->origin = origin;
    memory->evidence = manual ? 0 : evidence;
    memory->dirty = true;
    return true;
}

// Une erreur conserve l’état sale et reste visible, sans boucle de réessai automatique.
void ble_memory_saved(struct ble_os_memory *memory, int error) {
    memory->error = error;
    if (!error) memory->dirty = false;
}

// Les valeurs inconnues ne deviennent jamais Windows par défaut.
const char *ble_memory_os_name(enum ble_memory_os os) {
    switch (os) {
        case BLE_MEMORY_MACOS: return "macos";
        case BLE_MEMORY_WINDOWS: return "windows";
        case BLE_MEMORY_LINUX: return "linux";
        default: return "unknown";
    }
}
