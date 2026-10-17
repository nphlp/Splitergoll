// Vérifie le stockage des requêtes sans matériel et sans modifier les firmwares.
#include "usb_trace_store.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

// Vérifie l’ordre, la copie des octets et l’isolation des lectures retournées.
static void test_records(void) {
    struct usb_trace_store store = {0};
    uint8_t setup[8] = {0x80, 6, 0, 1, 0, 0, 18, 0};
    struct usb_trace_record record;
    assert(!usb_trace_get(&store, 0, &record));
    assert(usb_trace_append(&store, 42, setup, sizeof(setup)));
    setup[6] = 64;
    assert(usb_trace_append(&store, 51, setup, sizeof(setup)));
    assert(usb_trace_get(&store, 0, &record));
    assert(record.timestamp_ms == 42 && record.setup[6] == 18);
    memset(&record, 0, sizeof(record));
    assert(usb_trace_get(&store, 1, &record));
    assert(record.timestamp_ms == 51 && record.setup[6] == 64);
    assert(!usb_trace_get(&store, 2, &record));
    assert(!usb_trace_get(&store, SIZE_MAX, &record));
    assert(!usb_trace_get(&store, 0, NULL));
    assert(store.count == 2 && store.dropped == 0 && store.rejected == 0);
}

// Refuse toute taille autre qu’un SETUP complet, sans lire la mémoire correspondante.
static void test_invalid_reads(void) {
    struct usb_trace_store store = {0};
    uint8_t setup[8] = {0};
    for (size_t length = 0; length < 32; length++) {
        if (length != sizeof(setup)) {
            assert(!usb_trace_append(&store, 0, setup, length));
        }
    }
    assert(!usb_trace_append(&store, 0, NULL, sizeof(setup)));
    assert(store.count == 0 && store.rejected == 32);
    store.rejected = UINT32_MAX;
    assert(!usb_trace_append(&store, 0, NULL, 0));
    assert(store.rejected == UINT32_MAX);
}

// Sature le tampon sans remplacer les premières requêtes ni faire déborder le compteur.
static void test_capacity(void) {
    struct usb_trace_store store = {0};
    uint8_t setup[8] = {0};
    for (size_t i = 0; i < USB_TRACE_CAPACITY; i++) {
        setup[0] = i;
        assert(usb_trace_append(&store, (uint32_t)i, setup, sizeof(setup)));
    }
    assert(!usb_trace_append(&store, 999, setup, sizeof(setup)));
    assert(store.count == USB_TRACE_CAPACITY && store.dropped == 1);
    struct usb_trace_record record;
    assert(usb_trace_get(&store, 0, &record));
    assert(record.timestamp_ms == 0 && record.setup[0] == 0);
    assert(usb_trace_get(&store, USB_TRACE_CAPACITY - 1, &record));
    assert(record.timestamp_ms == USB_TRACE_CAPACITY - 1);
    store.dropped = UINT32_MAX;
    assert(!usb_trace_append(&store, 1000, setup, sizeof(setup)));
    assert(store.dropped == UINT32_MAX);
}

// Isole deux cycles, y compris si une lecture de l’ancien est encore en cours.
static void test_generation(void) {
    struct usb_trace_store store = {0};
    struct usb_trace_record record;
    uint8_t setup[8] = {0x80, 6, 0, 1, 0, 0, 8, 0};
    uint32_t generation = store.generation;
    assert(usb_trace_append(&store, 10, setup, sizeof(setup)));
    assert(usb_trace_get_generation(&store, generation, 0, &record));
    store.dropped = 12;
    store.rejected = 8;
    usb_trace_reset(&store);
    assert(store.generation == generation + 1);
    assert(store.count == 0 && store.dropped == 0 && store.rejected == 0);
    assert(!usb_trace_get(&store, 0, &record));
    setup[6] = 64;
    assert(usb_trace_append(&store, 20, setup, sizeof(setup)));
    assert(!usb_trace_get_generation(&store, generation, 0, &record));
    assert(usb_trace_get_generation(&store, store.generation, 0, &record));
    assert(record.timestamp_ms == 20 && record.setup[6] == 64);
    usb_trace_reset(&store);
    assert(!usb_trace_get_generation(&store, generation + 1, 0, &record));
    assert(store.count == 0);
}

// Lance les tests du code embarqué avec des assertions actives.
int main(void) {
    test_records();
    test_invalid_reads();
    test_capacity();
    test_generation();
    puts("USB trace store: OK");
    return 0;
}
