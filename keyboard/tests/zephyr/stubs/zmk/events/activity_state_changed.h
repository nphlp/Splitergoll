// Intercepte seulement les états publiés par le vrai activity.c.
#pragma once
#include <zmk/activity.h>
struct zmk_activity_state_changed {
    enum zmk_activity_state state;
};
int raise_zmk_activity_state_changed(struct zmk_activity_state_changed event);
