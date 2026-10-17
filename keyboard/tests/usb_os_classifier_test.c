// Vérifie les motifs de noms USB, leur stabilisation et les ambiguïtés sans matériel.
#include <splitergoll/usb_os_classifier.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

// Encode une demande de test et permet de contrôler l’horloge indépendamment du processeur.
static void observe_at(struct usb_os_classifier *classifier, uint8_t type, uint8_t request,
                       uint16_t value, uint16_t index, uint16_t length, uint32_t timestamp_ms) {
    uint8_t setup[8] = {type, request, (uint8_t)value, (uint8_t)(value >> 8),
                        (uint8_t)index, (uint8_t)(index >> 8),
                        (uint8_t)length, (uint8_t)(length >> 8)};
    usb_os_observe(classifier, setup, timestamp_ms);
}

// Place les demandes ordinaires à 100 ms pour lire leurs résultats stabilisés à 1000 ms.
static void observe(struct usb_os_classifier *classifier, uint8_t type, uint8_t request,
                    uint16_t value, uint16_t index, uint16_t length) {
    observe_at(classifier, type, request, value, index, length, 100);
}

// Demande le même nom dans la même langue en deux lectures, sans imposer sa taille finale.
static void string_pair(struct usb_os_classifier *classifier, uint8_t index, uint16_t language,
                        uint16_t probe, uint16_t full_length) {
    observe(classifier, 0x80, 0x06, 0x0300 | index, language, probe);
    observe(classifier, 0x80, 0x06, 0x0300 | index, language, full_length);
}

// Construit un motif Mac avec deux noms différents, sans aucun rapport HID.
static void macos_requests(struct usb_os_classifier *classifier) {
    observe(classifier, 0x80, 0x06, 0x0100, 0, 8);
    observe(classifier, 0x80, 0x06, 0x0100, 0, 18);
    string_pair(classifier, 1, 0x0409, 2, 18);
    string_pair(classifier, 2, 0x0409, 2, 38);
}

// Construit le motif Windows de nos captures avec noms, lecture large et sonde périphérique.
static void windows_requests(struct usb_os_classifier *classifier) {
    observe(classifier, 0x80, 0x06, 0x0100, 0, 64);
    observe(classifier, 0x80, 0x06, 0x0100, 0, 64);
    observe(classifier, 0x80, 0x06, 0x0100, 0, 18);
    observe(classifier, 0x80, 0x06, 0x0301, 0x0409, 255);
    observe(classifier, 0x80, 0x06, 0x0302, 0x0409, 255);
    observe(classifier, 0x80, 0x06, 0x0600, 0, 10);
    string_pair(classifier, 2, 0x0409, 4, 24);
}

// Les tailles complètes des noms et les rapports HID ne déterminent pas l’OS.
static void test_positive_patterns(void) {
    struct usb_os_classifier macos = {0};
    struct usb_os_classifier windows = {0};
    macos_requests(&macos);
    windows_requests(&windows);
    assert(usb_os_result(&macos, 1000, true) == USB_OS_MACOS);
    assert(usb_os_result(&windows, 1000, true) == USB_OS_WINDOWS);
    assert(macos.evidence == (USB_OS_DEVICE_8 | USB_OS_STRINGS_2));
    assert(windows.evidence == (USB_OS_DEVICE_64 | USB_OS_STRINGS_4 |
                                USB_OS_STRINGS_255 | USB_OS_QUALIFIER));
    assert(usb_os_result(&macos, 1000, false) == USB_OS_UNKNOWN);
    assert(usb_os_result(&windows, 1000, false) == USB_OS_UNKNOWN);

    macos = (struct usb_os_classifier){0};
    observe(&macos, 0x80, 0x06, 0x0100, 0, 8);
    string_pair(&macos, 7, 0x040c, 2, 4);
    string_pair(&macos, 9, 0x040c, 2, 98);
    assert(usb_os_result(&macos, 1000, true) == USB_OS_MACOS);
}

// Un nom relu plusieurs fois ou dans deux langues ne vaut pas deux noms indépendants.
static void test_repetition_and_language(void) {
    struct usb_os_classifier classifier = {0};
    observe(&classifier, 0x80, 0x06, 0x0100, 0, 8);
    for (unsigned i = 0; i < 50; i++) {
        string_pair(&classifier, 1, 0x0409, 2, 18);
    }
    string_pair(&classifier, 1, 0x040c, 2, 22);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    assert(classifier.string_count == 2);

    observe(&classifier, 0x80, 0x06, 0x0302, 0x0409, 2);
    observe(&classifier, 0x80, 0x06, 0x0302, 0x040c, 30);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    observe(&classifier, 0x80, 0x06, 0x0302, 0x0409, 30);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_MACOS);

    classifier = (struct usb_os_classifier){0};
    for (unsigned i = 0; i < 50; i++) {
        windows_requests(&classifier);
    }
    assert(classifier.string_count == 2);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_WINDOWS);
}

