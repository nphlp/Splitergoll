
#ifndef POWER_STUB_H
#define POWER_STUB_H
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define APPLICATION 0
#define SYS_INIT(f, l, p)
struct device {
    int unused;
};
static uint32_t clock_ms, random_id = 0x12345678;
static unsigned locks;
static inline uint32_t k_uptime_get_32(void) {
    return clock_ms;
}
static inline uint32_t sys_rand32_get(void) {
    return random_id++;
}
struct k_spinlock {
    int unused;
};
typedef int k_spinlock_key_t;
static inline int k_spin_lock(struct k_spinlock *lock) {
    (void)lock;
    locks++;
    return 0;
}
static inline void k_spin_unlock(struct k_spinlock *lock, int key) {
    (void)lock;
    (void)key;
    assert(locks);
    locks--;
}
enum zmk_activity_state { ZMK_ACTIVITY_ACTIVE, ZMK_ACTIVITY_IDLE, ZMK_ACTIVITY_SLEEP };
enum zmk_usb_conn_state { ZMK_USB_CONN_NONE, ZMK_USB_CONN_POWERED, ZMK_USB_CONN_HID };
static enum zmk_activity_state activity_state;
static enum zmk_usb_conn_state usb_state;
static inline enum zmk_activity_state zmk_activity_get_state(void) {
    return activity_state;
}
static inline enum zmk_usb_conn_state zmk_usb_get_conn_state(void) {
    return usb_state;
}
struct zmk_activity_state_changed {
    enum zmk_activity_state state;
};
struct zmk_usb_conn_state_changed {
    enum zmk_usb_conn_state conn_state;
};
typedef struct {
    struct zmk_activity_state_changed activity;
    struct zmk_usb_conn_state_changed usb;
} zmk_event_t;
static inline const struct zmk_activity_state_changed *
as_zmk_activity_state_changed(const zmk_event_t *e) {
    return &e->activity;
}
static inline const struct zmk_usb_conn_state_changed *
as_zmk_usb_conn_state_changed(const zmk_event_t *e) {
    return &e->usb;
}
#define ZMK_EV_EVENT_BUBBLE 0
#define ZMK_LISTENER(n, f)
#define ZMK_SUBSCRIPTION(n, t)
#endif
