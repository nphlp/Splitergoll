// Observe les SETUP de la centrale et restitue leur historique sur son port série USB.
// Les wrappers délèguent aux pilotes existants, sans modifier les réponses ni la keymap.
#include "usb_trace_store.h"
#include "usb_os_classifier.h"

#include <stdio.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/usb/usb_dc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>

static struct usb_trace_store store;
static struct k_spinlock store_lock;
static usb_dc_ep_callback setup_callback;
static usb_dc_status_callback status_callback;
static bool reading_setup;
static struct k_spinlock transmit_lock;
static const uint8_t *transmit_data;
static size_t transmit_length;
static size_t transmit_offset;
K_SEM_DEFINE(transmit_done, 0, 1);

// Points d’entrée réels du pilote, conservés par les redirections de l’éditeur de liens.
int __real_usb_dc_ep_set_callback(uint8_t ep, usb_dc_ep_callback callback);
int __real_usb_dc_ep_read(uint8_t ep, uint8_t *data, uint32_t size, uint32_t *read_bytes);
void __real_usb_dc_set_status_callback(usb_dc_status_callback callback);

// Sépare les énumérations USB sans modifier les événements transmis à Zephyr.
static void observe_status(enum usb_dc_status_code status, const uint8_t *param) {
    if (status == USB_DC_RESET || status == USB_DC_DISCONNECTED) {
        k_spinlock_key_t key = k_spin_lock(&store_lock);
        usb_trace_reset(&store);
        k_spin_unlock(&store_lock, key);
    }
    usb_dc_status_callback callback = status_callback;
    if (callback != NULL) {
        callback(status, param);
    }
}

// Ajoute l’observation des cycles au callback natif, en conservant sa désinscription.
void __wrap_usb_dc_set_status_callback(usb_dc_status_callback callback) {
    status_callback = callback;
    __real_usb_dc_set_status_callback(callback == NULL ? NULL : observe_status);
}

// Encadre le callback original pour distinguer un SETUP des données de contrôle.
static void observe_endpoint(uint8_t ep, enum usb_dc_ep_cb_status_code status) {
    reading_setup = status == USB_DC_EP_SETUP;
    setup_callback(ep, status);
    reading_setup = false;
}

// Observe seulement l’endpoint zéro OUT, sans changer les autres callbacks ni les erreurs.
int __wrap_usb_dc_ep_set_callback(uint8_t ep, usb_dc_ep_callback callback) {
    if (ep != 0 || callback == NULL) {
        return __real_usb_dc_ep_set_callback(ep, callback);
    }

    usb_dc_ep_callback previous = setup_callback;
    setup_callback = callback;
    int result = __real_usb_dc_ep_set_callback(ep, observe_endpoint);
    if (result != 0) {
        setup_callback = previous;
    }
    return result;
}

// Copie l’en-tête déjà lu par le pilote, sans seconde lecture ni capture de payload.
int __wrap_usb_dc_ep_read(uint8_t ep, uint8_t *data, uint32_t size, uint32_t *read_bytes) {
    int result = __real_usb_dc_ep_read(ep, data, size, read_bytes);

    if (reading_setup && ep == 0) {
        size_t length = result == 0 ? (read_bytes == NULL ? size : *read_bytes) : 0;
        k_spinlock_key_t key = k_spin_lock(&store_lock);
        usb_trace_append(&store, k_uptime_get_32(), data, length);
        k_spin_unlock(&store_lock, key);
    }
    return result;
}

// Alimente le FIFO dans le callback UART et signale la copie complète de la ligne.
static void transmit_ready(const struct device *uart, void *unused) {
    ARG_UNUSED(unused);
    if (!uart_irq_update(uart) || !uart_irq_tx_ready(uart)) {
        return;
    }

    k_spinlock_key_t key = k_spin_lock(&transmit_lock);
    if (transmit_offset < transmit_length) {
        int sent = uart_fifo_fill(uart, transmit_data + transmit_offset,
                                 transmit_length - transmit_offset);
        if (sent > 0) {
            transmit_offset += sent;
        }
    }
    bool complete = transmit_offset == transmit_length;
    if (complete) {
        uart_irq_tx_disable(uart);
        transmit_data = NULL;
    }
    k_spin_unlock(&transmit_lock, key);
    if (complete) {
        k_sem_give(&transmit_done);
    }
}

// Attend au plus une seconde la mise en file de la ligne, puis retire son pointeur.
static bool send_line(const struct device *uart, const char *line) {
    uint32_t dtr = 0;
    if (uart_line_ctrl_get(uart, UART_LINE_CTRL_DTR, &dtr) != 0 || !dtr) {
        return false;
    }

    k_sem_reset(&transmit_done);
    k_spinlock_key_t key = k_spin_lock(&transmit_lock);
    transmit_data = (const uint8_t *)line;
    transmit_length = strlen(line);
    transmit_offset = 0;
    k_spin_unlock(&transmit_lock, key);
    uart_irq_tx_enable(uart);

    bool complete = k_sem_take(&transmit_done, K_MSEC(1000)) == 0;
    key = k_spin_lock(&transmit_lock);
    uart_irq_tx_disable(uart);
    transmit_data = NULL;
    transmit_length = 0;
    transmit_offset = 0;
    k_spin_unlock(&transmit_lock, key);
    return complete;
}

// Restitue un instantané des premiers en-têtes, sans effacer le stockage après lecture.
static void dump_trace(const struct device *uart) {
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
    if (!send_line(uart, line)) {
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

        usb_os_observe(&classifier, record.setup, record.timestamp_ms);

        snprintf(line, sizeof(line), "USBTRACE SETUP %u %u %02x %02x %04x %04x %04x\n",
                 (unsigned)i, record.timestamp_ms, record.setup[0], record.setup[1],
                 sys_get_le16(&record.setup[2]), sys_get_le16(&record.setup[4]),
                 sys_get_le16(&record.setup[6]));
        if (!send_line(uart, line)) {
            return;
        }
    }

    key = k_spin_lock(&store_lock);
    bool same_generation = store.generation == generation;
    k_spin_unlock(&store_lock, key);
    if (!same_generation) {
        return;
    }
    enum usb_os_guess guess = usb_os_result(&classifier, snapshot_ms,
                                           dropped == 0 && rejected == 0);
    snprintf(line, sizeof(line), "USBTRACE OS result=%s evidence=%04x\n",
             usb_os_name(guess), (unsigned)classifier.evidence);
    if (!send_line(uart, line)) {
        return;
    }
    key = k_spin_lock(&store_lock);
    same_generation = store.generation == generation;
    k_spin_unlock(&store_lock, key);
    if (same_generation) {
        send_line(uart, "USBTRACE END\n");
    }
}

// Attend la commande D du lecteur, dans un thread dédié sans bloquer le démarrage USB.
static void trace_console(void *unused1, void *unused2, void *unused3) {
    ARG_UNUSED(unused1);
    ARG_UNUSED(unused2);
    ARG_UNUSED(unused3);
    const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(splitergoll_usb_trace_uart));
    if (!device_is_ready(uart)) {
        return;
    }
    if (uart_irq_callback_user_data_set(uart, transmit_ready, NULL) != 0) {
        return;
    }

    while (true) {
        unsigned char command;
        if (uart_poll_in(uart, &command) == 0 && command == 'D') {
            dump_trace(uart);
        }
        k_msleep(20);
    }
}

K_THREAD_DEFINE(usb_trace_console, 2048, trace_console, NULL, NULL, NULL, 10, 0, 0);
