#include "keyboard/src/connections/ble_reconnect.c"
#include <stdio.h>
static struct zmk_ble_profile profiles[5], persisted[5];
static bool saved[5];
static uint16_t bonds, links, allowed;
static unsigned selected, starts, stops, disconnects, writes, unpairs, acquisitions, adopts,
    split_pairs, split_completes;
static bool active, open_advertisement;
static int save_error, filter_error;
static bt_addr_le_t fresh_bond;
static const struct bt_conn_auth_cb *registered;
static struct bt_conn_auth_info_cb *registered_info;
static struct bt_conn mac = {.addr = {1}, .role = 1, .index = 0};
static struct bt_conn windows = {.addr = {2}, .role = 1, .index = 1};
static struct bt_conn stranger = {.addr = {8}, .role = 1, .index = 2};
static struct bt_conn other = {.addr = {7}, .role = 1, .index = 3};
static struct bt_conn right = {.addr = {9}, .role = 0, .index = 4};
void splitergoll_ble_auto_lock(void) {
    assert(!spin_depth);
    auto_depth++;
}
void splitergoll_ble_auto_unlock(void) {
    assert(auto_depth);
    auto_depth--;
}
unsigned bt_conn_index(struct bt_conn *c) {
    return c->index;
}
struct bt_conn *bt_conn_ref(struct bt_conn *c) {
    c->refs++;
    return c;
}
void bt_conn_unref(struct bt_conn *c) {
    assert(c->refs);
    c->refs--;
}
int bt_conn_get_info(struct bt_conn *c, struct bt_conn_info *i) {
    *i = (struct bt_conn_info){0, c->role};
    return 0;
}
const bt_addr_le_t *bt_conn_get_dst(struct bt_conn *c) {
    return &c->addr;
}
int zmk_ble_profile_index(const bt_addr_le_t *a) {
    for (int i = 0; i < 5; i++)
        if (a->value && a->value == profiles[i].peer.value)
            return i;
    return -1;
}
bt_addr_le_t *zmk_ble_profile_address(uint8_t p) {
    assert(p < 5);
    return &profiles[p].peer;
}
bool zmk_ble_profile_is_open(uint8_t p) {
    return !profiles[p].peer.value;
}
bool zmk_ble_profile_is_connected(uint8_t p) {
    return !!(links & (1u << p));
}
int zmk_ble_active_profile_index(void) {
    return selected;
}
void bt_foreach_bond(uint8_t id, void (*cb)(const struct bt_bond_info *, void *), void *ctx) {
    assert(id == 0);
    for (unsigned i = 0; i < 5; i++)
        if (bonds & (1u << i)) {
            struct bt_bond_info b = {profiles[i].peer};
            cb(&b, ctx);
        }
    if (fresh_bond.value) {
        struct bt_bond_info b = {fresh_bond};
        cb(&b, ctx);
    }
}
int bt_le_filter_accept_list_clear(void) {
    assert(!active && !spin_depth && !auto_depth);
    allowed = 0;
    return filter_error;
}
int bt_le_filter_accept_list_add(const bt_addr_le_t *a) {
    assert(!active && !spin_depth && !auto_depth);
    int p = zmk_ble_profile_index(a);
    assert(p >= 0 && (bonds & (1u << p)));
    allowed |= 1u << p;
    return 0;
}
int __real_bt_le_adv_stop(void) {
    assert(!spin_depth && !auto_depth);
    stops++;
    active = false;
    return 0;
}
int __real_bt_le_adv_start(const struct bt_le_adv_param *p, const struct bt_data *ad, size_t n,
                           const struct bt_data *sd, size_t sn) {
    assert(!active && !spin_depth && !auto_depth);
    unsigned flags = p->options & (128u | 64u);
    assert(flags == 0 || flags == (128u | 64u));
    open_advertisement = !flags;
    assert(open_advertisement || allowed);
    assert(n == 1 && ad[0].data[0] == 0x42 && !sd && !sn);
    starts++;
    active = true;
    return 0;
}
int __real_bt_conn_auth_cb_register(const struct bt_conn_auth_cb *c) {
    registered = c;
    return 0;
}
int __real_bt_conn_auth_info_cb_register(struct bt_conn_auth_info_cb *c) {
    if (c && c->pairing_complete)
        registered_info = c;
    return c ? 0 : -EINVAL;
}
int settings_save_one(const char *name, const void *v, size_t n) {
    assert(!spin_depth && auto_depth && n == sizeof(struct zmk_ble_profile));
    unsigned p;
    assert(sscanf(name, "ble/profiles/%u", &p) == 1 && p < 5);
    assert(zmk_ble_profile_is_open(p));
    writes++;
    if (save_error)
        return save_error;
    persisted[p] = *(const struct zmk_ble_profile *)v;
    saved[p] = true;
    return 0;
}
void set_profile_address(uint8_t p, const bt_addr_le_t *a) {
    assert(auto_depth && saved[p] && persisted[p].peer.value == a->value);
    profiles[p].peer = *a;
    bonds |= 1u << p;
    links |= 1u << p;
    fresh_bond.value = 0;
}
void splitergoll_ble_acquisition_paired(struct bt_conn *c, bool b) {
    assert(b && zmk_ble_profile_index(&c->addr) >= 0);
    acquisitions++;
}
void splitergoll_ble_auto_adopt_device(unsigned p, unsigned origin, uint32_t started_at_ms) {
    (void)started_at_ms;
    assert(p < 5);
    adopts++;
    if (selected == origin && links == (1u << p))
        selected = p;
}
int bt_conn_auth_cancel(struct bt_conn *c) {
    assert(!spin_depth && !auto_depth);
    registered_info->pairing_failed(c, BT_SECURITY_ERR_PAIR_NOT_ALLOWED);
    return 0;
}
int bt_conn_disconnect(struct bt_conn *c, uint8_t reason) {
    assert(c->role == 1 && reason == 19 && !spin_depth && !auto_depth);
    disconnects++;
    int p = zmk_ble_profile_index(&c->addr);
    if (p >= 0)
        links &= ~(1u << p);
    disconnected(c, reason);
    return 0;
}
int bt_unpair(uint8_t id, const bt_addr_le_t *a) {
    assert(id == 0 && zmk_ble_profile_index(a) < 0 && !spin_depth && !auto_depth);
    unpairs++;
    if (fresh_bond.value == a->value)
        fresh_bond.value = 0;
    return 0;
}
static enum bt_security_err native_pair(struct bt_conn *c,
                                        const struct bt_conn_pairing_feat *const f) {
    (void)c;
    (void)f;
    split_pairs++;
    return 0;
}
static void native_complete(struct bt_conn *c, bool bonded) {
    assert(c->role == 0 && bonded);
    split_completes++;
}
static void tick(uint32_t now) {
    clock_ms = now;
    for (unsigned n = 0; n < 40; n++) {
        if (!reconnect_work.queued || (int32_t)(clock_ms - reconnect_work.due) < 0)
            return;
        reconnect_work.queued = false;
        reconcile(&reconnect_work.work);
    }
    assert(!"work loop");
}
static void boot(uint32_t now, bool known) {
    assert(!pairing.conn && !mac.refs && !windows.refs && !stranger.refs && !other.refs);
    activity_state = ZMK_ACTIVITY_ACTIVE;
    clock_ms = now;
    native = (struct native_advertisement){0};
    discovery = (struct host_discovery_policy){0};
    status = (struct ble_reconnect_status){0};
    radio_state = (struct ble_reconnect_state){0};
    memset(profiles, 0, sizeof(profiles));
    memset(saved, 0, sizeof(saved));
    if (known) {
        profiles[0].peer.value = 1;
        profiles[1].peer.value = 2;
    }
    bonds = known ? 3 : 0;
    links = 0;
    selected = 0;
    revision += 100;
    active = false;
    starts = stops = disconnects = writes = unpairs = acquisitions = adopts = 0;
    save_error = filter_error = 0;
    fresh_bond.value = 0;
    reconnect_work.queued = false;
    pairing_error = 0;
    struct bt_le_adv_param p = {.options = 3};
    uint8_t byte = 0x42;
    struct bt_data ad = {.type = 1, .data_len = 1, .data = &byte};
    assert(__wrap_bt_le_adv_start(&p, &ad, 1, NULL, 0) == 0);
    tick(now);
    assert(active && open_advertisement);
}
static void connect_host(struct bt_conn *c, uint32_t now) {
    clock_ms = now;
    active = false;
    int p = zmk_ble_profile_index(&c->addr);
    if (p >= 0)
        links |= 1u << p;
    connected(c, 0);
    tick(now);
}

