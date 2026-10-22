// Sélectionne les couches de l’appareil BLE actif à partir de son OS sauvegardé.
#include <splitergoll/ble_auto.h>
#include <splitergoll/ble_auto_state.h>
#include <splitergoll/ble_device_state.h>
#include <splitergoll/ble_acquisition.h>
#include <splitergoll/ble_observer.h>
#include <splitergoll/ble_reconnect.h>
#include <splitergoll/host_window_policy.h>
#include "common/auto-layers.h"
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/sensor_event.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>
#include <zmk/matrix.h>

LOG_MODULE_REGISTER(splitergoll_ble_auto, CONFIG_ZMK_LOG_LEVEL);
BUILD_ASSERT(ZMK_KEYMAP_LEN == USB_AUTO_POSITION_COUNT);
BUILD_ASSERT(ZMK_KEYMAP_LAYERS_LEN == 13);
BUILD_ASSERT(ZMK_BLE_PROFILE_COUNT <= 16);
#define DEVICE_MODIFIER DT_PROP(DT_NODELABEL(os_auto), device_modifier_position)
#define DEVICE_BUTTON DT_PROP(DT_NODELABEL(os_auto), device_next_position)
BUILD_ASSERT(DEVICE_MODIFIER < USB_AUTO_POSITION_COUNT && DEVICE_BUTTON < USB_AUTO_POSITION_COUNT);
BUILD_ASSERT(DEVICE_MODIFIER != DEVICE_BUTTON);
static struct ble_auto_state state;
static struct ble_device_request device_request;
static struct host_window_policy device_window;
static bool arrival_search;
static struct ble_device_buffer input_buffer;
static struct {
    bt_addr_le_t peer;
    uint32_t token;
    bool open;
} identities[ZMK_BLE_PROFILE_COUNT];

static struct ble_auto_target target_for_profile(int profile);
static struct host_window_view host_view(void);
static void reconcile_selection(uint32_t now_ms);
static bt_addr_le_t requested_peer;
static bt_addr_le_t requested_source_peer;
static atomic_t auto_ready;
K_MUTEX_DEFINE(state_mutex);

int __real_zmk_event_manager_raise(zmk_event_t *event);
int __real_zmk_keymap_layer_to(zmk_keymap_layer_id_t layer, bool locking);

// Les commandes de diagnostic utilisent le même verrou récursif que le moteur de touches.
void splitergoll_ble_auto_lock(void) { k_mutex_lock(&state_mutex, K_FOREVER); }

// Termine la section critique sans changer de destination ou de couche.
void splitergoll_ble_auto_unlock(void) { k_mutex_unlock(&state_mutex); }

// Ne lit que les rapports locaux, sans les enregistrer ni les copier dans les traces.
static bool report_nonempty(const void *body, size_t length) {
    const uint8_t *bytes = body;
    for (size_t i = 0; i < length; i++) {
        if (bytes[i] != 0) return true;
    }
    return false;
}

// Un rapport ou modificateur encore actif doit finir avant toute nouvelle sélection.
static bool hid_busy(void) {
    return zmk_hid_get_explicit_mods() != 0 ||
           report_nonempty(&zmk_hid_get_keyboard_report()->body, sizeof(zmk_hid_get_keyboard_report()->body)) ||
           report_nonempty(&zmk_hid_get_consumer_report()->body, sizeof(zmk_hid_get_consumer_report()->body));
}

// Protège aussi les commandes USB lorsque la couche courante est encore inconnue.
bool splitergoll_ble_auto_busy(void) {
    splitergoll_ble_auto_lock();
    bool busy = input_buffer.active || state.input.held != 0 || state.input.admitted != 0 || hid_busy() ||
                usb_auto_drain_remaining(&state.input, k_uptime_get_32()) != 0;
    splitergoll_ble_auto_unlock();
    return busy;
}

// Les profils sont uniquement les hôtes, jamais le périphérique de la moitié droite.
static uint16_t connected_hosts(void) {
    uint16_t connected = 0;
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        if (!zmk_ble_profile_is_open(i) && zmk_ble_profile_is_connected(i)) {
            connected |= UINT16_C(1) << i;
        }
    }
    return connected;
}

