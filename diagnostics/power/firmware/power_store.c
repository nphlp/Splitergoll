// Stockage pur du diagnostic, indépendant de Zephyr et du matériel.
#include <splitergoll/power_store.h>

// Refuse les champs incohérents avant d’enregistrer une mesure.
static bool valid(struct power_trace_record record) {
    if (record.activity > 2 || record.usb > 2) return false;
    switch (record.kind) {
    case POWER_BOOT: return record.value == 0 && record.result == 0;
    case POWER_ACTIVITY: return record.value == record.activity && record.result == 0;
    case POWER_USB: return record.value == record.usb && record.result == 0;
    case POWER_WINDOW:
        return record.value >= 0 && record.value <= 120000 &&
               record.result >= 0 && record.result <= 1;
    case POWER_SAVE:
        return record.value >= 0 && record.value < 5 && record.result <= 0 &&
               record.result >= -4095;
    default: return false;
    }
}

// Borne les compteurs même après une longue expérience saturée.
static void increment(uint32_t *counter) {
    if (*counter != UINT32_MAX) (*counter)++;
}

// Déduplique seulement la fenêtre, jamais un résultat d’écriture ou une transition d’activité.
bool power_trace_append(struct power_trace_store *store, struct power_trace_record record) {
    if (!valid(record)) {
        increment(&store->rejected);
        return false;
    }
    if (record.kind == POWER_WINDOW && store->window_seen &&
        store->window_open == (record.value != 0) &&
        store->advertising == (record.result != 0)) return true;
    if (store->count >= POWER_TRACE_CAPACITY) {
        increment(&store->dropped);
        return false;
    }
    store->records[store->count++] = record;
    if (record.kind == POWER_WINDOW) {
        store->window_seen = true;
        store->window_open = record.value != 0;
        store->advertising = record.result != 0;
    }
    return true;
}

// Les enregistrements déjà ajoutés restent immuables.
bool power_trace_get(const struct power_trace_store *store, size_t index,
                     struct power_trace_record *record) {
    if (record == NULL || index >= store->count) return false;
    *record = store->records[index];
    return true;
}

// Aucun accès par index non contrôlé à une table de noms.
const char *power_trace_name(enum power_trace_kind kind) {
    switch (kind) {
    case POWER_BOOT: return "BOOT";
    case POWER_ACTIVITY: return "ACTIVITY";
    case POWER_USB: return "USB";
    case POWER_WINDOW: return "WINDOW";
    case POWER_SAVE: return "SAVE";
    default: return "INVALID";
    }
}
