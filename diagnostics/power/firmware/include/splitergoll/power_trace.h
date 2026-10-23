// Points d’observation explicites, compilés uniquement dans le diagnostic d’énergie.
#ifndef SPLITERGOLL_POWER_TRACE_H
#define SPLITERGOLL_POWER_TRACE_H
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/device.h>

// Observe l’état obtenu par le worker radio, sans lui demander une nouvelle action.
void splitergoll_power_window(uint32_t remaining_ms, bool advertising);

// Observe le résultat de l’écriture existante, sans ajouter de sauvegarde.
void splitergoll_power_saved(unsigned profile, int error);

// Exporte un instantané du journal, sans reset ni activité physique.
void splitergoll_power_dump(const struct device *uart);
#endif
