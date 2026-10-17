// Implémentation propre à Splitergoll à partir de motifs USB publics et de nos captures.
// Les demandes de noms suivent des motifs étudiés par QMK et FingerprintUSBHost.
// Aucune copie de leur code, aucun descripteur modifié et aucun support BOS ajouté.
#include "usb_os_classifier.h"

#include <stddef.h>

// Décode les champs USB little-endian sans accès non aligné ni dépendance au processeur.
static uint16_t read_u16(const uint8_t *bytes) {
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

// Retrouve une chaîne dans sa langue sans faire grandir l’état au-delà de sa capacité.
static struct usb_os_string *find_string(struct usb_os_classifier *classifier,
                                         uint8_t index, uint16_t language) {
    for (uint8_t i = 0; i < classifier->string_count; i++) {
        struct usb_os_string *string = &classifier->strings[i];
        if (string->index == index && string->language == language) {
            return string;
        }
    }

    if (classifier->string_count == USB_OS_STRING_CAPACITY) {
        classifier->evidence |= USB_OS_STRING_OVERFLOW;
        return NULL;
    }

    struct usb_os_string *string = &classifier->strings[classifier->string_count++];
    string->index = index;
    string->language = language;
    return string;
}

// Combine des noms distincts sans imposer leur longueur complète ni compter les répétitions.
static void update_string_evidence(struct usb_os_classifier *classifier) {
    unsigned pairs_2 = 0;
    unsigned pairs_4 = 0;
    unsigned bulk_strings = 0;
    for (uint8_t i = 0; i < classifier->string_count; i++) {
        bool counted_2 = false;
        bool counted_4 = false;
        bool counted_bulk = false;
        for (uint8_t j = 0; j < i; j++) {
            if (classifier->strings[j].index == classifier->strings[i].index) {
                counted_2 |= classifier->strings[j].pair_2;
                counted_4 |= classifier->strings[j].pair_4;
                counted_bulk |= classifier->strings[j].bulk;
            }
        }
        pairs_2 += classifier->strings[i].pair_2 && !counted_2;
        pairs_4 += classifier->strings[i].pair_4 && !counted_4;
        bulk_strings += classifier->strings[i].bulk && !counted_bulk;
    }
    if (pairs_2 >= 2) {
        classifier->evidence |= USB_OS_STRINGS_2;
    }
    if (pairs_4 >= 1) {
        classifier->evidence |= USB_OS_STRINGS_4;
    }
    if (bulk_strings >= 2) {
        classifier->evidence |= USB_OS_STRINGS_255;
    }
}

// Associe le petit sondage et sa relecture à la même chaîne, dans le bon ordre.
static void observe_string(struct usb_os_classifier *classifier, uint8_t index,
                            uint16_t language, uint16_t length) {
    // L’index zéro décrit les langues et ne suit pas forcément le motif des noms.
    if (index == 0 || length < 2) {
        return;
    }
    struct usb_os_string *string = find_string(classifier, index, language);
    if (string == NULL) {
        return;
    }

    string->bulk |= length == 255;
    if (length == 2) {
        string->probe_2 = true;
    } else {
        // Une chaîne d’un seul caractère a une longueur complète de 4 octets.
        if (string->probe_2 && length < 255) {
            string->pair_2 = true;
        }
        if (length == 4 && !string->probe_2) {
            string->probe_4 = true;
        } else if (length > 4 && string->probe_4) {
            string->pair_4 = true;
        }
    }
    update_string_evidence(classifier);
}

// Observe uniquement les descripteurs standard, sans utiliser les requêtes CDC ou les tailles HID.
void usb_os_observe(struct usb_os_classifier *classifier, const uint8_t setup[8],
                    uint32_t timestamp_ms) {
    uint16_t value = read_u16(&setup[2]);
    uint16_t index = read_u16(&setup[4]);
    uint16_t length = read_u16(&setup[6]);
    bool descriptor_request = setup[0] == 0x80 && setup[1] == 0x06;
    if (!descriptor_request) {
        return;
    }

    bool device = value == 0x0100 && index == 0;
    bool string = (value >> 8) == 0x03 && (value & 0xff) != 0 && length >= 2;
    bool qualifier = value == 0x0600 && index == 0 && length == 10;
    if (!device && !string && !qualifier) {
        return;
    }
    classifier->last_observation_ms = timestamp_ms;

    if (device) {
        if (length == 8) {
            classifier->evidence |= USB_OS_DEVICE_8;
        } else if (length == 64) {
            classifier->evidence |= USB_OS_DEVICE_64;
        }
    }
    if (string) {
        observe_string(classifier, (uint8_t)value, index, length);
    }
    if (qualifier) {
        classifier->evidence |= USB_OS_QUALIFIER;
    }
}

// Refuse les contradictions, les pertes et les motifs encore en train de se compléter.
enum usb_os_guess usb_os_result(const struct usb_os_classifier *classifier, uint32_t now_ms,
                                bool reliable) {
    if (!reliable || (classifier->evidence & USB_OS_STRING_OVERFLOW) != 0) {
        return USB_OS_UNKNOWN;
    }

    uint16_t probes = USB_OS_DEVICE_8 | USB_OS_DEVICE_64;
    bool mixed_device_probes = (classifier->evidence & probes) == probes;
    bool macos_pattern = (classifier->evidence & USB_OS_STRINGS_2) != 0;
    bool windows_pattern = (classifier->evidence & USB_OS_STRINGS_4) != 0;
    bool contradictory = mixed_device_probes || (macos_pattern && windows_pattern) ||
                         (macos_pattern && (classifier->evidence & USB_OS_DEVICE_64) != 0) ||
                         (windows_pattern && (classifier->evidence & USB_OS_DEVICE_8) != 0);
    if (contradictory) {
        return USB_OS_AMBIGUOUS;
    }

    uint32_t quiet_ms = now_ms - classifier->last_observation_ms;
    if (quiet_ms < USB_OS_SETTLE_MS) {
        return USB_OS_UNKNOWN;
    }
    // Un reset USB peut précéder notre instantané : les sondages 8/64 ne sont pas obligatoires.
    if (macos_pattern) {
        return USB_OS_MACOS;
    }
    bool bulk_string = (classifier->evidence & USB_OS_STRINGS_255) != 0;
    if (windows_pattern && bulk_string) {
        return USB_OS_WINDOWS;
    }
    return USB_OS_UNKNOWN;
}

// Garde une sortie définie même pour une valeur d’enum non reconnue.
const char *usb_os_name(enum usb_os_guess guess) {
    switch (guess) {
    case USB_OS_MACOS:
        return "macos";
    case USB_OS_WINDOWS:
        return "windows";
    case USB_OS_AMBIGUOUS:
        return "ambiguous";
    default:
        return "unknown";
    }
}
