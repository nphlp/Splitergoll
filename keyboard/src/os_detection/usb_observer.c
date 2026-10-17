// Encadre les callbacks du pilote, partagé entre auto et l’étude des signatures USB.
#include <splitergoll/usb_observer.h>
#include <zephyr/drivers/usb/usb_dc.h>
#include <zephyr/kernel.h>

static struct usb_os_classifier classifier;
static struct k_spinlock observer_lock;
static uint32_t generation;
static bool reliable = true;
static usb_dc_ep_callback setup_callback;
static usb_dc_status_callback status_callback;
static bool reading_setup;

// Points d’entrée natifs conservés par les redirections de l’éditeur de liens.
int __real_usb_dc_ep_set_callback(uint8_t ep, usb_dc_ep_callback callback);
int __real_usb_dc_ep_read(uint8_t ep, uint8_t *data, uint32_t size, uint32_t *read_bytes);
void __real_usb_dc_set_status_callback(usb_dc_status_callback callback);

// Invalide les indices de l’ancien hôte sans modifier les statuts transmis à Zephyr.
static void observe_status(enum usb_dc_status_code status, const uint8_t *param) {
    if (status == USB_DC_RESET || status == USB_DC_DISCONNECTED) {
        k_spinlock_key_t key = k_spin_lock(&observer_lock);
        classifier = (struct usb_os_classifier){0};
        generation++;
        reliable = true;
        if (IS_ENABLED(CONFIG_SPLITERGOLL_USB_TRACE)) {
            splitergoll_usb_trace_reset();
        }
        k_spin_unlock(&observer_lock, key);
    }
    usb_dc_status_callback callback = status_callback;
    if (callback != NULL) {
        callback(status, param);
    }
}

// Conserve aussi la désinscription du callback natif.
void __wrap_usb_dc_set_status_callback(usb_dc_status_callback callback) {
    status_callback = callback;
    __real_usb_dc_set_status_callback(callback == NULL ? NULL : observe_status);
}

// Distingue le SETUP des données de contrôle sans modifier l’appel original.
static void observe_endpoint(uint8_t ep, enum usb_dc_ep_cb_status_code status) {
    reading_setup = status == USB_DC_EP_SETUP;
    setup_callback(ep, status);
    reading_setup = false;
}

// Ajoute l’observation seulement à l’endpoint zéro OUT.
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

// Exploite les huit octets déjà lus, sans seconde lecture du matériel ni payload.
int __wrap_usb_dc_ep_read(uint8_t ep, uint8_t *data, uint32_t size, uint32_t *read_bytes) {
    int result = __real_usb_dc_ep_read(ep, data, size, read_bytes);
    if (reading_setup && ep == 0) {
        size_t length = result == 0 ? (read_bytes == NULL ? size : *read_bytes) : 0;
        uint32_t timestamp_ms = k_uptime_get_32();
        k_spinlock_key_t key = k_spin_lock(&observer_lock);
        if (data != NULL && length == 8) {
            usb_os_observe(&classifier, data, timestamp_ms);
        } else {
            reliable = false;
        }
        if (IS_ENABLED(CONFIG_SPLITERGOLL_USB_TRACE)) {
            splitergoll_usb_trace_record(timestamp_ms, data, length);
        }
        k_spin_unlock(&observer_lock, key);
    }
    return result;
}

// Ne conserve aucune ancienne hypothèse lorsque le pilote annonce un nouveau cycle.
struct usb_os_snapshot splitergoll_usb_os_snapshot(void) {
    k_spinlock_key_t key = k_spin_lock(&observer_lock);
    struct usb_os_snapshot snapshot = {
        .generation = generation,
        .guess = usb_os_result(&classifier, k_uptime_get_32(), reliable),
    };
    k_spin_unlock(&observer_lock, key);
    return snapshot;
}
