// Classification USB partagée par le clavier et son diagnostic, sans dépendance à Zephyr.
#ifndef SPLITERGOLL_USB_OS_CLASSIFIER_H
#define SPLITERGOLL_USB_OS_CLASSIFIER_H

#include <stdbool.h>
#include <stdint.h>

#define USB_OS_STRING_CAPACITY 16

enum usb_os_guess {
    USB_OS_UNKNOWN,
    USB_OS_MACOS,
    USB_OS_WINDOWS,
    USB_OS_AMBIGUOUS,
};

// Les motifs de noms sont distincts par index de chaîne et langue, sans compter les répétitions.
enum usb_os_evidence {
    USB_OS_DEVICE_8 = 0x01,
    USB_OS_DEVICE_64 = 0x02,
    USB_OS_STRINGS_2 = 0x04,
    USB_OS_STRINGS_4 = 0x08,
    USB_OS_QUALIFIER = 0x10,
    USB_OS_STRINGS_255 = 0x20,
    USB_OS_STRING_OVERFLOW = 0x40,
    USB_OS_STRINGS_262 = 0x80,
};

struct usb_os_string {
    uint16_t language;
    uint8_t index;
    bool probe_2;
    bool probe_4;
    bool pair_2;
    bool pair_4;
    bool bulk;
    bool pair_262;
};

struct usb_os_classifier {
    struct usb_os_string strings[USB_OS_STRING_CAPACITY];
    uint16_t evidence;
    uint8_t string_count;
};

// Ajoute un SETUP complet à un état initialisé à zéro.
void usb_os_observe(struct usb_os_classifier *classifier, const uint8_t setup[8]);

// Retourne l’hypothèse dès que les motifs sont suffisants, concordants et sans perte.
enum usb_os_guess usb_os_result(const struct usb_os_classifier *classifier, bool reliable);

// Retourne un identifiant stable pour le protocole de diagnostic, jamais un nom d’hôte.
const char *usb_os_name(enum usb_os_guess guess);

#endif
