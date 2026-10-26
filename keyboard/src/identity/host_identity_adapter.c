// Transport de diagnostic uniquement. Préserve le callback USB natif et toutes les destinations ZMK.
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/random/random.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <splitergoll/ble_auto.h>
#include <splitergoll/host_identity.h>
#include <splitergoll/usb_priority.h>
#include "common/auto-layers.h"

BUILD_ASSERT((!IS_ENABLED(CONFIG_ZMK_USB) || IS_ENABLED(CONFIG_SPLITERGOLL_USB_PRIORITY)) && IS_ENABLED(CONFIG_SPLITERGOLL_BLE_AUTO));
BUILD_ASSERT(ZMK_BLE_PROFILE_COUNT <= 8);
BUILD_ASSERT(CONFIG_ZMK_USB_INIT_PRIORITY > 90);

static struct hi_state state;
static struct k_spinlock lock;
static struct bt_conn *owners[8];
static usb_dc_status_callback native_status;
static bool configured;

// Les callbacks notifient le sélecteur après libération du spinlock, jamais de mutation HID dans un pilote.
static void identity_changed(void) {
#if IS_ENABLED(CONFIG_SPLITERGOLL_USB_PRIORITY)
    splitergoll_ble_auto_notify();
#endif
}

// L’identité n’est pas exportée. Les contextes permettent au garde natif de refuser un lien remplacé.
struct usb_priority_evidence splitergoll_identity_evidence(unsigned profile, uint32_t now) {
    struct usb_priority_evidence result = {.profile = (int)profile};
    if (profile >= ZMK_BLE_PROFILE_COUNT) return result;
    k_spinlock_key_t key = k_spin_lock(&lock);
    const struct hi_link *usb = &state.links[HI_USB], *ble = &state.links[profile + 1];
    uint8_t report[HI_SIZE];
    bool readable = hi_read(&state, HI_USB, now, report);
    result.context_valid = readable && usb->ready && ble->ready && usb->claimed && ble->claimed &&
        memcmp(usb->host, ble->host, HI_ID_SIZE) == 0 && report[7] != 3;
    result.usb_context = usb->generation;
    result.ble_context = ble->generation;
    result.matched = result.context_valid && report[7] == 1 && (report[56] & (1U << profile));
    if (result.matched) {
        uint32_t usb_age = now - usb->received, ble_age = now - ble->received;
        result.lease_ms = HI_LEASE_MS - (usb_age > ble_age ? usb_age : ble_age);
    }
    k_spin_unlock(&lock, key);
    return result;
}

// La collection vendor ne déclare aucune touche et n’envoie aucun rapport de saisie USB.
static const uint8_t descriptor[] = {
    0x06, 0x31, 0xFF, 0x0A, 0x50, 0x53, 0xA1, 0x01,
    0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, HI_SIZE,
    0x09, 0x01, 0xB1, 0x02, 0xC0
};

// Un reset, retrait ou suspend USB révoque le contexte avant le prochain transfert.
static void status_changed(enum usb_dc_status_code status, const uint8_t *param) {
    k_spinlock_key_t key = k_spin_lock(&lock);
    if (status == USB_DC_RESET || status == USB_DC_DISCONNECTED) {
        configured = false; hi_close(&state, HI_USB);
    } else if (status == USB_DC_SUSPEND) {
        hi_close(&state, HI_USB);
    } else if (status == USB_DC_CONFIGURED) {
        configured = true; hi_open(&state, HI_USB);
    } else if (status == USB_DC_RESUME && configured) {
        hi_open(&state, HI_USB);
    }
    k_spin_unlock(&lock, key);
    if (native_status) native_status(status, param);
    identity_changed();
}

int __real_usb_enable(usb_dc_status_callback callback);
// ZMK reste le seul initiateur USB, son callback reçoit chaque événement une fois.
int __wrap_usb_enable(usb_dc_status_callback callback) {
    native_status = callback;
    return __real_usb_enable(status_changed);
}

