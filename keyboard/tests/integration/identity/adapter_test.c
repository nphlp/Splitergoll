// Exerce le vrai adaptateur, seuls les appels Zephyr et la radio sont remplacés.
#include <identity_stubs.h>
#include <stdio.h>
#include "../../../src/identity/host_identity_adapter.c"

static usb_dc_status_callback callback;
static unsigned native_events;
// Capture le callback effectivement transmis au socle natif, sans le contourner.
int __real_usb_enable(usb_dc_status_callback next) { callback = next; return 0; }
// Vérifie que chaque événement USB est encore transmis au callback ZMK une fois.
static void native(enum usb_dc_status_code status, const uint8_t *param) { (void)status; (void)param; native_events++; }
// Produit l’annonce synthétique correspondant au challenge courant de cette liaison.
static void announcement(uint8_t *report) {
    report[5] = 2; report[7] = 0; memset(report + 40, 42, 16); memset(report + 56, 0, 8);
}
// Couvre les frontières de sécurité et les contextes avant toute interaction physique.
int main(void) {
    assert(identity_init() == 0 && registered_ops);
    assert(__wrap_usb_enable(native) == 0 && callback);
    struct usb_setup_packet setup = {.wValue = 0x0300, .wLength = HI_SIZE};
    int32_t length = HI_SIZE; uint8_t *response;
    assert(get_report(&hid_device, &setup, &length, &response) < 0);
    callback(USB_DC_CONFIGURED, NULL); assert(native_events == 1);
    assert(get_report(&hid_device, &setup, &length, &response) == 0 && length == HI_SIZE);
    uint8_t usb_claim[HI_SIZE]; memcpy(usb_claim, response, HI_SIZE); announcement(usb_claim);
    uint8_t *input = usb_claim; assert(set_report(&hid_device, &setup, &length, &input) == 0);
    struct bt_conn conn = {.info = {.type = BT_CONN_TYPE_LE, .role = BT_CONN_ROLE_PERIPHERAL}, .security = 1, .address = {0}};
    uint8_t ble_claim[HI_SIZE];
    assert(read_identity(&conn, NULL, ble_claim, HI_SIZE, 0) == -BT_ATT_ERR_AUTHORIZATION);
    conn.security = 2; conn.info.role = 0;
    assert(read_identity(&conn, NULL, ble_claim, HI_SIZE, 0) == -BT_ATT_ERR_AUTHORIZATION);
    conn.info.role = BT_CONN_ROLE_PERIPHERAL; bonds = 0;
    assert(read_identity(&conn, NULL, ble_claim, HI_SIZE, 0) == -BT_ATT_ERR_AUTHORIZATION);
    bonds = 1; conn.address.value = 9;
    assert(read_identity(&conn, NULL, ble_claim, HI_SIZE, 0) == -BT_ATT_ERR_AUTHORIZATION);
    conn.address.value = 0;
    uint8_t runtime[32];
    active_profile = 1; selected_endpoint.transport = ZMK_TRANSPORT_BLE;
    active_layers = 1U << AUTO_WINDOWS_BASE;
    assert(read_runtime(&conn, NULL, runtime, sizeof(runtime), 0) == sizeof(runtime));
    assert(!memcmp(runtime, "SGST", 4) && runtime[4] == 1 && runtime[5] == 2 && runtime[6] == 2 && runtime[7] == 1);
    assert(!memcmp(runtime + 8, state.boot, HI_ID_SIZE) && selector_lock_depth == 0);
    for (unsigned i = 24; i < sizeof(runtime); i++) assert(runtime[i] == 0);
    active_layers = 1U << AUTO_MACOS_BASE;
    assert(read_runtime(&conn, NULL, runtime, sizeof(runtime), 0) == sizeof(runtime) && runtime[5] == 1);
    active_layers |= 1U << AUTO_WINDOWS_BASE;
    assert(read_runtime(&conn, NULL, runtime, sizeof(runtime), 0) == sizeof(runtime) && runtime[5] == 0);
    active_layers = 0; active_profile = -1; selected_endpoint.transport = ZMK_TRANSPORT_NONE;
    assert(read_runtime(&conn, NULL, runtime, sizeof(runtime), 0) == sizeof(runtime) && runtime[5] == 0 && runtime[6] == 0 && runtime[7] == 255);
    conn.security = 1;
    assert(read_runtime(&conn, NULL, runtime, sizeof(runtime), 0) == -BT_ATT_ERR_AUTHORIZATION);
    conn.security = 2;
    assert(read_runtime(&conn, NULL, runtime, sizeof(runtime), 33) < 0);
    assert(read_runtime(&conn, NULL, runtime, 8, 24) == 8);
    assert(read_identity(&conn, NULL, ble_claim, HI_SIZE, 0) == HI_SIZE);
    announcement(ble_claim);
    assert(write_identity(&conn, NULL, ble_claim, 20, 0, BT_GATT_WRITE_FLAG_PREPARE) == 0);
    assert(!state.links[1].claimed);
    assert(write_identity(&conn, NULL, ble_claim, 20, 0, 0) < 0);
    assert(write_identity(&conn, NULL, ble_claim, HI_SIZE, 0, BT_GATT_WRITE_FLAG_CMD) < 0);
    assert(write_identity(&conn, NULL, ble_claim, HI_SIZE, 0, 0) == HI_SIZE);
    assert(get_report(&hid_device, &setup, &length, &response) == 0 && response[7] == 1);
    identity_callbacks.security_changed(&conn, 1, BT_SECURITY_ERR_SUCCESS);
    assert(!state.links[1].claimed);
    conn.security = 1;
    assert(read_identity(&conn, NULL, ble_claim, HI_SIZE, 0) == -BT_ATT_ERR_AUTHORIZATION);
    conn.security = 2;
    assert(read_identity(&conn, NULL, ble_claim, HI_SIZE, 0) == HI_SIZE); announcement(ble_claim);
    identity_callbacks.disconnected(&conn, 0);
    assert(write_identity(&conn, NULL, ble_claim, HI_SIZE, 0, 0) < 0);
    assert(read_identity(&conn, NULL, ble_claim, HI_SIZE, 0) == HI_SIZE); announcement(ble_claim);
    identity_callbacks.connected(&conn, 0);
    assert(write_identity(&conn, NULL, ble_claim, HI_SIZE, 0, 0) < 0);
    callback(USB_DC_RESET, NULL); callback(USB_DC_CONFIGURED, NULL);
    assert(native_events == 3 && set_report(&hid_device, &setup, &length, &input) < 0);
    callback(USB_DC_SUSPEND, NULL); assert(get_report(&hid_device, &setup, &length, &response) < 0);
    callback(USB_DC_RESUME, NULL); assert(get_report(&hid_device, &setup, &length, &response) == 0);
    setup.wValue = 0x0100; assert(get_report(&hid_device, &setup, &length, &response) < 0);
    assert(native_events == 5);
    puts("host identity adapter: sécurité, fragments, callbacks natifs et reconnexions passent");
    return 0;
}
