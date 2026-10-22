// Décide quand rechercher ou changer d’hôte, sans API ZMK, NVS ou opération radio.
#include <splitergoll/host_window_policy.h>

// Une topologie incomplète ne doit pas créer une destination par défaut.
static bool valid_view(const struct host_window_view *view) {
    if (view->count == 0 || view->count > HOST_WINDOW_CAPACITY ||
        view->selected < HOST_WINDOW_NO_TARGET || view->selected >= (int)view->count ||
        ((uint32_t)view->ready >> view->count) != 0) return false;
    for (unsigned i = 0; i < view->count; i++) {
        if ((view->ready & (UINT32_C(1) << i)) && view->identities[i] == 0) return false;
    }
    return true;
}

// Le ticket zéro reste réservé à l’absence de demande, même après débordement.
static void next_generation(struct host_window_policy *policy) {
    policy->generation++;
    if (policy->generation == 0) policy->generation = 1;
}

// Retire la cible sans modifier le délai d’annonce ni la source de saisie.
static void clear_pending(struct host_window_policy *policy) {
    policy->pending = HOST_WINDOW_NO_TARGET;
    policy->pending_identity = 0;
    policy->pending_search = false;
}

// Invalide aussi les travaux qui auraient déjà copié un ancien ticket.
void host_window_cancel_selection(struct host_window_policy *policy) {
    clear_pending(policy);
    policy->searching = false;
    next_generation(policy);
}

// Réarme une fenêtre sans interrompre la saisie et sans conserver une intention périmée.
static void open_window(struct host_window_policy *policy, uint32_t now_ms) {
    host_window_cancel_selection(policy);
    policy->opened_at_ms = now_ms;
    policy->active = true;
    policy->excluded = 0;
    policy->source = HOST_WINDOW_NO_TARGET;
    policy->source_identity = 0;
}

// L’initialisation distingue une absence de cible de la destination zéro.
void host_window_start(struct host_window_policy *policy, uint32_t now_ms) {
    *policy = (struct host_window_policy){0};
    open_window(policy, now_ms);
}

// Le temps est relatif, y compris lorsque l’horloge 32 bits revient à zéro.
uint32_t host_window_remaining(const struct host_window_policy *policy, uint32_t now_ms) {
    uint32_t elapsed = now_ms - policy->opened_at_ms;
    if (!policy->active || policy->gaming_usb || elapsed >= HOST_WINDOW_DURATION_MS) return 0;
    return HOST_WINDOW_DURATION_MS - elapsed;
}

// Les connexions connues et les slots d’appairage restent deux ressources distinctes.
bool host_window_pairing_allowed(const struct host_window_policy *policy, uint32_t now_ms,
                                 bool free_slot) {
    return free_slot && host_window_remaining(policy, now_ms) != 0;
}

// Le nombre de destinations est indépendant du nombre de liaisons vers chaque appareil.
static unsigned ready_count(uint16_t mask) {
    unsigned count = 0;
    while (mask != 0) {
        count += mask & 1u;
        mask >>= 1;
    }
    return count;
}

// L’ordre est circulaire et déterministe si plusieurs appareils deviennent prêts ensemble.
static int next_ready(const struct host_window_view *view, int after, uint16_t mask) {
    unsigned start = after >= 0 ? (unsigned)after : view->count - 1;
    for (unsigned step = 1; step <= view->count; step++) {
        unsigned target = (start + step) % view->count;
        if (mask & (UINT32_C(1) << target)) return (int)target;
    }
    return HOST_WINDOW_NO_TARGET;
}

// La cible conserve son identité pour refuser le remplacement d’une case en attente.
static void set_pending(struct host_window_policy *policy, const struct host_window_view *view,
                         int target, bool search) {
    policy->pending = target;
    policy->pending_identity = view->identities[target];
    policy->pending_search = search;
}

// Une autre commande explicite ou une identité source remplacée annule la recherche.
static bool source_unchanged(const struct host_window_policy *policy,
                              const struct host_window_view *view) {
    if (policy->source != view->selected) return false;
    return policy->source < 0 || policy->source_identity == view->identities[policy->source];
}

// Un geste fixe son contexte une fois, les arrivées ne changent pas ensuite sa catégorie.
bool host_window_shortcut(struct host_window_policy *policy, const struct host_window_view *view,
                          uint32_t now_ms) {
    if (!valid_view(view)) {
        policy->active = false;
        host_window_cancel_selection(policy);
        return false;
    }
    int cursor = view->selected;
    if (policy->pending >= 0 && !policy->pending_search && source_unchanged(policy, view) &&
        policy->pending < (int)view->count &&
        (view->ready & (UINT32_C(1) << policy->pending)) &&
        policy->pending_identity == view->identities[policy->pending]) cursor = policy->pending;

    policy->gaming_usb = false;
    open_window(policy, now_ms);
    policy->source = view->selected;
    policy->source_identity = view->selected >= 0 ? view->identities[view->selected] : 0;
    policy->excluded = view->ready;
    unsigned count = ready_count(view->ready);
    policy->searching = count <= 1;
    if (count > 1) {
        int target = next_ready(view, cursor, view->ready);
        if (target != view->selected) set_pending(policy, view, target, false);
    } else if (count == 1) {
        int target = next_ready(view, HOST_WINDOW_NO_TARGET, view->ready);
        if (target != view->selected) set_pending(policy, view, target, false);
    }
    return true;
}

