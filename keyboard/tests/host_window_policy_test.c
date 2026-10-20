// Vérifie les décisions de connexion, sans appareil, radio ni modification des appairages.
#include <splitergoll/host_window_policy.h>
#include <assert.h>
#include <stdio.h>

// Des jetons stables représentent les identités déjà vérifiées par le futur adaptateur.
static struct host_window_view view(unsigned count, uint16_t ready, int selected) {
    struct host_window_view result = {.count = count, .ready = ready, .selected = selected};
    for (unsigned i = 0; i < HOST_WINDOW_CAPACITY; i++) result.identities[i] = 100 + i;
    return result;
}

// L’ouverture ne déplace pas la destination, y compris quand toutes sont déjà connectées.
static void startup_and_deadlines(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 3, 0);
    host_window_start(&p, 20);
    assert(p.generation != 0 && p.pending == -1 && !p.searching);
    assert(host_window_remaining(&p, 20) == 10000);
    assert(host_window_pairing_allowed(&p, 10019, true));
    assert(!host_window_pairing_allowed(&p, 10019, false));
    host_window_update(&p, &v, 10019);
    assert(p.active && p.pending == -1);
    assert(!host_window_pairing_allowed(&p, 10020, true));
    host_window_update(&p, &v, 10020);
    assert(!p.active && p.pending == -1);

    host_window_start(&p, UINT32_MAX - 25);
    assert(host_window_remaining(&p, 0) == 9974);
    assert(host_window_remaining(&p, 9973) == 1);
    assert(host_window_remaining(&p, 9974) == 0);
}

// Une seule destination garde la saisie jusqu’au premier autre appareil utilisable.
static void search_once(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 1, 0);
    host_window_start(&p, 0);
    assert(host_window_shortcut(&p, &v, 100));
    uint32_t ticket = p.generation;
    assert(p.searching && p.pending == -1 && p.excluded == 1);
    host_window_update(&p, &v, 101);
    assert(host_window_take(&p, &v, 101, ticket, false) == -1);
    v.ready = 9;
    host_window_update(&p, &v, 102);
    assert(p.pending == 3 && p.pending_search);
    assert(host_window_take(&p, &v, 103, ticket, true) == -1);
    v.ready = 11;
    host_window_update(&p, &v, 104);
    assert(p.pending == 3);
    assert(host_window_take(&p, &v, 105, ticket, false) == 3);
    assert(!p.searching && p.pending == -1);
    v.selected = 3;
    v.ready = 31;
    host_window_update(&p, &v, 106);
    assert(host_window_take(&p, &v, 106, ticket, false) == -1);
    assert(host_window_pairing_allowed(&p, 107, true));
}

// La fenêtre coupe les arrivées tardives, mais laisse finir une décision acceptée avant l’échéance.
static void late_arrivals_and_input_guard(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 1, 0);
    host_window_start(&p, 0);
    assert(host_window_shortcut(&p, &v, 0));
    v.ready = 3;
    host_window_update(&p, &v, 9999);
    assert(host_window_take(&p, &v, 9999, p.generation, true) == -1);
    host_window_update(&p, &v, 10000);
    assert(!p.active && !p.searching && p.pending == 1);
    assert(host_window_take(&p, &v, 10750, p.generation, false) == 1);

    v.ready = 1;
    host_window_shortcut(&p, &v, 20000);
    v.ready = 3;
    host_window_update(&p, &v, 30000);
    assert(!p.searching && p.pending == -1);
    assert(host_window_take(&p, &v, 30001, p.generation, false) == -1);

    v.ready = 1;
    host_window_shortcut(&p, &v, 40000);
    v.ready = 3;
    host_window_update(&p, &v, 49999);
    v.ready = 1;
    host_window_update(&p, &v, 50000);
    v.ready = 5;
    assert(host_window_take(&p, &v, 50001, p.generation, false) == -1);
}

// Plusieurs destinations donnent une bascule explicite, jamais une poursuite du nouvel arrivant.
static void multiple_hosts_and_repeated_gestures(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 5, 0);
    host_window_start(&p, 0);
    assert(host_window_shortcut(&p, &v, 5));
    uint32_t first = p.generation;
    assert(!p.searching && p.pending == 2);
    v.ready = 7;
    host_window_update(&p, &v, 6);
    assert(p.pending == 2);
    assert(host_window_take(&p, &v, 7, first, false) == 2);
    v.selected = 2;
    host_window_update(&p, &v, 8);
    assert(p.pending == -1);

    v = view(5, 3, 0);
    host_window_shortcut(&p, &v, 100);
    first = p.generation;
    assert(p.pending == 1);
    host_window_shortcut(&p, &v, 200);
    assert(p.generation != first && p.pending == -1);
    assert(host_window_remaining(&p, 200) == 10000);
    assert(host_window_take(&p, &v, 201, first, false) == -1);

    host_window_shortcut(&p, &v, 300);
    first = p.generation;
    host_window_shortcut(&p, &v, 400);
    host_window_shortcut(&p, &v, 500);
    assert(p.pending == 1);
    assert(host_window_take(&p, &v, 501, first, false) == -1);
    assert(host_window_take(&p, &v, 501, p.generation, false) == 1);
}

