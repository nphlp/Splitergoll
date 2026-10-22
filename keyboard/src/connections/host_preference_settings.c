// Sauvegarde à l’échéance une empreinte opaque, sans adresse, clé ou historique de frappe.
#include <splitergoll/host_preference.h>
#include <splitergoll/host_preference_settings.h>
#include <splitergoll/ble_os_settings.h>
#include <splitergoll/ble_acquisition.h>
#include <errno.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/atomic.h>
#include <zmk/ble.h>

LOG_MODULE_REGISTER(splitergoll_host_preference, CONFIG_ZMK_LOG_LEVEL);
BUILD_ASSERT(HOST_PREFERENCE_ID_SIZE == BLE_OS_MEMORY_ID_SIZE);
static struct host_preference preference;
static atomic_t loaded;

// Une erreur de format invalide seulement notre préférence, jamais les réglages ZMK.
static int restore(const char *name, size_t length, settings_read_cb read, void *context) {
    if (name == NULL || strcmp(name, "value") != 0) return -ENOENT;
    if (!length) {
        preference = (struct host_preference){0};
        return 0;
    }
    uint8_t bytes[HOST_PREFERENCE_RECORD_SIZE];
    if (length != sizeof(bytes) || read(context, bytes, sizeof(bytes)) != sizeof(bytes) ||
        !host_preference_decode(&preference, bytes, sizeof(bytes))) {
        preference = (struct host_preference){.error = -EINVAL};
    }
    return 0;
}

// Le worker ne sauvegarde rien avant la fin du chargement des réglages.
static int committed(void) {
    atomic_set(&loaded, 1);
    splitergoll_ble_auto_notify();
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(host_preference_settings, "splitergoll/host-choice",
                               NULL, restore, committed, NULL);

// Ne consulte que les empreintes internes déjà contrôlées, pas les clés Bluetooth.
struct host_preference_status splitergoll_host_preference_status(void) {
    struct host_preference_status status = {.profile = -1, .loaded = atomic_get(&loaded),
        .present = false};
    if (!status.loaded) return status;
    status.present = preference.present;
    status.error = preference.error;
    if (!preference.present) return status;
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        uint8_t identity[HOST_PREFERENCE_ID_SIZE];
        if (!zmk_ble_profile_is_open(i) && ble_os_settings_identity(i, identity) &&
            memcmp(identity, preference.identity, sizeof(identity)) == 0) {
            status.profile = (int)i;
            break;
        }
    }
    return status;
}

// Déduplique les écritures et garde l’ancien choix en RAM si NVS refuse le nouveau.
void splitergoll_host_preference_save(unsigned profile) {
    uint8_t identity[HOST_PREFERENCE_ID_SIZE], bytes[HOST_PREFERENCE_RECORD_SIZE];
    if (!atomic_get(&loaded) || !ble_os_settings_identity(profile, identity)) return;
    if (!host_preference_prepare(&preference, identity, bytes)) return;
    int error = settings_save_one("splitergoll/host-choice/value", bytes, sizeof(bytes));
    host_preference_saved(&preference, bytes, error);
    if (error) LOG_WRN("BLE destination save failed: error=%d", error);
    else LOG_INF("BLE destination saved: profile=%u", profile);
}
