// Décode uniquement les métadonnées ATT utiles à la reconnaissance, sans payload utilisateur.
#include <splitergoll/ble_events.h>

// Lit un entier Bluetooth sans hypothèse d’alignement du tampon.
static uint16_t little16(const uint8_t *data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

// Parse une liste blanche de requêtes, sans copier les valeurs lues ou écrites.
int ble_trace_decode(const uint8_t *packet, size_t length, bool complete, uint32_t data[5]) {
    if (packet == NULL || length < 4) {
        return -1;
    }
    if (little16(packet + 2) != 0x0004) {
        return 0;
    }
    size_t payload_length = little16(packet);
    if (!complete || payload_length == 0 || payload_length != length - 4) {
        return -1;
    }
    const uint8_t *att = packet + 4;
    data[0] = att[0];
    data[1] = (uint32_t)payload_length;
    data[2] = data[3] = data[4] = 0;
    size_t minimum;
    switch (att[0]) {
        case 0x02: case 0x0a: minimum = 3; break;
        case 0x04: case 0x0c: minimum = 5; break;
        case 0x06: case 0x08: case 0x10: minimum = 7; break;
        case 0x0e: case 0x20: minimum = 5; break;
        case 0x12: case 0x52: minimum = 3; break;
        case 0x16: minimum = 5; break;
        case 0x18: minimum = 2; break;
        default: return 0;
    }
    if (payload_length < minimum) {
        return -1;
    }
    bool fixed_length = att[0] == 0x02 || att[0] == 0x0a || att[0] == 0x04 ||
                        att[0] == 0x0c || att[0] == 0x18;
    bool typed_read = att[0] == 0x08 || att[0] == 0x10;
    if ((fixed_length && payload_length != minimum) ||
        (typed_read && payload_length != 7 && payload_length != 21)) {
        return -1;
    }
    if (att[0] == 0x18) {
        // La valeur du drapeau Execute Write n’est pas nécessaire à cette étude.
    } else {
        data[2] = little16(att + 1);
        switch (att[0]) {
            case 0x04: case 0x06: case 0x08: case 0x10: case 0x0c: case 0x16:
                data[3] = little16(att + 3);
                break;
            default: break;
        }
        if (att[0] == 0x06 || ((att[0] == 0x08 || att[0] == 0x10) && payload_length == 7)) {
            data[4] = little16(att + 5);
        }
        // Read Multiple garde seulement le premier handle et le nombre demandé.
        if (att[0] == 0x0e || att[0] == 0x20) {
            if ((payload_length - 1) % 2 != 0) {
                return -1;
            }
            data[3] = (uint32_t)(payload_length - 1) / 2;
        }
    }
    return 1;
}
