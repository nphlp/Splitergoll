// Préférence durable de destination, indépendante du slot actif et de l’OS.
#ifndef SPLITERGOLL_HOST_PREFERENCE_H
#define SPLITERGOLL_HOST_PREFERENCE_H
#include <splitergoll/host_window_policy.h>
#include <stddef.h>

#define HOST_PREFERENCE_ID_SIZE 32
#define HOST_PREFERENCE_RECORD_SIZE (1 + HOST_PREFERENCE_ID_SIZE)

struct host_preference {
    uint8_t identity[HOST_PREFERENCE_ID_SIZE];
    bool present;
    int error;
};

struct host_session_policy {
    uint32_t generation;
    bool initialized;
    bool restoring;
    bool checkpoint;
};

// Refuse les formats inconnus, tronqués ou les identités vides.
bool host_preference_decode(struct host_preference *preference, const uint8_t *bytes, size_t size);

// Prépare une écriture seulement si l’identité sélectionnée diffère du choix durable.
bool host_preference_prepare(const struct host_preference *preference,
                             const uint8_t identity[HOST_PREFERENCE_ID_SIZE],
                             uint8_t bytes[HOST_PREFERENCE_RECORD_SIZE]);

// Conserve l’ancienne préférence si l’écriture échoue, sans présenter une réussite fictive.
void host_preference_saved(struct host_preference *preference,
                           const uint8_t bytes[HOST_PREFERENCE_RECORD_SIZE], int error);

// Suit l’échéance radio et arme une seule sauvegarde par fenêtre.
void host_session_update(struct host_session_policy *policy, uint32_t generation,
                         uint32_t remaining_ms, bool without_host);

// Sélectionne le précédent pendant le démarrage, sinon conserve une destination utilisable.
int host_session_target(const struct host_session_policy *policy,
                         const struct host_window_view *view, int preferred);
#endif