void splitergoll_ble_auto_notify(void) {
}
static void activity_to(enum zmk_activity_state value, uint32_t now) {
    clock_ms = now;
    activity_state = value;
    zmk_event_t event = {.activity = {value}};
    activity_changed(&event);
    tick(now);
}
int main(void) {
    struct bt_conn_auth_info_cb memory = {0};
    assert(__wrap_bt_conn_auth_info_cb_register(&memory) == 0);
    struct bt_conn_auth_cb auth = {.pairing_accept = native_pair};
    assert(__wrap_bt_conn_auth_cb_register(&auth) == 0);
    struct bt_conn_auth_info_cb info = {.pairing_complete = native_complete};
    assert(__wrap_bt_conn_auth_info_cb_register(&info) == 0);
    assert(registered->pairing_accept(&right, NULL) == 0);
    registered_info->pairing_complete(&right, true);
    assert(split_pairs == 1 && split_completes == 1);
    // Régression : le premier déconnecté revient après perte totale, sans refus ni reset.
    for (unsigned first = 0; first < 2; first++) {
        uint32_t start = first * 1000;
        boot(start, true);
        connect_host(&mac, start + 10);
        connect_host(&windows, start + 20);
        struct bt_conn *lost = first ? &mac : &windows, *last = first ? &windows : &mac;
        links = first ? 2 : 1;
        disconnected(lost, 19);
        tick(start + 30);
        assert(active && disconnects == 0);
        links = 0;
        disconnected(last, 19);
        tick(start + 40);
        assert(active && splitergoll_ble_reconnect_status().without_host);
        connect_host(lost, start + 50);
        assert(links == (first ? 1 : 2) && disconnects == 0);
        connect_host(last, start + 60);
        assert(links == 3 && disconnects == 0);
        // Un retour pendant la fenêtre est accepté, sans réarmer son échéance.
        uint32_t opened = splitergoll_ble_reconnect_status().opened_at_ms;
        links = first ? 2 : 1;
        disconnected(lost, 19);
        tick(start + 70);
        connect_host(lost, start + 80);
        assert(links == 3 && disconnects == 0);
        assert(splitergoll_ble_reconnect_status().opened_at_ms == opened);
        // Après fermeture, un hôte absent ne revient pas tant que l’autre reste connecté.
        tick(opened + HOST_WINDOW_DURATION_MS);
        assert(!active);
        clock_ms = opened + HOST_WINDOW_DURATION_MS + 1;
        links = first ? 2 : 1;
        disconnected(lost, 19);
        tick(clock_ms);
        assert(!active);
        connect_host(lost, clock_ms + 1);
        assert(links == (first ? 2 : 1) && disconnects == 1);
        // Le raccourci rouvre la fenêtre et le même connu est alors admis.
        splitergoll_ble_reconnect_open();
        tick(clock_ms);
        connect_host(lost, clock_ms + 1);
        assert(links == 3 && disconnects == 1);
        activity_to(ZMK_ACTIVITY_IDLE, clock_ms + 1);
        assert(active);
        activity_to(ZMK_ACTIVITY_SLEEP, clock_ms + 1);
        assert(!active);
    }
    boot(500000, true);
    tick(620000);
    activity_to(ZMK_ACTIVITY_IDLE, 620000);
    assert(!active);
    activity_to(ZMK_ACTIVITY_ACTIVE, 620001);
    assert(active);
    assert(!spin_depth && !auto_depth && !writes && !unpairs);
    puts("real radio adapter: partial/total loss in both orders, admission only by window, no "
         "reconnect loop, IDLE/SLEEP OK");
}
