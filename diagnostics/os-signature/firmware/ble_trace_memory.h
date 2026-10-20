// Acquisition initiale et diagnostic de mémoire, sans sélection de keymap.
#ifndef SPLITERGOLL_BLE_TRACE_MEMORY_H
#define SPLITERGOLL_BLE_TRACE_MEMORY_H
#include "ble_trace_store.h"
#include <zephyr/bluetooth/conn.h>
#include <zephyr/device.h>

// Copie des métadonnées dans une file bornée, sans écriture flash dans le callback radio.
void ble_trace_memory_event(struct bt_conn *conn, enum ble_trace_kind kind,
                             const uint32_t data[5], uint16_t uuid, bool reliable);

// Lit uniquement l’état courant et la sauvegarde, sans apprendre ni modifier un appairage.
void ble_trace_memory_dump(const struct device *uart);
#endif
