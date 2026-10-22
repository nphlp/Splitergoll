// Vérifie la préférence indépendante de ZMK, ses erreurs et les deux ordres de reconnexion.
#include <splitergoll/host_preference.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

// Une nouvelle préférence reste provisoire jusqu’au succès de l’écriture NVS.
static void persistence_and_errors(void) {
    uint8_t a[HOST_PREFERENCE_ID_SIZE] = {1}, b[HOST_PREFERENCE_ID_SIZE] = {2};
    uint8_t bytes[HOST_PREFERENCE_RECORD_SIZE];
    struct host_preference p = {0}, restored = {0};
    assert(host_preference_prepare(&p, a, bytes));
    assert(!p.present);
    host_preference_saved(&p, bytes, -ENOSPC);
    assert(!p.present && p.error == -ENOSPC);
    host_preference_saved(&p, bytes, 0);
    assert(p.present && !p.error && !memcmp(p.identity, a, sizeof(a)));
    assert(host_preference_decode(&restored, bytes, sizeof(bytes)));
    assert(!host_preference_prepare(&restored, a, bytes));
    assert(host_preference_prepare(&p, b, bytes));
    host_preference_saved(&p, bytes, -EIO);
    assert(!memcmp(p.identity, a, sizeof(a)) && p.error == -EIO);
    host_preference_saved(&p, bytes, 0);
    assert(!memcmp(p.identity, b, sizeof(b)) && !p.error);
    for (unsigned i = 0; i < sizeof(b); i++) {
        uint8_t replaced[sizeof(b)];
        memcpy(replaced, b, sizeof(b));
        replaced[i] ^= 0x80;
        assert(host_preference_prepare(&p, replaced, bytes));
    }
}

// Les données futures ou incomplètes ne deviennent jamais une préférence valide.
static void malformed_records(void) {
    struct host_preference p = {0};
    uint8_t bytes[HOST_PREFERENCE_RECORD_SIZE] = {1, 2};
    assert(!host_preference_decode(&p, NULL, sizeof(bytes)));
    for (unsigned size = 0; size < sizeof(bytes); size++) {
        assert(!host_preference_decode(&p, bytes, size));
    }
    assert(!host_preference_decode(&p, bytes, sizeof(bytes) + 1));
    bytes[0] = 2;
    assert(!host_preference_decode(&p, bytes, sizeof(bytes)));
    bytes[0] = 1;
    bytes[1] = 0;
    assert(!host_preference_decode(&p, bytes, sizeof(bytes)));
    uint8_t empty[HOST_PREFERENCE_ID_SIZE] = {0};
    assert(!host_preference_prepare(&p, empty, bytes));
    assert(!host_preference_prepare(&p, NULL, bytes));
}

// L’ordre d’arrivée ne remplace pas la préférence et une frappe n’entre pas dans la décision.
static void reconnect_orders(void) {
    for (int preferred = 0; preferred < 2; preferred++) {
        int other = 1 - preferred;
        struct host_session_policy p = {0};
        struct host_window_view v = {.count = 2, .selected = preferred, .identities = {1, 2}};
        host_session_update(&p, 1, HOST_WINDOW_DURATION_MS, true);
        assert(p.restoring && p.checkpoint && host_session_target(&p, &v, preferred) == -1);
        v.ready = (uint16_t)(UINT16_C(1) << other);
        host_session_update(&p, 2, HOST_WINDOW_DURATION_MS, false);
        assert(host_session_target(&p, &v, preferred) == other);
        v.selected = other;
        assert(host_session_target(&p, &v, preferred) == -1);
        v.ready = 3;
        assert(host_session_target(&p, &v, preferred) == preferred);
        host_session_update(&p, 2, 0, false);
        assert(!p.restoring && host_session_target(&p, &v, preferred) == -1);
        v.ready = (uint16_t)(UINT16_C(1) << preferred);
        assert(host_session_target(&p, &v, preferred) == preferred);
        v.selected = preferred;
        assert(host_session_target(&p, &v, preferred) == -1);
        p.checkpoint = false;
        host_session_update(&p, 2, 0, false);
        assert(!p.checkpoint);
        host_session_update(&p, 3, HOST_WINDOW_DURATION_MS, false);
        assert(!p.restoring && p.checkpoint);
    }
}

// Un geste manuel annule le rappel sans empêcher le repli si sa destination disparaît.
static void manual_choice_and_invalid_views(void) {
    struct host_session_policy p = {0};
    struct host_window_view v = {.count = 2, .selected = 1, .ready = 3, .identities = {1, 2}};
    host_session_update(&p, 1, 100, false);
    p.restoring = false;
    assert(host_session_target(&p, &v, 0) == -1);
    v.ready = 1;
    assert(host_session_target(&p, &v, 0) == 0);
    v.ready = 4;
    assert(host_session_target(&p, &v, 0) == -1);
    v.ready = 1;
    v.identities[0] = 0;
    assert(host_session_target(&p, &v, 0) == -1);
    v.count = 0;
    assert(host_session_target(&p, &v, 0) == -1);
    v.count = 17;
    assert(host_session_target(&p, &v, 0) == -1);
}

// Exécute les règles sans radio, minuterie réelle ou accès à la mémoire du clavier.
int main(void) {
    persistence_and_errors();
    malformed_records();
    reconnect_orders();
    manual_choice_and_invalid_views();
    puts("host_preference: ok");
    return 0;
}
