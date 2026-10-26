#include "keyboard/src/os_detection/ble_auto.c"
#include <stdio.h>
#if defined(CONFIG_SPLITERGOLL_USB_PRIORITY)
static uint32_t priority_deadline;
// Le harnais du vrai sélecteur contrôle seulement la frontière de la politique USB testée séparément.
uint32_t splitergoll_usb_priority_update(void) { return priority_deadline; }
// Les suites existantes gardent leurs demandes de bascule et de macro inchangées.
bool splitergoll_usb_priority_pending(void) { return false; }
#endif
static unsigned active, connected = 3, selected_calls, forwarded, base;
static struct host_preference_status saved_preference = {.profile = -1};
static unsigned preference_writes;
static struct ble_reconnect_status radio;
static unsigned activity_notes;
static int activity(const void *event) {
    (void)event;
    activity_notes++;
    return 0;
}
const struct zmk_listener zmk_listener_activity = {.callback = activity};
struct host_preference_status splitergoll_host_preference_status(void) {
    return saved_preference;
}
void splitergoll_host_preference_save(unsigned p) {
    assert(p < 5);
    preference_writes++;
    saved_preference.profile = (int)p;
    saved_preference.present = true;
}
void splitergoll_ble_reconnect_open(void) {
    radio.generation++;
    radio.opened_at_ms = clock_ms;
}
struct ble_reconnect_status splitergoll_ble_reconnect_status(void) {
    struct ble_reconnect_status r = radio;
    r.without_host = connected == 0;
    r.connected = connected;
    r.remaining_ms = r.without_host ? HOST_WINDOW_DURATION_MS
                     : clock_ms - radio.opened_at_ms < HOST_WINDOW_DURATION_MS
                         ? HOST_WINDOW_DURATION_MS - (clock_ms - radio.opened_at_ms)
                         : 0;
    return r;
}
static struct bt_conn hosts[5] = {{0}, {1}, {2}, {3}, {4}};
static bt_addr_le_t addresses[5] = {{1}, {2}, {3}, {4}, {5}};
static bool known[5] = {true, true, false, false, false};
static struct {
    unsigned host, position;
    bool pressed;
    int64_t timestamp;
} delivered[256];
static unsigned delivered_count;
static int selection_error, layer_error;
int zmk_ble_active_profile_index(void) {
    return active;
}
bt_addr_le_t *zmk_ble_profile_address(uint8_t p) {
    assert(p < 5);
    return &addresses[p];
}
bool zmk_ble_profile_is_connected(uint8_t p) {
    return connected & (1u << p);
}
bool zmk_ble_profile_is_open(uint8_t p) {
    return addresses[p].value == 0;
}
struct bt_conn *bt_conn_lookup_addr_le(int id, const bt_addr_le_t *addr) {
    (void)id;
    for (unsigned p = 0; p < 5; p++)
        if (!bt_addr_le_cmp(addr, &addresses[p]) && zmk_ble_profile_is_connected(p))
            return &hosts[p];
    return NULL;
}
unsigned bt_conn_get_security(struct bt_conn *c) {
    (void)c;
    return 2;
}
void bt_conn_unref(struct bt_conn *c) {
    (void)c;
}
uint32_t splitergoll_ble_connection_epoch(struct bt_conn *c) {
    return 10 + c->profile;
}
struct ble_acquisition_snapshot splitergoll_ble_acquisition_snapshot(unsigned p) {
    return (struct ble_acquisition_snapshot){
        .memory = {.os = known[p] ? (p == 0 ? BLE_MEMORY_MACOS : BLE_MEMORY_WINDOWS)
                                  : BLE_MEMORY_UNKNOWN,
                   .present = true,
                   .verified = true},
        .session = 1 + p,
        .epoch = 10 + p,
        .connected = zmk_ble_profile_is_connected(p),
        .secure = true,
        .reliable = true};
}
int zmk_ble_prof_select(uint8_t p) {
    assert(p < 5);
    if (selection_error)
        return selection_error;
    active = p;
    selected_calls++;
    profile_changed(&(zmk_event_t){.profile = {active}});
    return 0;
}
int __real_zmk_keymap_layer_to(uint8_t layer, bool locking) {
    (void)locking;
    if (layer_error)
        return layer_error;
    base = layer;
    return 0;
}
int __real_zmk_event_manager_raise(zmk_event_t *e) {
    forwarded++;
    if (e->type == 1 && e->position.position == 43 && e->position.state &&
        state.input.selected != USB_OS_UNKNOWN)
        splitergoll_ble_auto_next_device();
    if (e->type == 1 && e->position.position != 40 && e->position.position != 43) {
        assert(delivered_count < 256);
        delivered[delivered_count].host = active;
        delivered[delivered_count].position = e->position.position;
        delivered[delivered_count].pressed = e->position.state;
        delivered[delivered_count++].timestamp = e->position.timestamp;
    }
    return 0;
}
int raise_zmk_position_state_changed(struct zmk_position_state_changed event) {
    zmk_event_t e = {.type = 1, .position = event};
    return __wrap_zmk_event_manager_raise(&e);
}
static void tick(uint32_t now) {
    clock_ms = now;
    for (unsigned i = 0; i < 30; i++) {
        if (!selection_work.queued || (int32_t)(clock_ms - selection_work.due) < 0)
            return;
        selection_work.queued = false;
        select_os(&selection_work.work);
    }
    assert(!"work loop");
}
static void position(unsigned p, bool pressed, uint32_t now) {
    clock_ms = now;
    zmk_event_t e = {.type = 1,
                     .position = {.position = p, .state = pressed, .timestamp = now, .source = 0}};
    __wrap_zmk_event_manager_raise(&e);
    tick(now);
}
static void reset(unsigned mask) {
    saved_preference = (struct host_preference_status){.profile = -1};
    preference_writes = activity_notes = 0;
    radio = (struct ble_reconnect_status){
        .ready = true, .active = true, .generation = 1, .recovery = 1};
    session = (struct host_session_policy){0};
    requested_manual = false;
    save_candidate.profile = -1;
    internal_selection = false;
    automatic_target = -1;
    layer_error = selection_error = 0;
    active = 0;
    connected = mask;
    selected_calls = forwarded = delivered_count = 0;
    clock_ms = 0;
    state = (struct ble_auto_state){0};
    device_request = (struct ble_device_request){0};
    input_buffer = (struct ble_device_buffer){0};
    memset(identities, 0, sizeof(identities));
    arrival_search = false;
    known[0] = known[1] = true;
    known[2] = true;
    for (unsigned p = 0; p < 5; p++)
        addresses[p].value = p + 1;
    selection_work.queued = false;
    explicit_mods = 0;
    memset(&keyboard_report, 0, sizeof(keyboard_report));
    memset(&consumer_report, 0, sizeof(consumer_report));
    auto_init();
    tick(0);
    if (mask & 1)
        assert(base == AUTO_MACOS_BASE);
}
// La frappe immédiate attend la bascule et ne repousse pas son échéance.
static void immediate_typing(void) {
    reset(3);
    position(40, true, 1);
    position(43, true, 2);
    position(43, false, 3);
    position(40, false, 4);
    assert(device_window.pending == 1);
    position(1, true, 100);
    position(1, false, 150);
    position(2, true, 200);
    position(2, false, 250);
    assert(active == 0 && input_buffer.count == 4 && delivered_count == 0);
    tick(753);
    assert(active == 0);
    tick(754);
    assert(active == 1 && selected_calls == 1 && base == AUTO_WINDOWS_BASE && delivered_count == 4);
    for (unsigned i = 0; i < 4; i++)
        assert(delivered[i].host == 1);
    assert(delivered[0].timestamp == 100 && delivered[3].timestamp == 250);
    assert(!state.input.held && !state.input.admitted && !input_buffer.active);
}
// Les releases déjà admises restent sur l’ancien hôte.
static void old_release(void) {
    reset(3);
    position(1, true, 1);
    position(40, true, 2);
    position(43, true, 3);
    position(43, false, 4);
    position(40, false, 5);
    position(1, false, 20);
    position(2, true, 30);
    position(2, false, 50);
    tick(769);
    assert(active == 0);
    tick(770);
    assert(active == 1 && delivered_count == 4 && delivered[0].host == 0 &&
           delivered[1].host == 0 && delivered[2].host == 1 && delivered[3].host == 1);
}
// Un seul appareil au geste arme une recherche, plusieurs n’autorisent aucune arrivée à voler le
// choix.
static void arrival_and_timeout(void) {
    reset(1);
    position(40, true, 1);
    position(43, true, 2);
    position(43, false, 3);
    position(40, false, 4);
    position(1, true, 100);
    position(1, false, 150);
    assert(delivered_count == 2 && delivered[0].host == 0);
    connected = 3;
    splitergoll_ble_auto_notify();
    tick(1200);
    assert(active == 1 && base == AUTO_WINDOWS_BASE);
    connected = 7;
    splitergoll_ble_auto_notify();
    tick(1300);
    assert(active == 1 && selected_calls == 1);
    reset(1);
    splitergoll_ble_auto_next_device();
    tick(0);
    tick(HOST_WINDOW_DURATION_MS);
    connected = 3;
    splitergoll_ble_auto_notify();
    tick((HOST_WINDOW_DURATION_MS + 1));
    assert(active == 0 && selected_calls == 0);
    reset(3);
    splitergoll_ble_auto_next_device();
    connected = 7;
    tick(0);
    assert(active == 1);
    tick(10);
    assert(active == 1 && selected_calls == 1);
}
// Une cible perdue annule le lot, une release tardive n’est jamais transférée seule.
static void invalidated_target(void) {
    reset(3);
    position(40, true, 1);
    position(43, true, 2);
    position(43, false, 3);
    position(40, false, 4);
    position(1, true, 10);
    connected = 1;
    splitergoll_ble_auto_notify();
    tick(20);
    assert(active == 0 && !input_buffer.active);
    unsigned count = delivered_count;
    position(1, false, 30);
    assert(delivered_count == count && selected_calls == 0);
}
// Une association commencée dans la fenêtre peut finir après sa fermeture et rester la cible de
// recherche.
static void late_pairing(void) {
    reset(1);
    splitergoll_ble_auto_next_device();
    tick(0);
    tick(HOST_WINDOW_DURATION_MS);
    connected = 3;
    clock_ms = (HOST_WINDOW_DURATION_MS + 1000);
    splitergoll_ble_auto_adopt_device(1, 0, (HOST_WINDOW_DURATION_MS - 1000));
    tick((HOST_WINDOW_DURATION_MS + 1000));
    assert(active == 1 && selected_calls == 1);
    reset(1);
    splitergoll_ble_auto_next_device();
    tick(0);
    tick(HOST_WINDOW_DURATION_MS);
    connected = 3;
    clock_ms = (HOST_WINDOW_DURATION_MS + 1000);
    splitergoll_ble_auto_adopt_device(1, 0, HOST_WINDOW_DURATION_MS);
    tick((HOST_WINDOW_DURATION_MS + 1000));
    assert(active == 0 && selected_calls == 0);
}
// Une nouvelle touche encore tenue après la bascule garde sa release sur la nouvelle destination.
static void queued_hold(void) {
    reset(3);
    position(40, true, 1);
    position(43, true, 2);
    position(43, false, 3);
    position(40, false, 4);
    position(1, true, 10);
    tick(754);
    assert(active == 1 && state.input.held == 2 && delivered_count == 1);
    position(1, false, 800);
    assert(delivered_count == 2 && delivered[1].host == 1 && !state.input.held);
}
// Les keycodes de l'ancienne macro restent drainés, la frappe retenue n'ajoute pas d'attente.
static void delayed_macro(void) {
    reset(3);
    position(40, true, 1);
    position(43, true, 2);
    position(43, false, 3);
    position(40, false, 4);
    position(1, true, 10);
    position(1, false, 20);
    clock_ms = 500;
    zmk_event_t key = {.type = 3, .keycode = {true}};
    __wrap_zmk_event_manager_raise(&key);
    tick(500);
    clock_ms = 530;
    key.keycode.state = false;
    __wrap_zmk_event_manager_raise(&key);
    tick(530);
    tick(1279);
    assert(active == 0 && delivered_count == 0);
    tick(1280);
    assert(active == 1 && delivered_count == 2);
}
// Un échec natif ne transmet ni le lot ni une release orpheline à l'ancien appareil.
static void native_failure(void) {
    reset(3);
    position(40, true, 1);
    position(43, true, 2);
    position(43, false, 3);
    position(40, false, 4);
    position(1, true, 10);
    selection_error = -5;
    tick(754);
    assert(active == 0 && !input_buffer.active && delivered_count == 0);
    position(1, false, 800);
    assert(!delivered_count && state.input.selected == USB_OS_MACOS);
    selection_error = 0;
    position(2, true, 810);
    position(2, false, 820);
    assert(delivered_count == 2 && delivered[0].host == 0);
}

