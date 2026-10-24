// Exerce la frontière native : aucune annonce ni arrêt de radio, même après des demandes répétées.
#include "keyboard/src/connections/wire_usb_only.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned native_starts, native_stops;

// Une future délégation involontaire doit faire échouer le test, quel que soit son résultat natif.
int __real_bt_le_adv_start(const struct bt_le_adv_param *parameters, const struct bt_data *data,
                           size_t count, const struct bt_data *response, size_t response_count) {
    (void)parameters;
    (void)data;
    (void)count;
    (void)response;
    (void)response_count;
    native_starts++;
    return -5;
}

// La coupure globale d’annonces ne doit pas être utilisée pour remplacer leur filtrage.
int __real_bt_le_adv_stop(void) {
    native_stops++;
    return -5;
}

// Les payloads et paramètres appartiennent à ZMK et doivent rester intacts.
int main(void) {
    uint8_t payload[] = {1, 2, 3};
    struct bt_data data[] = {{.type = 1, .data_len = sizeof(payload), .data = payload}};
    struct bt_le_adv_param parameters = {.options = 1, .interval_min = 32, .interval_max = 64};
    struct bt_le_adv_param original = parameters;
    for (unsigned attempt = 0; attempt < 100; attempt++) {
        parameters.id = (uint8_t)attempt;
        parameters.options = attempt;
        original = parameters;
        assert(__wrap_bt_le_adv_start(&parameters, data, 1, data, 1) == 0);
        assert(!memcmp(&parameters, &original, sizeof(parameters)));
        assert(data[0].data == payload && data[0].data_len == 3 && payload[2] == 3);
        assert(__wrap_bt_le_adv_stop() == 0);
    }
    assert(__wrap_bt_le_adv_start(NULL, NULL, 0, NULL, 0) == 0);
    assert(native_starts == 0 && native_stops == 0);
    puts("wire USB-only: repeated host advertising suppressed, payloads intact, no radio stop OK");
    return 0;
}
