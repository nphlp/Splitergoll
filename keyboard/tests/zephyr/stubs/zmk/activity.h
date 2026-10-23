// Même contrat d’états que le socle ZMK, sans dépendance au matériel.
#pragma once
enum zmk_activity_state { ZMK_ACTIVITY_ACTIVE, ZMK_ACTIVITY_IDLE, ZMK_ACTIVITY_SLEEP };
enum zmk_activity_state zmk_activity_get_state(void);