// Le premier disponible fonctionne avant le préféré, puis le rappel survit à la saisie sur lui.
static void boot_restore_and_checkpoint(void) {
    reset(2);
    saved_preference =
        (struct host_preference_status){.profile = 0, .loaded = true, .present = true};
    splitergoll_ble_auto_notify();
    tick(1);
    assert(active == 1 && base == AUTO_WINDOWS_BASE);
    position(1, true, 100);
    position(1, false, 110);
    connected = 3;
    splitergoll_ble_auto_notify();
    tick(200);
    assert(active == 1 && automatic_target == 0);
    position(2, true, 210);
    position(2, false, 220);
    tick(860);
    assert(active == 0 && base == AUTO_MACOS_BASE && delivered[2].host == 0 &&
           delivered[3].host == 0);
    tick((HOST_WINDOW_DURATION_MS - 1));
    assert(!preference_writes);
    tick(HOST_WINDOW_DURATION_MS);
    assert(preference_writes == 1 && saved_preference.profile == 0);
    // Sans retour dans la fenêtre, le premier devient le choix durable et une arrivée tardive ne le
    // vole pas.
    reset(2);
    saved_preference =
        (struct host_preference_status){.profile = 0, .loaded = true, .present = true};
    splitergoll_ble_auto_notify();
    tick(1);
    assert(active == 1);
    tick(HOST_WINDOW_DURATION_MS);
    assert(preference_writes == 1 && saved_preference.profile == 1);
    connected = 3;
    splitergoll_ble_auto_notify();
    tick((HOST_WINDOW_DURATION_MS + 1));
    assert(active == 1);
    // Une demande manuelle annule le rappel même si l'ancien revient ensuite.
    reset(2);
    saved_preference =
        (struct host_preference_status){.profile = 0, .loaded = true, .present = true};
    splitergoll_ble_auto_notify();
    tick(1);
    splitergoll_ble_auto_next_device();
    tick(1);
    connected = 7;
    splitergoll_ble_auto_notify();
    tick(1000);
    assert(!session.restoring);
    // Aucun hôte : pas de sauvegarde vide, les frappes retenues réveillent toujours l'activité
    // native.
    reset(0);
    saved_preference =
        (struct host_preference_status){.profile = 0, .loaded = true, .present = true};
    splitergoll_ble_auto_notify();
    tick(1);
    tick((HOST_WINDOW_DURATION_MS + 1));
    assert(!preference_writes);
    position(1, true, (HOST_WINDOW_DURATION_MS + 1000));
    position(1, false, (HOST_WINDOW_DURATION_MS + 1001));
    assert(activity_notes >= 2);
}

