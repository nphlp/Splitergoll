// Politique de sélection et de saisie, testable sans Zephyr ni matériel.
#ifndef SPLITERGOLL_USB_AUTO_STATE_H
#define SPLITERGOLL_USB_AUTO_STATE_H
#include <stdbool.h>
#include <stdint.h>
#include <splitergoll/usb_os_classifier.h>

#define USB_AUTO_POSITION_COUNT 44
#define USB_AUTO_IDLE_MS 750

struct usb_auto_state {
    uint64_t held;
    uint64_t admitted;
    uint32_t generation;
    uint32_t last_activity_ms;
    enum usb_os_guess selected;
};

// Invalide la sélection lors d’un nouveau cycle en gardant les relâchements attendus.
void usb_auto_sync(struct usb_auto_state *state, uint32_t generation, uint32_t now_ms);

// Accepte les nouvelles frappes uniquement après sélection, sans rejouer celles de l’attente.
bool usb_auto_position(struct usb_auto_state *state, uint32_t position, bool pressed,
                       uint32_t now_ms);

// Attend le relâchement complet, la fin des comportements différés et une signature connue.
bool usb_auto_can_select(const struct usb_auto_state *state, enum usb_os_guess guess,
                         bool hid_ready, uint32_t now_ms);
// Lecture de la sélection réellement appliquée, disponible dans le firmware auto.
enum usb_os_guess splitergoll_usb_auto_selected(void);
#endif
