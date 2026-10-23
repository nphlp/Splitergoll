
#include "diagnostics/power/firmware/power_trace.c"
static char output[20000];
static bool append_during_send;
static unsigned sent_lines, fail_line;

bool trace_send_line(const struct device *uart, const char *line) {
    (void)uart;
    assert(!locks);
    sent_lines++;
    if (fail_line && sent_lines == fail_line)
        return false;
    if (append_during_send) {
        append_during_send = false;
        splitergoll_power_saved(2, -5);
    }
    assert(strlen(output) + strlen(line) < sizeof(output));
    strcat(output, line);
    return true;
}
// La collecte du vrai adaptateur ne réveille rien et la lecture ne change pas son historique.
int main(void) {
    initialize();
    assert(store.count == 1 && boot_id == 0x12345678);
    clock_ms = 1;
    splitergoll_power_window(10000, true);
    activity_state = ZMK_ACTIVITY_IDLE;
    clock_ms = 10000;
    zmk_event_t e = {.activity = {ZMK_ACTIVITY_IDLE}};
    assert(activity_changed(&e) == 0);
    splitergoll_power_window(0, false);
    splitergoll_power_saved(1, 0);
    clock_ms = 15000;
    usb_state = ZMK_USB_CONN_HID;
    e.usb.conn_state = usb_state;
    assert(usb_changed(&e) == 0);
    assert(store.count == 6 && clock_ms == 15000 && activity_state == ZMK_ACTIVITY_IDLE &&
           !store.dropped);
    splitergoll_power_dump(NULL);
    char first[20000];
    strcpy(first, output);
    output[0] = 0;
    splitergoll_power_dump(NULL);
    assert(!strcmp(first, output) && store.count == 6 && activity_state == ZMK_ACTIVITY_IDLE);
    assert(strstr(output, "kind=SAVE activity=1 usb=0 value=1 result=0"));
    assert(strstr(output, "POWERTRACE END\n"));
    output[0] = 0;
    append_during_send = true;
    splitergoll_power_dump(NULL);
    assert(store.count == 7 && strstr(output, "count=6") && !strstr(output, "index=6"));
    output[0] = 0;
    splitergoll_power_dump(NULL);
    assert(strstr(output, "count=7") && strstr(output, "index=6"));
    output[0] = 0;
    sent_lines = 0;
    fail_line = 2;
    splitergoll_power_dump(NULL);
    assert(strstr(output, "POWERTRACE BEGIN") && !strstr(output, "POWERTRACE END"));
    fail_line = 0;

    e.usb.conn_state = ZMK_USB_CONN_NONE;
    assert(usb_changed(&e) == 0);
    assert(store.rejected == 0 && store.records[store.count - 1].usb == 0 &&
           usb_state == ZMK_USB_CONN_HID);
    store = (struct power_trace_store){0};
    clock_ms = 0;
    initialize();
    assert(boot_id == 0x12345679 && store.count == 1);
    puts("real power adapter: passive events, immutable snapshot, concurrent append, TX failure "
         "and boot token OK");
    return 0;
}
