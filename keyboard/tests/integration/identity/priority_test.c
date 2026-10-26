// Intègre les vrais adaptateurs d’identité et de priorité USB avec la vraie politique, pilotes simulés.
#include <identity_stubs.h>
#include <stdio.h>
#include "../../../src/identity/host_identity_adapter.c"
#include "../../../src/connections/usb_priority.c"

static bool native_ready = true, held;
static unsigned sent_reports;
static unsigned release_reports;
// Les rapports protégés peuvent être envoyés vides sans réautoriser la sortie logique.
int __real_zmk_usb_hid_send_keyboard_report(void) { assert(native_ready); release_reports++; return 0; }
// La libération souris complète celle du clavier dans le socle figé.
int zmk_usb_hid_send_mouse_report(void) { assert(splitergoll_usb_mouse_report()->body[0] == 0); release_reports++; return 0; }
// Compte les appels natifs pour vérifier qu’une autorisation révoquée bloque clavier et média avant leur envoi.
int __real_zmk_endpoint_send_report(uint16_t usage_page) { (void)usage_page; sent_reports++; return 0; }
// La souris doit être bloquée de la même manière dès la révocation du contexte.
int __real_zmk_endpoint_send_mouse_report(void) { sent_reports++; return 0; }
// Expose la disponibilité physique indépendamment de notre garde d’autorisation.
bool __real_zmk_usb_is_hid_ready(void) { return native_ready; }
// La garde d’appuis représente aussi un rapport Shift ou un comportement encore actif.
bool splitergoll_ble_auto_busy(void) { return held; }
// Les callbacks doivent seulement notifier le worker, jamais changer la sortie directement.
void splitergoll_ble_auto_notify(void) { selector_notifications++; }
// L’activation USB conserve son callback natif, déjà couverte par le harnais d’identité.
int __real_usb_enable(usb_dc_status_callback callback) { (void)callback; return 0; }

// Déclare le même hôte synthétique depuis les deux transports sécurisés.
static void claim(struct bt_conn *conn, uint8_t identity) {
    uint8_t report[HI_SIZE];
    assert(read_identity(conn, NULL, report, HI_SIZE, 0) == HI_SIZE);
    report[5] = 2; report[7] = 0; memset(report + 40, identity, 16); memset(report + 56, 0, 8);
    assert(write_identity(conn, NULL, report, HI_SIZE, 0, 0) == HI_SIZE);
    struct usb_setup_packet setup = {.wValue = 0x0300, .wLength = HI_SIZE};
    uint8_t *reply; int32_t length = HI_SIZE;
    assert(get_report(&hid_device, &setup, &length, &reply) == 0);
    memcpy(report, reply, HI_SIZE);
    report[5] = 2; report[7] = 0; memset(report + 40, identity, 16); memset(report + 56, 0, 8);
    uint8_t *input = report;
    assert(set_report(&hid_device, &setup, &length, &input) == 0);
}

// Vérifie les bascules et leur révocation immédiate avant tout essai sur V2.
int main(void) {
    // Dans ce mode, HID_0 appartient à ZMK, HID_1 au canal d’identité.
    assert(identity_init() == 0);
    assert(!__wrap_zmk_usb_is_hid_ready());
    status_changed(USB_DC_CONFIGURED, NULL);
    assert(!__wrap_zmk_usb_is_hid_ready());
    struct bt_conn conn = {.info = {.type = BT_CONN_TYPE_LE, .role = BT_CONN_ROLE_PERIPHERAL}, .security = 2, .address = {0}};
    claim(&conn, 42);
    uint8_t runtime[32];
    assert(read_runtime(&conn, NULL, runtime, sizeof(runtime), 0) == sizeof(runtime));
    assert(runtime[4] == 2 && runtime[24] == 1);
    assert(selector_notifications > 0 && endpoint_notifications == 0);
    held = true;
    assert(splitergoll_usb_priority_update() == HI_LEASE_MS);
    assert(!__wrap_zmk_usb_is_hid_ready() && splitergoll_usb_priority_pending());
    held = false;
    assert(splitergoll_usb_priority_update() == HI_LEASE_MS);
    assert(__wrap_zmk_usb_is_hid_ready() && endpoint_notifications == 1 && preferred_writes == 1);
    selected_endpoint.transport = ZMK_TRANSPORT_USB;
    assert(__wrap_zmk_endpoint_send_report(7) == 0 && sent_reports == 1);
    now_ms = 1000; claim(&conn, 42); splitergoll_usb_priority_update();
    assert(endpoint_notifications == 1 && preferred_writes == 1);
    now_ms = 1000 + HI_LEASE_MS; held = true;
    assert(splitergoll_usb_priority_update() == 0 && __wrap_zmk_usb_is_hid_ready());
    held = false; splitergoll_usb_priority_update();
    assert(!__wrap_zmk_usb_is_hid_ready() && endpoint_notifications == 2);
    claim(&conn, 42); splitergoll_usb_priority_update();
    assert(__wrap_zmk_usb_is_hid_ready());
    unsigned prior_reports = sent_reports;
    active_profile = 1;
    assert(!__wrap_zmk_usb_is_hid_ready());
    assert(__wrap_zmk_endpoint_send_report(7) == -ENOTCONN && sent_reports == prior_reports);
    assert(__wrap_zmk_endpoint_send_report(12) == -ENOTCONN && sent_reports == prior_reports);
    assert(__wrap_zmk_endpoint_send_mouse_report() == -ENOTCONN && sent_reports == prior_reports);
    splitergoll_usb_priority_update();
    active_profile = 0; splitergoll_usb_priority_update();
    assert(__wrap_zmk_usb_is_hid_ready());
    status_changed(USB_DC_RESET, NULL);
    assert(!__wrap_zmk_usb_is_hid_ready());
    status_changed(USB_DC_CONFIGURED, NULL);
    assert(!__wrap_zmk_usb_is_hid_ready());
    claim(&conn, 42); splitergoll_usb_priority_update();
    assert(__wrap_zmk_usb_is_hid_ready());
    identity_callbacks.disconnected(&conn, 0);
    assert(!__wrap_zmk_usb_is_hid_ready());
    held = true; splitergoll_usb_priority_update();
    assert(!route.enabled);
    keyboard_report.body[0] = consumer_report.body[0] = mouse_report.body[0] = 42;
    assert(splitergoll_usb_keyboard_report()->body[0] == 0);
    assert(splitergoll_usb_consumer_report()->body[0] == 0);
    assert(splitergoll_usb_mouse_report()->body[0] == 0);
    assert(__wrap_zmk_usb_hid_send_keyboard_report() == 0 && release_reports == 2);
    held = false; claim(&conn, 42); splitergoll_usb_priority_update();
    assert(splitergoll_usb_keyboard_report()->body[0] == 42);
    assert(splitergoll_usb_consumer_report()->body[0] == 42);
    assert(splitergoll_usb_mouse_report()->body[0] == 42);
    struct bt_conn duplicate = {.info = {.type = BT_CONN_TYPE_LE, .role = BT_CONN_ROLE_PERIPHERAL}, .security = 2, .address = {1}};
    bonds |= 2; claim(&duplicate, 42);
    assert(!__wrap_zmk_usb_is_hid_ready());
    assert(splitergoll_usb_keyboard_report()->body[0] == 0);
    held = true; splitergoll_usb_priority_update();
    assert(!route.enabled);
    puts("usb priority adapter: USB natif bloqué sans preuve, bascule, relâchements, révocations et préférence dédupliquée passent");
}
