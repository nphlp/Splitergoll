// Arbitre des annonces et nouveaux slots hôtes de bluetooth auto, sans détourner la saisie.
#include <splitergoll/ble_reconnect.h>
#include <splitergoll/ble_reconnect_state.h>
#include <splitergoll/ble_pairing_state.h>
#include <splitergoll/host_window_policy.h>
#include <splitergoll/ble_auto.h>
#include <splitergoll/ble_acquisition.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zmk/ble.h>
#include <zmk/activity.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>

LOG_MODULE_REGISTER(splitergoll_ble_reconnect, CONFIG_ZMK_LOG_LEVEL);
BUILD_ASSERT(!IS_ENABLED(CONFIG_BT_EXT_ADV) && !IS_ENABLED(CONFIG_LTO));
BUILD_ASSERT(IS_ENABLED(CONFIG_BT_FILTER_ACCEPT_LIST) && IS_ENABLED(CONFIG_BT_SMP_APP_PAIRING_ACCEPT));
BUILD_ASSERT(CONFIG_BT_CTLR_FAL_SIZE >= ZMK_BLE_PROFILE_COUNT);
BUILD_ASSERT(CONFIG_BT_CTLR_RL_SIZE >= CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ZMK_BLE_PROFILE_COUNT <= 16);
BUILD_ASSERT(!IS_ENABLED(CONFIG_BT_KEYS_OVERWRITE_OLDEST));
BUILD_ASSERT(!IS_ENABLED(CONFIG_BT_SMP_ALLOW_UNAUTH_OVERWRITE));
BUILD_ASSERT(!IS_ENABLED(CONFIG_BT_ID_ALLOW_UNAUTH_OVERWRITE));

#define ADV_FIELD_LIMIT 4
#define ADV_PAYLOAD_LIMIT 31
#define RADIO_RETRIES 3
#define RADIO_RETRY_MS 100

struct native_advertisement {
    struct bt_le_adv_param parameters;
    size_t count;
    uint8_t types[ADV_FIELD_LIMIT];
    uint8_t lengths[ADV_FIELD_LIMIT];
    uint8_t offsets[ADV_FIELD_LIMIT];
    uint8_t payload[ADV_PAYLOAD_LIMIT];
};

static struct native_advertisement native;
static struct host_discovery_policy discovery;
static struct ble_reconnect_status status;
static struct k_spinlock control_lock;
static uint32_t revision;
static struct ble_reconnect_state radio_state;
static struct bt_conn_auth_cb native_auth;
static struct bt_conn_auth_cb reconnect_auth;
static bool auth_registered;
static struct bt_conn_auth_info_cb native_info;
static struct bt_conn_auth_info_cb pairing_info;
static struct bt_conn_auth_info_cb *native_info_source;
static bool info_registered;
static int selected_profile;
static struct {
    struct ble_pairing_state policy;
    struct bt_conn *conn;
    bool disconnected;
    bool abort_sent;
    bool created_bond;
} pairing = {.policy = {.slot = -1, .origin = -1}};
static int pairing_error;

