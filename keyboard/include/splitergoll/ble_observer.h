// Observation passive de la pile figée, sans historique permanent en fonctionnement normal.
#ifndef SPLITERGOLL_BLE_OBSERVER_H
#define SPLITERGOLL_BLE_OBSERVER_H
#include <splitergoll/ble_events.h>
#include <zephyr/bluetooth/conn.h>

// Résout seulement les UUID publics des handles, jamais les valeurs d’attribut.
uint16_t splitergoll_ble_attribute_uuid(const struct ble_trace_record *event, bool *reliable);

// Invalide immédiatement la session utilisable, avant le traitement différé de sa mémoire.
void splitergoll_ble_connection_changed(struct bt_conn *conn);

// Identifie l’état radio courant même si le worker n’a pas encore traité son événement.
uint32_t splitergoll_ble_connection_epoch(struct bt_conn *conn);

// Le diagnostic optionnel reçoit les mêmes métadonnées que le moteur de production.
void splitergoll_ble_trace_event(struct bt_conn *conn, enum ble_trace_kind kind,
                                const uint32_t data[5], uint16_t uuid, bool reliable);
#endif
