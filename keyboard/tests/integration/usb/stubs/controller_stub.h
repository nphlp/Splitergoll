
#ifndef CONTROLLER_STUB_H
#define CONTROLLER_STUB_H
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define BUILD_ASSERT(c, ...) _Static_assert(c, #c)
#define DT_NODELABEL(n) n
#define DT_PROP(n, p) prop_##p
#define prop_device_modifier_position 40
#define prop_device_next_position 43
#define ZMK_KEYMAP_LEN 44
#define ZMK_KEYMAP_LAYERS_LEN 13
#define ZMK_BLE_PROFILE_COUNT 5
#define K_FOREVER 0
#define K_NO_WAIT 0
#define K_MSEC(n) (n)
#define ARG_UNUSED(x) (void)(x)
#define CONFIG_ZMK_LOG_LEVEL 0
#define CONFIG_APPLICATION_INIT_PRIORITY 0
#define APPLICATION 0
#define LOG_MODULE_REGISTER(...)
#define LOG_INF(...)
#define LOG_WRN(...)
#define ZMK_EV_EVENT_BUBBLE 0
struct zmk_listener {
    int (*callback)(const void *event);
};
#define ZMK_LISTENER(...)
#define ZMK_SUBSCRIPTION(...)
#define SYS_INIT(...)
static uint32_t clock_ms;
static inline uint32_t k_uptime_get_32(void) {
    return clock_ms;
}
struct k_mutex {
    unsigned depth;
};
#define K_MUTEX_DEFINE(n) struct k_mutex n
static inline void k_mutex_lock(struct k_mutex *m, int t) {
    (void)t;
    m->depth++;
}
static inline void k_mutex_unlock(struct k_mutex *m) {
    assert(m->depth);
    m->depth--;
}
typedef int atomic_t;
static inline int atomic_get(atomic_t *p) {
    return *p;
}
static inline void atomic_set(atomic_t *p, int n) {
    *p = n;
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
struct bt_conn {
    unsigned profile;
};
#define BT_ID_DEFAULT 0
#define BT_SECURITY_L2 2
static inline int bt_addr_le_cmp(const bt_addr_le_t *a, const bt_addr_le_t *b) {
    return a->value != b->value;
}
static inline void bt_addr_le_copy(bt_addr_le_t *a, const bt_addr_le_t *b) {
    *a = *b;
}
struct bt_conn *bt_conn_lookup_addr_le(int id, const bt_addr_le_t *addr);
unsigned bt_conn_get_security(struct bt_conn *conn);
void bt_conn_unref(struct bt_conn *conn);
int zmk_ble_active_profile_index(void);
bt_addr_le_t *zmk_ble_profile_address(uint8_t p);
bool zmk_ble_profile_is_connected(uint8_t p);
bool zmk_ble_profile_is_open(uint8_t p);
int zmk_ble_prof_select(uint8_t p);
struct zmk_position_state_changed {
    uint8_t source;
    uint32_t position;
    bool state;
    int64_t timestamp;
};
struct zmk_keycode_state_changed {
    bool state;
};
struct zmk_sensor_event {
    int value;
};
struct zmk_ble_active_profile_changed {
    unsigned index;
};
typedef struct {
    int type;
    struct zmk_ble_active_profile_changed profile;
    struct zmk_position_state_changed position;
    struct zmk_keycode_state_changed keycode;
    struct zmk_sensor_event sensor;
} zmk_event_t;
static inline const struct zmk_position_state_changed *
as_zmk_position_state_changed(const zmk_event_t *e) {
    return e->type == 1 ? &e->position : NULL;
}
static inline const struct zmk_sensor_event *as_zmk_sensor_event(const zmk_event_t *e) {
    return e->type == 2 ? &e->sensor : NULL;
}
static inline const struct zmk_keycode_state_changed *
as_zmk_keycode_state_changed(const zmk_event_t *e) {
    return e->type == 3 ? &e->keycode : NULL;
}
static inline const struct zmk_ble_active_profile_changed *
as_zmk_ble_active_profile_changed(const zmk_event_t *e) {
    return &e->profile;
}
int raise_zmk_position_state_changed(struct zmk_position_state_changed event);
struct report {
    struct {
        uint8_t data[8];
    } body;
};
static struct report keyboard_report, consumer_report;
static inline uint8_t zmk_hid_get_explicit_mods(void) {
    return 0;
}
static inline struct report *zmk_hid_get_keyboard_report(void) {
    return &keyboard_report;
}
static inline struct report *zmk_hid_get_consumer_report(void) {
    return &consumer_report;
}
typedef uint8_t zmk_keymap_layer_id_t;
static uint8_t activated_base;
static inline int zmk_keymap_layer_activate(uint8_t p, bool lock) {
    activated_base = p;
    (void)lock;
    return 0;
}
#endif