// Les lectures larges répétées d’un seul nom, même traduit, ne suffisent pas pour Windows.
static void test_windows_bulk_diversity(void) {
    struct usb_os_classifier classifier = {0};
    observe(&classifier, 0x80, 0x06, 0x0100, 0, 64);
    string_pair(&classifier, 1, 0x0409, 4, 18);
    for (unsigned i = 0; i < 50; i++) {
        observe(&classifier, 0x80, 0x06, 0x0301, 0x0409, 255);
        observe(&classifier, 0x80, 0x06, 0x0301, 0x040c, 255);
    }
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    observe(&classifier, 0x80, 0x06, 0x0302, 0x0409, 255);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_WINDOWS);
}

// N’invente pas de résultat à partir d’une sonde initiale ou d’un motif Linux publié seul.
static void test_incomplete_patterns(void) {
    struct usb_os_classifier classifier = {0};
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    observe(&classifier, 0x80, 0x06, 0x0100, 0, 8);
    string_pair(&classifier, 1, 0x0409, 2, 18);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);

    classifier = (struct usb_os_classifier){0};
    observe(&classifier, 0x80, 0x06, 0x0100, 0, 64);
    for (uint8_t i = 1; i <= 3; i++) {
        observe(&classifier, 0x80, 0x06, 0x0300 | i, 0x0409, 255);
    }
    observe(&classifier, 0x80, 0x06, 0x0600, 0, 10);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    string_pair(&classifier, 2, 0x0409, 4, 24);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_WINDOWS);

    classifier = (struct usb_os_classifier){0};
    observe(&classifier, 0x80, 0x06, 0x0100, 0, 64);
    string_pair(&classifier, 1, 0x0409, 4, 18);
    string_pair(&classifier, 2, 0x0409, 4, 22);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    observe(&classifier, 0x80, 0x06, 0x0300, 0, 255);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    observe(&classifier, 0x80, 0x06, 0x0301, 0x0409, 255);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    observe(&classifier, 0x80, 0x06, 0x0302, 0x0409, 255);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_WINDOWS);

}

// Une capture après reset USB peut commencer par 18, avec les motifs de noms complets.
static void test_missing_initial_device_probe(void) {
    struct usb_os_classifier windows = {0};
    observe(&windows, 0x80, 0x06, 0x0100, 0, 18);
    observe(&windows, 0x80, 0x06, 0x0303, 0x0409, 255);
    observe(&windows, 0x80, 0x06, 0x0302, 0x0409, 255);
    observe(&windows, 0x80, 0x06, 0x0600, 0, 10);
    string_pair(&windows, 2, 0x0409, 4, 24);
    assert(windows.evidence == (USB_OS_STRINGS_4 | USB_OS_STRINGS_255 | USB_OS_QUALIFIER));
    assert(usb_os_result(&windows, 349, true) == USB_OS_UNKNOWN);
    assert(usb_os_result(&windows, 350, true) == USB_OS_WINDOWS);
    assert(usb_os_result(&windows, 1000, false) == USB_OS_UNKNOWN);

    struct usb_os_classifier macos = {0};
    observe(&macos, 0x80, 0x06, 0x0100, 0, 18);
    string_pair(&macos, 1, 0x0409, 2, 18);
    string_pair(&macos, 2, 0x0409, 2, 38);
    assert(macos.evidence == USB_OS_STRINGS_2);
    assert(usb_os_result(&macos, 1000, true) == USB_OS_MACOS);

    // Une sonde contraire conserve son rôle de veto, même si elle arrive plus tard.
    observe(&windows, 0x80, 0x06, 0x0100, 0, 8);
    observe(&macos, 0x80, 0x06, 0x0100, 0, 64);
    assert(usb_os_result(&windows, 1000, true) == USB_OS_AMBIGUOUS);
    assert(usb_os_result(&macos, 1000, true) == USB_OS_AMBIGUOUS);
}

// Les relectures doivent suivre le sondage, les fragments dans l’ordre inverse restent insuffisants.
static void test_probe_order(void) {
    struct usb_os_classifier classifier = {0};
    observe(&classifier, 0x80, 0x06, 0x0100, 0, 8);
    string_pair(&classifier, 1, 0x0409, 18, 2);
    string_pair(&classifier, 2, 0x0409, 22, 2);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
    observe(&classifier, 0x80, 0x06, 0x0301, 0x0409, 18);
    observe(&classifier, 0x80, 0x06, 0x0302, 0x0409, 22);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_MACOS);
}

