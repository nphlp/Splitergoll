// Décisions pures de préférence et de reprise, sans accès Bluetooth ou mémoire flash.
#include <splitergoll/host_preference.h>
#include <string.h>

// Une empreinte nulle ne représente jamais un bond vérifié.
static bool valid_identity(const uint8_t identity[HOST_PREFERENCE_ID_SIZE]) {
    if (identity == NULL) return false;
    uint8_t combined = 0;
    for (unsigned i = 0; i < HOST_PREFERENCE_ID_SIZE; i++) combined |= identity[i];
    return combined != 0;
}

// Un enregistrement ne contient qu’une version et l’empreinte opaque du bond.
bool host_preference_decode(struct host_preference *preference, const uint8_t *bytes, size_t size) {
    if (bytes == NULL || size != HOST_PREFERENCE_RECORD_SIZE || bytes[0] != 1 ||
        !valid_identity(bytes + 1)) return false;
    *preference = (struct host_preference){.present = true};
    memcpy(preference->identity, bytes + 1, HOST_PREFERENCE_ID_SIZE);
    return true;
}

// La comparaison évite de réécrire la flash à chaque fenêtre sans changement de choix.
bool host_preference_prepare(const struct host_preference *preference,
                             const uint8_t identity[HOST_PREFERENCE_ID_SIZE],
                             uint8_t bytes[HOST_PREFERENCE_RECORD_SIZE]) {
    if (!valid_identity(identity) || (preference->present &&
        memcmp(identity, preference->identity, HOST_PREFERENCE_ID_SIZE) == 0)) return false;
    bytes[0] = 1;
    memcpy(bytes + 1, identity, HOST_PREFERENCE_ID_SIZE);
    return true;
}

// Le choix durable ne change qu’après confirmation du backend NVS.
void host_preference_saved(struct host_preference *preference,
                           const uint8_t bytes[HOST_PREFERENCE_RECORD_SIZE], int error) {
    if (!error) host_preference_decode(preference, bytes, HOST_PREFERENCE_RECORD_SIZE);
    preference->error = error;
}

// L’admission continue sans hôte ne déclenche pas de sauvegarde à une fausse échéance.
void host_session_update(struct host_session_policy *policy, uint32_t generation,
                         uint32_t remaining_ms, bool without_host, uint32_t recovery) {
    if (!generation) return;
    if (!policy->initialized || policy->recovery != recovery) {
        bool manual = !policy->initialized && policy->manual;
        policy->initialized = true;
        policy->recovery = recovery;
        policy->manual = manual;
        policy->restoring = !manual;
        policy->checkpoint = !manual;
        policy->failover = false;
        policy->used = -1;
    }
    if (policy->generation != generation) {
        policy->generation = generation;
    }
    if (remaining_ms == 0 && !without_host) policy->restoring = false;
}

// Un geste sans résultat ne remplace jamais la préférence durable à l’échéance.
void host_session_manual(struct host_session_policy *policy) {
    policy->manual = true;
    policy->restoring = false;
    policy->checkpoint = false;
    policy->failover = false;
}

// Une perte partielle déclenche un repli, une perte totale est traitée par recovery.
void host_session_observe_loss(struct host_session_policy *policy,
                               const struct host_window_view *view, uint16_t connected) {
    if (view->count > HOST_WINDOW_CAPACITY || policy->used < 0 || policy->used >= (int)view->count) return;
    if (view->identities[policy->used] != policy->used_identity) {
        policy->used = -1;
        return;
    }
    if (connected != 0 && !(connected & (UINT16_C(1) << policy->used))) {
        policy->failover = true;
        policy->restoring = false;
        policy->checkpoint = false;
    }
}

// La première sélection au démarrage devient utilisée sans devenir durable immédiatement.
void host_session_used(struct host_session_policy *policy, const struct host_window_view *view) {
    if (view->count > HOST_WINDOW_CAPACITY || view->selected < 0 || view->selected >= (int)view->count ||
        !(view->ready & (UINT16_C(1) << view->selected))) return;
    policy->used = view->selected;
    policy->used_identity = view->identities[view->selected];
}

// Un OS absent ou une topologie invalide ne crée aucune destination implicite.
int host_session_target(const struct host_session_policy *policy,
                         const struct host_window_view *view, int preferred) {
    if (!view->count || view->count > HOST_WINDOW_CAPACITY ||
        view->selected < -1 || view->selected >= (int)view->count ||
        ((uint32_t)view->ready >> view->count) != 0) return HOST_WINDOW_NO_TARGET;
    for (unsigned i = 0; i < view->count; i++) {
        if ((view->ready & (UINT16_C(1) << i)) && !view->identities[i]) return HOST_WINDOW_NO_TARGET;
    }
    if (policy->restoring && preferred >= 0 && preferred < (int)view->count &&
        (view->ready & (UINT16_C(1) << preferred))) {
        return preferred != view->selected ? preferred : HOST_WINDOW_NO_TARGET;
    }
    if (view->selected >= 0 && view->selected < (int)view->count &&
        (view->ready & (UINT16_C(1) << view->selected))) return HOST_WINDOW_NO_TARGET;
    for (unsigned i = 0; i < view->count; i++) {
        if (view->ready & (UINT16_C(1) << i)) return (int)i;
    }
    return HOST_WINDOW_NO_TARGET;
}
