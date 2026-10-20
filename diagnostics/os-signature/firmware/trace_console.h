// Restitution bornée du diagnostic sur son port série dédié.
#ifndef SPLITERGOLL_TRACE_CONSOLE_H
#define SPLITERGOLL_TRACE_CONSOLE_H
#include <stdbool.h>
#include <zephyr/device.h>

// Envoie une ligne si DTR est actif, avec une échéance d’une seconde.
bool trace_send_line(const struct device *uart, const char *line);

// Formate le diagnostic sélectionné, sans effacer l’historique à la lecture.
void splitergoll_trace_dump(const struct device *uart);

// Traite uniquement les commandes explicites d’emplacement du diagnostic Bluetooth.
void splitergoll_ble_profile_command(const struct device *uart, char command);
#endif
