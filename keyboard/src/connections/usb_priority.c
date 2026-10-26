// Sépare la présence physique USB, son autorisation de saisie et les rapports visibles par l’hôte.
#include <zephyr/kernel.h>
#include <errno.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/usb.h>
#include <zmk/hid.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <splitergoll/ble_auto.h>
#include <splitergoll/usb_priority.h>

static struct usb_priority_state route;
static struct k_spinlock route_lock;
bool __real_zmk_usb_is_hid_ready(void);
int __real_zmk_endpoint_send_report(uint16_t usage_page);
int __real_zmk_usb_hid_send_keyboard_report(void);

// Aucun mutex dans le garde natif : un contexte ou profil remplacé révoque immédiatement la sortie.
static bool authorized(void) {
    if (!__real_zmk_usb_is_hid_ready()) return false;
    k_spinlock_key_t key = k_spin_lock(&route_lock);
    struct usb_priority_state snapshot = route;
    k_spin_unlock(&route_lock, key);
    return usb_priority_allowed(&snapshot, splitergoll_identity_evidence(
        zmk_ble_active_profile_index(), k_uptime_get_32()));
}

// Les endpoints voient l’autorisation logique, pas seulement un câble présent.
bool __wrap_zmk_usb_is_hid_ready(void) { return authorized(); }

// Le pilote et GET_REPORT n’exposent jamais les frappes d’un autre ordinateur au câble non autorisé.
struct zmk_hid_keyboard_report *splitergoll_usb_keyboard_report(void) {
    static struct zmk_hid_keyboard_report empty = {.report_id = ZMK_HID_REPORT_ID_KEYBOARD};
    return authorized() ? zmk_hid_get_keyboard_report() : &empty;
}

// La même protection s’applique aux commandes média, y compris en lecture de contrôle USB.
struct zmk_hid_consumer_report *splitergoll_usb_consumer_report(void) {
    static struct zmk_hid_consumer_report empty = {.report_id = ZMK_HID_REPORT_ID_CONSUMER};
    return authorized() ? zmk_hid_get_consumer_report() : &empty;
}

#if defined(CONFIG_ZMK_USB_BOOT)
// Le mode boot optionnel ne doit pas contourner la protection des rapports classiques.
zmk_hid_boot_report_t *splitergoll_usb_boot_report(void) {
    static zmk_hid_boot_report_t empty;
    return authorized() ? zmk_hid_get_boot_report() : &empty;
}
#endif

#if defined(CONFIG_ZMK_POINTING)
int __real_zmk_endpoint_send_mouse_report(void);
int zmk_usb_hid_send_mouse_report(void);
// Les boutons, coordonnées et molettes restent nuls quand USB n’est pas autorisé.
struct zmk_hid_mouse_report *splitergoll_usb_mouse_report(void) {
    static struct zmk_hid_mouse_report empty = {.report_id = ZMK_HID_REPORT_ID_MOUSE};
    return authorized() ? zmk_hid_get_mouse_report() : &empty;
}
// Les mouvements suivent le même garde immédiat que les touches et médias.
int __wrap_zmk_endpoint_send_mouse_report(void) {
    if (zmk_endpoint_get_selected().transport == ZMK_TRANSPORT_USB && !authorized()) return -ENOTCONN;
    return __real_zmk_endpoint_send_mouse_report();
}
#endif

// Le socle figé vide la souris sans envoyer sa libération. Complète cet envoi lors d’une révocation USB.
int __wrap_zmk_usb_hid_send_keyboard_report(void) {
    int result = __real_zmk_usb_hid_send_keyboard_report();
#if defined(CONFIG_ZMK_POINTING)
    if (!authorized() && __real_zmk_usb_is_hid_ready()) zmk_usb_hid_send_mouse_report();
#endif
    return result;
}

// Le garde est aussi vérifié juste avant envoi, sans attendre l’événement d’endpoint natif.
int __wrap_zmk_endpoint_send_report(uint16_t usage_page) {
    if (zmk_endpoint_get_selected().transport == ZMK_TRANSPORT_USB && !authorized()) return -ENOTCONN;
    return __real_zmk_endpoint_send_report(usage_page);
}

// L’unique worker du sélecteur applique les transitions après les gardes de touches et de rapports HID.
uint32_t splitergoll_usb_priority_update(void) {
    struct usb_priority_evidence evidence = splitergoll_identity_evidence(
        zmk_ble_active_profile_index(), k_uptime_get_32());
    if (!__real_zmk_usb_is_hid_ready()) evidence.context_valid = evidence.matched = false;
    bool busy = splitergoll_ble_auto_busy();
    k_spinlock_key_t key = k_spin_lock(&route_lock);
    bool changed = usb_priority_step(&route, evidence, busy);
    bool enabled = route.enabled;
    k_spin_unlock(&route_lock, key);
    if (changed) {
        // La préférence native est dédupliquée par ZMK, sans nouvelle écriture à chaque annonce.
        if (enabled) zmk_endpoint_set_preferred_transport(ZMK_TRANSPORT_USB);
        raise_zmk_usb_conn_state_changed((struct zmk_usb_conn_state_changed){.conn_state = zmk_usb_get_conn_state()});
    }
    return evidence.matched ? evidence.lease_ms : 0;
}

// Les lecteurs natifs peuvent interroger l’état pendant que le worker prépare sa prochaine transition.
bool splitergoll_usb_priority_pending(void) {
    k_spinlock_key_t key = k_spin_lock(&route_lock);
    bool pending = route.pending;
    k_spin_unlock(&route_lock, key);
    return pending;
}
