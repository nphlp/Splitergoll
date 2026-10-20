// Vérifie qu’une sélection libre conserve les emplacements appairés et refuse les cibles ambiguës.
#include "ble_profile_control.h"
#include <assert.h>
#include <stdio.h>

// Couvre les touches de couche, les rapports HID, les comportements différés et le rebouclage.
static void input_activity(void) {
    struct ble_profile_activity activity = {0};
    assert(!ble_profile_input_busy(&activity, false, 0));
    assert(ble_profile_input_busy(&activity, true, 0));
    ble_profile_position(&activity, 43, true, 100);
    assert(ble_profile_input_busy(&activity, false, 10000));
    ble_profile_position(&activity, 43, false, 10000);
    assert(ble_profile_input_busy(&activity, false, 10749));
    assert(!ble_profile_input_busy(&activity, false, 10750));
    ble_profile_touch(&activity, 11000);
    assert(ble_profile_input_busy(&activity, false, 11749));
    assert(!ble_profile_input_busy(&activity, false, 11750));
    ble_profile_touch(&activity, UINT32_MAX - 100);
    assert(ble_profile_input_busy(&activity, false, 648));
    assert(!ble_profile_input_busy(&activity, false, 649));
    ble_profile_position(&activity, 64, true, 700);
    assert(activity.invalid && ble_profile_input_busy(&activity, false, 100000));
}

// Couvre aussi tous les octets de commande, les masques et les index des cinq emplacements actuels.
int main(void) {
    input_activity();
    for (unsigned active = 0; active < 5; active++) {
        for (unsigned mask = 0; mask < 32; mask++) {
            for (unsigned busy = 0; busy < 2; busy++) {
                for (unsigned command = 0; command < 256; command++) {
                    struct ble_profile_choice c = ble_profile_choose((char)command, 5, active, mask, busy);
                    if (command == 'S') {
                        assert(c.result == BLE_PROFILE_OK && !c.change && c.target == (int)active);
                    } else if (command == 'F') {
                        int first_free = -1;
                        for (unsigned i = 0; i < 5; i++) {
                            if (mask & (1u << i)) { first_free = (int)i; break; }
                        }
                        if (first_free < 0) {
                            assert(c.result == BLE_PROFILE_FULL && !c.change);
                        } else if (busy && first_free != (int)active) {
                            assert(c.result == BLE_PROFILE_BUSY && !c.change);
                        } else {
                            assert(c.result == BLE_PROFILE_OK && c.target == first_free);
                            assert(mask & (1u << c.target));
                            assert(c.change == (first_free != (int)active));
                        }
                    } else if (command >= '0' && command <= '4') {
                        int target = (int)command - '0';
                        assert(c.target == target);
                        if (busy && target != (int)active) {
                            assert(c.result == BLE_PROFILE_BUSY && !c.change);
                        } else {
                            assert(c.result == BLE_PROFILE_OK && c.change == (target != (int)active));
                        }
                    } else {
                        assert(c.result == BLE_PROFILE_INVALID && !c.change && c.target == -1);
                    }
                }
            }
        }
    }
    assert(ble_profile_choose('F', 5, 0, 0xffff, false).target == 0);
    assert(ble_profile_choose('F', 5, 0, 0x20, false).result == BLE_PROFILE_FULL);
    assert(ble_profile_choose('F', 0, 0, 1, false).result == BLE_PROFILE_INVALID);
    assert(ble_profile_choose('F', 11, 0, 1, false).result == BLE_PROFILE_INVALID);
    assert(ble_profile_choose('F', 5, 5, 1, false).result == BLE_PROFILE_INVALID);
    assert(ble_profile_choose('9', 10, 0, 0, false).target == 9);
    puts("BLE profile control: OK");
    return 0;
}
