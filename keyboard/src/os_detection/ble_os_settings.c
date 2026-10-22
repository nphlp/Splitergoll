// Persiste seulement l’OS et une empreinte opaque du bond, jamais une copie de ses clés.
#include <splitergoll/ble_os_settings.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <mbedtls/sha256.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/atomic.h>
#include <zmk/ble.h>

// Le blob NVS reste stable aux reconnexions dans cette configuration Zephyr figée.
// Réexaminer cette identité avant d’activer signature, vieillissement ou une autre pile BLE.
BUILD_ASSERT(!IS_ENABLED(CONFIG_BT_SIGNING));
BUILD_ASSERT(!IS_ENABLED(CONFIG_BT_KEYS_OVERWRITE_OLDEST));
BUILD_ASSERT(IS_ENABLED(CONFIG_BT_SMP_SC_PAIR_ONLY));

#define BOND_RECORD_LIMIT 128
static struct ble_os_memory memories[ZMK_BLE_PROFILE_COUNT];
static atomic_t ready;
K_MUTEX_DEFINE(memory_mutex);
K_SEM_DEFINE(memory_ready, 0, 1);

// Nomme uniquement notre espace, sans toucher aux réglages natifs ble/ ou bt/.
static void memory_key(unsigned profile, char key[32]) {
    snprintf(key, 32, "splitergoll/ble-os/%u", profile);
}

// Refuse les formats inconnus tout en laissant ZMK charger ses propres réglages.
static int restore(const char *name, size_t length, settings_read_cb read, void *context) {
    if (name == NULL || name[0] < '0' || name[0] > '9' || name[1] != '\0') return -ENOENT;
    unsigned profile = name[0] - '0';
    if (profile >= ZMK_BLE_PROFILE_COUNT) return -ENOENT;
    uint8_t bytes[BLE_OS_MEMORY_SIZE];
    if (length == 0) {
        memories[profile] = (struct ble_os_memory){0};
    } else if (length != sizeof(bytes) || read(context, bytes, sizeof(bytes)) != sizeof(bytes) ||
               !ble_memory_decode(&memories[profile], bytes, sizeof(bytes))) {
        memories[profile] = (struct ble_os_memory){.error = -EINVAL};
    }
    return 0;
}