static void reconcile(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(reconnect_work, reconcile);

int __real_bt_le_adv_start(const struct bt_le_adv_param *parameters,
                           const struct bt_data *ad, size_t count,
                           const struct bt_data *sd, size_t sd_count);
int __real_bt_le_adv_stop(void);
int __real_bt_conn_auth_cb_register(const struct bt_conn_auth_cb *callbacks);
int __real_bt_conn_auth_info_cb_register(struct bt_conn_auth_info_cb *callbacks);

// Helper exporté par ble.c du commit ZMK figé, à recontrôler lors de toute migration.
void set_profile_address(uint8_t index, const bt_addr_le_t *address);

// Réveille un seul travail sans faire de commande HCI depuis un callback Bluetooth.
static void notify(void) { k_work_reschedule(&reconnect_work, K_NO_WAIT); }

// Le slot sélectionné ne sert jamais de variable temporaire pour rechercher un autre hôte.
void splitergoll_ble_reconnect_open(void) {
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    if (status.ready) {
        host_discovery_open(&discovery, k_uptime_get_32());
        revision++;
    }
    k_spin_unlock(&control_lock, key);
    notify();
}

// Le temps restant est calculé à la lecture, sans réarmer la fenêtre ni déclencher la radio.
struct ble_reconnect_status splitergoll_ble_reconnect_status(void) {
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    struct ble_reconnect_status snapshot = status;
    snapshot.remaining_ms = status.ready ? host_discovery_remaining(&discovery, k_uptime_get_32()) : 0;
    snapshot.generation = discovery.generation;
    snapshot.opened_at_ms = discovery.opened_at_ms;
    snapshot.active = discovery.awake;
    snapshot.without_host = discovery.awake && discovery.connected == 0;
    snapshot.pairing_slot = pairing.conn != NULL ? pairing.policy.slot : -1;
    snapshot.pairing_remaining_ms = ble_pairing_remaining(&pairing.policy, k_uptime_get_32(), snapshot.remaining_ms);
    snapshot.accepting_new = snapshot.ready && snapshot.remaining_ms != 0 &&
                             snapshot.free_slots != 0 && pairing.conn == NULL;
    if (pairing_error != 0) snapshot.error = pairing_error;
    k_spin_unlock(&control_lock, key);
    return snapshot;
}

// Copie les annonces natives figées, dont les données ne doivent pas rester sur une pile appelante.
static bool copy_advertisement(struct native_advertisement *copy,
                               const struct bt_le_adv_param *parameters,
                               const struct bt_data *ad, size_t count, size_t sd_count) {
    if (parameters == NULL || parameters->peer != NULL || parameters->id != BT_ID_DEFAULT ||
        (parameters->options & BT_LE_ADV_OPT_CONN) != BT_LE_ADV_OPT_CONN ||
        sd_count != 0 || count == 0 || count > ADV_FIELD_LIMIT || ad == NULL) return false;
    *copy = (struct native_advertisement){.parameters = *parameters, .count = count};
    size_t length = 0;
    for (size_t i = 0; i < count; i++) {
        if ((ad[i].data_len != 0 && ad[i].data == NULL) ||
            length + ad[i].data_len + 2 * (i + 1) > ADV_PAYLOAD_LIMIT) return false;
        copy->types[i] = ad[i].type;
        copy->lengths[i] = ad[i].data_len;
        copy->offsets[i] = (uint8_t)length;
        if (ad[i].data_len != 0) memcpy(copy->payload + length, ad[i].data, ad[i].data_len);
        length += ad[i].data_len;
    }
    return true;
}

// Les appels natifs deviennent des notifications, seul reconcile commande réellement les annonces.
int __wrap_bt_le_adv_start(const struct bt_le_adv_param *parameters,
                           const struct bt_data *ad, size_t count,
                           const struct bt_data *sd, size_t sd_count) {
    ARG_UNUSED(sd);
    struct native_advertisement copy;
    if (!copy_advertisement(&copy, parameters, ad, count, sd_count)) {
        LOG_ERR("Unsupported native BLE advertisement");
        return -ENOTSUP;
    }
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    native = copy;
    if (!status.ready) {
        host_discovery_start(&discovery, k_uptime_get_32());
        status.ready = true;
    }
    revision++;
    k_spin_unlock(&control_lock, key);
    notify();
    return 0;
}

// Une demande native d’arrêt ne ferme pas la fenêtre quand le premier ordinateur se connecte.
int __wrap_bt_le_adv_stop(void) {
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    revision++;
    k_spin_unlock(&control_lock, key);
    notify();
    return 0;
}

struct bond_lookup {
    const bt_addr_le_t *peer;
    bool found;
};

// Confirme un bond existant sans lire, copier ou exporter sa clé.
static void find_bond(const struct bt_bond_info *bond, void *context) {
    struct bond_lookup *lookup = context;
    if (bt_addr_le_eq(&bond->addr, lookup->peer)) lookup->found = true;
}

// Une adresse de profil seule ne suffit pas si son bond a été supprimé ou est absent.
static bool bonded(const bt_addr_le_t *peer) {
    struct bond_lookup lookup = {.peer = peer};
    bt_foreach_bond(BT_ID_DEFAULT, find_bond, &lookup);
    return lookup.found;
}

// Le filtre hôte ignore la connexion centrale utilisée pour la moitié droite.
static bool is_host(struct bt_conn *conn) {
    struct bt_conn_info info;
    return bt_conn_get_info(conn, &info) == 0 && info.type == BT_CONN_TYPE_LE &&
           info.role == BT_CONN_ROLE_PERIPHERAL;
}

// Seule la connexion qui possède un slot réservé peut commencer SMP dans la fenêtre.
static enum bt_security_err pairing_accept(struct bt_conn *conn,
                                           const struct bt_conn_pairing_feat *const features) {
    struct bt_conn_info info;
    if (bt_conn_get_info(conn, &info) != 0) return BT_SECURITY_ERR_UNSPECIFIED;
    if (info.role == BT_CONN_ROLE_PERIPHERAL) {
        if (zmk_ble_profile_index(bt_conn_get_dst(conn)) >= 0 || bonded(bt_conn_get_dst(conn))) {
            return BT_SECURITY_ERR_PAIR_NOT_ALLOWED;
        }
        k_spinlock_key_t key = k_spin_lock(&control_lock);
        bool accepted = pairing.conn == conn && ble_pairing_begin(&pairing.policy,
            (uint32_t)bt_conn_index(conn) + 1, k_uptime_get_32(),
            status.ready && host_discovery_remaining(&discovery, k_uptime_get_32()) != 0);
        if (accepted) revision++;
        k_spin_unlock(&control_lock, key);
        notify();
        return accepted ? BT_SECURITY_ERR_SUCCESS : BT_SECURITY_ERR_PAIR_NOT_ALLOWED;
    }
    return native_auth.pairing_accept != NULL ? native_auth.pairing_accept(conn, features) :
                                               BT_SECURITY_ERR_SUCCESS;
}

// Le callback ne touche ni à NVS ni au slot actif, l’attribution sera faite hors radio.
static void pairing_complete(struct bt_conn *conn, bool bonded_now) {
    if (!is_host(conn)) {
        if (native_info.pairing_complete != NULL) native_info.pairing_complete(conn, bonded_now);
        return;
    }
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    bool owned = pairing.conn == conn;
    if (owned) {
        pairing.created_bond |= bonded_now;
        if (pairing.policy.phase != BLE_PAIRING_COMPLETE &&
            !ble_pairing_complete(&pairing.policy, (uint32_t)bt_conn_index(conn) + 1,
                                   k_uptime_get_32(), bonded_now)) {
            pairing.policy.phase = BLE_PAIRING_ABORTED;
            if (pairing_error == 0) pairing_error = bonded_now ? -ETIMEDOUT : -EPROTO;
        }
        revision++;
    }
    k_spin_unlock(&control_lock, key);
    if (!owned) {
        LOG_WRN("BLE pairing completed without a reserved slot");
        bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    }
    notify();
}

// Un échec libérera sa seule réservation après déconnexion, jamais un bond existant.
static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason) {
    if (!is_host(conn)) {
        if (native_info.pairing_failed != NULL) native_info.pairing_failed(conn, reason);
        return;
    }
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    if (pairing.conn == conn) {
        pairing.policy.phase = BLE_PAIRING_ABORTED;
        if (pairing_error == 0) pairing_error = -EACCES;
        revision++;
    }
    k_spin_unlock(&control_lock, key);
    LOG_WRN("BLE pairing failed: error=%u", reason);
    notify();
}

