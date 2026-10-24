// Contraintes du seul firmware autorisé à intercepter les annonces.
#pragma once
#define BUILD_ASSERT(expression) _Static_assert(expression, #expression)
#define IS_ENABLED(option) (option)
#define ARG_UNUSED(value) (void)(value)