// Autorise le worker seulement après le chargement des réglages persistants.
static int loaded(void) {
    atomic_set(&ready, 1);
    k_sem_give(&memory_ready);
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(ble_os_settings, "splitergoll/ble-os", NULL, restore, loaded, NULL);

// La file d’acquisition attend ici, sans lire ni modifier les tableaux pendant leur restauration.
void ble_os_settings_wait_ready(void) {
    k_sem_take(&memory_ready, K_FOREVER);
    k_sem_give(&memory_ready);
}

struct bond_digest {
    uint8_t bytes[7 + BOND_RECORD_LIMIT];
    size_t size;
    int error;
};

// Lit le blob par l’API publique de settings, uniquement pour le hacher localement.
static int read_bond(const char *name, size_t length, settings_read_cb read, void *context,
                      void *parameter) {
    struct bond_digest *digest = parameter;
    if (name != NULL && name[0] != '\0') return 0;
    if (length == 0 || length > BOND_RECORD_LIMIT || digest->size != 0) {
        digest->error = -EINVAL;
        return 0;
    }
    ssize_t count = read(context, digest->bytes + 7, length);
    if (count < 0 || (size_t)count != length) digest->error = -EIO;
    else digest->size = 7 + length;
    return 0;
}

// Lie l’identité à l’adresse stable ET au bond stocké, même après un autre firmware.
static int fingerprint(unsigned profile, uint8_t identity[BLE_OS_MEMORY_ID_SIZE]) {
    const bt_addr_le_t *peer = zmk_ble_profile_address(profile);
    if (!atomic_get(&ready) || peer == NULL || bt_addr_le_eq(peer, BT_ADDR_LE_ANY) || peer->type > 1) return -ENOENT;
    char key[32];
    const uint8_t *address = peer->a.val;
    snprintf(key, sizeof(key), "bt/keys/%02x%02x%02x%02x%02x%02x%u",
             address[5], address[4], address[3], address[2], address[1], address[0], peer->type);
    struct bond_digest digest = {.error = 0};
    digest.bytes[0] = peer->type;
    memcpy(digest.bytes + 1, address, 6);
    int error = settings_load_subtree_direct(key, read_bond, &digest);
    if (!error) error = digest.error;
    if (!error && digest.size == 0) error = -ENOENT;
    if (!error && mbedtls_sha256(digest.bytes, digest.size, identity, 0) != 0) error = -EIO;
    // Efface les octets temporaires des clés, sans les journaliser ni les exposer au lecteur.
    volatile uint8_t *temporary = digest.bytes;
    for (size_t i = 0; i < sizeof(digest.bytes); i++) temporary[i] = 0;
    return error;
}

// Écrit un petit enregistrement versionné et conserve l’erreur si NVS refuse la sauvegarde.
static void save(unsigned profile) {
    char key[32];
    uint8_t bytes[BLE_OS_MEMORY_SIZE];
    memory_key(profile, key);
    ble_memory_encode(&memories[profile], bytes);
    ble_memory_saved(&memories[profile], settings_save_one(key, bytes, sizeof(bytes)));
}

// Un ancien appairage sans cache reste inconnu et n’est pas appris lors de sa reconnexion.
bool ble_os_settings_bind(unsigned profile, bool new_pairing) {
    if (profile >= ZMK_BLE_PROFILE_COUNT || !atomic_get(&ready)) return false;
    k_mutex_lock(&memory_mutex, K_FOREVER);
    uint8_t identity[BLE_OS_MEMORY_ID_SIZE];
    int error = fingerprint(profile, identity);
    if (error) {
        if (new_pairing) memories[profile] = (struct ble_os_memory){0};
        memories[profile].verified = false;
        memories[profile].error = error;
    } else {
        ble_memory_bind(&memories[profile], identity, new_pairing);
        if (new_pairing) save(profile);
    }
    k_mutex_unlock(&memory_mutex);
    return error == 0;
}

// Seul le worker d’acquisition initiale appelle cette entrée, jamais une lecture USB.
void ble_os_settings_learn(unsigned profile, enum ble_memory_os os, uint16_t evidence) {
    if (profile >= ZMK_BLE_PROFILE_COUNT) return;
    k_mutex_lock(&memory_mutex, K_FOREVER);
    if (ble_memory_assign(&memories[profile], os, evidence, true, false)) save(profile);
    k_mutex_unlock(&memory_mutex);
}

// Supprime notre cache après un effacement natif, sans appeler bt_unpair ni settings_reset.
void ble_os_settings_forget(unsigned profile) {
    if (profile >= ZMK_BLE_PROFILE_COUNT) return;
    k_mutex_lock(&memory_mutex, K_FOREVER);
    memories[profile] = (struct ble_os_memory){0};
    char key[32];
    memory_key(profile, key);
    memories[profile].error = settings_delete(key);
    k_mutex_unlock(&memory_mutex);
}

// Fournit un instantané dont le formatteur ne doit exporter que les champs publics.
struct ble_os_memory ble_os_settings_snapshot(unsigned profile) {
    struct ble_os_memory snapshot = {0};
    if (profile >= ZMK_BLE_PROFILE_COUNT || !atomic_get(&ready)) return snapshot;
    k_mutex_lock(&memory_mutex, K_FOREVER);
    snapshot = memories[profile];
    k_mutex_unlock(&memory_mutex);
    memset(snapshot.identity, 0, sizeof(snapshot.identity));
    return snapshot;
}

// Une association remplacée ou non vérifiée ne peut restaurer une préférence de destination.
bool ble_os_settings_identity(unsigned profile, uint8_t identity[BLE_OS_MEMORY_ID_SIZE]) {
    if (profile >= ZMK_BLE_PROFILE_COUNT || !atomic_get(&ready)) return false;
    k_mutex_lock(&memory_mutex, K_FOREVER);
    const struct ble_os_memory *memory = &memories[profile];
    bool valid = memory->present && memory->verified && !memory->dirty && !memory->error;
    if (valid) memcpy(identity, memory->identity, BLE_OS_MEMORY_ID_SIZE);
    k_mutex_unlock(&memory_mutex);
    return valid;
}
