// Mémoire de destination propre au clavier, séparée des réglages natifs ZMK.
#ifndef SPLITERGOLL_HOST_PREFERENCE_SETTINGS_H
#define SPLITERGOLL_HOST_PREFERENCE_SETTINGS_H
#include <stdbool.h>

struct host_preference_status {
    int profile;
    int error;
    bool loaded;
    bool present;
};

// Résout le choix sauvegardé parmi les bonds dont l’identité a été vérifiée.
struct host_preference_status splitergoll_host_preference_status(void);

// Persiste un hôte choisi par le worker, sans effacer de bond ni modifier sa keymap.
void splitergoll_host_preference_save(unsigned profile);
#endif
