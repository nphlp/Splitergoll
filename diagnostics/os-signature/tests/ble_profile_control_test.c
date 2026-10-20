// Vérifie qu’une sélection libre conserve les emplacements appairés et refuse les cibles ambiguës.
#include "ble_profile_control.h"
#include <assert.h>
#include <stdio.h>

// Couvre aussi tous les octets de commande, les masques et les index des cinq emplacements actuels.
int main(void) {
    for (unsigned active = 0; active < 5; active++) {
        for (unsigned mask = 0; mask < 32; mask++) {
            for (unsigned connected = 0; connected < 2; connected++) {
                for (unsigned command = 0; command < 256; command++) {
                    struct ble_profile_choice c = ble_profile_choose((char)command, 5, active, mask, connected);
                    if (command == 'S') {
                        assert(c.result == BLE_PROFILE_OK && !c.change && c.target == (int)active);
                    } else if (command == 'F') {
                        int first_free = -1;
                        for (unsigned i = 0; i < 5; i++) {
                            if (mask & (1u << i)) { first_free = (int)i; break; }
                        }
                        if (first_free < 0) {
                            assert(c.result == BLE_PROFILE_FULL && !c.change);
                        } else if (connected && first_free != (int)active) {
                            assert(c.result == BLE_PROFILE_BUSY && !c.change);
                        } else {
                            assert(c.result == BLE_PROFILE_OK && c.target == first_free);
                            assert(mask & (1u << c.target));
                            assert(c.change == (first_free != (int)active));
                        }
                    } else if (command >= '0' && command <= '4') {
                        int target = (int)command - '0';
                        assert(c.target == target);
                        if (connected && target != (int)active) {
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