// Une bascule manuelle réussie sauvegarde immédiatement, pas les gestes sans cible ou les échecs.
static void immediate_preference(void) {
    reset(3);
    splitergoll_ble_auto_next_device();
    tick(0);
    assert(active == 1 && preference_writes == 1 && saved_preference.profile == 1);
    tick(HOST_WINDOW_DURATION_MS);
    assert(preference_writes == 1);
    reset(1);
    splitergoll_ble_auto_next_device();
    tick(0);
    tick(HOST_WINDOW_DURATION_MS);
    assert(!preference_writes);
    reset(1);
    splitergoll_ble_auto_next_device();
    tick(0);
    connected = 3;
    splitergoll_ble_auto_notify();
    tick(50);
    assert(active == 1 && preference_writes == 1 && saved_preference.profile == 1);
    reset(1);
    splitergoll_ble_auto_next_device();
    tick(0);
    tick(HOST_WINDOW_DURATION_MS);
    connected = 3;
    clock_ms = HOST_WINDOW_DURATION_MS + 1000;
    splitergoll_ble_auto_adopt_device(1, 0, HOST_WINDOW_DURATION_MS - 1000);
    tick(clock_ms);
    assert(active == 1 && preference_writes == 1 && saved_preference.profile == 1);
    reset(3);
    selection_error = -5;
    splitergoll_ble_auto_next_device();
    tick(0);
    assert(active == 0 && !preference_writes);
}
// Un profil natif sélectionné ne suffit pas : attendre l’application réussie de la keymap.
static void failed_keymap_and_replaced_request(void) {
    reset(3);
    layer_error = -5;
    splitergoll_ble_auto_next_device();
    tick(0);
    assert(active == 1 && state.input.selected == USB_OS_UNKNOWN && !preference_writes &&
           save_candidate.profile == 1);
    layer_error = 0;
    splitergoll_ble_auto_notify();
    tick(1);
    assert(base == AUTO_WINDOWS_BASE && saved_preference.profile == 1 && preference_writes == 1);
    reset(3);
    position(40, true, 1);
    position(43, true, 2);
    position(43, false, 3);
    splitergoll_ble_auto_next_device(); // Remplace la première cible avant son application.
    position(40, false, 4);
    tick(754);
    assert(active == 0 &&
           !preference_writes); // Retour sur la source : aucune bascule réussie à inventer.
}
// La perte de l’hôte utilisé sauvegarde le repli et un retour ne vole pas la sélection.
static void failover_and_full_recovery(void) {
    reset(3);
    saved_preference =
        (struct host_preference_status){.profile = 0, .loaded = true, .present = true};
    splitergoll_ble_auto_notify();
    tick(1);
    assert(session.used == 0);
    connected = 2;
    splitergoll_ble_auto_notify();
    tick(2);
    assert(active == 1 && saved_preference.profile == 1 && preference_writes == 1 &&
           !session.restoring);
    connected = 3;
    splitergoll_ble_auto_notify();
    tick(3);
    assert(active == 1 && target_for_profile(0).ready);
    splitergoll_ble_auto_next_device();
    tick(4);
    tick(5);
    assert(active == 0 &&
           saved_preference.profile == 0); // Le raccourci choisit le Mac déjà reconnecté.
    tick(HOST_WINDOW_DURATION_MS + 5);
    assert(saved_preference.profile == 0);
    reset(3);
    saved_preference =
        (struct host_preference_status){.profile = 0, .loaded = true, .present = true};
    splitergoll_ble_auto_notify();
    tick(1);
    connected = 0;
    radio.recovery++;
    splitergoll_ble_auto_notify();
    tick(2);
    assert(session.restoring && session.checkpoint);
    connected = 2;
    radio.opened_at_ms = 3;
    radio.generation++;
    splitergoll_ble_auto_notify();
    tick(3);
    tick(1000);
    assert(active == 1 && !preference_writes);
    connected = 3;
    splitergoll_ble_auto_notify();
    tick(1001);
    tick(2000);
    assert(active == 0 && !preference_writes);
    tick(HOST_WINDOW_DURATION_MS + 3);
    assert(saved_preference.profile == 0 && preference_writes == 1);
}

