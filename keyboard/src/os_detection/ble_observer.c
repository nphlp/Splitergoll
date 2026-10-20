// Observe les en-têtes publics des connexions hôtes, sans lire les frappes ou les clés.
#include <splitergoll/ble_observer.h>
#include <splitergoll/ble_acquisition.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/net_buf.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/atomic.h>
#include <zmk/ble.h>

BUILD_ASSERT(CONFIG_BT_MAX_CONN <= BLE_TRACE_CONNECTION_LIMIT);
static atomic_t epochs[BLE_TRACE_CONNECTION_LIMIT];

// La génération est locale à la connexion, sans adresse ni identifiant exporté.
uint32_t splitergoll_ble_connection_epoch(struct bt_conn *conn) {
    unsigned index = bt_conn_index(conn);
    return index < BLE_TRACE_CONNECTION_LIMIT ? (uint32_t)atomic_get(&epochs[index]) : 0;
}

// Le sélecteur refuse un ancien instantané tant que le worker n’a pas rejoint cette génération.
void splitergoll_ble_connection_changed(struct bt_conn *conn) {
    unsigned index = bt_conn_index(conn);
    if (index < BLE_TRACE_CONNECTION_LIMIT) atomic_inc(&epochs[index]);
}

// La moitié droite est une connexion centrale locale, distincte des ordinateurs hôtes.
static bool is_host(struct bt_conn *conn) {
    struct bt_conn_info info;
    return bt_conn_get_info(conn, &info) == 0 && info.type == BT_CONN_TYPE_LE &&
           info.role == BT_CONN_ROLE_PERIPHERAL;
}

// Aucun historique ni accès série en mode normal, le diagnostic remplace seulement ce hook.
__weak void splitergoll_ble_trace_event(struct bt_conn *conn, enum ble_trace_kind kind,
                                       const uint32_t data[5], uint16_t uuid, bool reliable) {
    ARG_UNUSED(conn); ARG_UNUSED(kind); ARG_UNUSED(data); ARG_UNUSED(uuid); ARG_UNUSED(reliable);
}

// Les deux consommateurs partagent les mêmes observations, sans modifier le paquet natif.
static void emit(struct bt_conn *conn, enum ble_trace_kind kind, const uint32_t data[5],
                  uint16_t uuid, bool reliable) {
    if (kind == BLE_TRACE_CONNECTED || kind == BLE_TRACE_DISCONNECTED || kind == BLE_TRACE_SECURITY) {
        splitergoll_ble_connection_changed(conn);
    }
    splitergoll_ble_acquisition_event(conn, kind, data, uuid, reliable);
    splitergoll_ble_trace_event(conn, kind, data, uuid, reliable);
}

// Copie seulement des métadonnées admises, sans appeler NVS dans les callbacks Bluetooth.
static void record(struct bt_conn *conn, enum ble_trace_kind kind,
                   uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    if (!is_host(conn)) return;
    const uint32_t data[5] = {a, b, c, d, e};
    emit(conn, kind, data, 0, true);
}

// Un hôte appairé ouvre une reconnexion, un hôte neuf attend encore pairing_complete.
static void connected(struct bt_conn *conn, uint8_t error) {
    struct bt_conn_info info;
    if (error || !is_host(conn) || bt_conn_get_info(conn, &info) != 0) return;
    int profile = zmk_ble_profile_index(bt_conn_get_dst(conn));
    if (profile < 0) profile = -1;
    const uint32_t data[5] = {info.le.interval, info.le.latency, info.le.timeout,
                             info.le.dst->type, (uint32_t)(profile + 1)};
    emit(conn, BLE_TRACE_CONNECTED, data, 0, true);
}

// Le moteur invalide la disponibilité de la connexion, sans effacer son OS mémorisé.
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    record(conn, BLE_TRACE_DISCONNECTED, reason, 0, 0, 0, 0);
}

// Observe les paramètres proposés, puis laisse les callbacks ZMK décider normalement.
static bool parameter_request(struct bt_conn *conn, struct bt_le_conn_param *parameters) {
    record(conn, BLE_TRACE_PARAM_REQUEST, parameters->interval_min, parameters->interval_max,
           parameters->latency, parameters->timeout, 0);
    return true;
}