// Remplace seulement le callback ZMK enregistré juste après son auth_cb dans le socle figé.
int __wrap_bt_conn_auth_info_cb_register(struct bt_conn_auth_info_cb *callbacks) {
    if (info_registered && callbacks == native_info_source) return -EALREADY;
    if (!auth_registered || info_registered || callbacks == NULL || callbacks->pairing_complete == NULL) {
        return __real_bt_conn_auth_info_cb_register(callbacks);
    }
    native_info = *callbacks;
    pairing_info = *callbacks;
    pairing_info.pairing_complete = pairing_complete;
    pairing_info.pairing_failed = pairing_failed;
    int result = __real_bt_conn_auth_info_cb_register(&pairing_info);
    if (result == 0) {
        info_registered = true;
        native_info_source = callbacks;
    }
    return result;
}

// Conserve les callbacks natifs et protège aussi la liaison entre les deux moitiés.
int __wrap_bt_conn_auth_cb_register(const struct bt_conn_auth_cb *callbacks) {
    if (callbacks == NULL) {
        int result = __real_bt_conn_auth_cb_register(NULL);
        if (result == 0) auth_registered = false;
        return result;
    }
    if (auth_registered) return -EALREADY;
    native_auth = *callbacks;
    reconnect_auth = *callbacks;
    reconnect_auth.pairing_accept = pairing_accept;
    int result = __real_bt_conn_auth_cb_register(&reconnect_auth);
    if (result == 0) auth_registered = true;
    return result;
}

