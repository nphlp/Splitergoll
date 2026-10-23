// Collecte passive des transitions, sans worker périodique ni écriture en flash.
#include <splitergoll/power_store.h>
#include <splitergoll/power_trace.h>
#include "trace_console.h"
#include <stdio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zmk/activity.h>
#include <zmk/usb.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>

static struct power_trace_store store;
static struct k_spinlock store_lock;
static uint32_t boot_id;

// Copie seulement l’horloge et les états publics, sans appel de radio ni de sélection.
static void record(enum power_trace_kind kind, int32_t value, int32_t result) {
    struct power_trace_record event = {.timestamp_ms = k_uptime_get_32(), .kind = kind,
        .activity = kind == POWER_ACTIVITY ? value : zmk_activity_get_state(),
        .usb = kind == POWER_USB ? value : zmk_usb_get_conn_state(),
        .value = value, .result = result};
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    power_trace_append(&store, event);
    k_spin_unlock(&store_lock, key);
}

// Rend les lectures avant/après comparables sans exposer l’identité du contrôleur.
static int initialize(void) {
    boot_id = sys_rand32_get();
    record(POWER_BOOT, 0, 0);
    return 0;
}
SYS_INIT(initialize, APPLICATION, 0);

// Le callback retourne immédiatement, sans modifier l’événement natif.
static int activity_changed(const zmk_event_t *event) {
    const struct zmk_activity_state_changed *activity = as_zmk_activity_state_changed(event);
    record(POWER_ACTIVITY, activity->state, 0);
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(splitergoll_power_activity, activity_changed);
ZMK_SUBSCRIPTION(splitergoll_power_activity, zmk_activity_state_changed);

// Le branchement USB est une observation d’alimentation, pas un état du chargeur.
static int usb_changed(const zmk_event_t *event) {
    const struct zmk_usb_conn_state_changed *usb = as_zmk_usb_conn_state_changed(event);
    record(POWER_USB, usb->conn_state, 0);
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(splitergoll_power_usb, usb_changed);
ZMK_SUBSCRIPTION(splitergoll_power_usb, zmk_usb_conn_state_changed);

// Les valeurs restantes sont horodatées, les états radio identiques sont dédupliqués.
void splitergoll_power_window(uint32_t remaining_ms, bool advertising) {
    record(POWER_WINDOW, (int32_t)remaining_ms, advertising ? 1 : 0);
}

// Le numéro de slot est public, aucune donnée NVS n’est copiée.
void splitergoll_power_saved(unsigned profile, int error) {
    record(POWER_SAVE, (int32_t)profile, error);
}

// Fige la borne de lecture puis copie chaque événement hors du verrou pour l’envoi USB.
void splitergoll_power_dump(const struct device *uart) {
    char line[220];
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    size_t count = store.count;
    uint32_t dropped = store.dropped, rejected = store.rejected;
    uint32_t uptime = k_uptime_get_32();
    k_spin_unlock(&store_lock, key);
    snprintf(line, sizeof(line), "POWERTRACE BEGIN version=1 boot=%08x count=%u dropped=%u rejected=%u uptime_ms=%u\n",
             boot_id, (unsigned)count, dropped, rejected, uptime);
    if (!trace_send_line(uart, line)) return;
    for (size_t i = 0; i < count; i++) {
        struct power_trace_record event;
        key = k_spin_lock(&store_lock);
        bool found = power_trace_get(&store, i, &event);
        k_spin_unlock(&store_lock, key);
        if (!found) return;
        snprintf(line, sizeof(line), "POWERTRACE EVENT index=%u ms=%u kind=%s activity=%u usb=%u value=%d result=%d\n",
                 (unsigned)i, event.timestamp_ms, power_trace_name(event.kind),
                 event.activity, event.usb, event.value, event.result);
        if (!trace_send_line(uart, line)) return;
    }
    trace_send_line(uart, "POWERTRACE END\n");
}