// Une transition interne entre les deux couches Gaming ne réouvre jamais la fenêtre.
bool host_window_enter_gaming(struct host_window_policy *policy, bool usb_usable) {
    if (!usb_usable || policy->gaming_usb) return false;
    policy->gaming_usb = true;
    policy->active = false;
    host_window_cancel_selection(policy);
    return true;
}

// Le retour simple à Windows conserve la sélection, contrairement à une recherche explicite.
bool host_window_leave_gaming(struct host_window_policy *policy, uint32_t now_ms) {
    if (!policy->gaming_usb) return false;
    policy->gaming_usb = false;
    open_window(policy, now_ms);
    return true;
}

// L’échéance ferme les nouvelles arrivées, sans sacrifier une cible déjà prête et gardée.
void host_window_update(struct host_window_policy *policy, const struct host_window_view *view,
                         uint32_t now_ms) {
    if (!valid_view(view)) {
        policy->active = false;
        host_window_cancel_selection(policy);
        return;
    }
    if (policy->gaming_usb) return;
    if ((policy->searching || policy->pending >= 0) && !source_unchanged(policy, view)) {
        host_window_cancel_selection(policy);
        return;
    }
    if (policy->active && host_window_remaining(policy, now_ms) == 0) {
        policy->active = false;
        policy->searching = false;
    }
    if (policy->pending >= 0) {
        int target = policy->pending;
        if (target >= (int)view->count || policy->pending_identity != view->identities[target]) {
            host_window_cancel_selection(policy);
            return;
        }
        if (view->ready & (UINT32_C(1) << target)) return;
        clear_pending(policy);
    }
    if (policy->searching && policy->active) {
        uint16_t candidates = view->ready & (uint16_t)~policy->excluded;
        int target = next_ready(view, view->selected, candidates);
        if (target >= 0) set_pending(policy, view, target, true);
    }
}

// Le ticket et l’identité sont revérifiés avant de consommer une décision en contexte sûr.
int host_window_take(struct host_window_policy *policy, const struct host_window_view *view,
                      uint32_t now_ms, uint32_t generation, bool input_busy) {
    if (generation != policy->generation) return HOST_WINDOW_NO_TARGET;
    host_window_update(policy, view, now_ms);
    if (input_busy || policy->gaming_usb || policy->pending < 0) return HOST_WINDOW_NO_TARGET;
    int target = policy->pending;
    bool search = policy->pending_search;
    clear_pending(policy);
    if (search) policy->searching = false;
    policy->source = target;
    policy->source_identity = view->identities[target];
    return target;
}

// Toute nouvelle fenêtre invalide une échéance copiée par le travail radio.
void host_discovery_open(struct host_discovery_policy *policy, uint32_t now_ms) {
    policy->opened = true;
    policy->opened_at_ms = now_ms;
    if (++policy->generation == 0) policy->generation = 1;
}

// Le démarrage ne présume aucune connexion et ne modifie aucune sélection native.
void host_discovery_start(struct host_discovery_policy *policy, uint32_t now_ms) {
    *policy = (struct host_discovery_policy){.awake = true};
    host_discovery_open(policy, now_ms);
}

// IDLE ferme l’admission, le retour ACTIVE seul ne rouvre que s’il n’y a aucun hôte.
void host_discovery_update(struct host_discovery_policy *policy, uint32_t now_ms,
                           uint16_t connected, bool awake) {
    if (policy->opened && (uint32_t)(now_ms - policy->opened_at_ms) >= HOST_WINDOW_DURATION_MS) {
        policy->opened = false;
    }
    if (awake && policy->connected == 0 && connected != 0) host_discovery_open(policy, now_ms);
    if (!awake) policy->opened = false;
    policy->connected = connected;
    policy->awake = awake;
}

// La radio sans hôte reste ouverte sans devoir relancer un minuteur ou l’activité ZMK.
uint32_t host_discovery_remaining(const struct host_discovery_policy *policy, uint32_t now_ms) {
    if (!policy->awake) return 0;
    if (policy->connected == 0) return HOST_WINDOW_DURATION_MS;
    uint32_t elapsed = now_ms - policy->opened_at_ms;
    if (!policy->opened || elapsed >= HOST_WINDOW_DURATION_MS) return 0;
    return HOST_WINDOW_DURATION_MS - elapsed;
}
