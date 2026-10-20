// Vérifie les hypothèses BLE, les preuves manquantes et l’isolation des sessions.
#include "ble_os_classifier.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct request {
    uint32_t data[5];
    uint16_t uuid;
};

// Extraits anonymes des requêtes observées, sans valeurs, radio, horloge ni nom d’hôte.
static const struct request macos[] = {
    {{0x08, 7, 0x1b, 0x21, 0x2a29}, 0},
    {{0x08, 7, 0x1b, 0x21, 0x2a24}, 0},
    {{0x08, 7, 0x1b, 0x21, 0x2a50}, 0},
    {{0x0a, 3, 0x1d, 0, 0}, 0x2a24},
    {{0x0a, 3, 0x1f, 0, 0}, 0x2a29},
    {{0x0a, 3, 0x21, 0, 0}, 0x2a50},
};

static const struct request windows[] = {
    {{0x12, 4, 0x06, 0, 0}, 0x2b29},
    {{0x0a, 3, 0x08, 0, 0}, 0x2b2a},
    {{0x08, 7, 0x22, 0x34, 0x2802}, 0},
    {{0x0a, 3, 0x0d, 0, 0}, 0x2a01},
};

// Produit un événement sans contenu privé sur la session courante.
static void event(struct ble_os_classifier *classifier, enum ble_trace_kind kind,
                   uint32_t a, uint32_t b) {
    struct ble_trace_record record = {.session = classifier->session, .kind = kind,
                                      .data = {a, b, 0, 0, 0}};
    ble_os_observe(classifier, &record, 0);
}

// Ouvre une connexion chiffrée, sans assimiler le numéro de session à un OS.
static void connect(struct ble_os_classifier *classifier, uint32_t session) {
    struct ble_trace_record record = {.session = session, .kind = BLE_TRACE_CONNECTED};
    ble_os_observe(classifier, &record, 0);
    event(classifier, BLE_TRACE_SECURITY, 2, 0);
}

// Passe une requête et son UUID de handle résolu au même C que le rejeu privé.
static void observe(struct ble_os_classifier *classifier, struct request request) {
    struct ble_trace_record record = {.session = classifier->session, .kind = BLE_TRACE_ATT};
    memcpy(record.data, request.data, sizeof(record.data));
    ble_os_observe(classifier, &record, request.uuid);
}

// Rejoue un extrait sans inventer de découverte pour les sessions qui n’en ont pas.
static void sequence(struct ble_os_classifier *classifier, const struct request *requests,
                      size_t count) {
    for (size_t i = 0; i < count; i++) {
        observe(classifier, requests[i]);
    }
}

// Les motifs complets sont nécessaires, et aucune perte ne peut confirmer une hypothèse.
static void complete_and_partial(void) {
    const struct request *fixtures[] = {macos, windows};
    const size_t counts[] = {6, 4};
    const enum ble_os_guess guesses[] = {BLE_OS_MACOS, BLE_OS_WINDOWS};
    for (size_t fixture = 0; fixture < 2; fixture++) {
        struct ble_os_classifier classifier = {0};
        assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
        connect(&classifier, 1);
        for (size_t i = 0; i < counts[fixture]; i++) {
            assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
            observe(&classifier, fixtures[fixture][i]);
        }
        assert(ble_os_result(&classifier, true) == guesses[fixture]);
        assert(ble_os_result(&classifier, false) == BLE_OS_UNKNOWN);
        struct ble_os_classifier copy = classifier;
        (void)ble_os_result(&classifier, true);
        assert(memcmp(&copy, &classifier, sizeof(copy)) == 0);
        for (size_t missing = 0; missing < counts[fixture]; missing++) {
            classifier = (struct ble_os_classifier){0};
            connect(&classifier, 1);
            for (size_t i = 0; i < counts[fixture]; i++) {
                if (i != missing) observe(&classifier, fixtures[fixture][i]);
            }
            assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
        }
    }
}

