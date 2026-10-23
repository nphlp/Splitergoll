// Vérifie le journal comme une acquisition, pas seulement comme un tableau d’événements.
#include <assert.h>
#include <splitergoll/power_store.h>
#include <stdio.h>
#include <string.h>

// Produit un événement cohérent sans dépendre de l’horloge ou du matériel hôte.
static struct power_trace_record event(enum power_trace_kind kind, uint32_t time, int32_t value,
                                       int32_t result) {
    return (struct power_trace_record){.timestamp_ms = time,
                                       .kind = kind,
                                       .activity = 1,
                                       .usb = 0,
                                       .value = value,
                                       .result = result};
}

// La sauvegarde peut arriver avant, pendant ou après IDLE sans être perdue.
static void timeline(void) {
    for (unsigned order = 0; order < 3; order++) {
        struct power_trace_store store = {0};
        assert(power_trace_append(&store, event(POWER_BOOT, 0, 0, 0)));
        assert(power_trace_append(&store, event(POWER_WINDOW, 1, 10000, 1)));
        if (order == 0)
            assert(power_trace_append(&store, event(POWER_SAVE, 9999, 1, 0)));
        assert(power_trace_append(&store, event(POWER_ACTIVITY, 10000, 1, 0)));
        if (order == 1)
            assert(power_trace_append(&store, event(POWER_SAVE, 10000, 1, 0)));
        assert(power_trace_append(&store, event(POWER_WINDOW, 10001, 0, 0)));
        if (order == 2)
            assert(power_trace_append(&store, event(POWER_SAVE, 10002, 1, 0)));
        assert(store.count == 5 && store.dropped == 0 && store.rejected == 0);
        unsigned saves = 0;
        for (size_t i = 0; i < store.count; i++) {
            struct power_trace_record first, second;
            assert(power_trace_get(&store, i, &first));
            assert(power_trace_get(&store, i, &second));
            assert(first.kind == second.kind && first.timestamp_ms == second.timestamp_ms);
            if (first.kind == POWER_SAVE) {
                saves++;
                assert(first.value == 1 && first.result == 0);
            }
        }
        assert(saves == 1 && store.count == 5);
        assert(!power_trace_get(&store, store.count, &(struct power_trace_record){0}));
        assert(!power_trace_get(&store, 0, NULL));
    }
}

// Un compte à rebours ne remplit pas le journal, mais les changements radio restent visibles.
static void deduplication(void) {
    struct power_trace_store store = {0};
    assert(power_trace_append(&store, event(POWER_WINDOW, 0, 10000, 1)));
    assert(power_trace_append(&store, event(POWER_ACTIVITY, 1, 1, 0)));
    for (int32_t time = 1; time < 10000; time++)
        assert(power_trace_append(&store, event(POWER_WINDOW, (uint32_t)time, 10000 - time, 1)));
    assert(store.count == 2);
    assert(power_trace_append(&store, event(POWER_WINDOW, 10000, 0, 1)));
    assert(power_trace_append(&store, event(POWER_WINDOW, 10001, 0, 0)));
    assert(power_trace_append(&store, event(POWER_WINDOW, 10002, 10000, 1)));
    assert(store.count == 5 && !store.dropped);
}

// Une acquisition pleine n’écrase jamais les preuves du début et expose ses pertes.
static void capacity(void) {
    struct power_trace_store store = {0};
    for (size_t i = 0; i < POWER_TRACE_CAPACITY; i++)
        assert(power_trace_append(&store, event(POWER_SAVE, (uint32_t)i, 0, 0)));
    assert(!power_trace_append(&store, event(POWER_SAVE, 999, 1, -5)));
    assert(store.count == POWER_TRACE_CAPACITY && store.dropped == 1);
    assert(store.records[0].timestamp_ms == 0);
    assert(store.records[POWER_TRACE_CAPACITY - 1].timestamp_ms == POWER_TRACE_CAPACITY - 1);
    store.dropped = UINT32_MAX;
    assert(!power_trace_append(&store, event(POWER_SAVE, 1000, 1, -5)));
    assert(store.dropped == UINT32_MAX);
}

// Les erreurs, bornes, horloges débordantes et champs invalides ne deviennent pas des succès.
static void validation(void) {
    struct power_trace_store store = {0};
    assert(power_trace_append(&store, event(POWER_SAVE, UINT32_MAX, 4, -4095)));
    assert(power_trace_append(&store, event(POWER_SAVE, 0, 0, 0)));
    assert(store.records[0].timestamp_ms == UINT32_MAX && store.records[1].timestamp_ms == 0);
    struct power_trace_record bad[] = {
        event((enum power_trace_kind)0, 0, 0, 0),
        event(POWER_BOOT, 0, 1, 0),
        event(POWER_ACTIVITY, 0, 0, 0),
        event(POWER_USB, 0, 1, 0),
        event(POWER_WINDOW, 0, -1, 0),
        event(POWER_WINDOW, 0, 120001, 0),
        event(POWER_WINDOW, 0, 10000, 2),
        event(POWER_SAVE, 0, 5, 0),
        event(POWER_SAVE, 0, -1, 0),
        event(POWER_SAVE, 0, 0, 1),
        event(POWER_SAVE, 0, 0, -4096),
    };
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++)
        assert(!power_trace_append(&store, bad[i]));
    struct power_trace_record invalid = event(POWER_SAVE, 0, 0, 0);
    invalid.activity = 3;
    assert(!power_trace_append(&store, invalid));
    invalid.activity = 0;
    invalid.usb = 3;
    assert(!power_trace_append(&store, invalid));
    assert(store.count == 2 && store.rejected == sizeof(bad) / sizeof(bad[0]) + 2);
    store.rejected = UINT32_MAX;
    assert(!power_trace_append(&store, invalid) && store.rejected == UINT32_MAX);
    assert(strcmp(power_trace_name(POWER_SAVE), "SAVE") == 0);
    assert(strcmp(power_trace_name((enum power_trace_kind) - 1), "INVALID") == 0);
    assert(power_trace_append(&store, event(POWER_WINDOW, 1, 120000, 1)));
    invalid = event(POWER_USB, 2, 2, 0);
    invalid.usb = 2;
    assert(power_trace_append(&store, invalid));
}

// Exécute tous les cas, sans périphérique ni modification d’environnement.
int main(void) {
    timeline();
    deduplication();
    capacity();
    validation();
    puts("power journal: timelines, reads, bounds, saturation and errors OK");
    return 0;
}
