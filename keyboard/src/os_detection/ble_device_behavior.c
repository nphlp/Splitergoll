// Comportement local, sans rapport HID ni effacement d’appairage.
#define DT_DRV_COMPAT splitergoll_behavior_device_next
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <splitergoll/ble_auto.h>

// Une pression formule la demande, le moteur attend ensuite tous les relâchements.
static int pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return splitergoll_ble_auto_next_device();
}

// Le relâchement ne répète jamais la commande et ne produit aucune frappe.
static int released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api driver = {.binding_pressed = pressed, .binding_released = released};
BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &driver);