// La fermeture des annonces n’annule pas une bascule explicite déjà demandée sur une cible prête.
static void pending_manual_after_window(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 3, 0);
    host_window_start(&p, 0);
    host_window_shortcut(&p, &v, 0);
    uint32_t ticket = p.generation;
    assert(host_window_take(&p, &v, 10000, ticket, true) == -1);
    assert(!p.active && p.pending == 1);
    assert(host_window_take(&p, &v, 10001, ticket, false) == 1);
    assert(host_window_take(&p, &v, 10002, ticket, false) == -1);

    v = view(5, 17, 0);
    host_window_shortcut(&p, &v, 20000);
    assert(p.pending == 4);
    v.count = 2;
    v.ready = 1;
    assert(host_window_take(&p, &v, 20001, p.generation, false) == -1);
}

// Une observation groupée de plusieurs arrivées choisit une cible déterministe, pas deux.
static void simultaneous_arrivals(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 16, 4);
    host_window_start(&p, 0);
    host_window_shortcut(&p, &v, 0);
    v.ready = 21;
    host_window_update(&p, &v, 1);
    assert(p.pending == 0);
    assert(host_window_take(&p, &v, 2, p.generation, false) == 0);
    v.selected = 0;
    host_window_update(&p, &v, 3);
    assert(p.pending == -1 && !p.searching);
}

// Une source modifiée ou une cible remplacée ne doit pas être écrasée par un ancien travail.
static void invalidated_selections(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 1, 0);
    host_window_start(&p, 0);
    host_window_shortcut(&p, &v, 0);
    v.ready = 3;
    host_window_update(&p, &v, 1);
    uint32_t ticket = p.generation;
    v.identities[1]++;
    assert(host_window_take(&p, &v, 2, ticket, false) == -1);
    assert(!p.searching && p.pending == -1 && p.generation != ticket);

    v = view(5, 1, 0);
    host_window_shortcut(&p, &v, 10);
    v.identities[0]++;
    host_window_update(&p, &v, 11);
    assert(!p.searching && p.pending == -1);

    host_window_shortcut(&p, &v, 20);
    v.selected = 3;
    host_window_update(&p, &v, 21);
    assert(!p.searching && p.pending == -1);
    v.selected = 0;
    v.ready = 3;
    host_window_update(&p, &v, 22);
    assert(p.pending == -1);

    host_window_shortcut(&p, &v, 30);
    ticket = p.generation;
    host_window_cancel_selection(&p);
    assert(host_window_take(&p, &v, 31, ticket, false) == -1);
    assert(host_window_remaining(&p, 31) == 9999);
}

// Une disparition peut être suivie d’un autre candidat seulement pendant la recherche encore ouverte.
static void disappearing_target(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 1, 0);
    host_window_start(&p, 0);
    host_window_shortcut(&p, &v, 0);
    v.ready = 3;
    host_window_update(&p, &v, 1);
    v.ready = 5;
    host_window_update(&p, &v, 2);
    assert(p.pending == 2);
    assert(host_window_take(&p, &v, 3, p.generation, false) == 2);

    v = view(5, 3, 0);
    host_window_shortcut(&p, &v, 10);
    v.ready = 5;
    host_window_update(&p, &v, 11);
    assert(!p.searching && p.pending == -1);
}

// Zéro hôte et un ancien profil indisponible ne doivent pas rendre le raccourci inutilisable.
static void recover_without_selected_host(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 0, -1);
    host_window_start(&p, 0);
    host_window_shortcut(&p, &v, 0);
    v.ready = 16;
    host_window_update(&p, &v, 1);
    assert(host_window_take(&p, &v, 2, p.generation, false) == 4);

    v = view(5, 4, 0);
    host_window_shortcut(&p, &v, 10);
    assert(p.pending == 2 && p.searching && !p.pending_search);
    assert(host_window_take(&p, &v, 11, p.generation, false) == 2);
    v.selected = 2;
    v.ready = 6;
    host_window_update(&p, &v, 12);
    assert(host_window_take(&p, &v, 13, p.generation, false) == 1);
}