// Attend 250 ms après les demandes utiles et gère le débordement de l’horloge 32 bits.
static void test_settle_time(void) {
    struct usb_os_classifier classifier = {0};
    macos_requests(&classifier);
    assert(usb_os_result(&classifier, 349, true) == USB_OS_UNKNOWN);
    assert(usb_os_result(&classifier, 350, true) == USB_OS_MACOS);
    observe_at(&classifier, 0x80, 0x06, 0x0301, 0x0409, 18, 500);
    assert(usb_os_result(&classifier, 749, true) == USB_OS_UNKNOWN);
    assert(usb_os_result(&classifier, 750, true) == USB_OS_MACOS);
    observe_at(&classifier, 0x80, 0x06, 0x0301, 0x0409, 18, UINT32_MAX - 100);
    assert(usb_os_result(&classifier, 148, true) == USB_OS_UNKNOWN);
    assert(usb_os_result(&classifier, 149, true) == USB_OS_MACOS);
}

// Les requêtes CDC, HID et les langues ne changent ni l’hypothèse ni son horloge.
static void test_noise(void) {
    struct usb_os_classifier classifier = {0};
    macos_requests(&classifier);
    uint16_t evidence = classifier.evidence;
    observe_at(&classifier, 0x21, 0x20, 0, 0, 7, 999);
    observe_at(&classifier, 0x21, 0x22, 3, 0, 0, 999);
    observe_at(&classifier, 0xa1, 0x21, 0, 0, 7, 999);
    observe_at(&classifier, 0x81, 0x06, 0x2200, 255, 240, 999);
    observe_at(&classifier, 0x21, 0x0a, 0, 255, 0, 999);
    observe_at(&classifier, 0x80, 0x06, 0x0300, 0, 255, 999);
    observe_at(&classifier, 0x81, 0x06, 0x0301, 0x0409, 4, 999);
    observe_at(&classifier, 0x80, 0x07, 0x0301, 0x0409, 4, 999);
    observe_at(&classifier, 0x80, 0x06, 0x0100, 1, 64, 999);
    assert(classifier.evidence == evidence);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_MACOS);
}

// Des motifs contraires invalident une hypothèse, même si un côté semblait déjà complet.
static void test_contradictions(void) {
    struct usb_os_classifier classifier = {0};
    macos_requests(&classifier);
    string_pair(&classifier, 3, 0x0409, 4, 18);
    string_pair(&classifier, 4, 0x0409, 4, 22);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_AMBIGUOUS);
    assert(usb_os_result(&classifier, 1000, false) == USB_OS_UNKNOWN);

    classifier = (struct usb_os_classifier){0};
    windows_requests(&classifier);
    macos_requests(&classifier);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_AMBIGUOUS);

    classifier = (struct usb_os_classifier){0};
    observe(&classifier, 0x80, 0x06, 0x0100, 0, 64);
    string_pair(&classifier, 1, 0x0409, 2, 18);
    string_pair(&classifier, 2, 0x0409, 2, 22);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_AMBIGUOUS);
}

// Le nombre de noms suivis est borné et un dépassement ne produit pas un résultat trompeur.
static void test_capacity(void) {
    struct usb_os_classifier classifier = {0};
    macos_requests(&classifier);
    for (uint8_t i = 3; i <= USB_OS_STRING_CAPACITY; i++) {
        string_pair(&classifier, i, 0x0409, 2, 18);
    }
    assert(classifier.string_count == USB_OS_STRING_CAPACITY);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_MACOS);
    string_pair(&classifier, 17, 0x0409, 2, 18);
    assert(classifier.string_count == USB_OS_STRING_CAPACITY);
    assert((classifier.evidence & USB_OS_STRING_OVERFLOW) != 0);
    assert(usb_os_result(&classifier, 1000, true) == USB_OS_UNKNOWN);
}

// Vérifie les noms du protocole et le traitement d’une valeur d’enum invalide.
static void test_names(void) {
    assert(strcmp(usb_os_name(USB_OS_UNKNOWN), "unknown") == 0);
    assert(strcmp(usb_os_name(USB_OS_MACOS), "macos") == 0);
    assert(strcmp(usb_os_name(USB_OS_WINDOWS), "windows") == 0);
    assert(strcmp(usb_os_name(USB_OS_AMBIGUOUS), "ambiguous") == 0);
    assert(strcmp(usb_os_name((enum usb_os_guess)-1), "unknown") == 0);
}

// Lance les tests du classifieur réel avec assertions et sanitizers côté hôte.
int main(void) {
    test_positive_patterns();
    test_repetition_and_language();
    test_windows_bulk_diversity();
    test_incomplete_patterns();
    test_missing_initial_device_probe();
    test_probe_order();
    test_settle_time();
    test_noise();
    test_contradictions();
    test_capacity();
    test_names();
    puts("USB OS classifier: OK");
    return 0;
}
