// Observation partagée, sans contenu de frappe ni modification des réponses USB.
#ifndef SPLITERGOLL_USB_OBSERVER_H
#define SPLITERGOLL_USB_OBSERVER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <splitergoll/usb_os_classifier.h>

struct usb_os_snapshot {
    uint32_t generation;
    enum usb_os_guess guess;
};

// Lit une hypothèse fondée sur les requêtes déjà reçues et son cycle USB.
struct usb_os_snapshot splitergoll_usb_os_snapshot(void);

// Réveille le sélecteur hors du callback USB, uniquement dans le profil auto.
void splitergoll_usb_auto_notify(void);

// Points de capture facultatifs, implémentés uniquement par le module de diagnostic.
void splitergoll_usb_trace_reset(void);
void splitergoll_usb_trace_record(uint32_t timestamp_ms, const uint8_t *data, size_t length);
#endif
