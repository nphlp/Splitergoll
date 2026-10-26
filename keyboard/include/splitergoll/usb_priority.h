// Autorisation de la sortie USB à partir du rapprochement, sans identité persistée.
#pragma once
#include <stdbool.h>
#include <stdint.h>

struct usb_priority_evidence {
    bool context_valid, matched;
    int profile;
    uint32_t usb_context, ble_context, lease_ms;
};
struct usb_priority_state {
    bool enabled, pending;
    struct usb_priority_evidence accepted;
};

// Vérifie le contexte autorisé même si un callback natif précède le prochain worker.
bool usb_priority_allowed(const struct usb_priority_state *state, struct usb_priority_evidence evidence);
// Les changements de contexte révoquent immédiatement, les transitions ordinaires attendent les relâchements.
bool usb_priority_step(struct usb_priority_state *state, struct usb_priority_evidence evidence, bool busy);
// Instantané borné du moteur d’identité, sans UUID exporté.
struct usb_priority_evidence splitergoll_identity_evidence(unsigned profile, uint32_t now);
// Appelé uniquement par le worker du sélecteur, renvoie la prochaine expiration utile.
uint32_t splitergoll_usb_priority_update(void);
// Permet au sélecteur de se réveiller sur le relâchement d’un ancien appui.
bool splitergoll_usb_priority_pending(void);
