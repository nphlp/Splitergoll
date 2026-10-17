// Conserve des en-têtes SETUP de huit octets, sans allocation ni traitement USB.
#include "usb_trace_store.h"

#include <string.h>

// Incrémente un compteur de perte sans revenir à zéro en cas de dépassement.
static void increment_saturated(uint32_t *value) {
    if (*value != UINT32_MAX) {
        (*value)++;
    }
}

// Refuse les lectures incomplètes et garde les premières requêtes si le tampon est plein.
bool usb_trace_append(struct usb_trace_store *store, uint32_t timestamp_ms,
                      const uint8_t *setup, size_t length) {
    if (setup == NULL || length != USB_TRACE_SETUP_SIZE) {
        increment_saturated(&store->rejected);
        return false;
    }

    if (store->count == USB_TRACE_CAPACITY) {
        increment_saturated(&store->dropped);
        return false;
    }

    struct usb_trace_record *record = &store->records[store->count++];
    record->timestamp_ms = timestamp_ms;
    memcpy(record->setup, setup, sizeof(record->setup));
    return true;
}

// Vérifie les bornes avant de rendre une copie indépendante de l’enregistrement.
bool usb_trace_get(const struct usb_trace_store *store, size_t index,
                   struct usb_trace_record *record) {
    if (record == NULL || index >= store->count) {
        return false;
    }

    *record = store->records[index];
    return true;
}