// La sauvegarde de fin de fenêtre aboutit de part et d’autre d’IDLE sans activité artificielle.
static void idle_at_checkpoint(void) {
    for (unsigned order = 0; order < 3; order++) {
        reset(2);
        saved_preference =
            (struct host_preference_status){.profile = 0, .loaded = true, .present = true};
        splitergoll_ble_auto_notify();
        tick(1);
        assert(active == 1 && !preference_writes);
        unsigned notes = activity_notes, generation = radio.generation;
        tick(HOST_WINDOW_DURATION_MS - 2);
        if (order == 0) {
            radio.active = false;
            splitergoll_ble_auto_notify();
            tick(HOST_WINDOW_DURATION_MS - 1);
        }
        if (order == 1)
            radio.active = false;
        tick(HOST_WINDOW_DURATION_MS);
        assert(preference_writes == 1 && saved_preference.profile == 1 && !session.checkpoint);
        if (order == 2) {
            radio.active = false;
            splitergoll_ble_auto_notify();
            tick(HOST_WINDOW_DURATION_MS + 1);
        }
        splitergoll_ble_auto_notify();
        tick(HOST_WINDOW_DURATION_MS * 2);
        assert(preference_writes == 1 && activity_notes == notes &&
               radio.generation == generation && !radio.active);
    }
}

