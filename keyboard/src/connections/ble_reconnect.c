// Arbitre unique des annonces hôtes de bluetooth auto, sans changer les profils ZMK.
#include <splitergoll/ble_reconnect.h>
#include <splitergoll/ble_reconnect_state.h>
#include <splitergoll/host_window_policy.h>
#include <splitergoll/ble_auto.h>
#include <errno.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/ble.h>

LOG_MODULE_REGISTER(splitergoll_ble_reconnect, CONFIG_ZMK_LOG_LEVEL);
BUILD_ASSERT(!IS_ENABLED(CONFIG_BT_EXT_ADV) && !IS_ENABLED(CONFIG_LTO));
BUILD_ASSERT(IS_ENABLED(CONFIG_BT_FILTER_ACCEPT_LIST) && IS_ENABLED(CONFIG_BT_SMP_APP_PAIRING_ACCEPT));
BUILD_ASSERT(CONFIG_BT_CTLR_FAL_SIZE >= ZMK_BLE_PROFILE_COUNT);
BUILD_ASSERT(CONFIG_BT_CTLR_RL_SIZE >= CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ZMK_BLE_PROFILE_COUNT <= 16);

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
static struct host_window_policy window;
static struct ble_reconnect_status status;
static struct k_spinlock control_lock;
static uint32_t revision;
static struct ble_reconnect_state radio_state;
static struct bt_conn_auth_cb native_auth;
static struct bt_conn_auth_cb reconnect_auth;
static bool auth_registered;

static void reconcile(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(reconnect_work, reconcile);

int __real_bt_le_adv_start(const struct bt_le_adv_param *parameters,
                           const struct bt_data *ad, size_t count,
                           const struct bt_data *sd, size_t sd_count);
int __real_bt_le_adv_stop(void);
int __real_bt_conn_auth_cb_register(const struct bt_conn_auth_cb *callbacks);

// Réveille un seul travail sans faire de commande HCI depuis un callback Bluetooth.
static void notify(void) { k_work_reschedule(&reconnect_work, K_NO_WAIT); }

// Le slot sélectionné ne sert jamais de variable temporaire pour rechercher un autre hôte.
void splitergoll_ble_reconnect_open(void) {
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    if (status.ready) {
        host_window_start(&window, k_uptime_get_32());
        revision++;
    }
    k_spin_unlock(&control_lock, key);
    notify();
}

// Le temps restant est calculé à la lecture, sans réarmer la fenêtre ni déclencher la radio.
struct ble_reconnect_status splitergoll_ble_reconnect_status(void) {
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    struct ble_reconnect_status snapshot = status;
    snapshot.remaining_ms = status.ready ? host_window_remaining(&window, k_uptime_get_32()) : 0;
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
        host_window_start(&window, k_uptime_get_32());
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

// Ce lot réutilise exclusivement les clés existantes, sans réappairage même sur un slot libre actif.
static enum bt_security_err pairing_accept(struct bt_conn *conn,
                                           const struct bt_conn_pairing_feat *const features) {
    struct bt_conn_info info;
    if (bt_conn_get_info(conn, &info) != 0) return BT_SECURITY_ERR_UNSPECIFIED;
    if (info.role == BT_CONN_ROLE_PERIPHERAL) return BT_SECURITY_ERR_PAIR_NOT_ALLOWED;
    return native_auth.pairing_accept != NULL ? native_auth.pairing_accept(conn, features) :
                                               BT_SECURITY_ERR_SUCCESS;
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

// Un nouvel hôte consomme les annonces, puis le travail reprend pour les connus encore absents.
static void connected(struct bt_conn *conn, uint8_t error) {
    if (!is_host(conn)) return;
    if (error == 0) {
        struct ble_reconnect_status snapshot = splitergoll_ble_reconnect_status();
        int profile = zmk_ble_profile_index(bt_conn_get_dst(conn));
        bool allowed = ble_reconnect_accept(snapshot.remaining_ms != 0, profile >= 0,
                                              profile >= 0 && bonded(bt_conn_get_dst(conn)));
        if (!allowed) {
            int result = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            LOG_WRN("BLE reconnect refused: known=%u window=%u error=%d",
                    profile >= 0, snapshot.remaining_ms != 0, result);
        }
    }
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    revision++;
    k_spin_unlock(&control_lock, key);
    notify();
}

// Une déconnexion ne réarme jamais dix secondes et ne supprime aucun appairage.
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    ARG_UNUSED(reason);
    if (!is_host(conn)) return;
    k_spinlock_key_t key = k_spin_lock(&control_lock);
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
};

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
                   host_window_remaining(&window, k_uptime_get_32()) != 0;
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
    advertisement->parameters.options |= BT_LE_ADV_OPT_FILTER_CONN | BT_LE_ADV_OPT_FILTER_SCAN_REQ;
    return __real_bt_le_adv_start(&advertisement->parameters, ad, advertisement->count, NULL, 0);
}

// Un seul travail exécute HCI, sans verrou attendu depuis les callbacks Bluetooth.
static void reconcile(struct k_work *work) {
    static unsigned retries;
    static uint32_t retry_revision;
    static uint32_t retry_at_ms;
    struct radio_request request;
    k_spinlock_key_t key = k_spin_lock(&control_lock);
    if (!status.ready) {
        k_spin_unlock(&control_lock, key);
        return;
    }
    request.advertisement = native;
    request.revision = revision;
    uint32_t remaining = host_window_remaining(&window, k_uptime_get_32());
    k_spin_unlock(&control_lock, key);

    if (retry_revision == request.revision && radio_state.error != 0 &&
        radio_state.error != -EAGAIN && remaining != 0) {
        uint32_t retry_wait = retry_at_ms - k_uptime_get_32();
        if (retries > RADIO_RETRIES || (retry_wait > 0 && retry_wait <= RADIO_RETRY_MS)) {
            uint32_t delay = retries > RADIO_RETRIES ? remaining : MIN(remaining, retry_wait);
            k_work_schedule(k_work_delayable_from_work(work), K_MSEC(delay));
            return;
        }
    }

    struct ble_reconnect_view view = {.count = ZMK_BLE_PROFILE_COUNT,
        .revision = request.revision, .window_open = remaining != 0};
    splitergoll_ble_auto_lock();
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        bt_addr_le_copy(&request.peers[i], zmk_ble_profile_address(i));
        if (!zmk_ble_profile_is_open(i) && bonded(&request.peers[i])) {
            view.known |= UINT16_C(1) << i;
            if (zmk_ble_profile_is_connected(i)) view.connected |= UINT16_C(1) << i;
        }
    }
    splitergoll_ble_auto_unlock();
    const struct ble_reconnect_ops ops = {&request, stop_radio, filter_radio, start_radio, request_current};
    int error = ble_reconnect_apply(&radio_state, view, &ops);
    key = k_spin_lock(&control_lock);
    bool changed = status.advertising != radio_state.advertising || status.known != view.known ||
                   status.connected != view.connected || status.error != error;
    status.advertising = radio_state.advertising;
    status.known = view.known;
    status.connected = view.connected;
    status.error = error == -EAGAIN ? 0 : error;
    remaining = host_window_remaining(&window, k_uptime_get_32());
    bool stale = revision != request.revision;
    k_spin_unlock(&control_lock, key);
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
    } else if (remaining != 0) {
        k_work_schedule(k_work_delayable_from_work(work), K_MSEC(remaining));
    }
}
