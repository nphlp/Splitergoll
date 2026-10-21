// Vérifie les appels radio et les refus de reconnexion avec un contrôleur simulé.
#include <splitergoll/ble_reconnect_state.h>
#include <splitergoll/host_window_policy.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

struct radio {
    unsigned stops, filters, starts;
    uint16_t allowed;
    bool active;
    bool current;
    unsigned expire_on;
    int stop_error, filter_error, start_error;
};

// L’arrêt précède obligatoirement chaque changement de filtre.
static int stop(void *context) {
    struct radio *radio = context;
    radio->stops++;
    if (radio->stop_error) return radio->stop_error;
    radio->active = false;
    return 0;
}

// Une erreur de filtre interdit le démarrage, même si une partie de la liste a été écrite.
static int filter(void *context, uint16_t allowed) {
    struct radio *radio = context;
    assert(!radio->active && allowed != 0);
    radio->filters++;
    radio->allowed = allowed;
    if (radio->expire_on == 1) radio->current = false;
    return radio->filter_error;
}

// Le contrôleur n’accepte jamais un démarrage doublé ou sans filtre.
static int start(void *context) {
    struct radio *radio = context;
    assert(!radio->active && radio->allowed != 0);
    radio->starts++;
    if (radio->start_error) return radio->start_error;
    radio->active = true;
    if (radio->expire_on == 2) radio->current = false;
    return 0;
}

// Simule l’échéance ou une nouvelle génération pendant une commande HCI.
static bool current(void *context) { return ((struct radio *)context)->current; }

// Le test utilise le même contrat d’opérations que l’adaptateur embarqué.
static struct ble_reconnect_ops operations(struct radio *radio) {
    return (struct ble_reconnect_ops){radio, stop, filter, start, current};
}

// Mac connecté, Windows manquant : annoncer Windows sans toucher à la sélection Mac.
static void known_hosts(void) {
    struct radio radio = {.current = true};
    struct ble_reconnect_ops ops = operations(&radio);
    struct ble_reconnect_state state = {0};
    struct ble_reconnect_view view = {5, 3, 1, 1, true};
    assert(ble_reconnect_apply(&state, view, &ops) == 0);
    assert(state.advertising && radio.allowed == 2);
    assert(radio.stops == 1 && radio.filters == 1 && radio.starts == 1);
    assert(ble_reconnect_apply(&state, view, &ops) == 0);
    assert(radio.starts == 1);
    view.connected = 3;
    view.revision++;
    radio.active = false;
    assert(ble_reconnect_apply(&state, view, &ops) == 0);
    assert(!state.advertising && radio.starts == 1);
    view.connected = 1;
    view.revision++;
    assert(ble_reconnect_apply(&state, view, &ops) == 0);
    assert(state.advertising && radio.allowed == 2);
    view.window_open = false;
    assert(ble_reconnect_apply(&state, view, &ops) == 0);
    assert(!state.advertising && !radio.active);
    view.connected = 0;
    view.revision++;
    assert(ble_reconnect_apply(&state, view, &ops) == 0 && !state.advertising);
}

// Chaque masque autorise uniquement les hôtes connus qui ne sont pas déjà connectés.
static void all_topologies(void) {
    for (unsigned known = 0; known < 256; known++) {
        for (unsigned connected = 0; connected < 256; connected++) {
            struct radio radio = {.current = true};
            struct ble_reconnect_ops ops = operations(&radio);
            struct ble_reconnect_state state = {0};
            struct ble_reconnect_view view = {8, (uint16_t)known, (uint16_t)connected, 1, true};
            int result = ble_reconnect_apply(&state, view, &ops);
            if (connected & ~known) {
                assert(result == -EINVAL && radio.starts == 0);
            } else {
                assert(result == 0);
                assert(state.advertising == ((known & ~connected) != 0));
                if (state.advertising) assert(radio.allowed == (known & ~connected));
            }
        }
    }
    struct radio radio = {.current = true};
    struct ble_reconnect_ops ops = operations(&radio);
    struct ble_reconnect_state state = {0};
    assert(ble_reconnect_apply(&state, (struct ble_reconnect_view){16, 0x8000, 0, 1, true}, &ops) == 0);
    assert(radio.allowed == 0x8000);
    assert(ble_reconnect_apply(&state, (struct ble_reconnect_view){0}, &ops) == -EINVAL);
    assert(!state.advertising);
    assert(ble_reconnect_apply(&state, (struct ble_reconnect_view){17, 1, 0, 1, true}, &ops) == -EINVAL);
    assert(ble_reconnect_apply(&state, (struct ble_reconnect_view){5, 32, 0, 1, true}, &ops) == -EINVAL);
}