// GET_FEATURE ne retourne que le contexte actuel et les états publics, sans identifiant d’hôte.
static int get_report(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data) {
    ARG_UNUSED(dev);
    static uint8_t response[HI_SIZE];
    if (setup->wValue != 0x0300 || setup->wLength != HI_SIZE) return -EINVAL;
    k_spinlock_key_t key = k_spin_lock(&lock);
    bool ok = hi_read(&state, HI_USB, k_uptime_get_32(), response);
    k_spin_unlock(&lock, key);
    if (!ok) return -EACCES;
    *data = response; *len = HI_SIZE;
    return 0;
}

// SET_FEATURE accepte uniquement une annonce complète pour ce contexte USB.
static int set_report(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data) {
    ARG_UNUSED(dev);
    if (setup->wValue != 0x0300 || setup->wLength != HI_SIZE || *len != HI_SIZE) return -EINVAL;
    k_spinlock_key_t key = k_spin_lock(&lock);
    bool ok = hi_write(&state, HI_USB, k_uptime_get_32(), *data, (size_t)*len);
    k_spin_unlock(&lock, key);
    if (ok) identity_changed();
    return ok ? 0 : -EINVAL;
}

static const struct hid_ops ops = {.get_report = get_report, .set_report = set_report};

struct bond_lookup { const bt_addr_le_t *address; bool found; };
// Recherche le bond avec l’API publique du Zephyr figé, sans lire de clé privée.
static void find_bond(const struct bt_bond_info *info, void *user_data) {
    struct bond_lookup *lookup = user_data;
    if (bt_addr_le_cmp(&info->addr, lookup->address) == 0) lookup->found = true;
}

// Seul un hôte déjà connu, chiffré et connecté en rôle périphérique peut utiliser le service.
static int host_link(struct bt_conn *conn) {
    struct bt_conn_info info;
    if (bt_conn_get_info(conn, &info) || info.type != BT_CONN_TYPE_LE || info.role != BT_CONN_ROLE_PERIPHERAL || bt_conn_get_security(conn) < BT_SECURITY_L2) return -1;
    int slot = zmk_ble_profile_index(bt_conn_get_dst(conn));
    struct bond_lookup lookup = {.address = bt_conn_get_dst(conn)};
    bt_foreach_bond(BT_ID_DEFAULT, find_bond, &lookup);
    if (slot < 0 || slot >= ZMK_BLE_PROFILE_COUNT || !lookup.found) return -1;
    return slot + 1;
}

// La liaison propriétaire est renouvelée à la première lecture sécurisée, sans adresse exportée.
static ssize_t read_identity(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) {
    int link = host_link(conn);
    if (link < 0) return BT_GATT_ERR(BT_ATT_ERR_AUTHORIZATION);
    uint8_t response[HI_SIZE];
    k_spinlock_key_t key = k_spin_lock(&lock);
    if (owners[link - 1] != conn) { hi_close(&state, (unsigned)link); owners[link - 1] = conn; }
    bool ok = hi_open(&state, (unsigned)link) && hi_read(&state, (unsigned)link, k_uptime_get_32(), response);
    k_spin_unlock(&lock, key);
    return ok ? bt_gatt_attr_read(conn, attr, buf, len, offset, response, HI_SIZE) : BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
}

// Zephyr assemble les longs writes. Les fragments préparatoires ne modifient jamais le modèle.
static ssize_t write_identity(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags) {
    ARG_UNUSED(attr);
    int link = host_link(conn);
    if (link < 0) return BT_GATT_ERR(BT_ATT_ERR_AUTHORIZATION);
    if (offset > HI_SIZE || len > HI_SIZE - offset) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    if (flags & BT_GATT_WRITE_FLAG_PREPARE) return 0;
    if (offset || len != HI_SIZE || (flags & BT_GATT_WRITE_FLAG_CMD)) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    k_spinlock_key_t key = k_spin_lock(&lock);
    bool ok = owners[link - 1] == conn && hi_write(&state, (unsigned)link, k_uptime_get_32(), buf, len);
    k_spin_unlock(&lock, key);
    if (ok) identity_changed();
    return ok ? len : BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
}

