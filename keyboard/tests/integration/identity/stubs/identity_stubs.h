// Frontières publiques USB/BLE simulées, aucune clé ou radio réelle.
#pragma once
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#define CONFIG_ZMK_USB 0
#define CONFIG_SPLITERGOLL_BLE_AUTO 1
#define CONFIG_ZMK_USB_INIT_PRIORITY 96
#define ZMK_BLE_PROFILE_COUNT 5
#define IS_ENABLED(x) (x)
#define BUILD_ASSERT(x) _Static_assert(x, #x)
#define ARG_UNUSED(x) (void)(x)
#define SYS_INIT(fn, level, priority)
struct device { int unused; };
struct k_spinlock { int unused; };
typedef int k_spinlock_key_t;
static uint32_t now_ms;
static inline uint32_t k_uptime_get_32(void) { return now_ms; }
static inline k_spinlock_key_t k_spin_lock(struct k_spinlock *lock) { (void)lock; return 0; }
static inline void k_spin_unlock(struct k_spinlock *lock, int key) { (void)lock; (void)key; }
static inline int sys_csrand_get(void *data, size_t len) { memset(data, 1, len); return 0; }
static struct device hid_device;
static inline const struct device *device_get_binding(const char *name) { return strcmp(name, "HID_0") == 0 ? &hid_device : NULL; }
static inline bool device_is_ready(const struct device *dev) { return dev != NULL; }
enum usb_dc_status_code { USB_DC_UNKNOWN, USB_DC_CONFIGURED, USB_DC_RESET, USB_DC_DISCONNECTED, USB_DC_SUSPEND, USB_DC_RESUME };
typedef void (*usb_dc_status_callback)(enum usb_dc_status_code, const uint8_t *);
struct usb_setup_packet { uint16_t wValue, wLength; };
struct hid_ops {
    int (*get_report)(const struct device *, struct usb_setup_packet *, int32_t *, uint8_t **);
    int (*set_report)(const struct device *, struct usb_setup_packet *, int32_t *, uint8_t **);
};
static const struct hid_ops *registered_ops;
static inline void usb_hid_register_device(const struct device *dev, const uint8_t *desc, size_t len, const struct hid_ops *ops) {
    (void)dev; assert(len > 0 && desc[len - 1] == 0xC0); registered_ops = ops;
}
static inline int usb_hid_init(const struct device *dev) { return dev ? 0 : -1; }
typedef struct { uint8_t value; } bt_addr_le_t;
struct bt_conn_info { int type, role; };
struct bt_conn { struct bt_conn_info info; int security; bt_addr_le_t address; };
typedef int bt_security_t;
enum bt_security_err { BT_SECURITY_ERR_SUCCESS = 0, BT_SECURITY_ERR_AUTH_FAIL = 1 };
#define BT_CONN_TYPE_LE 1
#define BT_CONN_ROLE_PERIPHERAL 1
#define BT_SECURITY_L2 2
#define BT_ID_DEFAULT 0
static inline int bt_conn_get_info(struct bt_conn *conn, struct bt_conn_info *out) { *out = conn->info; return 0; }
static inline int bt_conn_get_security(struct bt_conn *conn) { return conn->security; }
static inline const bt_addr_le_t *bt_conn_get_dst(struct bt_conn *conn) { return &conn->address; }
static inline int zmk_ble_profile_index(const bt_addr_le_t *addr) { return addr->value < 5 ? addr->value : -1; }
struct bt_bond_info { bt_addr_le_t addr; };
static uint8_t bonds = 1;
static inline int bt_addr_le_cmp(const bt_addr_le_t *a, const bt_addr_le_t *b) { return a->value - b->value; }
static inline void bt_foreach_bond(uint8_t id, void (*fn)(const struct bt_bond_info *, void *), void *context) {
    (void)id;
    for (uint8_t i = 0; i < 5; i++) if (bonds & (1U << i)) { struct bt_bond_info info = {.addr = {i}}; fn(&info, context); }
}
struct bt_gatt_attr { int unused; };
#define BT_ATT_ERR_AUTHORIZATION 8
#define BT_ATT_ERR_UNLIKELY 14
#define BT_ATT_ERR_INVALID_ATTRIBUTE_LEN 13
#define BT_ATT_ERR_VALUE_NOT_ALLOWED 19
#define BT_GATT_WRITE_FLAG_PREPARE 1
#define BT_GATT_WRITE_FLAG_CMD 2
#define BT_GATT_ERR(x) (-(x))
#define BT_GATT_SERVICE_DEFINE(name, ...) static const int name __attribute__((unused)) = 0
static inline ssize_t bt_gatt_attr_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset, const void *data, uint16_t size) {
    (void)conn; (void)attr;
    if (offset > size) return -1;
    size_t count = size - offset; if (count > len) count = len;
    memcpy(buf, (const uint8_t *)data + offset, count); return (ssize_t)count;
}
struct bt_conn_cb {
    void (*connected)(struct bt_conn *, uint8_t);
    void (*disconnected)(struct bt_conn *, uint8_t);
    void (*security_changed)(struct bt_conn *, bt_security_t, enum bt_security_err);
};
#define BT_CONN_CB_DEFINE(name) struct bt_conn_cb name
