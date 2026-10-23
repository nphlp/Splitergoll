// Harnais local du vrai worker et du vrai backend, sans radio ni USB matériel.
#include "diagnostics/os-signature/firmware/ble_trace_memory.c"
#include "keyboard/src/os_detection/ble_acquisition.c"
#include "keyboard/src/os_detection/ble_os_settings.c"
#include <stdlib.h>
static struct bt_conn hosts[2];
static const char *test_output;
static uint32_t epochs[6];
uint32_t splitergoll_ble_connection_epoch(struct bt_conn *conn) {
    return epochs[conn->index];
}
void splitergoll_ble_connection_changed(struct bt_conn *conn) {
    epochs[conn->index]++;
}
static uint8_t disk[5][BLE_OS_MEMORY_SIZE];
static bool on_disk[5], bonds[5];
static uint8_t bond_value[5];
static int write_error, read_error;
static unsigned writes, deletes, hashes, references;
static char output[8192];
static size_t output_length;

// L’identité synthétique représente le contenu privé du bond sans clé réelle.
unsigned bt_conn_index(struct bt_conn *c) {
    return c->index;
}
struct bt_conn *bt_conn_ref(struct bt_conn *c) {
    references++;
    return c;
}
void bt_conn_unref(struct bt_conn *c) {
    (void)c;
    assert(references);
    references--;
}
const bt_addr_le_t *bt_conn_get_dst(struct bt_conn *c) {
    return &c->peer;
}
int bt_conn_get_info(struct bt_conn *c, struct bt_conn_info *i) {
    *i = (struct bt_conn_info){BT_CONN_TYPE_LE, c->role};
    return 0;
}
int bt_conn_auth_info_cb_register(struct bt_conn_auth_info_cb *c) {
    (void)c;
    return 0;
}
int zmk_ble_profile_index(const bt_addr_le_t *p) {
    for (unsigned i = 0; i < 2; i++)
        if (bt_addr_le_eq(p, &hosts[i].peer))
            return hosts[i].profile;
    return -ENODEV;
}
const bt_addr_le_t *zmk_ble_profile_address(unsigned p) {
    for (unsigned i = 0; i < 2; i++)
        if (hosts[i].profile == (int)p)
            return &hosts[i].peer;
    return BT_ADDR_LE_ANY;
}

// Les tests interdisent toute écriture hors de notre propre namespace.
int settings_save_one(const char *key, const void *v, size_t n) {
    unsigned p;
    assert(sscanf(key, "splitergoll/ble-os/%u", &p) == 1 && p < 5);
    assert(n == BLE_OS_MEMORY_SIZE);
    writes++;
    if (write_error)
        return write_error;
    memcpy(disk[p], v, n);
    on_disk[p] = true;
    return 0;
}
int settings_delete(const char *key) {
    unsigned p;
    assert(sscanf(key, "splitergoll/ble-os/%u", &p) == 1 && p < 5);
    deletes++;
    on_disk[p] = false;
    return 0;
}
ssize_t read_bytes(void *context, void *out, size_t n) {
    if (read_error)
        return read_error;
    memcpy(out, context, n);
    return (ssize_t)n;
}
int settings_load_subtree_direct(const char *key, settings_load_direct_cb cb, void *context) {
    if (read_error)
        return read_error;
    for (unsigned i = 0; i < 2; i++) {
        char expected[32];
        const bt_addr_le_t *p = &hosts[i].peer;
        const uint8_t *a = p->a.val;
        snprintf(expected, sizeof(expected), "bt/keys/%02x%02x%02x%02x%02x%02x%u", a[5], a[4], a[3],
                 a[2], a[1], a[0], p->type);
        if (strcmp(key, expected) == 0) {
            unsigned profile = hosts[i].profile;
            assert(profile < 5);
            if (bonds[profile]) {
                uint8_t v[80];
                memset(v, bond_value[profile], sizeof(v));
                return cb(NULL, sizeof(v), read_bytes, v, context);
            }
            return 0;
        }
    }
    assert(!"unexpected bond path");
    return -ENOENT;
}
int mbedtls_sha256(const unsigned char *in, size_t n, unsigned char *out, int type) {
    assert(n == 87 && type == 0);
    hashes++;
    memset(out, in[7], 32);
    out[0] ^= in[1];
    return 0;
}
bool trace_send_line(const struct device *d, const char *line) {
    (void)d;
    size_t n = strlen(line);
    assert(output_length + n < sizeof(output));
    memcpy(output + output_length, line, n + 1);
    output_length += n;
    return true;
}

