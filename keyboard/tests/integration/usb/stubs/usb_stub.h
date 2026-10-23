
#ifndef USB_STUB_H
#define USB_STUB_H
#include "controller_stub.h"
#define CONFIG_SPLITERGOLL_USB_TRACE 0
#define CONFIG_SPLITERGOLL_OS_AUTO 1
#define IS_ENABLED(x) (x)
struct k_spinlock {
    unsigned depth;
};
typedef int k_spinlock_key_t;
static inline int k_spin_lock(struct k_spinlock *p) {
    assert(!p->depth);
    p->depth++;
    return 0;
}
static inline void k_spin_unlock(struct k_spinlock *p, int key) {
    (void)key;
    assert(p->depth == 1);
    p->depth--;
}
enum usb_dc_ep_cb_status_code { USB_DC_EP_SETUP, USB_DC_EP_DATA_OUT };
enum usb_dc_status_code {
    USB_DC_RESET,
    USB_DC_DISCONNECTED,
    USB_DC_CONFIGURED,
    USB_DC_SUSPEND,
    USB_DC_RESUME,
    USB_DC_SOF
};
typedef void (*usb_dc_ep_callback)(uint8_t, enum usb_dc_ep_cb_status_code);
typedef void (*usb_dc_status_callback)(enum usb_dc_status_code, const uint8_t *);
bool zmk_usb_is_hid_ready(void);
#endif
