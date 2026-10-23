// Transport série partagé des diagnostics, indépendant des consoles et des logs ZMK.
#include "trace_console.h"
#if defined(CONFIG_SPLITERGOLL_POWER_TRACE)
#include <splitergoll/power_trace.h>
#endif
#if defined(CONFIG_SPLITERGOLL_BLE_TRACE)
#include "ble_trace_memory.h"
#include "ble_profile_control.h"
#endif
#include <string.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

static struct k_spinlock transmit_lock;
static const uint8_t *transmit_data;
static size_t transmit_length;
static size_t transmit_offset;
K_SEM_DEFINE(transmit_done, 0, 1);

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
bool trace_send_line(const struct device *uart, const char *line) {
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

// Attend la commande D du lecteur, dans un thread dédié sans bloquer le démarrage USB.
static void trace_console(void *unused1, void *unused2, void *unused3) {
    ARG_UNUSED(unused1);
    ARG_UNUSED(unused2);
    ARG_UNUSED(unused3);
#if defined(CONFIG_SPLITERGOLL_BLE_TRACE)
    const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(splitergoll_ble_trace_uart));
    struct ble_profile_frame frame = {0};
#else
    const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(splitergoll_usb_trace_uart));
#endif
    if (!device_is_ready(uart)) {
        return;
    }
    if (uart_irq_callback_user_data_set(uart, transmit_ready, NULL) != 0) {
        return;
    }

    while (true) {
        unsigned char command;
        if (uart_poll_in(uart, &command) == 0) {
#if defined(CONFIG_SPLITERGOLL_BLE_TRACE)
            enum ble_frame_result parsed = ble_profile_frame_feed(&frame, command);
            if (parsed == BLE_FRAME_READY) {
                splitergoll_ble_forget_command(uart, frame.bytes);
                continue;
            }
            if (parsed == BLE_FRAME_CONSUMED) continue;
#endif
            if (command == 'D') {
                splitergoll_trace_dump(uart);
            }
#if defined(CONFIG_SPLITERGOLL_POWER_TRACE)
            else if (command == 'P') {
                splitergoll_power_dump(uart);
            }
#endif
#if defined(CONFIG_SPLITERGOLL_BLE_TRACE)
            else if (command == 'M') {
                ble_trace_memory_dump(uart);
            }
            else if (command == 'S' || command == 'F' || (command >= '0' && command <= '9')) {
                splitergoll_ble_profile_command(uart, command);
            }
#endif
        }
        k_msleep(20);
    }
}

K_THREAD_DEFINE(trace_console_thread, 2048, trace_console, NULL, NULL, NULL, 10, 0, 0);
