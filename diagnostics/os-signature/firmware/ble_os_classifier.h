// Hypothèses BLE expérimentales sur une session, sans sélection d’OS ni stockage persistant.
#ifndef SPLITERGOLL_BLE_OS_CLASSIFIER_H
#define SPLITERGOLL_BLE_OS_CLASSIFIER_H

#include "ble_trace_store.h"

enum ble_os_guess {
    BLE_OS_UNKNOWN,
    BLE_OS_MACOS,
    BLE_OS_WINDOWS,
    BLE_OS_AMBIGUOUS,
};

enum ble_os_evidence {
    BLE_OS_DIS_QUERIES = 0x01,
    BLE_OS_DIS_VALUES = 0x02,
    BLE_OS_CLIENT_FEATURES_WRITE = 0x04,
    BLE_OS_DATABASE_HASH_READ = 0x08,
    BLE_OS_APPEARANCE_READ = 0x10,
    BLE_OS_INCLUDED_SERVICES = 0x20,
    BLE_OS_DATABASE_HASH_CYCLE = 0x40,
};

struct ble_os_classifier {
    uint32_t session;
    uint16_t evidence;
    uint16_t dis_start;
    uint16_t dis_end;
    uint8_t dis_step;
    uint8_t dis_reads;
    uint8_t hash_step;
    bool connected;
    bool encrypted;
    bool security_failed;
    bool invalid;
};

// Consomme un événement de capture, à partir d’un état initialisé à zéro.
// attribute_uuid est l’UUID 16 bits résolu du handle pour une lecture/écriture directe,
// ou zéro s’il est inconnu. Les Read By Type utilisent leur UUID dans l’événement.
// CONNECTED ouvre une session neuve et retire toutes les preuves de la précédente.
void ble_os_observe(struct ble_os_classifier *classifier,
                    const struct ble_trace_record *record, uint16_t attribute_uuid);

// Refuse les pertes/rejets, les sessions incohérentes et toute connexion non chiffrée.
// Une hypothèse peut changer si de nouvelles preuves arrivent. Ce moteur ne la sauvegarde pas.
enum ble_os_guess ble_os_result(const struct ble_os_classifier *classifier, bool reliable);

// Donne un nom stable pour les résultats de diagnostic, sans OS par défaut.
const char *ble_os_name(enum ble_os_guess guess);

#endif