// Réserve une place pour un inconnu, sans changer celle de l’ordinateur déjà sélectionné.
static void connected(struct bt_conn *conn, uint8_t error) {
    if (!is_host(conn)) return;
    if (error == 0) {
        int profile = zmk_ble_profile_index(bt_conn_get_dst(conn));
        bool has_bond = bonded(bt_conn_get_dst(conn));
        k_spinlock_key_t key = k_spin_lock(&control_lock);
        bool open = status.ready && host_discovery_remaining(&discovery, k_uptime_get_32()) != 0;
        bool allowed = ble_reconnect_accept(open, profile >= 0, has_bond);
        if (!allowed && profile < 0 && !has_bond && info_registered && open && pairing.conn == NULL) {
            int slot = ble_pairing_reserve(&pairing.policy, ZMK_BLE_PROFILE_COUNT,
                status.free_slots, selected_profile, (uint32_t)bt_conn_index(conn) + 1, open);
            if (slot >= 0) {
                pairing.conn = bt_conn_ref(conn);
                pairing_error = 0;
                allowed = true;
            }
        }
        k_spin_unlock(&control_lock, key);
        if (!allowed) {
            int result = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            LOG_WRN("BLE reconnect refused: known=%u window=%u error=%d",
                    profile >= 0, open, result);
        }
    }
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    revision++;
    k_spin_unlock(&control_lock, key);
    notify();
}

// Une perte partielle ne rouvre pas la fenêtre, la perte du dernier hôte permet la recherche active.
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    ARG_UNUSED(reason);
    if (!is_host(conn)) return;
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    if (pairing.conn == conn) pairing.disconnected = true;
    revision++;
    k_spin_unlock(&control_lock, key);
    notify();
}

// Une place libérée permet de reprendre après ENOMEM, seulement dans la fenêtre encore ouverte.
static void recycled(void) { notify(); }

BT_CONN_CB_DEFINE(splitergoll_ble_reconnect_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .recycled = recycled,
};

struct radio_request {
    struct native_advertisement advertisement;
    uint32_t revision;
    bt_addr_le_t peers[ZMK_BLE_PROFILE_COUNT];
    bool pairing_open;
};

