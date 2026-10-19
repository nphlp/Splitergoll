// Applique la décision USB au groupe de couches correspondant, sans sauvegarde NVS.
#include <splitergoll/usb_auto_state.h>
#include <splitergoll/usb_observer.h>
#include "common/auto-layers.h"

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/sensor_event.h>
#include <zmk/keymap.h>
#include <zmk/matrix.h>
#include <zmk/usb.h>

LOG_MODULE_REGISTER(splitergoll_os, CONFIG_ZMK_LOG_LEVEL);
BUILD_ASSERT(ZMK_KEYMAP_LEN == USB_AUTO_POSITION_COUNT, "Auto requires the 44-key layout");
BUILD_ASSERT(ZMK_KEYMAP_LAYERS_LEN == 13, "Auto requires both complete fixed layer groups");

static struct usb_auto_state state;
static atomic_t auto_ready;
K_MUTEX_DEFINE(state_mutex);

// Les appels réels restent ceux du moteur ZMK figé, sans patch de dépendance.
int __real_zmk_event_manager_raise(zmk_event_t *event);
int __real_zmk_keymap_layer_to(zmk_keymap_layer_id_t layer, bool locking);

// Retrouve la base persistante du groupe choisi, sans OS par défaut.
static uint8_t selected_base(void) {
    return state.selected == USB_OS_MACOS ? AUTO_MACOS_BASE : AUTO_WINDOWS_BASE;
}

// Filtre les appuis avant les tap-dances et les modificateurs du moteur d’événements.
int __wrap_zmk_event_manager_raise(zmk_event_t *event) {
    const struct zmk_position_state_changed *position = as_zmk_position_state_changed(event);
    const struct zmk_sensor_event *sensor = as_zmk_sensor_event(event);
    const struct zmk_keycode_state_changed *keycode = as_zmk_keycode_state_changed(event);
    if (position == NULL && sensor == NULL && keycode == NULL) {
        return __real_zmk_event_manager_raise(event);
    }

    k_mutex_lock(&state_mutex, K_FOREVER);
    uint32_t now_ms = k_uptime_get_32();
    struct usb_os_snapshot snapshot = splitergoll_usb_os_snapshot();
    usb_auto_sync(&state, snapshot.generation, now_ms);
    bool forward;
    if (position != NULL) {
        forward = usb_auto_position(&state, position->position, position->state, now_ms);
    } else if (sensor != NULL) {
        forward = usb_auto_sensor(&state, now_ms);
    } else {
        forward = usb_auto_keycode(&state, keycode->state, now_ms);
    }
    int result = forward ? __real_zmk_event_manager_raise(event) : 0;
    bool waiting = state.selected == USB_OS_UNKNOWN;
    k_mutex_unlock(&state_mutex);
    if (waiting) {
        splitergoll_usb_auto_notify();
    }
    return result;
}

// Garde la base sous le pavé et le jeu, comme la couche zéro des profils fixes.
int __wrap_zmk_keymap_layer_to(zmk_keymap_layer_id_t layer, bool locking) {
    k_mutex_lock(&state_mutex, K_FOREVER);
    int result = __real_zmk_keymap_layer_to(layer, locking);
    if (state.selected != USB_OS_UNKNOWN) {
        zmk_keymap_layer_activate(selected_base(), false);
    }
    k_mutex_unlock(&state_mutex);
    return result;
}

// Expose la sélection appliquée au diagnostic, en refusant une valeur d’un ancien cycle.
enum usb_os_guess splitergoll_usb_auto_selected(void) {
    k_mutex_lock(&state_mutex, K_FOREVER);
    struct usb_os_snapshot snapshot = splitergoll_usb_os_snapshot();
    enum usb_os_guess selected = state.generation == snapshot.generation ?
                                state.selected : USB_OS_UNKNOWN;
    k_mutex_unlock(&state_mutex);
    return selected;
}

// Applique les indices disponibles hors callback USB, ou attend seulement un ancien comportement.
static void select_os(struct k_work *work) {
    k_mutex_lock(&state_mutex, K_FOREVER);
    struct usb_os_snapshot snapshot = splitergoll_usb_os_snapshot();
    uint32_t now_ms = k_uptime_get_32();
    usb_auto_sync(&state, snapshot.generation, now_ms);
    bool known = snapshot.guess == USB_OS_MACOS || snapshot.guess == USB_OS_WINDOWS;
    bool hid_ready = zmk_usb_is_hid_ready();
    if (state.selected == USB_OS_UNKNOWN && known &&
        usb_auto_can_select(&state, snapshot.guess, hid_ready, now_ms)) {
        uint8_t base = snapshot.guess == USB_OS_MACOS ? AUTO_MACOS_BASE : AUTO_WINDOWS_BASE;
        int result = __real_zmk_keymap_layer_to(base, true);
        // Un reset arrivé pendant la transition invalide immédiatement son résultat.
        struct usb_os_snapshot after = splitergoll_usb_os_snapshot();
        if (result == 0 && snapshot.generation == after.generation &&
            snapshot.guess == after.guess) {
            state.selected = snapshot.guess;
            LOG_INF("USB OS selected: %s", usb_os_name(state.selected));
        }
    }
    uint32_t remaining = usb_auto_drain_remaining(&state, now_ms);
    if (state.selected == USB_OS_UNKNOWN && known && hid_ready && state.held == 0 &&
        remaining != 0) {
        // Un événement déjà en file doit garder la priorité sur cette échéance.
        k_work_schedule(k_work_delayable_from_work(work), K_MSEC(remaining));
    }
    k_mutex_unlock(&state_mutex);
}

K_WORK_DELAYABLE_DEFINE(selection_work, select_os);

// Les notifications se regroupent en un travail, sans attente fixe ni boucle périodique.
void splitergoll_usb_auto_notify(void) {
    if (atomic_get(&auto_ready)) {
        k_work_reschedule(&selection_work, K_NO_WAIT);
    }
}

// Démarre le contrôle après l’initialisation du clavier, sans thread série supplémentaire.
static int auto_init(void) {
    atomic_set(&auto_ready, 1);
    splitergoll_usb_auto_notify();
    return 0;
}

SYS_INIT(auto_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