// Aucun rapport clavier, média ou modificateur en cours ne fuit vers la nouvelle cible.
static void reports_block_switch(void) {
    for (unsigned kind = 0; kind < 3; kind++) {
        reset(3);
        position(40, true, 1);
        position(43, true, 2);
        position(43, false, 3);
        position(40, false, 4);
        if (kind == 0)
            keyboard_report.body.data[3] = 4;
        if (kind == 1)
            consumer_report.body.data[1] = 1;
        if (kind == 2)
            explicit_mods = 2;
        tick(1000);
        assert(active == 0 && !preference_writes && splitergoll_ble_auto_busy());
        memset(&keyboard_report, 0, sizeof(keyboard_report));
        memset(&consumer_report, 0, sizeof(consumer_report));
        explicit_mods = 0;
        splitergoll_ble_auto_notify();
        tick(1001);
        assert(active == 1 && saved_preference.profile == 1 && preference_writes == 1);
    }
}

int main(void) {
    reports_block_switch();
    idle_at_checkpoint();
    failed_keymap_and_replaced_request();
    immediate_preference();
    failover_and_full_recovery();
    boot_restore_and_checkpoint();
    immediate_typing();
    old_release();
    arrival_and_timeout();
    invalidated_target();
    late_pairing();
    queued_hold();
    delayed_macro();
    native_failure();
#if defined(CONFIG_SPLITERGOLL_USB_PRIORITY)
    reset(3);
    priority_deadline = 15000;
    session.checkpoint = true;
    radio.opened_at_ms = clock_ms - (HOST_WINDOW_DURATION_MS - 1000);
    splitergoll_ble_auto_notify(); tick(clock_ms);
    assert(selection_work.queued && selection_work.due == clock_ms + 1000);
    priority_deadline = 0;
    reset(3);
    priority_deadline = 1000;
    session.checkpoint = true;
    splitergoll_ble_auto_notify(); tick(clock_ms);
    assert(selection_work.queued && selection_work.due == clock_ms + 1000);
#endif
    assert(!state_mutex.depth);
    puts("real BLE selector: immediate typing, releases, search, late pairing and cancellation OK");
}
