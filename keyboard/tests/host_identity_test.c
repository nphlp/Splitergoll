// Vérifie le vrai protocole et ses contextes, sans radio, USB ou stockage.
#include <splitergoll/host_identity.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t boot[HI_ID_SIZE] = {1, 2, 3};

// Construit une annonce à partir du challenge réellement émis par le moteur.
static void claim(struct hi_state *state, unsigned link, uint32_t now, uint8_t host) {
    uint8_t message[HI_SIZE];
    assert(hi_read(state, link, now, message));
    message[5] = 2; message[7] = 0;
    memset(message + 56, 0, 8); memset(message + 40, host, HI_ID_SIZE);
    assert(hi_write(state, link, now, message, sizeof(message)));
}

// Exige un état public et l’absence d’identifiant d’hôte dans toute réponse.
static void status(struct hi_state *state, uint32_t now, uint8_t result, uint8_t mask) {
    uint8_t report[HI_SIZE];
    assert(hi_read(state, HI_USB, now, report));
    assert(report[7] == result && report[56] == mask);
    for (unsigned i = 40; i < 56; i++) assert(report[i] == 0);
}

// Les identités clonées sont un conflit, jamais une preuve autorisant une fusion.
int main(void) {
    struct hi_state state;
    uint8_t report[HI_SIZE], stale[HI_SIZE];
    hi_init(&state, boot);
    assert(!hi_read(&state, 0, 0, report));
    assert(!hi_open(&state, HI_LINKS));
    assert(hi_open(&state, 0) && hi_open(&state, 1) && hi_open(&state, 2));
    status(&state, 0, 0, 0);
    claim(&state, 0, 10, 42); claim(&state, 1, 10, 42);
    status(&state, 10, 1, 1);
    claim(&state, 2, 10, 43); status(&state, 10, 1, 1);
    claim(&state, 2, 10, 42); status(&state, 10, 3, 0);
    hi_close(&state, 2); status(&state, 10, 1, 1);
    status(&state, 10 + HI_LEASE_MS - 1, 1, 1);
    status(&state, 10 + HI_LEASE_MS, 0, 0);
    claim(&state, 0, UINT32_MAX - 100, 42); claim(&state, 1, UINT32_MAX - 100, 43);
    status(&state, 20, 2, 0);
    assert(hi_read(&state, 0, 20, stale));
    stale[5] = 2; stale[7] = 0; memset(stale + 56, 0, 8); memset(stale + 40, 42, 16);
    hi_close(&state, 0); assert(!hi_write(&state, 0, 20, stale, sizeof(stale)));
    assert(hi_open(&state, 0)); assert(!hi_write(&state, 0, 20, stale, sizeof(stale)));
    assert(hi_read(&state, 0, 20, report));
    memcpy(stale + 24, report + 24, 16);
    assert(hi_write(&state, 0, 20, stale, sizeof(stale)));
    assert(!hi_write(&state, 1, 20, stale, sizeof(stale)));
    stale[6] = 1;
    assert(!hi_write(&state, 1, 20, stale, sizeof(stale)));
    stale[6] = 0;
    assert(!hi_write(&state, 0, 20, stale, HI_SIZE + 1));
    for (unsigned len = 0; len < HI_SIZE; len++) assert(!hi_write(&state, 0, 20, stale, len));
    for (unsigned i = 0; i < HI_SIZE; i++) {
        if (i >= 40 && i < 56) continue;
        uint8_t bad[HI_SIZE]; memcpy(bad, stale, sizeof(bad)); bad[i] ^= 0x80;
        assert(!hi_write(&state, 0, 20, bad, sizeof(bad)));
    }
    memset(stale + 40, 0, 16); assert(!hi_write(&state, 0, 20, stale, sizeof(stale)));
    for (unsigned i = 1; i < HI_LINKS; i++) { assert(hi_open(&state, i)); claim(&state, i, 20, (uint8_t)i); }
    claim(&state, 0, 20, 8); status(&state, 20, 1, 0x80);
    assert(hi_read(&state, 8, 20, report) && report[60] == 7 && report[6] == 1);
    hi_close(&state, 8); status(&state, 20, 2, 0);
    state.links[8].generation = UINT32_MAX; assert(!hi_open(&state, 8));
    uint8_t empty[16] = {0}; hi_init(&state, empty); assert(!hi_open(&state, 0));
    hi_init(&state, boot); assert(hi_open(&state, 0));
    assert(!hi_write(&state, 0, 0, NULL, HI_SIZE));
    puts("host_identity: formats, contextes, expiration, confidentialité et conflits passent");
    return 0;
}