// Réinitialise la RAM et restaure seulement les enregistrements effectivement écrits.
static void reboot(void) {
    memset(sessions, 0, sizeof(sessions));
    memset(memories, 0, sizeof(memories));
    memset(measured, 0, sizeof(measured));
    memset(learning, 0, sizeof(learning));
    ready = 0;
    lost = 0;
    overflow_handled = false;
    next_session = 0;
    for (unsigned p = 0; p < 5; p++)
        if (on_disk[p]) {
            char key[8];
            snprintf(key, sizeof(key), "%u", p);
            assert(restore(key, BLE_OS_MEMORY_SIZE, read_bytes, disk[p]) == 0);
        }
    loaded();
}
static void event(unsigned h, enum ble_trace_kind kind, uint32_t a, uint32_t b, uint32_t c,
                  uint32_t d, uint32_t e, uint16_t uuid) {
    struct message m = {.conn = &hosts[h],
                        .epoch = epochs[h],
                        .reliable = true,
                        .event = {.kind = kind, .data = {a, b, c, d, e}},
                        .uuid = uuid};
    process(&m);
}
static void pair(unsigned h) {
    struct message m = {.conn = &hosts[h], .epoch = epochs[h], .kind = PAIRED};
    process(&m);
}
static void mac(unsigned h) {
    event(h, BLE_TRACE_ATT, 8, 7, 0x1b, 0x21, 0x2a29, 0);
    event(h, BLE_TRACE_ATT, 8, 7, 0x1b, 0x21, 0x2a24, 0);
    event(h, BLE_TRACE_ATT, 8, 7, 0x1b, 0x21, 0x2a50, 0);
    event(h, BLE_TRACE_ATT, 10, 3, 0x1d, 0, 0, 0x2a29);
    event(h, BLE_TRACE_ATT, 10, 3, 0x1f, 0, 0, 0x2a24);
    event(h, BLE_TRACE_ATT, 10, 3, 0x21, 0, 0, 0x2a50);
}
static void windows(unsigned h) {
    event(h, BLE_TRACE_ATT, 0x12, 4, 4, 0, 0, 0x2b29);
    event(h, BLE_TRACE_ATT, 0x0a, 3, 5, 0, 0, 0x2b2a);
    event(h, BLE_TRACE_ATT, 0x0a, 3, 6, 0, 0, 0x2a01);
    event(h, BLE_TRACE_ATT, 8, 7, 1, 60, 0x2802, 0);
}
static void connect_host(unsigned h) {
    event(h, BLE_TRACE_CONNECTED, 0, 0, 0, 0, 0, 0);
    event(h, BLE_TRACE_SECURITY, 2, 0, 0, 0, 0, 0);
}
static void close_host(unsigned h) {
    event(h, BLE_TRACE_DISCONNECTED, 0x13, 0, 0, 0, 0, 0);
}
static void dump(const char *path) {
    output_length = 0;
    ble_trace_memory_dump(NULL);
    const char *directory = test_output;
    assert(directory);
    char filename[1024];
    assert(snprintf(filename, sizeof(filename), "%s/%s", directory, path) < (int)sizeof(filename));
    FILE *f = fopen(filename, "w");
    assert(f);
    fputs(output, f);
    fclose(f);
}

