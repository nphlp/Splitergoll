#include "keyboard/src/connections/host_preference_settings.c"
#include <stdio.h>
static uint8_t durable[HOST_PREFERENCE_RECORD_SIZE];
void splitergoll_ble_auto_notify(void) {
}
static unsigned writes;
#if defined(CONFIG_SPLITERGOLL_POWER_TRACE)
static unsigned trace_writes;
// L’observation suit l’écriture native et n’en déclenche jamais une autre.
void splitergoll_power_saved(unsigned profile, int error) {
    trace_writes++;
    assert(trace_writes == writes && profile < 2);
    assert((profile == 0 && error == 0) || (profile == 1 && error == -ENOSPC));
}
#endif
static int save_error;
static bool verified[5] = {true, true, false, false, false};
static uint8_t identities[5][32] = {{1}, {2}};
bool zmk_ble_profile_is_open(unsigned p) {
    assert(p < 5);
    return p > 1;
}
bool ble_os_settings_identity(unsigned p, uint8_t out[32]) {
    if (p >= 5 || !verified[p])
        return false;
    memcpy(out, identities[p], 32);
    return true;
}
int settings_save_one(const char *key, const void *bytes, size_t length) {
    assert(!strcmp(key, "splitergoll/host-choice/value") && length == sizeof(durable));
    writes++;
    if (save_error)
        return save_error;
    memcpy(durable, bytes, length);
    return 0;
}
static ssize_t read_record(void *ctx, void *bytes, size_t length) {
    memcpy(bytes, ctx, length);
    return (ssize_t)length;
}
int main(void) {
    splitergoll_host_preference_save(0);
    assert(writes == 0);
    committed();
    assert(splitergoll_host_preference_status().loaded &&
           !splitergoll_host_preference_status().present);
    splitergoll_host_preference_save(0);
    assert(writes == 1 && splitergoll_host_preference_status().profile == 0);
    splitergoll_host_preference_save(0);
    assert(writes == 1);
#if defined(CONFIG_SPLITERGOLL_POWER_TRACE)
    assert(trace_writes == 1);
#endif
    save_error = -ENOSPC;
    splitergoll_host_preference_save(1);
    assert(writes == 2 && splitergoll_host_preference_status().profile == 0 &&
           splitergoll_host_preference_status().error == -ENOSPC);
#if defined(CONFIG_SPLITERGOLL_POWER_TRACE)
    assert(trace_writes == 2);
#endif
    preference = (struct host_preference){0};
    atomic_set(&loaded, 0);
    assert(restore("value", sizeof(durable), read_record, durable) == 0);
    committed();
    assert(splitergoll_host_preference_status().profile == 0);
    verified[0] = false;
    assert(splitergoll_host_preference_status().profile == -1 &&
           splitergoll_host_preference_status().present);
    verified[0] = true;
    identities[0][31] = 0x80;
    assert(splitergoll_host_preference_status().profile == -1);
    assert(restore("other", sizeof(durable), read_record, durable) == -ENOENT);
    assert(restore("value", 1, read_record, durable) == 0 &&
           splitergoll_host_preference_status().error == -EINVAL);
    assert(restore("value", 0, read_record, durable) == 0 &&
           !splitergoll_host_preference_status().present);
    puts(
        "real preference backend: namespace, deduplication, reset, identity and write failures OK");
}