// Une radio, le MTU, le HID ou des lectures DIS seules ne déterminent aucun OS.
static void generic_and_cached(void) {
    struct ble_os_classifier classifier = {0};
    connect(&classifier, 1);
    for (uint32_t i = 0; i < 256; i++) {
        event(&classifier, BLE_TRACE_FEATURES, i * 0x1010101, UINT32_MAX - i);
        event(&classifier, BLE_TRACE_PARAM_REQUEST, i, i + 6);
        event(&classifier, BLE_TRACE_PARAM_UPDATED, i, i + 1);
        event(&classifier, BLE_TRACE_MTU, i, i + 1);
        observe(&classifier, (struct request){{2, 3, i, 0, 0}, 0});
    }
    const struct request common[] = {
        {{0x0a, 3, 0x24, 0, 0}, 0x2a4a},
        {{0x0a, 3, 0x26, 0, 0}, 0x2a4b},
        {{0x0a, 3, 0x21, 0, 0}, 0x2a50},
        {{0x0a, 3, 0x0d, 0, 0}, 0x2a01},
        {{0x0a, 3, 0x12, 0, 0}, 0x2a19},
    };
    sequence(&classifier, common, sizeof(common) / sizeof(common[0]));
    assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    // Une reconnexion Windows limitée au MTU et à la batterie reste inconnue.
    event(&classifier, BLE_TRACE_DISCONNECTED, 0x13, 0);
    connect(&classifier, 2);
    observe(&classifier, (struct request){{2, 3, 527, 0, 0}, 0});
    observe(&classifier, common[4]);
    assert(classifier.evidence == 0 && ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
}

// Les répétitions ne remplacent pas les preuves, les handles ne sont jamais figés.
static void repetitions_and_handles(void) {
    for (unsigned offset = 0; offset <= 0xffde; offset += 0x111) {
        struct ble_os_classifier classifier = {0};
        connect(&classifier, 7);
        for (size_t i = 0; i < 6; i++) {
            struct request request = macos[i];
            request.data[2] += offset;
            if (request.data[0] == 0x08) request.data[3] += offset;
            observe(&classifier, request);
            observe(&classifier, request);
        }
        assert(ble_os_result(&classifier, true) == BLE_OS_MACOS);
    }
    for (size_t repeated = 0; repeated < 6; repeated++) {
        struct ble_os_classifier classifier = {0};
        connect(&classifier, 1);
        for (size_t i = 0; i < 200; i++) observe(&classifier, macos[repeated]);
        assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    }
    struct ble_os_classifier classifier = {0};
    connect(&classifier, 1);
    for (size_t i = 0; i < 4; i++) {
        struct request request = windows[i];
        request.data[2] += 0x400;
        if (request.data[0] == 0x08) request.data[3] += 0x400;
        for (size_t j = 0; j < 20; j++) observe(&classifier, request);
    }
    assert(ble_os_result(&classifier, true) == BLE_OS_WINDOWS);
}

// Les recherches Mac doivent rester ordonnées et viser une même plage de service.
static void ranges_and_order(void) {
    const unsigned permutations[][3] = {{0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
    for (size_t p = 0; p < 5; p++) {
        struct ble_os_classifier classifier = {0};
        connect(&classifier, 1);
        for (size_t i = 0; i < 3; i++) observe(&classifier, macos[permutations[p][i]]);
        sequence(&classifier, macos + 3, 3);
        assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    }
    for (size_t changed = 1; changed < 6; changed++) {
        struct ble_os_classifier classifier = {0};
        connect(&classifier, 1);
        for (size_t i = 0; i < 6; i++) {
            struct request request = macos[i];
            if (i == changed) {
                request.data[2] += 100;
                if (i < 3) request.data[3] += 100;
            }
            observe(&classifier, request);
        }
        assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    }
    struct ble_os_classifier classifier = {0};
    connect(&classifier, 1);
    sequence(&classifier, macos + 3, 3);
    sequence(&classifier, macos, 3);
    assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    // Les UUID non résolus ne doivent pas être devinés à partir des handles historiques.
    for (size_t i = 3; i < 6; i++) {
        struct request request = macos[i];
        request.uuid = 0;
        observe(&classifier, request);
    }
    assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
}

// Les familles contradictoires restent ambiguës, quel que soit leur ordre d’arrivée.
static void contradictions(void) {
    for (unsigned reverse = 0; reverse < 2; reverse++) {
        struct ble_os_classifier classifier = {0};
        connect(&classifier, 1);
        if (reverse) {
            sequence(&classifier, windows, 4);
            sequence(&classifier, macos, 6);
        } else {
            sequence(&classifier, macos, 6);
            sequence(&classifier, windows, 4);
        }
        assert(classifier.evidence == 0x3f);
        assert(ble_os_result(&classifier, true) == BLE_OS_AMBIGUOUS);
        assert(ble_os_result(&classifier, false) == BLE_OS_UNKNOWN);
    }
}

// Les preuves et le chiffrement ne survivent jamais au changement de connexion.
static void sessions_and_security(void) {
    struct ble_os_classifier classifier = {0};
    connect(&classifier, 1);
    sequence(&classifier, macos, 6);
    assert(ble_os_result(&classifier, true) == BLE_OS_MACOS);
    event(&classifier, BLE_TRACE_DISCONNECTED, 0x13, 0);
    assert(ble_os_result(&classifier, true) == BLE_OS_MACOS);
    connect(&classifier, 2);
    assert(classifier.evidence == 0 && classifier.dis_step == 0 && classifier.dis_reads == 0);
    assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    sequence(&classifier, windows, 4);
    assert(ble_os_result(&classifier, true) == BLE_OS_WINDOWS);
    event(&classifier, BLE_TRACE_SECURITY, 1, 0);
    assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    event(&classifier, BLE_TRACE_SECURITY, 2, 4);
    event(&classifier, BLE_TRACE_SECURITY, 2, 0);
    assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    event(&classifier, BLE_TRACE_DISCONNECTED, 0x13, 0);
    connect(&classifier, 3);
    sequence(&classifier, macos, 6);
    assert(ble_os_result(&classifier, true) == BLE_OS_MACOS);
    classifier = (struct ble_os_classifier){0};
    struct ble_trace_record connected = {.session = 1, .kind = BLE_TRACE_CONNECTED};
    ble_os_observe(&classifier, &connected, 0);
    sequence(&classifier, macos, 6);
    assert(ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    event(&classifier, BLE_TRACE_SECURITY, 2, 0);
    assert(ble_os_result(&classifier, true) == BLE_OS_MACOS);
}

// Refuse les tailles, bornes, sessions mélangées et événements incohérents.
static void malformed(void) {
    const struct request invalid[] = {
        {{0x08, 6, 0x1b, 0x21, 0x2a29}, 0},
        {{0x08, 7, 0, 0x21, 0x2a29}, 0},
        {{0x08, 7, 0x22, 0x21, 0x2a29}, 0},
        {{0x08, 7, 1, 0x10000, 0x2a29}, 0},
        {{0x08, 7, 1, 2, 0x10000}, 0},
        {{0x08, 21, 1, 2, 0x2a29}, 0},
        {{0x0a, 4, 1, 0, 0}, 0x2a29},
        {{0x0a, 3, 0, 0, 0}, 0x2a29},
        {{0x0a, 3, 0x10000, 0, 0}, 0x2a29},
        {{0x0a, 3, 1, 1, 0}, 0x2a29},
        {{0x12, 2, 1, 0, 0}, 0x2b29},
        {{0x12, 0x10000, 1, 0, 0}, 0x2b29},
    };
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        struct ble_os_classifier classifier = {0};
        connect(&classifier, 1);
        sequence(&classifier, windows, 4);
        observe(&classifier, invalid[i]);
        assert(classifier.invalid && ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    }
    for (unsigned error = 0; error < 7; error++) {
        struct ble_os_classifier classifier = {0};
        connect(&classifier, 1);
        sequence(&classifier, macos, 6);
        struct ble_trace_record record = {.session = 1, .kind = BLE_TRACE_ATT};
        switch (error) {
            case 0: record.session = 2; break;
            case 1: record.kind = BLE_TRACE_KIND_COUNT; break;
            case 2: event(&classifier, BLE_TRACE_DISCONNECTED, 0, 0); break;
            case 3: record.kind = BLE_TRACE_CONNECTED; break;
            case 4: record.kind = BLE_TRACE_CONNECTED; record.session = 0; break;
            case 5:
                event(&classifier, BLE_TRACE_DISCONNECTED, 0, 0);
                record.kind = BLE_TRACE_CONNECTED;
                record.session = 3;
                break;
            default: ble_os_observe(&classifier, NULL, 0); break;
        }
        if (error != 6) ble_os_observe(&classifier, &record, 0);
        assert(classifier.invalid && ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
    }
    struct ble_os_classifier classifier = {0};
    event(&classifier, BLE_TRACE_SECURITY, 2, 0);
    assert(classifier.invalid && ble_os_result(&classifier, true) == BLE_OS_UNKNOWN);
}

// Des preuves complémentaires sur deux hôtes ne doivent jamais former une seule signature.
static void concurrent_proofs(void) {
    struct ble_os_classifier first = {0}, second = {0}, third = {0};
    connect(&first, 4);
    connect(&second, 9);
    connect(&third, 12);
    for (size_t i = 0; i < 6; i++) {
        observe(i < 3 ? &first : &second, macos[i]);
        if (i < 4) observe(&third, windows[i]);
    }
    assert(ble_os_result(&first, true) == BLE_OS_UNKNOWN);
    assert(ble_os_result(&second, true) == BLE_OS_UNKNOWN);
    assert(ble_os_result(&third, true) == BLE_OS_WINDOWS);
    sequence(&first, macos + 3, 3);
    sequence(&second, macos, 6);
    assert(ble_os_result(&first, true) == BLE_OS_MACOS);
    assert(ble_os_result(&second, true) == BLE_OS_MACOS);
    event(&first, BLE_TRACE_SECURITY, 1, 4);
    assert(ble_os_result(&first, true) == BLE_OS_UNKNOWN);
    assert(ble_os_result(&second, true) == BLE_OS_MACOS);
    assert(ble_os_result(&third, true) == BLE_OS_WINDOWS);
}

// Le protocole conserve des noms définis, y compris pour un enum hors limites.
static void names(void) {
    assert(strcmp(ble_os_name(BLE_OS_UNKNOWN), "unknown") == 0);
    assert(strcmp(ble_os_name(BLE_OS_MACOS), "macos") == 0);
    assert(strcmp(ble_os_name(BLE_OS_WINDOWS), "windows") == 0);
    assert(strcmp(ble_os_name(BLE_OS_AMBIGUOUS), "ambiguous") == 0);
    assert(strcmp(ble_os_name((enum ble_os_guess)-1), "unknown") == 0);
}

// Exécute les cas sans Zephyr, USB, Bluetooth matériel ou changement de keymap.
int main(void) {
    complete_and_partial();
    generic_and_cached();
    repetitions_and_handles();
    ranges_and_order();
    contradictions();
    sessions_and_security();
    malformed();
    concurrent_proofs();
    names();
    puts("BLE OS classifier: OK");
    return 0;
}
