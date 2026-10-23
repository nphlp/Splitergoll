
#ifndef RECONNECT_STUB_H
#define RECONNECT_STUB_H
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define CONTAINER_OF(p, t, m) ((t *)((char *)(p) - offsetof(t, m)))
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 0
#define CONFIG_BT_SMP_ALLOW_UNAUTH_OVERWRITE 0
#define CONFIG_BT_ID_ALLOW_UNAUTH_OVERWRITE 0
#define CONFIG_ZMK_LOG_LEVEL 0
#define CONFIG_BT_EXT_ADV 0
#define CONFIG_LTO 0
#define CONFIG_BT_FILTER_ACCEPT_LIST 1
#define CONFIG_BT_SMP_APP_PAIRING_ACCEPT 1
#define CONFIG_BT_CTLR_FAL_SIZE 8
#define CONFIG_BT_CTLR_RL_SIZE 8
#define CONFIG_BT_MAX_PAIRED 6
#define ZMK_BLE_PROFILE_COUNT 5
#define IS_ENABLED(v) (v)
#define BUILD_ASSERT(v) _Static_assert(v, #v)
#define ARG_UNUSED(v) (void)(v)
#define LOG_MODULE_REGISTER(...)
#define LOG_INF(...)                                                                               \
    do {                                                                                           \
        printf(__VA_ARGS__);                                                                       \
        putchar(10);                                                                               \
    } while (0)
#define LOG_WRN(...)                                                                               \
    do {                                                                                           \
        printf(__VA_ARGS__);                                                                       \
        putchar(10);                                                                               \
    } while (0)
#define LOG_ERR(...)                                                                               \
    do {                                                                                           \
        printf(__VA_ARGS__);                                                                       \
        putchar(10);                                                                               \
    } while (0)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define K_NO_WAIT 0
#define K_MSEC(n) (n)
static uint32_t clock_ms;
static unsigned spin_depth, auto_depth;
static inline uint32_t k_uptime_get_32(void) {
    return clock_ms;
}
struct k_spinlock {
    int unused;
};
typedef int k_spinlock_key_t;
static inline int k_spin_lock(struct k_spinlock *s) {
    (void)s;
    spin_depth++;
    return 0;
}
static inline void k_spin_unlock(struct k_spinlock *s, int k) {
    (void)s;
    (void)k;
    assert(spin_depth);
    spin_depth--;
}
struct k_work {
    int unused;
};
struct k_work_delayable {
    struct k_work work;
    void (*handler)(struct k_work *);
    bool queued;
    uint32_t due;
};
#define K_WORK_DELAYABLE_DEFINE(n, f) struct k_work_delayable n = {.handler = f}
static inline int k_work_reschedule(struct k_work_delayable *w, uint32_t d) {
    w->queued = true;
    w->due = clock_ms + d;
    return 0;
}
static inline int k_work_schedule(struct k_work_delayable *w, uint32_t d) {
    if (!w->queued)
        k_work_reschedule(w, d);
    return 0;
}
static inline struct k_work_delayable *k_work_delayable_from_work(struct k_work *w) {
    return (struct k_work_delayable *)w;
}
typedef struct {
    uint8_t value;
} bt_addr_le_t;
static inline bool bt_addr_le_eq(const bt_addr_le_t *a, const bt_addr_le_t *b) {
    return a->value == b->value;
}
static inline int bt_addr_le_cmp(const bt_addr_le_t *a, const bt_addr_le_t *b) {
    return (a->value > b->value) - (a->value < b->value);
}
static inline void bt_addr_le_copy(bt_addr_le_t *a, const bt_addr_le_t *b) {
    *a = *b;
}
#define BT_ID_DEFAULT 0
#define BT_LE_ADV_OPT_CONN 3u
#define BT_LE_ADV_OPT_FILTER_CONN 128u
#define BT_LE_ADV_OPT_FILTER_SCAN_REQ 64u
#define BT_CONN_TYPE_LE 0
#define BT_CONN_ROLE_PERIPHERAL 1
#define BT_CONN_ROLE_CENTRAL 0
#define BT_HCI_ERR_REMOTE_USER_TERM_CONN 19
struct bt_le_adv_param {
    uint8_t id;
    uint32_t options;
    const bt_addr_le_t *peer;
};
struct bt_data {
    uint8_t type, data_len;
    const uint8_t *data;
};
struct bt_bond_info {
    bt_addr_le_t addr;
};
struct bt_conn {
    bt_addr_le_t addr;
    uint8_t role;
    unsigned index, refs;
};
struct zmk_ble_profile {
    char name[15];
    bt_addr_le_t peer;
};
struct bt_conn_info {
    uint8_t type, role;
};
struct bt_conn_pairing_feat {
    int unused;
};
enum bt_security_err {
    BT_SECURITY_ERR_SUCCESS = 0,
    BT_SECURITY_ERR_PAIR_NOT_ALLOWED = 8,
    BT_SECURITY_ERR_UNSPECIFIED = 9
};
struct bt_conn_auth_cb {
    enum bt_security_err (*pairing_accept)(struct bt_conn *,
                                           const struct bt_conn_pairing_feat *const);
    void (*cancel)(struct bt_conn *);
};
struct bt_conn_auth_info_cb {
    void (*pairing_complete)(struct bt_conn *, bool);
    void (*pairing_failed)(struct bt_conn *, enum bt_security_err);
    void (*bond_deleted)(uint8_t, const bt_addr_le_t *);
};
struct bt_conn_cb {
    void (*connected)(struct bt_conn *, uint8_t);
    void (*disconnected)(struct bt_conn *, uint8_t);
    void (*recycled)(void);
};
#define BT_CONN_CB_DEFINE(n) struct bt_conn_cb n
void bt_foreach_bond(uint8_t id, void (*cb)(const struct bt_bond_info *, void *), void *context);
int bt_conn_get_info(struct bt_conn *, struct bt_conn_info *);
const bt_addr_le_t *bt_conn_get_dst(struct bt_conn *);
int bt_conn_disconnect(struct bt_conn *, uint8_t);
int bt_le_filter_accept_list_clear(void);
int bt_le_filter_accept_list_add(const bt_addr_le_t *);
int zmk_ble_profile_index(const bt_addr_le_t *);
bt_addr_le_t *zmk_ble_profile_address(uint8_t);
bool zmk_ble_profile_is_open(uint8_t);
bool zmk_ble_profile_is_connected(uint8_t);
unsigned bt_conn_index(struct bt_conn *);
struct bt_conn *bt_conn_ref(struct bt_conn *);
void bt_conn_unref(struct bt_conn *);
int bt_conn_auth_cancel(struct bt_conn *);
int zmk_ble_active_profile_index(void);
int settings_save_one(const char *, const void *, size_t);
int bt_unpair(uint8_t, const bt_addr_le_t *);

enum zmk_activity_state { ZMK_ACTIVITY_ACTIVE, ZMK_ACTIVITY_IDLE, ZMK_ACTIVITY_SLEEP };
static enum zmk_activity_state activity_state;
static inline enum zmk_activity_state zmk_activity_get_state(void) {
    return activity_state;
}
struct zmk_activity_state_changed {
    enum zmk_activity_state state;
};
typedef struct {
    struct zmk_activity_state_changed activity;
} zmk_event_t;
static inline const struct zmk_activity_state_changed *
as_zmk_activity_state_changed(const zmk_event_t *e) {
    return &e->activity;
}
#define ZMK_EV_EVENT_BUBBLE 0
#define ZMK_LISTENER(...)
#define ZMK_SUBSCRIPTION(...)

#endif
