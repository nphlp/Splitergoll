#pragma once

#include "../function.h"

// Letters
#define    _A    Q       // A
#define    _B    B       // B
#define    _C    C       // C
#define    _D    D       // D
#define    _E    E       // E
#define    _F    F       // F
#define    _G    G       // G
#define    _H    H       // H
#define    _I    I       // I
#define    _J    J       // J
#define    _K    K       // K
#define    _L    L       // L
#define    _M    SEMI    // M
#define    _N    N       // N
#define    _O    O       // O
#define    _P    P       // P
#define    _Q    A       // Q
#define    _R    R       // R
#define    _S    S       // S
#define    _T    T       // T
#define    _U    U       // U
#define    _V    V       // V
#define    _W    Z       // W
#define    _X    X       // X
#define    _Y    Y       // Y
#define    _Z    W       // Z


// Dead accents
#define    _AGU    RA(SQT)      // ' (does not work)
#define    _GRV    RA(N7)       // ` (dead)
#define    _CIR    LBKT         // ^ (dead)
#define    _TRE    LBRC         // ¨ (dead)
#define    _TLD    RA(N2)       // ~ (dead)


// Letters with accents
#define       _AG         N0        // à
MACRO(fr_ac, &kp _CIR &kp _A)       // â
MACRO(fr_agu, &kp _GRV &kp LS(_A))  // À
#define       _CC         N9        // ç
ALT_CODE_3(fr_ccu, 1, 2, 8)         // Ç
#define       _EA         N2        // é
ALT_CODE_3(fr_eau, 1, 4, 4)         // É
MACRO(fr_ec, &kp _CIR &kp _E)       // ê
MACRO(fr_ecu, &kp _CIR &kp LS(_E))  // Ê
#define       _EG         N7        // è
MACRO(fr_et, &kp _TRE &kp _E)       // ë
MACRO(fr_ic, &kp _CIR &kp _I)       // î
MACRO(fr_it, &kp _TRE &kp _I)       // ï
MACRO(fr_oc, &kp _CIR &kp _O)       // ô
MACRO(fr_ocu, &kp _CIR &kp LS(_O))  // Ô
#define       _UG         SQT       // ù
MACRO(fr_uc, &kp _CIR &kp _U)       // û
#define       _MU         LS(NUHS)  // µ

ALT_CODE_4(fr_oe, 0, 1, 5, 6)       // œ
ALT_CODE_4(fr_oeu, 0, 1, 4, 0)      // Œ


// Numbers
#define    _1    KP_N1     // 1
#define    _2    KP_N2     // 2
#define    _3    KP_N3     // 3
#define    _4    KP_N4     // 4
#define    _5    KP_N5     // 5
#define    _6    KP_N6     // 6
#define    _7    KP_N7     // 7
#define    _8    KP_N8     // 8
#define    _9    KP_N9     // 9
#define    _0    KP_N0     // 0


// Punctuation
#define    _COMMA     M         // ,
#define    _DOT       LS(COMMA) // .
#define    _SEMI      COMMA     // ;
#define    _COLON     DOT       // :
#define    _EXCL      SLASH     // !
#define    _QMARK     LS(M)     // ?
#define    _SLASH     LS(DOT)   // slash
#define    _BSLASH    RA(N8)    // backslash


// Calculus
#define    _EQAL    EQUAL       // =
#define    _PLUS    KP_PLUS     // +
#define    _MINS    KP_MINUS    // -
#define    _ASTR    KP_MULTIPLY // *
#define    _DIVI    KP_SLASH    // /
#define    _NUMB    LS(MINUS)   // °


// Brackets
#define    _LPAR    N5          // (
#define    _RPAR    MINUS       // )
#define    _LACC    RA(N4)      // {
#define    _RACC    RA(EQUAL)   // }
#define    _LCRO    RA(N5)      // [
#define    _RCRO    RA(MINUS)   // ]
#define    _LCHE    NUBS        // <
#define    _RCHE    LS(NUBS)    // >


// Quotes
#define    _QUOT    N3            // "
#define    _SQUO    N4            // '
MACRO(fr_grv, &kp _GRV &kp SPACE) // `
MACRO(fr_tld, &kp _TLD &kp SPACE) // ~


// Symbols
#define    _AT       RA(N0)     // @
#define    _HASH     RA(N3)     // #
#define    _EURO     RA(E)      // €
#define    _DOLR     RBKT       // $
#define    _PND      LS(RBKT)   // £
#define    _PERC     LS(SQT)    // %
#define    _AND      N1         // &
#define    _PIPE     RA(N6)     // |
#define    _UDSC     N8         // _


// Exponents
#define    _EX2  GRAVE         // ²
ALT_CODE_3(fr_ex3, 2, 5, 2)    // ³


// Coding arrows
MACRO(fr_arw_s, &kp _MINS &kp _RCHE) // ->
MACRO(fr_arw_d, &kp _EQAL &kp _RCHE) // =>


// Arrows
ALT_CODE_2(arw_up,  2, 4) // ↑
ALT_CODE_2(arw_dwn, 2, 5) // ↓
ALT_CODE_2(arw_rgt, 2, 6) // →
ALT_CODE_2(arw_lft, 2, 7) // ←
