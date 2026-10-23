// Doubles minimaux pour exercer le vrai dump C sans USB ni radio.
#ifndef BLE_OBSERVER_STUB_H
#define BLE_OBSERVER_STUB_H
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define ARG_UNUSED(value) (void)(value)
#define APPLICATION 0
#define CONFIG_BT_MAX_CONN 6
#define CONFIG_BT_REMOTE_INFO 1
#define BUILD_ASSERT(c) _Static_assert(c, #c)
#ifdef __weak
#undef __weak
#endif
#define __weak __attribute__((weak))
#define SYS_INIT(function, ...) int (*test_observer_init)(void) = function
#define BT_CONN_TYPE_LE 1
#define BT_CONN_ROLE_PERIPHERAL 1
#define BT_UUID_TYPE_16 0
#define BT_UUID_TYPE_128 2
#define BT_UUID_STR_LEN 37
#define BT_GATT_ITER_STOP 0
#define BT_GATT_ITER_CONTINUE 1
#define BT_CONN_CB_DEFINE(name) struct bt_conn_cb name
struct device {
    int unused;
};
struct k_spinlock {
    int unused;
};
typedef int k_spinlock_key_t;
extern unsigned lock_depth;
static inline k_spinlock_key_t k_spin_lock(struct k_spinlock *lock) {
    (void)lock;
    lock_depth++;
    return 0;
}
static inline void k_spin_unlock(struct k_spinlock *lock, k_spinlock_key_t key) {
    (void)lock;
    (void)key;
    assert(lock_depth);
    lock_depth--;
}
uint32_t k_uptime_get_32(void);
struct bt_conn {
    unsigned index;
    int role, type, profile, error;
};
struct peer {
    uint8_t type;
};
struct bt_conn_info {
    int type, role;
    struct {
        uint16_t interval, latency, timeout;
        struct peer *dst;
    } le;
};
struct bt_le_conn_param {
    uint16_t interval_min, interval_max, latency, timeout;
};
typedef unsigned bt_security_t;
enum bt_security_err { SECURITY_SUCCESS = 0 };
struct bt_conn_remote_info {
    int type;
    struct {
        const uint8_t *features;
    } le;
};
struct bt_conn_cb {
    void (*connected)(struct bt_conn *, uint8_t);
    void (*disconnected)(struct bt_conn *, uint8_t);
    bool (*le_param_req)(struct bt_conn *, struct bt_le_conn_param *);
    void (*le_param_updated)(struct bt_conn *, uint16_t, uint16_t, uint16_t);
    void (*security_changed)(struct bt_conn *, bt_security_t, enum bt_security_err);
    void (*remote_info_available)(struct bt_conn *, struct bt_conn_remote_info *);
};
int bt_conn_get_info(struct bt_conn *, struct bt_conn_info *);
unsigned bt_conn_index(struct bt_conn *);
const struct peer *bt_conn_get_dst(struct bt_conn *);
int zmk_ble_profile_index(const struct peer *);
typedef int atomic_t;
static inline int atomic_get(atomic_t *value) {
    return *value;
}
static inline void atomic_inc(atomic_t *value) {
    ++*value;
}
struct bt_uuid {
    uint8_t type;
};
struct bt_uuid_16 {
    struct bt_uuid uuid;
    uint16_t val;
};
#define BT_UUID_16(value) ((const struct bt_uuid_16 *)(value))
struct bt_gatt_attr {
    const struct bt_uuid *uuid;
};
struct bt_gatt_cb {
    void (*att_mtu_updated)(struct bt_conn *, uint16_t, uint16_t);
};
void bt_gatt_cb_register(struct bt_gatt_cb *);
void bt_uuid_to_str(const struct bt_uuid *, char *, size_t);
void bt_gatt_foreach_attr(uint16_t, uint16_t,
                          uint8_t (*callback)(const struct bt_gatt_attr *, uint16_t, void *),
                          void *);
struct net_buf {
    uint8_t *data;
    size_t len;
};
static inline uint32_t sys_get_le32(const uint8_t *p) {
    return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
#endif
