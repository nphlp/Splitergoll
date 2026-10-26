// Protocole diagnostique borné, sans sélection de sortie ni écriture en flash.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HI_SIZE 64
#define HI_ID_SIZE 16
#define HI_LINKS 9
#define HI_LEASE_MS 15000U
#define HI_USB 0

struct hi_link {
    bool ready, claimed;
    uint32_t generation, received;
    uint8_t nonce[HI_ID_SIZE], host[HI_ID_SIZE];
};
struct hi_state {
    uint8_t boot[HI_ID_SIZE];
    struct hi_link links[HI_LINKS];
};

void hi_init(struct hi_state *state, const uint8_t boot[HI_ID_SIZE]);
bool hi_open(struct hi_state *state, unsigned link);
void hi_close(struct hi_state *state, unsigned link);
bool hi_read(const struct hi_state *state, unsigned link, uint32_t now, uint8_t out[HI_SIZE]);
bool hi_write(struct hi_state *state, unsigned link, uint32_t now, const uint8_t *data, size_t len);