// Persiste avant le helper ZMK void, sans remplacer une case qui aurait changé depuis sa réservation.
static int assign_pairing(struct bt_conn *conn, int slot, int origin, uint32_t started_at_ms) {
    const bt_addr_le_t *peer = bt_conn_get_dst(conn);
    splitergoll_ble_auto_lock();
    if (slot < 0 || slot >= ZMK_BLE_PROFILE_COUNT || !zmk_ble_profile_is_open(slot) ||
        zmk_ble_profile_index(peer) >= 0 || !bonded(peer)) {
        splitergoll_ble_auto_unlock();
        return -ESTALE;
    }
    const struct zmk_ble_profile *current = CONTAINER_OF(zmk_ble_profile_address(slot), struct zmk_ble_profile, peer);
    struct zmk_ble_profile saved = *current;
    bt_addr_le_copy(&saved.peer, peer);
    char key[24];
    snprintf(key, sizeof(key), "ble/profiles/%d", slot);
    int error = settings_save_one(key, &saved, sizeof(saved));
    if (error == 0) {
        // La seconde écriture identique du helper est dédupliquée par le backend NVS figé.
        set_profile_address(slot, peer);
        splitergoll_ble_acquisition_paired(conn, true);
        splitergoll_ble_auto_adopt_device((unsigned)slot, (unsigned)origin, started_at_ms);
    }
    splitergoll_ble_auto_unlock();
    return error;
}

// Détache notre référence seulement quand la transaction a été enregistrée ou sa connexion fermée.
static void release_pairing(struct bt_conn *conn) {
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    if (pairing.conn != conn) {
        k_spin_unlock(&control_lock, key);
        return;
    }
    memset(&pairing, 0, sizeof(pairing));
    ble_pairing_release(&pairing.policy);
    revision++;
    k_spin_unlock(&control_lock, key);
    bt_conn_unref(conn);
}

// Attribue les nouveaux bonds hors callback radio et borne les transactions qui ne se terminent pas.
static uint32_t process_pairing(void) {
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    struct bt_conn *conn = pairing.conn;
    struct ble_pairing_state policy = pairing.policy;
    bool gone = pairing.disconnected;
    bool created = pairing.created_bond;
    uint32_t now_ms = k_uptime_get_32();
    uint32_t window_remaining = host_discovery_remaining(&discovery, now_ms);
    bool expired = ble_pairing_expired(&policy, now_ms, window_remaining);
    k_spin_unlock(&control_lock, key);
    if (conn == NULL) return 0;

    if (policy.phase == BLE_PAIRING_COMPLETE) {
        int error = assign_pairing(conn, policy.slot, policy.origin, policy.started_at_ms);
        if (error == 0) {
            key = k_spin_lock(&control_lock);
            status.free_slots &= (uint16_t)~(UINT32_C(1) << policy.slot);
            k_spin_unlock(&control_lock, key);
            LOG_INF("BLE pairing saved: profile=%d", policy.slot);
            release_pairing(conn);
            return 0;
        }
        key = k_spin_lock(&control_lock);
        pairing.policy.phase = BLE_PAIRING_ABORTED;
        pairing_error = error;
        k_spin_unlock(&control_lock, key);
        LOG_ERR("BLE pairing profile save failed: profile=%d error=%d", policy.slot, error);
        expired = true;
    }

    if (gone) {
        // Nettoie seulement le bond neuf de notre transaction, jamais celui d’un slot connu.
        if (created && zmk_ble_profile_index(bt_conn_get_dst(conn)) < 0) {
            int error = bt_unpair(BT_ID_DEFAULT, bt_conn_get_dst(conn));
            if (error != 0) {
                key = k_spin_lock(&control_lock);
                pairing_error = error;
                k_spin_unlock(&control_lock, key);
                LOG_ERR("BLE incomplete pairing cleanup failed: error=%d", error);
            }
        }
        release_pairing(conn);
        return 0;
    }

    if (expired) {
        key = k_spin_lock(&control_lock);
        bool abort = !pairing.abort_sent;
        pairing.abort_sent = true;
        pairing.policy.phase = BLE_PAIRING_ABORTED;
        if (pairing_error == 0) pairing_error = -ETIMEDOUT;
        k_spin_unlock(&control_lock, key);
        if (abort) {
            bt_conn_auth_cancel(conn);
            int error = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            if (error == -ENOTCONN) {
                key = k_spin_lock(&control_lock);
                pairing.disconnected = true;
                k_spin_unlock(&control_lock, key);
                notify();
            } else if (error != 0) {
                key = k_spin_lock(&control_lock);
                pairing_error = error;
                k_spin_unlock(&control_lock, key);
                LOG_ERR("BLE pairing disconnect failed: error=%d", error);
            }
            LOG_INF("BLE pairing closed: profile=%d", policy.slot);
        }
        return 0;
    }
    return ble_pairing_remaining(&policy, now_ms, window_remaining);
}

