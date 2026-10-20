// Observe les SETUP de la centrale et restitue leur historique sur son port série USB.
// Les wrappers délèguent aux pilotes existants, sans modifier les réponses ni la keymap.
#include "usb_trace_store.h"
#include "trace_console.h"
#include <splitergoll/usb_os_classifier.h>
#include <splitergoll/usb_observer.h>
#include <splitergoll/usb_auto_state.h>

#include <stdio.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/usb/usb_dc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>

static struct usb_trace_store store;
static struct k_spinlock store_lock;

// Efface les compteurs du diagnostic au changement de cycle observé par le clavier.
void splitergoll_usb_trace_reset(void) {
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    usb_trace_reset(&store);
    k_spin_unlock(&store_lock, key);
}

// Copie un en-tête observé dans le tampon borné, sans interpréter son contenu HID.
void splitergoll_usb_trace_record(uint32_t timestamp_ms, const uint8_t *data, size_t length) {
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    usb_trace_append(&store, timestamp_ms, data, length);
    k_spin_unlock(&store_lock, key);
}

// Restitue un instantané des premiers en-têtes, sans effacer le stockage après lecture.
void splitergoll_trace_dump(const struct device *uart) {
    char line[160];
    struct usb_os_classifier classifier = {0};
    k_spinlock_key_t key = k_spin_lock(&store_lock);
    size_t count = store.count;
    uint32_t dropped = store.dropped;
    uint32_t rejected = store.rejected;
    uint32_t generation = store.generation;
    uint32_t snapshot_ms = k_uptime_get_32();
    k_spin_unlock(&store_lock, key);

    snprintf(line, sizeof(line),
             "USBTRACE BEGIN version=1 side=left count=%u dropped=%u rejected=%u uptime_ms=%u\n",
             (unsigned)count, dropped, rejected, snapshot_ms);
    if (!trace_send_line(uart, line)) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        struct usb_trace_record record;
        key = k_spin_lock(&store_lock);
        bool found = usb_trace_get_generation(&store, generation, i, &record);
        k_spin_unlock(&store_lock, key);
        if (!found) {
            return;
        }

        usb_os_observe(&classifier, record.setup);

        snprintf(line, sizeof(line), "USBTRACE SETUP %u %u %02x %02x %04x %04x %04x\n",
                 (unsigned)i, record.timestamp_ms, record.setup[0], record.setup[1],
                 sys_get_le16(&record.setup[2]), sys_get_le16(&record.setup[4]),
                 sys_get_le16(&record.setup[6]));
        if (!trace_send_line(uart, line)) {
            return;
        }
    }

    key = k_spin_lock(&store_lock);
    bool same_generation = store.generation == generation;
    k_spin_unlock(&store_lock, key);
    if (!same_generation) {
        return;
    }
    enum usb_os_guess guess = usb_os_result(&classifier, dropped == 0 && rejected == 0);
    snprintf(line, sizeof(line), "USBTRACE OS result=%s evidence=%04x\n",
             usb_os_name(guess), (unsigned)classifier.evidence);
    if (!trace_send_line(uart, line)) {
        return;
    }
    if (IS_ENABLED(CONFIG_SPLITERGOLL_OS_AUTO)) {
        snprintf(line, sizeof(line), "USBTRACE ACTIVE result=%s\n",
                 usb_os_name(splitergoll_usb_auto_selected()));
        if (!trace_send_line(uart, line)) {
            return;
        }
    }
    key = k_spin_lock(&store_lock);
    same_generation = store.generation == generation;
    k_spin_unlock(&store_lock, key);
    if (same_generation) {
        trace_send_line(uart, "USBTRACE END\n");
    }
}