// Gaming USB ferme tout, une sortie normale et le raccourci n’ont pas la même intention.
static void gaming_transitions(void) {
    struct host_window_policy p;
    struct host_window_view v = view(5, 1, 0);
    host_window_start(&p, 0);
    assert(!host_window_enter_gaming(&p, false));
    assert(host_window_pairing_allowed(&p, 1, true));
    host_window_shortcut(&p, &v, 2);
    assert(host_window_enter_gaming(&p, true));
    assert(p.gaming_usb && !p.searching && p.pending == -1);
    assert(!host_window_pairing_allowed(&p, 3, true));
    assert(!host_window_enter_gaming(&p, true));
    v.ready = 3;
    host_window_update(&p, &v, 4);
    assert(p.pending == -1 && !p.active);

    assert(host_window_leave_gaming(&p, 5));
    assert(host_window_remaining(&p, 5) == 10000 && !p.searching);
    assert(!host_window_leave_gaming(&p, 6));
    assert(host_window_remaining(&p, 6) == 9999);
    host_window_update(&p, &v, 7);
    assert(p.pending == -1);

    host_window_enter_gaming(&p, true);
    v.ready = 1;
    uint32_t previous = p.generation;
    assert(host_window_shortcut(&p, &v, 8));
    assert(!p.gaming_usb && p.searching && p.generation == previous + 1);
    assert(!host_window_leave_gaming(&p, 9));
    v.ready = 3;
    host_window_update(&p, &v, 10);
    assert(host_window_take(&p, &v, 11, p.generation, false) == 1);
}

// Les masques invalides, jetons absents et limites ne produisent aucun choix de repli.
static void malformed_views(void) {
    struct host_window_policy p;
    for (unsigned kind = 0; kind < 7; kind++) {
        struct host_window_view v = view(5, 3, 0);
        host_window_start(&p, 0);
        host_window_shortcut(&p, &v, 1);
        if (kind == 0) v.count = 0;
        if (kind == 1) v.count = 17;
        if (kind == 2) v.selected = 5;
        if (kind == 3) v.selected = -2;
        if (kind == 4) v.ready = 32;
        if (kind == 5) v.identities[1] = 0;
        if (kind == 6) { v.count = 1; v.ready = 1; v.selected = 0; v.identities[0] = 0; }
        uint32_t ticket = p.generation;
        assert(!host_window_shortcut(&p, &v, 2));
        assert(p.pending == -1 && !host_window_pairing_allowed(&p, 2, true));
        assert(host_window_take(&p, &v, 3, ticket, false) == -1);
        host_window_start(&p, 4);
        host_window_update(&p, &v, 5);
        assert(!p.active && p.pending == -1 && !p.searching);
    }
    struct host_window_view v = view(16, 0x8001, 0);
    host_window_start(&p, 0);
    p.generation = UINT32_MAX;
    assert(host_window_shortcut(&p, &v, 1));
    assert(p.generation == 1 && p.pending == 15);
    assert(host_window_take(&p, &v, 2, p.generation, false) == 15);
}

// Énumère les topologies courtes pour vérifier l’ordre circulaire et l’absence de cible hors masque.
static void exhaustive_topologies(void) {
    for (unsigned count = 1; count <= 8; count++) {
        for (int selected = -1; selected < (int)count; selected++) {
            for (unsigned mask = 0; mask < (1u << count); mask++) {
                struct host_window_policy p;
                struct host_window_view v = view(count, (uint16_t)mask, selected);
                host_window_start(&p, 0);
                assert(host_window_shortcut(&p, &v, 1));
                int expected = -1;
                unsigned start = selected >= 0 ? (unsigned)selected : count - 1;
                for (unsigned step = 1; step <= count; step++) {
                    unsigned candidate = (start + step) % count;
                    if (mask & (1u << candidate)) {
                        if ((int)candidate != selected) expected = (int)candidate;
                        break;
                    }
                }
                int actual = host_window_take(&p, &v, 2, p.generation, false);
                assert(actual == expected);
                if (actual >= 0) assert(mask & (1u << actual));
            }
        }
    }
}

// Exécute le contrat de décision destiné au futur adaptateur ZMK.
int main(void) {
    startup_and_deadlines();
    search_once();
    late_arrivals_and_input_guard();
    multiple_hosts_and_repeated_gestures();
    pending_manual_after_window();
    simultaneous_arrivals();
    invalidated_selections();
    disappearing_target();
    recover_without_selected_host();
    gaming_transitions();
    malformed_views();
    exhaustive_topologies();
    puts("host_window_policy: ok");
    return 0;
}