// Lit les couches réellement appliquées et la sortie ZMK, sans sélectionner d’OS ou d’appareil.
static ssize_t read_runtime(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) {
    if (host_link(conn) < 0) return BT_GATT_ERR(BT_ATT_ERR_AUTHORIZATION);
    uint8_t response[32] = {0};
    memcpy(response, "SGST", 4); response[4] = IS_ENABLED(CONFIG_SPLITERGOLL_USB_PRIORITY) ? 2 : 1;
    response[24] = IS_ENABLED(CONFIG_SPLITERGOLL_USB_PRIORITY) ? 1 : 0;
    memcpy(response + 8, state.boot, HI_ID_SIZE);
    splitergoll_ble_auto_lock();
    bool macos = zmk_keymap_layer_active(AUTO_MACOS_BASE);
    bool windows = zmk_keymap_layer_active(AUTO_WINDOWS_BASE);
    response[5] = macos != windows ? (macos ? 1 : 2) : 0;
    struct zmk_endpoint_instance endpoint = zmk_endpoint_get_selected();
    response[6] = endpoint.transport == ZMK_TRANSPORT_USB ? 1 : endpoint.transport == ZMK_TRANSPORT_BLE ? 2 : 0;
    int profile = zmk_ble_active_profile_index();
    response[7] = profile >= 0 && profile < ZMK_BLE_PROFILE_COUNT ? (uint8_t)profile : 255;
    splitergoll_ble_auto_unlock();
    return bt_gatt_attr_read(conn, attr, buf, len, offset, response, sizeof(response));
}

#define ID_UUID(last) BT_UUID_128_ENCODE(0x7d2f6767, 0x4c2b, 0x4d13, 0xa528, 0x80b87ac03800ULL + (last))
BT_GATT_SERVICE_DEFINE(identity_service,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_DECLARE_128(ID_UUID(0x10))),
    BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_128(ID_UUID(0x11)), BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT | BT_GATT_PERM_PREPARE_WRITE,
        read_identity, write_identity, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_128(ID_UUID(0x12)), BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ_ENCRYPT, read_runtime, NULL, NULL));

// Une déconnexion invalide l’annonce sans toucher au bond, au slot sélectionné ou à sa keymap.
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    ARG_UNUSED(reason);
    k_spinlock_key_t key = k_spin_lock(&lock);
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++)
        if (owners[i] == conn) { owners[i] = NULL; hi_close(&state, i + 1); }
    k_spin_unlock(&lock, key);
    identity_changed();
}

// Un objet bt_conn réutilisé ne doit jamais hériter du contexte d’une connexion précédente.
static void connected(struct bt_conn *conn, uint8_t error) {
    ARG_UNUSED(error);
    disconnected(conn, 0);
}

// Une sécurité perdue révoque les déclarations existantes, pas seulement les écritures futures.
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err error) {
    if (error || level < BT_SECURITY_L2) disconnected(conn, 0);
}
BT_CONN_CB_DEFINE(identity_callbacks) = {.connected = connected, .disconnected = disconnected, .security_changed = security_changed};

// Prépare le contexte avant que ZMK active USB. Un échec n’empêche pas la saisie BLE native.
static int identity_init(void) {
    uint8_t boot[HI_ID_SIZE];
    if (sys_csrand_get(boot, sizeof(boot))) return -EIO;
    hi_init(&state, boot);
    const struct device *dev = device_get_binding(IS_ENABLED(CONFIG_SPLITERGOLL_USB_PRIORITY) ? "HID_1" : "HID_0");
    if (!dev || !device_is_ready(dev)) return -ENODEV;
    usb_hid_register_device(dev, descriptor, sizeof(descriptor), &ops);
    return usb_hid_init(dev);
}
SYS_INIT(identity_init, APPLICATION, 90);