// Une première association utilise les mêmes gardes de relâchement qu’une bascule explicite.
void splitergoll_ble_auto_adopt_device(unsigned target, unsigned origin, uint32_t started_at_ms) {
    splitergoll_ble_auto_lock();
    struct host_window_view view = host_view();
    bool search_pairing = arrival_search && target < ZMK_BLE_PROFILE_COUNT &&
        view.selected >= 0 && view.selected < ZMK_BLE_PROFILE_COUNT &&
        (int)origin == view.selected && device_window.source == view.selected &&
        device_window.source_identity == view.identities[view.selected] &&
        (uint32_t)(started_at_ms - device_window.opened_at_ms) < HOST_WINDOW_DURATION_MS &&
        device_window.pending < 0 && !device_request.pending;
    bool pending;
    if (search_pairing) {
        device_request = (struct ble_device_request){.origin = origin, .target = target, .pending = true};
        pending = true;
    } else pending = ble_device_adopt(&device_request, ZMK_BLE_PROFILE_COUNT,
                                       zmk_ble_active_profile_index(), connected_hosts(), target, origin);
    if (pending) {
        bt_addr_le_copy(&requested_peer, zmk_ble_profile_address(target));
        bt_addr_le_copy(&requested_source_peer, zmk_ble_profile_address(origin));
    }
    splitergoll_ble_auto_unlock();
    if (pending) splitergoll_ble_auto_notify();
}

// Les jetons changent avec l’identité native du slot, jamais avec une reconnexion de cet appareil.
static struct host_window_view host_view(void) {
    struct host_window_view view = {.count = ZMK_BLE_PROFILE_COUNT,
                                    .selected = zmk_ble_active_profile_index()};
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        const bt_addr_le_t *peer = zmk_ble_profile_address(i);
        bool open = zmk_ble_profile_is_open(i);
        if (!identities[i].token || identities[i].open != open ||
            bt_addr_le_cmp(&identities[i].peer, peer) != 0) {
            bt_addr_le_copy(&identities[i].peer, peer);
            identities[i].open = open;
            if (++identities[i].token == 0) identities[i].token = 1;
        }
        view.identities[i] = identities[i].token;
        if (!open && target_for_profile((int)i).ready) view.ready |= UINT16_C(1) << i;
    }
    return view;
}

// Le geste fige le contexte : prochaine destination prête ou première arrivée pendant la recherche.
int splitergoll_ble_auto_next_device(void) {
    splitergoll_ble_auto_lock();
    if (input_buffer.active) ble_device_buffer_discard(&input_buffer);
    device_request.pending = false;
    struct host_window_view view = host_view();
    host_window_shortcut(&device_window, &view, k_uptime_get_32());
    arrival_search = device_window.searching;
    if (device_window.pending >= 0) ble_device_buffer_begin(&input_buffer);
    splitergoll_ble_reconnect_open();
    splitergoll_ble_auto_unlock();
    splitergoll_ble_auto_notify();
    return 0;
}

// Les frappes reçues pendant le drainage attendent la nouvelle keymap dans une file RAM bornée.
static void select_requested_device(uint32_t now_ms) {
    if (!device_window.searching && device_window.pending < 0 && !device_request.pending) return;
    struct host_window_view view = host_view();
    host_window_update(&device_window, &view, now_ms);
    bool explicit_target = device_window.pending >= 0;
    bool adoption = device_request.pending && !device_window.searching && !explicit_target;
    if (device_request.pending &&
        (bt_addr_le_cmp(&requested_peer, zmk_ble_profile_address(device_request.target)) != 0 ||
         bt_addr_le_cmp(&requested_source_peer, zmk_ble_profile_address(device_request.origin)) != 0)) {
        device_request.pending = false;
        adoption = false;
    }
    if (explicit_target || (adoption && (view.ready & (UINT16_C(1) << device_request.target)))) {
        ble_device_buffer_begin(&input_buffer);
    } else if (input_buffer.active) {
        ble_device_buffer_discard(&input_buffer);
    }
    bool reports_busy = hid_busy();
    bool busy = state.input.held || state.input.admitted || reports_busy ||
                usb_auto_drain_remaining(&state.input, now_ms);
    bool searched = device_window.pending_search;
    int target = host_window_take(&device_window, &view, now_ms, device_window.generation, busy);
    if (target < 0 && adoption) {
        searched = true;
        target = ble_device_take(&device_request, ZMK_BLE_PROFILE_COUNT,
                                 (unsigned)view.selected, connected_hosts(),
                                 busy || !(view.ready & (UINT16_C(1) << device_request.target)));
    }
    if (target < 0 || !ble_device_quiesce(&state.input, reports_busy, now_ms)) return;
    if (searched) arrival_search = false;
    device_request.pending = false;
    device_window.source = target;
    device_window.source_identity = view.identities[target];
    int result = zmk_ble_prof_select(target);
    if (result) {
        ble_device_buffer_discard(&input_buffer);
        LOG_WRN("BLE device selection failed: profile=%d error=%d", target, result);
    } else LOG_INF("BLE device selected: profile=%d", target);
}

