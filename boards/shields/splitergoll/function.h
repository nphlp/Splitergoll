#pragma once

// Layers
#define LAYER(name, layout, sensors) \
/ { \
    keymap { \
        compatible = "zmk,keymap"; \
        name##_layer { \
            bindings = <layout>; \
            sensor-bindings = <sensors>; \
        }; \
    }; \
};

#define COND_LAYER(if_layer, and_layer, then_layer) \
/ { \
    conditional_layers { \
        compatible = "zmk,conditional-layers"; \
        if_layer##_##and_layer##_##then_layer##_condition { \
            if-layers = <if_layer and_layer>; \
            then-layer = <then_layer>; \
        }; \
    }; \
};


// Behaviors
#define MORPH(name, mod, k1, k2) \
/ { \
    behaviors { \
        name: name { \
            compatible = "zmk,behavior-mod-morph"; \
            #binding-cells = <0>; \
            bindings = <k1>, <k2>; \
            mods = <(MOD_##mod)>; \
        }; \
    }; \
};

#define TAP_DANCE_2(name, k1, k2) \
/ { \
    behaviors { \
        name: name { \
            compatible = "zmk,behavior-tap-dance"; \
            #binding-cells = <0>; \
            tapping-term-ms = <250>; \
            bindings = <k1>, <k2>; \
        }; \
    }; \
};

#define TAP_DANCE_3(name, k1, k2, k3) \
/ { \
    behaviors { \
        name: name { \
            compatible = "zmk,behavior-tap-dance"; \
            #binding-cells = <0>; \
            tapping-term-ms = <250>; \
            bindings = <k1>, <k2>, <k3>; \
        }; \
    }; \
};

// Macros
#define MACRO(name, d1) \
/ { \
    macros { \
        name: name { \
            compatible = "zmk,behavior-macro"; \
            #binding-cells = <0>; \
            bindings \
                = <d1> \
            ; \
        }; \
    }; \
};

#define MACRO_WRITE(name, d1, d2) \
/ { \
    macros { \
        name: name { \
            compatible = "zmk,behavior-macro"; \
            #binding-cells = <0>; \
            bindings \
                = <&macro_tap d1> \
                , <&macro_tap &kp SPACE> \
                , <&macro_wait_time 500> \
                , <&macro_tap &kp BSPC> \
                , <&macro_wait_time 0> \
                , <&macro_tap d2> \
            ; \
        }; \
    }; \
};

#define ALT_CODE_2(name, d1, d2) \
/ { \
    macros { \
            name: name { \
            compatible = "zmk,behavior-macro"; \
            #binding-cells = <0>; \
            bindings \
                = <&macro_press &kp LALT> \
                , <&macro_tap &kp KP_N##d1> \
                , <&macro_tap &kp KP_N##d2> \
                , <&macro_release &kp LALT> \
            ; \
        }; \
    }; \
};

#define ALT_CODE_3(name, d1, d2, d3) \
/ { \
    macros { \
        name: name { \
            compatible = "zmk,behavior-macro"; \
            #binding-cells = <0>; \
            bindings \
                = <&macro_press &kp LALT> \
                , <&macro_tap &kp KP_N##d1> \
                , <&macro_tap &kp KP_N##d2> \
                , <&macro_tap &kp KP_N##d3> \
                , <&macro_release &kp LALT> \
            ; \
        }; \
    }; \
};

#define ALT_CODE_4(name, d1, d2, d3, d4) \
/ { \
    macros { \
        name: name { \
            compatible = "zmk,behavior-macro"; \
            #binding-cells = <0>; \
            bindings \
                = <&macro_press &kp LALT> \
                , <&macro_tap &kp KP_N##d1> \
                , <&macro_tap &kp KP_N##d2> \
                , <&macro_tap &kp KP_N##d3> \
                , <&macro_tap &kp KP_N##d4> \
                , <&macro_release &kp LALT> \
            ; \
        }; \
    }; \
};
