#pragma once

// Une base OS fixe, trois couches de fonctions et un pavé numérique.
#include "function.h"

#define BASE 0
#define PRIMARY 1
#define SECONDARY 2
#define TERTIARY 3
#define NUMPAD 4
#define GAMING 5
#define GAMING_2 6

COND_LAYER(PRIMARY, SECONDARY, TERTIARY)