// L’USB d’alimentation n’intervient jamais dans l’OS ou la disponibilité de l’hôte BLE.
static struct ble_auto_target target_for_profile(int profile) {
    struct ble_acquisition_snapshot snapshot = splitergoll_ble_acquisition_snapshot(profile);
    struct ble_os_memory *memory = &snapshot.memory;
    struct bt_conn *conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, zmk_ble_profile_address(profile));
    bool encrypted = conn != NULL && bt_conn_get_security(conn) >= BT_SECURITY_L2;
    uint32_t epoch = conn != NULL ? splitergoll_ble_connection_epoch(conn) : 0;
    if (conn != NULL) bt_conn_unref(conn);
    struct ble_auto_link link = {.observed_epoch = epoch, .processed_epoch = snapshot.epoch,
        .connected = snapshot.connected && zmk_ble_profile_is_connected(profile),
        .secure = snapshot.secure && encrypted, .reliable = snapshot.reliable};
    bool ready = ble_auto_memory_ready(memory, link);
    return (struct ble_auto_target){.profile = profile, .session = snapshot.session,
                                    .os = memory->os, .ready = ready};
}

// La cible de saisie reste distincte des autres hôtes prêts à être sélectionnés.
static struct ble_auto_target current_target(void) {
    return target_for_profile(zmk_ble_active_profile_index());
}

// Les groupes partagés ont les mêmes affectations que leurs profils fixes respectifs.
static uint8_t selected_base(void) {
    return state.input.selected == USB_OS_MACOS ? AUTO_MACOS_BASE : AUTO_WINDOWS_BASE;
}

// Filtre avant les tap-dances, mais laisse toujours finir les appuis précédemment admis.
int __wrap_zmk_event_manager_raise(zmk_event_t *event) {
    const struct zmk_position_state_changed *position = as_zmk_position_state_changed(event);
    const struct zmk_sensor_event *sensor = as_zmk_sensor_event(event);
    const struct zmk_keycode_state_changed *keycode = as_zmk_keycode_state_changed(event);
    if (position == NULL && sensor == NULL && keycode == NULL) {
        return __real_zmk_event_manager_raise(event);
    }
    splitergoll_ble_auto_lock();
    uint32_t now_ms = k_uptime_get_32();
    ble_auto_sync(&state, current_target(), now_ms);
    if (position != NULL) {
        uint64_t bit = position->position < USB_AUTO_POSITION_COUNT ?
            UINT64_C(1) << position->position : 0;
        bool passthrough = (state.input.held & bit) != 0 ||
            position->position == DEVICE_MODIFIER || position->position == DEVICE_BUTTON;
        struct ble_device_position queued = {.position = position->position,
            .source = position->source, .timestamp = position->timestamp, .pressed = position->state};
        if (!ble_device_buffer_forward(&input_buffer, queued, passthrough)) {
            splitergoll_ble_auto_unlock();
            splitergoll_ble_auto_notify();
            return 0;
        }
    } else if (sensor != NULL && input_buffer.active) {
        splitergoll_ble_auto_unlock();
        return 0;
    }
    bool forward;
    if (position != NULL) {
        bool fallback = ble_device_fallback(&state.input, DEVICE_MODIFIER, DEVICE_BUTTON,
                                             position->position, position->state);
        forward = usb_auto_position(&state.input, position->position, position->state, now_ms);
        if (fallback) splitergoll_ble_auto_next_device();
    } else if (sensor != NULL) {
        forward = usb_auto_sensor(&state.input, now_ms);
    } else {
        forward = usb_auto_keycode(&state.input, keycode->state, now_ms);
    }
    int result = forward ? __real_zmk_event_manager_raise(event) : 0;
    bool waiting = state.input.selected == USB_OS_UNKNOWN || device_request.pending ||
                   device_window.pending >= 0 || device_window.searching;
    splitergoll_ble_auto_unlock();
    if (waiting) splitergoll_ble_auto_notify();
    return result;
}

// Le pavé numérique et le gaming gardent la base de l’OS déjà sélectionné sous leur couche.
int __wrap_zmk_keymap_layer_to(zmk_keymap_layer_id_t layer, bool locking) {
    splitergoll_ble_auto_lock();
    int result = __real_zmk_keymap_layer_to(layer, locking);
    if (state.input.selected != USB_OS_UNKNOWN) zmk_keymap_layer_activate(selected_base(), false);
    splitergoll_ble_auto_unlock();
    return result;
}