// Une échéance pendant la mise à jour ou le démarrage ne laisse pas les annonces ouvertes.
static void stale_work(void) {
    for (unsigned expire = 0; expire <= 2; expire++) {
        struct radio radio = {.current = expire != 0, .expire_on = expire};
        struct ble_reconnect_ops ops = operations(&radio);
        struct ble_reconnect_state state = {0};
        assert(ble_reconnect_apply(&state, (struct ble_reconnect_view){5, 3, 0, 1, true}, &ops) == -EAGAIN);
        assert(!state.advertising && !radio.active);
        assert(radio.starts == (expire == 2 ? 1u : 0u));
    }
}

// Les erreurs restent visibles et aucun filtre ou démarrage ne suit un arrêt échoué.
static void radio_errors(void) {
    for (unsigned failure = 0; failure < 3; failure++) {
        struct radio radio = {.current = true};
        if (failure == 0) radio.stop_error = -EIO;
        if (failure == 1) radio.filter_error = -ENOMEM;
        if (failure == 2) radio.start_error = -ENOMEM;
        struct ble_reconnect_ops ops = operations(&radio);
        struct ble_reconnect_state state = {0};
        struct ble_reconnect_view view = {5, 3, 0, 1, true};
        assert(ble_reconnect_apply(&state, view, &ops) == (failure == 0 ? -EIO : -ENOMEM));
        assert(state.error != 0 && !state.advertising && !radio.active);
        assert(radio.filters == (failure == 0 ? 0u : 1u));
        assert(radio.starts == (failure == 2 ? 1u : 0u));
        radio.stop_error = radio.filter_error = radio.start_error = 0;
        assert(ble_reconnect_apply(&state, view, &ops) == 0 && state.advertising);
        radio.stop_error = -EIO;
        view.window_open = false;
        assert(ble_reconnect_apply(&state, view, &ops) == -EIO);
        assert(state.advertising && radio.active);
        radio.stop_error = 0;
        assert(ble_reconnect_apply(&state, view, &ops) == 0 && !state.advertising);
    }
}

// Les dix secondes ne sont ni réarmées par une connexion ni bloquantes pour la saisie.
static void deadlines_and_admission(void) {
    struct host_window_policy window;
    host_window_start(&window, UINT32_MAX - 5000);
    assert(host_window_remaining(&window, 4998) == 1);
    assert(host_window_remaining(&window, 4999) == 0);
    for (unsigned bits = 0; bits < 8; bits++) {
        assert(ble_reconnect_accept(bits & 1, bits & 2, bits & 4) == (bits == 7));
    }
    struct radio radio = {.current = true};
    struct ble_reconnect_ops ops = operations(&radio);
    struct ble_reconnect_state state = {0};
    struct ble_reconnect_view view = {5, 3, 1, 1, true};
    assert(ble_reconnect_apply(&state, view, &ops) == 0);
    host_window_start(&window, 20000);
    assert(host_window_remaining(&window, 29999) == 1);
    view.window_open = host_window_remaining(&window, 30000) != 0;
    assert(ble_reconnect_apply(&state, view, &ops) == 0 && !state.advertising);
    host_window_start(&window, 40000);
    view.window_open = host_window_remaining(&window, 40000) != 0;
    assert(ble_reconnect_apply(&state, view, &ops) == 0 && state.advertising);
}

// Exécute les scénarios utilisés par le contrôleur réel, sans matériel ni appairage.
int main(void) {
    known_hosts();
    all_topologies();
    stale_work();
    radio_errors();
    deadlines_and_admission();
    puts("ble_reconnect_state: ok");
    return 0;
}
