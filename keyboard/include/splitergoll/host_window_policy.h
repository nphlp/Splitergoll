// Politique pure des fenêtres hôtes, indépendante des callbacks Bluetooth et des keymaps.
#ifndef SPLITERGOLL_HOST_WINDOW_POLICY_H
#define SPLITERGOLL_HOST_WINDOW_POLICY_H
#include <stdbool.h>
#include <stdint.h>

#define HOST_WINDOW_CAPACITY 16
#define HOST_WINDOW_DURATION_MS UINT32_C(120000)
#define HOST_WINDOW_NO_TARGET (-1)

struct host_window_view {
    unsigned count;
    uint16_t ready;
    int selected;
    uint32_t identities[HOST_WINDOW_CAPACITY];
};

struct host_window_policy {
    uint32_t opened_at_ms;
    uint32_t generation;
    uint16_t initial_ready;
    int source;
    uint32_t source_identity;
    int pending;
    uint32_t pending_identity;
    bool active;
    bool searching;
    bool pending_search;
    bool gaming_usb;
};

// Ouvre la fenêtre de démarrage, sans demander de changement de destination.
void host_window_start(struct host_window_policy *policy, uint32_t now_ms);

// Réarme deux minutes et prépare une recherche ou une bascule selon l’instantané fourni.
bool host_window_shortcut(struct host_window_policy *policy, const struct host_window_view *view,
                          uint32_t now_ms);

// Ferme les annonces et demandes, seulement si une vraie sortie Gaming USB est disponible.
bool host_window_enter_gaming(struct host_window_policy *policy, bool usb_usable);

// La sortie ordinaire de Gaming ouvre deux minutes sans recherche de nouvel appareil.
bool host_window_leave_gaming(struct host_window_policy *policy, uint32_t now_ms);

// Observe des destinations prêtes, sans sélectionner ni modifier un appareil.
void host_window_update(struct host_window_policy *policy, const struct host_window_view *view,
                         uint32_t now_ms);

// Consomme une décision une seule fois, avec ticket courant et saisie au repos.
int host_window_take(struct host_window_policy *policy, const struct host_window_view *view,
                      uint32_t now_ms, uint32_t generation, bool input_busy);

// Retourne le temps restant, sans attente fixe à ajouter à la saisie.
uint32_t host_window_remaining(const struct host_window_policy *policy, uint32_t now_ms);

// Une fenêtre ouverte ne permet aucun nouvel appairage sans slot libre.
bool host_window_pairing_allowed(const struct host_window_policy *policy, uint32_t now_ms,
                                 bool free_slot);

// Une sélection explicite externe annule les décisions en attente, pas les connexions.
void host_window_cancel_selection(struct host_window_policy *policy);

struct host_discovery_policy {
    uint32_t opened_at_ms;
    uint32_t generation;
    uint32_t recovery;
    uint16_t connected;
    bool awake;
    bool sleeping;
    bool opened;
};

// Démarre la recherche, prolongée sans hôte tant que le clavier reste actif.
void host_discovery_start(struct host_discovery_policy *policy, uint32_t now_ms);

// Ouvre une fenêtre explicite, sans changer les appareils connectés.
void host_discovery_open(struct host_discovery_policy *policy, uint32_t now_ms);

// Le premier retour ouvre une fenêtre bornée, une perte partielle ne la réarme pas.
void host_discovery_update(struct host_discovery_policy *policy, uint32_t now_ms,
                           uint16_t connected, bool awake, bool sleeping);

// Sans hôte actif, retourne la durée nominale pour indiquer une admission continue.
uint32_t host_discovery_remaining(const struct host_discovery_policy *policy, uint32_t now_ms);
#endif