// Applique la mémoire hors callbacks radio, sans attendre arbitrairement la fin d’un délai.
static void reconcile_selection(uint32_t now_ms) {
    select_requested_device(now_ms);
    struct ble_auto_target before = current_target();
    ble_auto_sync(&state, before, now_ms);
    if (input_buffer.active && device_window.pending < 0 && !device_request.pending && !before.ready) {
        ble_device_buffer_discard(&input_buffer);
    }
    if (ble_auto_can_select(&state, hid_busy(), now_ms)) {
        enum usb_os_guess guess = ble_auto_guess(before);
        uint8_t base = guess == USB_OS_MACOS ? AUTO_MACOS_BASE : AUTO_WINDOWS_BASE;
        int result = __real_zmk_keymap_layer_to(base, true);
        uint32_t generation = state.input.generation;
        ble_auto_sync(&state, current_target(), k_uptime_get_32());
        if (!result && generation == state.input.generation) {
            state.input.selected = guess;
            LOG_INF("BLE keymap selected: profile=%d os=%s", before.profile, ble_memory_os_name(before.os));
        }
    }
    if (input_buffer.active && device_window.pending < 0 && !device_request.pending &&
        state.input.selected != USB_OS_UNKNOWN) {
        if (input_buffer.overflow) {
            LOG_WRN("BLE switch input buffer full: discarded queued input");
            ble_device_buffer_discard(&input_buffer);
        } else {
            unsigned count = input_buffer.count;
            input_buffer.active = false;
            input_buffer.count = 0;
            input_buffer.held = 0;
            for (unsigned i = 0; i < count; i++) {
                struct ble_device_position event = input_buffer.events[i];
                raise_zmk_position_state_changed((struct zmk_position_state_changed){
                    .source = event.source, .position = event.position,
                    .state = event.pressed, .timestamp = event.timestamp});
            }
        }
    }
}

// Programme seulement les échéances utiles, la radio et l’acquisition notifient les arrivées.
static void select_os(struct k_work *work) {
    splitergoll_ble_auto_lock();
    uint32_t now_ms = k_uptime_get_32();
    reconcile_selection(now_ms);
    uint32_t remaining = usb_auto_drain_remaining(&state.input, now_ms);
    bool waiting = state.input.selected == USB_OS_UNKNOWN && ble_auto_guess(state.target) != USB_OS_UNKNOWN;
    if ((waiting || device_request.pending || device_window.pending >= 0) &&
        state.input.held == 0 && remaining != 0) {
        k_work_schedule(k_work_delayable_from_work(work), K_MSEC(remaining));
    }
    uint32_t window_remaining = host_window_remaining(&device_window, now_ms);
    if (device_window.searching && window_remaining != 0) {
        k_work_schedule(k_work_delayable_from_work(work), K_MSEC(window_remaining));
    }
    splitergoll_ble_auto_unlock();
}
K_WORK_DELAYABLE_DEFINE(selection_work, select_os);
// Regroupe les événements sans poller et sans temporisation initiale fixe.
void splitergoll_ble_auto_notify(void) {
    if (atomic_get(&auto_ready)) k_work_reschedule(&selection_work, K_NO_WAIT);
}

// Une sélection USB ou native change seulement la cible, le travail applique ensuite son OS.
static int profile_changed(const zmk_event_t *event) {
    const struct zmk_ble_active_profile_changed *profile = as_zmk_ble_active_profile_changed(event);
    splitergoll_ble_auto_lock();
    if (device_request.pending && profile->index != device_request.origin) device_request.pending = false;
    if ((int)profile->index != device_window.source) {
        host_window_cancel_selection(&device_window);
        arrival_search = false;
        if (input_buffer.active) ble_device_buffer_discard(&input_buffer);
    }
    splitergoll_ble_auto_unlock();
    splitergoll_ble_auto_notify();
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(splitergoll_ble_auto, profile_changed);
ZMK_SUBSCRIPTION(splitergoll_ble_auto, zmk_ble_active_profile_changed);

// Attend l’initialisation du clavier avant de lire les états chargés par ZMK.
static int auto_init(void) {
    host_window_start(&device_window, k_uptime_get_32());
    atomic_set(&auto_ready, 1);
    splitergoll_ble_auto_notify();
    return 0;
}
SYS_INIT(auto_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
