// Acquisition initiale et diagnostic de mémoire, sans sélection de keymap.
#ifndef SPLITERGOLL_BLE_TRACE_MEMORY_H
#define SPLITERGOLL_BLE_TRACE_MEMORY_H
#include "ble_trace_store.h"
#include <zephyr/bluetooth/conn.h>
#include <zephyr/device.h>

// Lit uniquement l’état courant et la sauvegarde, sans apprendre ni modifier un appairage.
void ble_trace_memory_dump(const struct device *uart);
#endif
