// Historique RAM borné, sans valeur de frappe ni identité d’appareil.
#ifndef SPLITERGOLL_POWER_STORE_H
#define SPLITERGOLL_POWER_STORE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define POWER_TRACE_CAPACITY 128

enum power_trace_kind { POWER_BOOT = 1, POWER_ACTIVITY, POWER_USB, POWER_WINDOW, POWER_SAVE };

struct power_trace_record {
    uint32_t timestamp_ms;
    enum power_trace_kind kind;
    uint8_t activity;
    uint8_t usb;
    int32_t value;
    int32_t result;
};

struct power_trace_store {
    struct power_trace_record records[POWER_TRACE_CAPACITY];
    size_t count;
    uint32_t dropped;
    uint32_t rejected;
    bool window_seen;
    bool window_open;
    bool advertising;
};

// Ajoute un événement validé, sans écraser le début du journal.
bool power_trace_append(struct power_trace_store *store, struct power_trace_record record);

// Lit une copie sans modifier le journal, y compris pendant un export répété.
bool power_trace_get(const struct power_trace_store *store, size_t index,
                     struct power_trace_record *record);

// Renvoie un nom de protocole stable, même pour une valeur inconnue.
const char *power_trace_name(enum power_trace_kind kind);
#endif
