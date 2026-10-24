// Retire les annonces hôtes de la centrale wire, sans arrêter le scanner du split.
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/sys/util.h>

BUILD_ASSERT(IS_ENABLED(CONFIG_ZMK_USB) && IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL));
BUILD_ASSERT(IS_ENABLED(CONFIG_ZMK_SPLIT_BLE));
BUILD_ASSERT(!IS_ENABLED(CONFIG_BT_EXT_ADV) && !IS_ENABLED(CONFIG_LTO));
BUILD_ASSERT(!IS_ENABLED(CONFIG_SPLITERGOLL_BLE_RECONNECT));

// ZMK peut réessayer après restauration des bonds ou changement de profil : ne jamais annoncer.
int __wrap_bt_le_adv_start(const struct bt_le_adv_param *parameters, const struct bt_data *data,
                           size_t data_count, const struct bt_data *scan_response,
                           size_t scan_response_count) {
    ARG_UNUSED(parameters);
    ARG_UNUSED(data);
    ARG_UNUSED(data_count);
    ARG_UNUSED(scan_response);
    ARG_UNUSED(scan_response_count);
    return 0;
}

// Aucun arrêt radio réel : les annonces n’ont pas démarré et le split reste indépendant.
int __wrap_bt_le_adv_stop(void) {
    return 0;
}

// Une migration du socle ne doit pas changer silencieusement la signature des API interceptées.
BUILD_ASSERT(__builtin_types_compatible_p(__typeof__(&bt_le_adv_start),
                                          __typeof__(&__wrap_bt_le_adv_start)));
BUILD_ASSERT(__builtin_types_compatible_p(__typeof__(&bt_le_adv_stop),
                                          __typeof__(&__wrap_bt_le_adv_stop)));
