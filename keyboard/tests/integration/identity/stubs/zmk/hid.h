#include <identity_stubs.h>
#define ZMK_HID_REPORT_ID_KEYBOARD 1
#define ZMK_HID_REPORT_ID_CONSUMER 2
#define ZMK_HID_REPORT_ID_MOUSE 3
struct zmk_hid_keyboard_report { uint8_t report_id; uint8_t body[8]; };
struct zmk_hid_consumer_report { uint8_t report_id; uint8_t body[8]; };
struct zmk_hid_mouse_report { uint8_t report_id; uint8_t body[8]; };
static struct zmk_hid_keyboard_report keyboard_report;
static struct zmk_hid_consumer_report consumer_report;
static struct zmk_hid_mouse_report mouse_report;
static inline struct zmk_hid_keyboard_report *zmk_hid_get_keyboard_report(void) { return &keyboard_report; }
static inline struct zmk_hid_consumer_report *zmk_hid_get_consumer_report(void) { return &consumer_report; }
static inline struct zmk_hid_mouse_report *zmk_hid_get_mouse_report(void) { return &mouse_report; }