// Conserve aussi les paramètres effectivement négociés, distincts de la proposition.
static void parameter_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency,
                               uint16_t timeout) {
    record(conn, BLE_TRACE_PARAM_UPDATED, interval, latency, timeout, 0, 0);
}

// Note le niveau de sécurité et l’erreur, sans lire les clés ou modifier l’appairage.
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err error) {
    record(conn, BLE_TRACE_SECURITY, level, error, 0, 0, 0);
}

// Observe les fonctionnalités déjà échangées par Zephyr, sans activer de requête de version.
#if defined(CONFIG_BT_REMOTE_INFO)
static void remote_info(struct bt_conn *conn, struct bt_conn_remote_info *info) {
    if (info->type == BT_CONN_TYPE_LE && info->le.features != NULL) {
        record(conn, BLE_TRACE_FEATURES, sys_get_le32(info->le.features),
               sys_get_le32(info->le.features + 4), 0, 0, 0);
    }
}

#endif

BT_CONN_CB_DEFINE(splitergoll_ble_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .le_param_req = parameter_request,
    .le_param_updated = parameter_updated,
    .security_changed = security_changed,
#if defined(CONFIG_BT_REMOTE_INFO)
    .remote_info_available = remote_info,
#endif
};

// Enregistre le MTU négocié dans les deux directions, sans lancer d’échange supplémentaire.
static void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx) {
    record(conn, BLE_TRACE_MTU, tx, rx, 0, 0, 0);
}

// Déclare le callback GATT avant l’initialisation Bluetooth de ZMK.
static int trace_init(void) {
    static struct bt_gatt_cb callbacks = {.att_mtu_updated = mtu_updated};
    bt_gatt_cb_register(&callbacks);
    return 0;
}

SYS_INIT(trace_init, APPLICATION, 0);


// Signature interne de Zephyr 4.1 vérifiée au build, sans LTO ni modification amont.
void __real_bt_l2cap_recv(struct bt_conn *conn, struct net_buf *buffer, bool complete);

// Observe avant le traitement natif et lui délègue exactement une fois le tampon intact.
void __wrap_bt_l2cap_recv(struct bt_conn *conn, struct net_buf *buffer, bool complete) {
    if (is_host(conn)) {
        struct ble_trace_record event = {.kind = BLE_TRACE_ATT};
        int decoded = ble_trace_decode(buffer->data, buffer->len, complete, event.data);
        if (decoded != 0) {
            bool reliable = decoded > 0;
            uint16_t uuid = reliable ? splitergoll_ble_attribute_uuid(&event, &reliable) : 0;
            splitergoll_ble_acquisition_event(conn, BLE_TRACE_ATT, event.data, uuid, reliable);
            // Une résolution UUID impossible invalide l’hypothèse, pas l’en-tête brut observé.
            splitergoll_ble_trace_event(conn, BLE_TRACE_ATT, event.data, uuid, decoded > 0);
        }
    }
    __real_bt_l2cap_recv(conn, buffer, complete);
}

struct attribute_lookup {
    uint16_t uuid;
    bool found;
};

// Lit le type de l’attribut local sans demander sa valeur au pilote GATT.
static uint8_t lookup_attribute(const struct bt_gatt_attr *attribute, uint16_t handle, void *context) {
    ARG_UNUSED(handle);
    struct attribute_lookup *lookup = context;
    if (attribute->uuid != NULL) {
        lookup->found = true;
        if (attribute->uuid->type == BT_UUID_TYPE_16) {
            lookup->uuid = BT_UUID_16(attribute->uuid)->val;
        }
    }
    return BT_GATT_ITER_STOP;
}

// Résout uniquement les UUID publics des lectures/écritures directes du clavier.
uint16_t splitergoll_ble_attribute_uuid(const struct ble_trace_record *event, bool *reliable) {
    if (event->kind != BLE_TRACE_ATT || (event->data[0] != 0x0a && event->data[0] != 0x12)) {
        return 0;
    }
    uint32_t handle = event->data[2];
    if (handle == 0 || handle > UINT16_MAX) {
        *reliable = false;
        return 0;
    }
    struct attribute_lookup lookup = {0};
    bt_gatt_foreach_attr((uint16_t)handle, (uint16_t)handle, lookup_attribute, &lookup);
    if (!lookup.found) {
        *reliable = false;
    }
    return lookup.uuid;
}
