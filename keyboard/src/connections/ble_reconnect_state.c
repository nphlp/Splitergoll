// Ordre des opérations radio, testable sans Zephyr et sans effet sur les appairages.
#include <splitergoll/ble_reconnect_state.h>
#include <errno.h>

// Une topologie incohérente ne permet jamais de démarrer des annonces ouvertes.
static bool valid_view(struct ble_reconnect_view view) {
    return view.count > 0 && view.count <= 16 &&
           ((uint32_t)(view.known | view.connected) >> view.count) == 0 &&
           (view.connected & (uint16_t)~view.known) == 0;
}

// Un échec d’arrêt conserve l’état actif et interdit toute modification du filtre.
static int stop_advertising(struct ble_reconnect_state *state, const struct ble_reconnect_ops *ops) {
    int error = ops->stop(ops->context);
    if (error == 0) {
        state->advertising = false;
        state->allowed = 0;
        state->pairing_open = false;
    }
    state->error = error;
    return error;
}

// Les appels asynchrones et les connexions rendent une ancienne demande caduque.
int ble_reconnect_apply(struct ble_reconnect_state *state, struct ble_reconnect_view view,
                         const struct ble_reconnect_ops *ops) {
    bool valid = valid_view(view);
    uint16_t allowed = valid && view.window_open ? view.known & (uint16_t)~view.connected : 0;
    bool pairing_open = valid && view.window_open && view.pairing_open;
    bool advertise = allowed != 0 || pairing_open;
    if (valid && advertise && state->advertising && state->allowed == allowed &&
        state->pairing_open == pairing_open &&
        state->revision == view.revision && ops->current(ops->context)) return state->error = 0;

    if (state->advertising || advertise || state->error != 0) {
        if (stop_advertising(state, ops) != 0) return state->error;
    }
    if (!valid) return state->error = -EINVAL;
    if (!advertise) return state->error = 0;
    if (!ops->current(ops->context)) return state->error = -EAGAIN;

    int error = ops->filter(ops->context, allowed);
    if (error != 0) return state->error = error;
    if (!ops->current(ops->context)) return state->error = -EAGAIN;
    error = ops->start(ops->context);
    if (error != 0) return state->error = error;

    state->advertising = true;
    state->allowed = allowed;
    state->pairing_open = pairing_open;
    state->revision = view.revision;
    if (!ops->current(ops->context)) {
        error = stop_advertising(state, ops);
        return state->error = error != 0 ? error : -EAGAIN;
    }
    return state->error = 0;
}

// Réutiliser les clés existantes ne doit jamais créer un nouvel appairage.
bool ble_reconnect_accept(bool window_open, bool profile_known, bool bonded) {
    return window_open && profile_known && bonded;
}
