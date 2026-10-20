// Compare des métadonnées ATT publiques, sans modifier les échanges Bluetooth.
#include "ble_os_classifier.h"

#include <string.h>

#define MACOS_EVIDENCE (BLE_OS_DIS_QUERIES | BLE_OS_DIS_VALUES)
#define WINDOWS_COMMON (BLE_OS_CLIENT_FEATURES_WRITE | BLE_OS_DATABASE_HASH_READ | BLE_OS_APPEARANCE_READ)
#define WINDOWS_DISCOVERY (BLE_OS_INCLUDED_SERVICES | BLE_OS_DATABASE_HASH_CYCLE)

// Suit les recherches DIS et les sondages du hash sur toute la base GATT.
static void observe_type(struct ble_os_classifier *classifier, const uint32_t data[5]) {
    uint32_t length = data[1];
    uint32_t start = data[2];
    uint32_t end = data[3];
    uint32_t uuid = data[4];
    if ((length != 7 && length != 21) || start == 0 || start > end ||
        end > UINT16_MAX || uuid > UINT16_MAX || (length == 21 && uuid != 0)) {
        classifier->invalid = true;
        return;
    }
    if (length != 7) {
        return;
    }
    if (uuid == 0x2802) {
        classifier->evidence |= BLE_OS_INCLUDED_SERVICES;
    }
    if (uuid == 0x2b2a && start == 1 && end == UINT16_MAX) {
        // Étapes : sondage (1), lecture directe (2), nouveau sondage qui complète le motif.
        if (classifier->hash_step == 2) {
            classifier->evidence |= BLE_OS_DATABASE_HASH_CYCLE;
        } else {
            classifier->hash_step = 1;
        }
    }
    if (uuid == 0x2a29) {
        classifier->dis_start = (uint16_t)start;
        classifier->dis_end = (uint16_t)end;
        classifier->dis_step = 1;
        classifier->dis_reads = 0;
    } else if (start == classifier->dis_start && end == classifier->dis_end) {
        if (uuid == 0x2a24 && (classifier->dis_step == 1 || classifier->dis_step == 2)) {
            classifier->dis_step = 2;
        } else if (uuid == 0x2a50 && classifier->dis_step >= 2) {
            classifier->dis_step = 3;
            classifier->evidence |= BLE_OS_DIS_QUERIES;
        }
    }
}

// Suit les lectures DIS et du hash, sans lire leur contenu ni supposer leurs handles.
static void observe_read(struct ble_os_classifier *classifier, uint16_t handle, uint16_t uuid) {
    if (uuid == 0x2b2a) {
        classifier->evidence |= BLE_OS_DATABASE_HASH_READ;
        if (classifier->hash_step == 1) classifier->hash_step = 2;
    } else if (uuid == 0x2a01) {
        classifier->evidence |= BLE_OS_APPEARANCE_READ;
    }
    if (classifier->dis_step != 3 || handle < classifier->dis_start ||
        handle > classifier->dis_end) {
        return;
    }
    if (uuid == 0x2a29) {
        classifier->dis_reads |= 1;
    } else if (uuid == 0x2a24) {
        classifier->dis_reads |= 2;
    } else if (uuid == 0x2a50) {
        classifier->dis_reads |= 4;
    }
    if (classifier->dis_reads == 7) {
        classifier->evidence |= BLE_OS_DIS_VALUES;
    }
}

// Ignore les sondages génériques, y compris MTU, et vérifie les champs exploités.
static void observe_att(struct ble_os_classifier *classifier, const uint32_t data[5],
                         uint16_t attribute_uuid) {
    uint32_t opcode = data[0];
    if (opcode == 0x08) {
        observe_type(classifier, data);
    } else if (opcode == 0x0a || opcode == 0x12) {
        bool length_valid = opcode == 0x0a ? data[1] == 3 : data[1] >= 3;
        if (!length_valid || data[1] > UINT16_MAX || data[2] == 0 ||
            data[2] > UINT16_MAX || data[3] != 0 || data[4] != 0) {
            classifier->invalid = true;
            return;
        }
        if (opcode == 0x0a) {
            observe_read(classifier, (uint16_t)data[2], attribute_uuid);
        } else if (data[1] == 4 && attribute_uuid == 0x2b29) {
            classifier->evidence |= BLE_OS_CLIENT_FEATURES_WRITE;
        }
    }
}

// Isole chaque connexion et ne prend aucun paramètre radio pour une identité d’OS.
void ble_os_observe(struct ble_os_classifier *classifier,
                    const struct ble_trace_record *record, uint16_t attribute_uuid) {
    if (record == NULL) {
        classifier->invalid = true;
        return;
    }
    if (record->kind == BLE_TRACE_CONNECTED) {
        bool bad_session = record->session == 0 || (classifier->session != 0 &&
            (classifier->connected || classifier->session == UINT32_MAX ||
             record->session != classifier->session + 1));
        memset(classifier, 0, sizeof(*classifier));
        classifier->session = record->session;
        classifier->connected = true;
        classifier->invalid = bad_session;
        return;
    }
    if (!classifier->connected || record->session != classifier->session ||
        (unsigned)record->kind >= BLE_TRACE_KIND_COUNT) {
        classifier->invalid = true;
        return;
    }
    if (record->kind == BLE_TRACE_DISCONNECTED) {
        classifier->connected = false;
    } else if (record->kind == BLE_TRACE_SECURITY) {
        classifier->encrypted = record->data[0] >= 2 && record->data[0] <= 4 &&
                                record->data[1] == 0;
        if (record->data[1] != 0) {
            classifier->security_failed = true;
        }
    } else if (record->kind == BLE_TRACE_ATT) {
        observe_att(classifier, record->data, attribute_uuid);
    }
}

// Combine uniquement des familles complètes, sans confondre silence et Windows.
enum ble_os_guess ble_os_result(const struct ble_os_classifier *classifier, bool reliable) {
    if (!reliable || classifier->invalid || classifier->security_failed || !classifier->encrypted) {
        return BLE_OS_UNKNOWN;
    }
    bool macos = (classifier->evidence & MACOS_EVIDENCE) == MACOS_EVIDENCE;
    bool windows = (classifier->evidence & WINDOWS_COMMON) == WINDOWS_COMMON &&
                   (classifier->evidence & WINDOWS_DISCOVERY) != 0;
    if (macos && windows) {
        return BLE_OS_AMBIGUOUS;
    }
    return macos ? BLE_OS_MACOS : windows ? BLE_OS_WINDOWS : BLE_OS_UNKNOWN;
}

// Rend les identifiants anglais utilisés par les autres diagnostics du dépôt.
const char *ble_os_name(enum ble_os_guess guess) {
    switch (guess) {
        case BLE_OS_MACOS: return "macos";
        case BLE_OS_WINDOWS: return "windows";
        case BLE_OS_AMBIGUOUS: return "ambiguous";
        default: return "unknown";
    }
}
