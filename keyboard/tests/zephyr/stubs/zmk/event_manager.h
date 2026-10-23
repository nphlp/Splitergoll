// Remplace le bus ZMK, pas l’horloge ni les files de travail Zephyr.
#pragma once
typedef int zmk_event_t;
#define ZMK_LISTENER(name, callback)                                                               \
    static int (*const test_listener_##name)(const zmk_event_t *) __attribute__((unused)) = callback
#define ZMK_SUBSCRIPTION(name, event)
