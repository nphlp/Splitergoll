// Acquisition initiale indépendante de la console USB et de l’OS qui alimente le clavier.
#ifndef SPLITERGOLL_BLE_ACQUISITION_H
#define SPLITERGOLL_BLE_ACQUISITION_H
#include <splitergoll/ble_os_classifier.h>
#include <splitergoll/ble_os_settings.h>
#include <zephyr/bluetooth/conn.h>

struct ble_acquisition_snapshot {
    struct ble_os_memory memory;
    enum ble_os_guess measured;
    uint32_t session;
    uint32_t epoch;
    bool connected;
    bool secure;
    bool reliable;
    bool learning;
};

// Copie uniquement des métadonnées dans la file bornée, sans écriture dans le callback radio.
void splitergoll_ble_acquisition_event(struct bt_conn *conn, enum ble_trace_kind kind,
                                      const uint32_t data[5], uint16_t uuid, bool reliable);

// Vérifie ensemble l’association sauvegardée et la sécurité de sa connexion courante.
struct ble_acquisition_snapshot splitergoll_ble_acquisition_snapshot(unsigned profile);

// Fournit un instantané passif pour la console optionnelle, sans identités privées.
void splitergoll_ble_acquisition_status(struct ble_os_memory *snapshots, enum ble_os_guess *observations,
                                       bool *acquisitions, unsigned *dropped);

// Réveille le sélecteur de couches après une connexion ou une décision de mémoire.
void splitergoll_ble_auto_notify(void);
#endif
