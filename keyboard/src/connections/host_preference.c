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
                         uint32_t remaining_ms, bool without_host) {
    if (!generation) return;
    if (!policy->initialized) {
        policy->initialized = true;
        policy->restoring = true;
    }
    if (policy->generation != generation) {
        policy->generation = generation;
        policy->checkpoint = true;
    }
    if (remaining_ms == 0 && !without_host) policy->restoring = false;
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
