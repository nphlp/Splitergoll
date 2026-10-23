#ifndef MEMORY_STUB_H
#define MEMORY_STUB_H
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#define ZMK_BLE_PROFILE_COUNT 5
#define CONFIG_BT_SMP_SC_PAIR_ONLY 1
#define CONFIG_BT_SIGNING 0
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 0
#define IS_ENABLED(x) (x)
#define LOG_MODULE_REGISTER(...)
#define LOG_INF(...)
#define LOG_WRN(...)
#define BUILD_ASSERT(x) _Static_assert(x, #x)
#define ARG_UNUSED(x) (void)(x)
#define APPLICATION 0
#define SYS_INIT(fn, ...) static const void *init_##fn __attribute__((unused)) = (const void *)&fn
#define K_FOREVER -1
#define K_NO_WAIT 0
#define K_MUTEX_DEFINE(name) int name
#define K_SEM_DEFINE(name, ...) int name
static inline void k_sem_give(int *p) {
    *p = 1;
}
static inline int k_sem_take(int *p, int timeout) {
    (void)timeout;
    assert(*p);
    *p = 0;
    return 0;
}
#define K_MSGQ_DEFINE(name, ...) int name
#define K_THREAD_DEFINE(name, size, fn, ...)                                                       \
    static const void *name __attribute__((unused)) = (const void *)&fn
static inline int k_mutex_lock(int *p, int t) {
    (void)t;
    (*p)++;
    return 0;
}
static inline void k_mutex_unlock(int *p) {
    assert(*p > 0);
    (*p)--;
}
static inline int k_msgq_put(int *p, const void *v, int t) {
    (void)p;
    (void)v;
    (void)t;
    return 0;
}
static inline int k_msgq_get(int *p, void *v, int t) {
    (void)p;
    (void)v;
    (void)t;
    return 0;
}
typedef int atomic_t;
static inline void atomic_set(atomic_t *p, int v) {
    *p = v;
}
static inline int atomic_get(atomic_t *p) {
    return *p;
}
struct device {
    int unused;
};
#define BT_ID_DEFAULT 0
#define BT_CONN_TYPE_LE 1
#define BT_CONN_ROLE_PERIPHERAL 1
struct bt_addr {
    uint8_t val[6];
};
typedef struct {
    uint8_t type;
    struct bt_addr a;
} bt_addr_le_t;
static const bt_addr_le_t empty_address;
#define BT_ADDR_LE_ANY (&empty_address)
static inline bool bt_addr_le_eq(const bt_addr_le_t *a, const bt_addr_le_t *b) {
    return __builtin_memcmp(a, b, sizeof(*a)) == 0;
}
struct bt_conn {
    unsigned index;
    int role;
    int profile;
    bt_addr_le_t peer;
};
struct bt_conn_info {
    int type, role;
};
struct bt_conn_auth_info_cb {
    void (*pairing_complete)(struct bt_conn *, bool);
    void (*bond_deleted)(uint8_t, const bt_addr_le_t *);
};
unsigned bt_conn_index(struct bt_conn *);
struct bt_conn *bt_conn_ref(struct bt_conn *);
void bt_conn_unref(struct bt_conn *);
const bt_addr_le_t *bt_conn_get_dst(struct bt_conn *);
int bt_conn_get_info(struct bt_conn *, struct bt_conn_info *);
int bt_conn_auth_info_cb_register(struct bt_conn_auth_info_cb *);
bool zmk_ble_profile_is_open(unsigned);
int zmk_ble_profile_index(const bt_addr_le_t *);
const bt_addr_le_t *zmk_ble_profile_address(unsigned);
typedef ssize_t (*settings_read_cb)(void *, void *, size_t);
typedef int (*settings_load_direct_cb)(const char *, size_t, settings_read_cb, void *, void *);
int settings_load_subtree_direct(const char *, settings_load_direct_cb, void *);
int settings_save_one(const char *, const void *, size_t);
int settings_delete(const char *);
#define SETTINGS_STATIC_HANDLER_DEFINE(name, tree, get, set, commit, export)                       \
    static const void *handler_##name[]                                                            \
        __attribute__((unused)) = {(const void *)&set, (const void *)&commit}
int mbedtls_sha256(const unsigned char *, size_t, unsigned char *, int);
#endif
