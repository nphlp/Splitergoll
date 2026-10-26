// Compare seulement des déclarations fraîches. Un identifiant n’est pas une preuve d’authenticité.
#include <splitergoll/host_identity.h>
#include <string.h>

// Un identifiant vide ne constitue jamais une annonce exploitable.
static bool nonzero(const uint8_t *bytes) {
    uint8_t value = 0;
    for (size_t i = 0; i < HI_ID_SIZE; i++) value |= bytes[i];
    return value != 0;
}

// La soustraction non signée garde la durée correcte au passage de l’horloge 32 bits.
static bool fresh(const struct hi_link *link, uint32_t now) {
    return link->ready && link->claimed && (uint32_t)(now - link->received) < HI_LEASE_MS;
}

// Le jeton de démarrage aléatoire est fourni par l’adaptateur, jamais persisté.
void hi_init(struct hi_state *state, const uint8_t boot[HI_ID_SIZE]) {
    memset(state, 0, sizeof(*state));
    memcpy(state->boot, boot, HI_ID_SIZE);
}

// Chaque liaison reçoit un contexte unique dans ce démarrage, même après une reconnexion rapide.
bool hi_open(struct hi_state *state, unsigned link) {
    if (link >= HI_LINKS || !nonzero(state->boot)) return false;
    struct hi_link *entry = &state->links[link];
    if (entry->ready) return true;
    if (entry->generation == UINT32_MAX) return false;
    entry->generation++;
    entry->ready = true;
    entry->claimed = false;
    memcpy(entry->nonce, state->boot, HI_ID_SIZE);
    entry->nonce[0] ^= 0xA5;
    entry->nonce[1] ^= (uint8_t)link;
    for (unsigned i = 0; i < 4; i++) entry->nonce[12 + i] ^= (uint8_t)(entry->generation >> (8 * i));
    if (!nonzero(entry->nonce)) entry->nonce[0] = 1;
    return true;
}

// Invalide immédiatement l’annonce, mais conserve le compteur pour ne pas réutiliser un contexte.
void hi_close(struct hi_state *state, unsigned link) {
    if (link >= HI_LINKS) return;
    state->links[link].ready = false;
    state->links[link].claimed = false;
    memset(state->links[link].host, 0, HI_ID_SIZE);
}

// La réponse ne révèle jamais les identifiants d’hôte, uniquement des masques et un état.
bool hi_read(const struct hi_state *state, unsigned link, uint32_t now, uint8_t out[HI_SIZE]) {
    if (link >= HI_LINKS || !state->links[link].ready) return false;
    memset(out, 0, HI_SIZE);
    memcpy(out, "SGID", 4); out[4] = 1; out[5] = 1; out[6] = link == HI_USB ? 0 : 1;
    memcpy(out + 8, state->boot, HI_ID_SIZE);
    memcpy(out + 24, state->links[link].nonce, HI_ID_SIZE);
    out[58] = state->links[HI_USB].ready;
    out[59] = fresh(&state->links[HI_USB], now);
    out[60] = link == HI_USB ? 0xFF : (uint8_t)(link - 1);
    bool conflict = false;
    for (unsigned i = 1; i < HI_LINKS; i++) {
        if (!fresh(&state->links[i], now)) continue;
        out[57] |= (uint8_t)(1U << (i - 1));
        if (out[59] && memcmp(state->links[HI_USB].host, state->links[i].host, HI_ID_SIZE) == 0)
            out[56] |= (uint8_t)(1U << (i - 1));
        for (unsigned j = 1; j < i; j++)
            if (fresh(&state->links[j], now) && memcmp(state->links[i].host, state->links[j].host, HI_ID_SIZE) == 0)
                conflict = true;
    }
    out[7] = conflict ? 3 : out[56] ? 1 : out[59] && out[57] ? 2 : 0;
    if (conflict) out[56] = 0;
    return true;
}

// Refuse toute trame inconnue, fragmentée, ancienne ou attribuée à une autre liaison.
bool hi_write(struct hi_state *state, unsigned link, uint32_t now, const uint8_t *data, size_t len) {
    if (link >= HI_LINKS || !state->links[link].ready || len != HI_SIZE || !data) return false;
    if (memcmp(data, "SGID", 4) || data[4] != 1 || data[5] != 2 || data[6] != (link == HI_USB ? 0 : 1) || data[7]) return false;
    if (memcmp(data + 8, state->boot, HI_ID_SIZE) || memcmp(data + 24, state->links[link].nonce, HI_ID_SIZE)) return false;
    for (size_t i = 56; i < HI_SIZE; i++) if (data[i]) return false;
    if (!nonzero(data + 40)) return false;
    memcpy(state->links[link].host, data + 40, HI_ID_SIZE);
    state->links[link].received = now;
    state->links[link].claimed = true;
    return true;
}
