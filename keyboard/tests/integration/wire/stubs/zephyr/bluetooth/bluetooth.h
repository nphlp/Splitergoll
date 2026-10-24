// Frontière du pilote : ses fonctions natives ne doivent jamais être appelées par wire.
#pragma once
#include <stddef.h>
#include <stdint.h>
struct bt_le_adv_param {
    uint32_t options;
    uint16_t interval_min, interval_max;
    uint8_t id;
};
struct bt_data {
    uint8_t type, data_len;
    const uint8_t *data;
};
int bt_le_adv_start(const struct bt_le_adv_param *, const struct bt_data *, size_t,
                    const struct bt_data *, size_t);
int bt_le_adv_stop(void);