// Le contrôleur legacy figé accepte cet arrêt même si la connexion a déjà consommé les annonces.
static int stop_radio(void *context) {
    ARG_UNUSED(context);
    return __real_bt_le_adv_stop();
}

// La liste est reconstruite uniquement radio arrêtée, sans toucher au scanner de la moitié droite.
static int filter_radio(void *context, uint16_t allowed) {
    struct radio_request *request = context;
    int error = bt_le_filter_accept_list_clear();
    if (error != 0) return error;
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        if (!(allowed & (UINT16_C(1) << i))) continue;
        error = bt_le_filter_accept_list_add(&request->peers[i]);
        if (error != 0) return error;
    }
    return 0;
}

// Aucune décision copiée avant une échéance ou un nouvel événement ne démarre tardivement.
static bool request_current(void *context) {
    struct radio_request *request = context;
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    bool current = status.ready && revision == request->revision &&
                   host_discovery_remaining(&discovery, k_uptime_get_32()) != 0;
    k_spin_unlock(&control_lock, key);
    return current;
}

// Réutilise les données et paramètres ZMK, en imposant le filtrage des connexions et scans.
static int start_radio(void *context) {
    struct radio_request *request = context;
    struct native_advertisement *advertisement = &request->advertisement;
    struct bt_data ad[ADV_FIELD_LIMIT];
    for (size_t i = 0; i < advertisement->count; i++) {
        ad[i] = (struct bt_data){.type = advertisement->types[i], .data_len = advertisement->lengths[i],
                                .data = advertisement->payload + advertisement->offsets[i]};
    }
    if (request->pairing_open) {
        advertisement->parameters.options &= ~(BT_LE_ADV_OPT_FILTER_CONN | BT_LE_ADV_OPT_FILTER_SCAN_REQ);
    } else {
        advertisement->parameters.options |= BT_LE_ADV_OPT_FILTER_CONN | BT_LE_ADV_OPT_FILTER_SCAN_REQ;
    }
    return __real_bt_le_adv_start(&advertisement->parameters, ad, advertisement->count, NULL, 0);
}

