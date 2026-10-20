// Présente la mémoire du clavier sur le port de diagnostic, sans apprentissage à la lecture.
#include "ble_trace_memory.h"
#include "trace_console.h"
#include <splitergoll/ble_acquisition.h>
#include <stdio.h>
#include <zmk/ble.h>

// Ne restitue ni empreinte, ni adresse, ni clé. La lecture ne déclenche aucune sauvegarde.
void ble_trace_memory_dump(const struct device *uart) {
    struct ble_os_memory snapshots[ZMK_BLE_PROFILE_COUNT];
    enum ble_os_guess observations[ZMK_BLE_PROFILE_COUNT];
    bool acquisitions[ZMK_BLE_PROFILE_COUNT];
    unsigned dropped;
    splitergoll_ble_acquisition_status(snapshots, observations, acquisitions, &dropped);
    char line[240];
    snprintf(line, sizeof(line), "BLEMEM BEGIN version=1 count=%u dropped=%u\n",
             ZMK_BLE_PROFILE_COUNT, dropped);
    if (!trace_send_line(uart, line)) return;
    for (unsigned i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        const struct ble_os_memory *memory = &snapshots[i];
        const char *state = memory->error ? "error" : !memory->present ? "empty" :
                            !memory->verified ? "unverified" : memory->dirty ? "pending" : "saved";
        const char *origin = memory->origin == BLE_MEMORY_MANUAL ? "manual" :
                             memory->origin == BLE_MEMORY_INITIAL ? "initial" : "none";
        enum ble_memory_os os = memory->verified && !memory->dirty && !memory->error ?
                                memory->os : BLE_MEMORY_UNKNOWN;
        snprintf(line, sizeof(line), "BLEMEM SLOT index=%u measured=%s stored=%s origin=%s "
                 "state=%s learning=%u evidence=%04x error=%d\n", i,
                 ble_os_name(observations[i]), ble_memory_os_name(os), origin, state,
                 acquisitions[i], memory->evidence, memory->error);
        if (!trace_send_line(uart, line)) return;
    }
    trace_send_line(uart, "BLEMEM END\n");
}
