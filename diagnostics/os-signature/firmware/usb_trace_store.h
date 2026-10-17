// Stockage borné des premiers en-têtes USB, indépendant des pilotes et sans payload HID.
#ifndef SPLITERGOLL_USB_TRACE_STORE_H
#define SPLITERGOLL_USB_TRACE_STORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define USB_TRACE_CAPACITY 256
#define USB_TRACE_SETUP_SIZE 8

struct usb_trace_record {
    uint32_t timestamp_ms;
    uint8_t setup[USB_TRACE_SETUP_SIZE];
};

struct usb_trace_store {
    struct usb_trace_record records[USB_TRACE_CAPACITY];
    size_t count;
    uint32_t dropped;
    uint32_t rejected;
    uint32_t generation;
};

// Ouvre un nouveau cycle USB et invalide les lectures de l’ancien, sans parcourir les records.
void usb_trace_reset(struct usb_trace_store *store);

// Copie un en-tête complet, conserve les premiers et compte les pertes sans écrasement.
bool usb_trace_append(struct usb_trace_store *store, uint32_t timestamp_ms,
                      const uint8_t *setup, size_t length);

// Copie un enregistrement existant sans exposer le tampon interne à l’appelant.
bool usb_trace_get(const struct usb_trace_store *store, size_t index,
                   struct usb_trace_record *record);

// Refuse une lecture si le bus a changé de cycle pendant la restitution de l’instantané.
bool usb_trace_get_generation(const struct usb_trace_store *store, uint32_t generation,
                              size_t index, struct usb_trace_record *record);

#endif