// Un seul travail exécute HCI, sans verrou attendu depuis les callbacks Bluetooth.
static void reconcile(struct k_work *work) {
    static unsigned retries;
    static uint32_t retry_revision;
    static uint32_t retry_at_ms;
    uint32_t pairing_delay = process_pairing();
    struct radio_request request;
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    if (!status.ready) {
        k_spin_unlock(&control_lock, key);
        return;
    }
    host_discovery_update(&discovery, k_uptime_get_32(), status.connected,
                           zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE);
    request.advertisement = native;
    request.revision = revision;
    uint32_t remaining = host_discovery_remaining(&discovery, k_uptime_get_32());
    k_spin_unlock(&control_lock, key);

    if (retry_revision == request.revision && radio_state.error != 0 &&
        radio_state.error != -EAGAIN && remaining != 0) {
        uint32_t retry_wait = retry_at_ms - k_uptime_get_32();
        if (retries > RADIO_RETRIES || (retry_wait > 0 && retry_wait <= RADIO_RETRY_MS)) {
            uint32_t delay = retries > RADIO_RETRIES ? remaining : MIN(remaining, retry_wait);
            if (pairing_delay != 0) delay = MIN(delay, pairing_delay);
            k_work_schedule(k_work_delayable_from_work(work), K_MSEC(delay));
            return;
        }
    }

    struct ble_reconnect_view view = {.count = ZMK_BLE_PROFILE_COUNT,
        .revision = request.revision, .window_open = remaining != 0};
    uint16_t free_slots = 0;
    splitergoll_ble_auto_lock();
    int selected = zmk_ble_active_profile_index();
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        bt_addr_le_copy(&request.peers[i], zmk_ble_profile_address(i));
        if (zmk_ble_profile_is_open(i)) free_slots |= UINT16_C(1) << i;
        if (!zmk_ble_profile_is_open(i) && bonded(&request.peers[i])) {
            view.known |= UINT16_C(1) << i;
            if (zmk_ble_profile_is_connected(i)) view.connected |= UINT16_C(1) << i;
        }
    }
    splitergoll_ble_auto_unlock();
    key = k_spin_lock(&control_lock);
    uint32_t previous_generation = discovery.generation;
    host_discovery_update(&discovery, k_uptime_get_32(), view.connected,
                           zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE);
    if (discovery.generation != previous_generation) {
        revision++;
        request.revision = revision;
        view.revision = revision;
    }
    remaining = host_discovery_remaining(&discovery, k_uptime_get_32());
    view.window_open = remaining != 0;
    status.free_slots = free_slots;
    selected_profile = selected;
    view.pairing_open = free_slots != 0 && pairing.conn == NULL && info_registered;
    request.pairing_open = view.pairing_open;
    k_spin_unlock(&control_lock, key);
    const struct ble_reconnect_ops ops = {&request, stop_radio, filter_radio, start_radio, request_current};
    int error = ble_reconnect_apply(&radio_state, view, &ops);
    key = k_spin_lock(&control_lock);
    bool changed = status.advertising != radio_state.advertising || status.known != view.known ||
                   status.connected != view.connected || status.error != error;
    status.advertising = radio_state.advertising;
    status.known = view.known;
    status.connected = view.connected;
    status.error = error == -EAGAIN ? 0 : error;
    remaining = host_discovery_remaining(&discovery, k_uptime_get_32());
    bool stale = revision != request.revision;
    bool without_host = discovery.awake && discovery.connected == 0;
    k_spin_unlock(&control_lock, key);
    splitergoll_ble_auto_notify();
    if (changed) LOG_INF("BLE reconnect: remaining_ms=%u advertising=%u known=%04x connected=%04x error=%d",
                         remaining, radio_state.advertising, view.known, view.connected, error);
    if (error != 0 && error != -EAGAIN) LOG_WRN("BLE reconnect radio failed: error=%d", error);
    if (retry_revision != request.revision) {
        retries = 0;
        retry_revision = request.revision;
    }
    if (stale) {
        notify();
    } else if (error != 0 && error != -EAGAIN && retries++ < RADIO_RETRIES) {
        retry_at_ms = k_uptime_get_32() + RADIO_RETRY_MS;
        k_work_schedule(k_work_delayable_from_work(work), K_MSEC(RADIO_RETRY_MS));
    } else {
        uint32_t delay = without_host ? 0 : remaining;
        if (pairing_delay != 0 && (delay == 0 || pairing_delay < delay)) delay = pairing_delay;
        if (delay != 0) k_work_schedule(k_work_delayable_from_work(work), K_MSEC(delay));
    }
}

// Les états d’activité ferment l’admission sans générer de frappe ni retarder le sommeil.
static int activity_changed(const zmk_event_t *event) {
    const struct zmk_activity_state_changed *activity = as_zmk_activity_state_changed(event);
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    host_discovery_update(&discovery, k_uptime_get_32(), status.connected,
                           activity->state == ZMK_ACTIVITY_ACTIVE);
    revision++;
    k_spin_unlock(&control_lock, key);
    notify();
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(splitergoll_ble_discovery, activity_changed);
ZMK_SUBSCRIPTION(splitergoll_ble_discovery, zmk_activity_state_changed);