int main(int argc, char **argv) {
    assert(argc == 2);
    test_output = argv[1];
    hosts[0] = (struct bt_conn){.index = 0,
                                .role = BT_CONN_ROLE_PERIPHERAL,
                                .profile = 0,
                                .peer = {.type = 0, .a.val = {1, 2, 3, 4, 5, 6}}};
    hosts[1] = (struct bt_conn){.index = 1,
                                .role = BT_CONN_ROLE_PERIPHERAL,
                                .profile = 1,
                                .peer = {.type = 1, .a.val = {2, 3, 4, 5, 6, 7}}};
    assert(!ble_os_settings_bind(0, false));
    assert(!ble_os_settings_snapshot(0).present && writes == 0);
    bonds[0] = bonds[1] = true;
    bond_value[0] = 11;
    bond_value[1] = 22;
    reboot();
    ble_os_settings_wait_ready();
    ble_os_settings_wait_ready();
    // Les anciens appairages n’acquièrent aucun OS sur une reconnexion, même riche en indices.
    connect_host(0);
    mac(0);
    assert(writes == 0 && !memories[0].present);
    dump("legacy.txt");
    close_host(0);
    event(0, BLE_TRACE_CONNECTED, 0, 0, 0, 0, 1, 0);
    event(0, BLE_TRACE_SECURITY, 2, 0, 0, 0, 0, 0);
    mac(0);
    pair(0);
    assert(memories[0].os == BLE_MEMORY_UNKNOWN);
    mac(0);
    assert(memories[0].os == BLE_MEMORY_MACOS && !memories[0].dirty && on_disk[0]);
    connect_host(1);
    pair(1);
    windows(1);
    assert(memories[1].os == BLE_MEMORY_WINDOWS && !memories[1].dirty);
    dump("initial.txt");
    unsigned before = writes;
    close_host(1);
    connect_host(1);
    event(1, BLE_TRACE_ATT, 2, 3, 23, 0, 0, 0);
    assert(writes == before && memories[1].os == BLE_MEMORY_WINDOWS);
    close_host(0);
    connect_host(0);
    windows(0);
    assert(memories[0].os == BLE_MEMORY_MACOS && writes == before); // Pas de redétection.
    // Redémarrage, chargement et identité vérifiée sans découverte GATT.
    reboot();
    assert(memories[0].present && !memories[0].verified);
    connect_host(0);
    connect_host(1);
    assert(memories[0].os == BLE_MEMORY_MACOS && memories[1].os == BLE_MEMORY_WINDOWS &&
           writes == before);
    struct ble_acquisition_snapshot m0 = splitergoll_ble_acquisition_snapshot(0),
                                    m1 = splitergoll_ble_acquisition_snapshot(1);
    assert(m0.connected && m1.connected && m0.secure && m1.secure && m0.reliable && m1.reliable);
    assert(m0.memory.verified && m1.memory.verified && !m0.learning && !m1.learning);
    assert(m1.epoch == epochs[1]);
    splitergoll_ble_connection_changed(&hosts[1]);
    assert(splitergoll_ble_acquisition_snapshot(1).epoch != epochs[1]);
    event(1, BLE_TRACE_SECURITY, 2, 0, 0, 0, 0, 0);
    assert(splitergoll_ble_acquisition_snapshot(1).epoch == epochs[1]);
    dump("reboot.txt");
    // Remplacement pendant un autre firmware : la même adresse avec une autre clé ne reprend pas
    // l’OS.
    bond_value[1]++;
    reboot();
    connect_host(1);
    assert(!memories[1].present && memories[1].os == BLE_MEMORY_UNKNOWN);
    pair(1);
    windows(1);
    assert(memories[1].os == BLE_MEMORY_WINDOWS);
    // Nouveau pairing_complete avec le même digest invalide également le résultat précédent.
    pair(1);
    assert(memories[1].os == BLE_MEMORY_UNKNOWN);
    windows(1);
    assert(memories[1].os == BLE_MEMORY_WINDOWS);
    // Une ambiguïté tardive retire la décision acquise, pas les appairages.
    mac(1);
    assert(measured[1] == BLE_OS_AMBIGUOUS && memories[1].os == BLE_MEMORY_UNKNOWN);
    // Aucun résultat n’est annoncé sauvegardé si NVS refuse l’écriture.
    close_host(0);
    connect_host(0);
    pair(0);
    write_error = -ENOSPC;
    mac(0);
    assert(memories[0].dirty && memories[0].error == -ENOSPC);
    before = writes;
    dump("error.txt");
    assert(writes == before && strstr(output, "stored=unknown origin=initial state=error"));
    write_error = 0;
    // La saturation du transport de travail invalide l’acquisition, indépendamment des traces
    // historiques.
    lost = 1;
    event(0, BLE_TRACE_SECURITY, 2, 0, 0, 0, 0, 0);
    assert(memories[0].os == BLE_MEMORY_UNKNOWN);
    dump("overflow.txt");
    // Une notification de suppression efface seulement notre association ciblée.
    struct message m = {.kind = FORGOTTEN, .profile = 1};
    process(&m);
    assert(!on_disk[1] && on_disk[0] && deletes == 1);
    // Réglages absents/corrompus : refus, jamais un OS inventé.
    read_error = -EIO;
    assert(!ble_os_settings_bind(0, false) && memories[0].error == -EIO && !memories[0].verified);
    read_error = 0;
    uint8_t invalid[BLE_OS_MEMORY_SIZE] = {0};
    assert(!restore("0", sizeof(invalid), read_bytes, invalid) && memories[0].error == -EINVAL);
    assert(!restore("0", 0, read_bytes, invalid) && !memories[0].present);
    assert(restore("0/extra", 40, read_bytes, invalid) == -ENOENT);
    // La correction reste réservée à un nouvel appairage, sans réinterpréter un ancien unknown.
    reboot();
    connect_host(0);
    pair(0);
    mac(0);
    assert(memories[0].os == BLE_MEMORY_MACOS);
    assert(ble_os_settings_bind(1, true));
    assert(memories[1].os == BLE_MEMORY_UNKNOWN && !memories[1].dirty);
    uint8_t saved_mac[BLE_OS_MEMORY_SIZE];
    memcpy(saved_mac, disk[0], sizeof(saved_mac));
    reboot();
    connect_host(1);
    before = writes;
    event(1, BLE_TRACE_ATT, 8, 7, 1, 0xffff, 0x2b2a, 0);
    event(1, BLE_TRACE_ATT, 0x12, 4, 6, 0, 0, 0x2b29);
    event(1, BLE_TRACE_ATT, 0x0a, 3, 0x0d, 0, 0, 0x2a01);
    event(1, BLE_TRACE_ATT, 0x0a, 3, 8, 0, 0, 0x2b2a);
    event(1, BLE_TRACE_ATT, 8, 7, 1, 0xffff, 0x2b2a, 0);
    assert(writes == before && memories[1].os == BLE_MEMORY_UNKNOWN);
    // Rejoue la vraie session Windows, avec pairing_complete après SECURITY comme sur le matériel.
    close_host(1);
    ble_os_settings_forget(1);
    FILE *capture = fopen(".local/ble-traces/macos-2026-10-20-14-26-33-P67lbi", "r");
    assert(capture);
    uint16_t attr_uuid[512] = {0};
    char row[512], kind[40];
    unsigned handle;
    char uuid_text[40];
    while (fgets(row, sizeof(row), capture))
        if (sscanf(row, "BLETRACE ATTR %x %39s", &handle, uuid_text) == 2) {
            assert(handle < 512);
            attr_uuid[handle] = (uint16_t)strtoul(uuid_text, NULL, 16);
        }
    rewind(capture);
    unsigned index, timestamp, sid, data[5], replayed = 0;
    while (fgets(row, sizeof(row), capture)) {
        if (sscanf(row, "BLETRACE EVENT %u %u %u %39s %x %x %x %x %x", &index, &timestamp, &sid,
                   kind, &data[0], &data[1], &data[2], &data[3], &data[4]) != 9 ||
            sid != 9)
            continue;
        enum ble_trace_kind type = BLE_TRACE_KIND_COUNT;
        for (unsigned i = 0; i < BLE_TRACE_KIND_COUNT; i++)
            if (strcmp(kind, ble_trace_kind_name(i)) == 0)
                type = (enum ble_trace_kind)i;
        assert(type < BLE_TRACE_KIND_COUNT);
        uint16_t uuid =
            type == BLE_TRACE_ATT && (data[0] == 0x0a || data[0] == 0x12) ? attr_uuid[data[2]] : 0;
        event(1, type, data[0], data[1], data[2], data[3], data[4], uuid);
        if (type == BLE_TRACE_SECURITY)
            pair(1);
        replayed++;
    }
    fclose(capture);
    assert(replayed == 49);
    assert(memories[1].os == BLE_MEMORY_WINDOWS && memories[1].evidence == 0x5c &&
           !memories[1].dirty && memories[1].error == 0);
    assert(memcmp(saved_mac, disk[0], sizeof(saved_mac)) == 0);
    before = writes;
    reboot();
    connect_host(0);
    connect_host(1);
    assert(memories[0].os == BLE_MEMORY_MACOS && memories[1].os == BLE_MEMORY_WINDOWS &&
           memories[1].evidence == 0x5c && writes == before);
    assert(!learning[1] && measured[1] == BLE_OS_UNKNOWN);
    dump("reboot-memory.txt");
    puts("real partial Windows pairing: learned/saved 005c, reboot/reconnect, existing unknown and "
         "Mac preserved OK");
    assert(references == 0 && hashes > 0);
    puts("real BLE worker/settings: initial, reconnect, reboot, replacement, ambiguity, NVS "
         "errors, overflow, deletion OK");
    return 0;
}
