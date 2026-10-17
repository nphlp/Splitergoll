#pragma once

#include "../function.h"
#include "windows.h"

// Modifiers
#define M_CTL  LCTRL
#define M_OPN  LALT
#define M_CMD  LGUI

// Punctuation
#define M_EXCL  N8              // !
#define M_BSLASH  LS(RA(DOT))   // backslash

// Dead accents
#define    M_AGU    LS(RA(N1))   // ´ (dead)
#define    M_GRV    BSLH         // ` (dead)
#define    M_CIR    LBKT         // ^ (dead)
#define    M_TRE    LBRC         // ¨ (dead)
#define    M_TLD    RA(_N)       // ~ (dead)

// Letters with accents
MACRO(m_agu, &kp BSLH       &kp LS(_A)) // À
MACRO(m_eau, &kp LS(RA(N1)) &kp LS(_E)) // É
#define      M_CCU          RA(N9)      // Ç
#define      M_MU           RA(_M)      // µ
#define      M_OE           RA(_O)      // œ
#define      M_OEU          LS(RA(_O))  // Œ

// Brackets
#define    M_RACC    RA(MINUS)       // }
#define    M_LACC    RA(N5)          // {
#define    M_LCRO    LS(RA(N5))      // [
#define    M_RCRO    LS(RA(MINUS))   // ]
#define    M_LCHE    GRAVE           // <
#define    M_RCHE    LS(GRAVE)       // >
#define    M_AT      NUBS            // @
#define    M_PIPE    LS(RA(_L))      // |
#define    M_HASH    LS(NUBS)        // #
#define    M_EURO    RA(RBKT)        // €
#define    M_DOLR    RBKT            // $
#define    M_PND     LS(BSLH)        // £

// Quotes
MACRO(m_grv, &kp BSLH &kp SPACE)     // `

// Calculus
#define M_EQAL     SLASH          // =
#define M_PLUS     LS(SLASH)      // +
#define M_MINS     EQUAL          // -
#define M_UDSC     LS(EQUAL)      // _

// Moves
#define M_HOME     LG(LEFT)      // home
#define M_END      LG(RIGHT)     // end
#define M_LWRD     RA(LEFT)      // left word
#define M_RWRD     RA(RIGHT)     // right word

// Coding arrows
MACRO(m_arw_s, &kp M_MINS &kp M_RCHE) // ->
MACRO(m_arw_d, &kp M_EQAL &kp M_RCHE) // =>




// Aerospace Layer 3

// LG = CMD
// LS = SHIFT
// LA = OPTION
// LC = CTRL

// Toogle tiles or accordion (CTRL + OPTION + CMD)
#define F_T LC(LA(LG(N0))) // Set tiles mode
#define F_A LC(LA(LG(N9))) // Set accordion mode
#define F_F LC(LA(LG(N8))) // Set floating mode

// Focus window (CTRL + OPTION + CMD)
#define F_U LC(LA(LG(N1))) // Focus UP
#define F_D LC(LA(LG(N2))) // Focus DOWN
#define F_L LC(LA(LG(N3))) // Focus LEFT
#define F_R LC(LA(LG(N4))) // Focus RIGHT

// Focus workspace (CTRL + OPTION + CMD)
#define F_1 LC(LA(LG(N5))) // Focus workspace 1
#define F_2 LC(LA(LG(N6))) // Focus workspace 2
#define F_3 LC(LA(LG(N7))) // Focus workspace 3

// Move window into workspace (CTRL + OPTION + SHIFT)
#define M_U LC(LA(LS(N1))) // Move UP
#define M_D LC(LA(LS(N2))) // Move DOWN
#define M_L LC(LA(LS(N3))) // Move LEFT
#define M_R LC(LA(LS(N4))) // Move RIGHT

// Move window to workspace (CTRL + OPTION + SHIFT)
#define M_1 LC(LA(LS(N5))) // Move workspace 1
#define M_2 LC(LA(LS(N6))) // Move workspace 2
#define M_3 LC(LA(LS(N7))) // Move workspace 3
