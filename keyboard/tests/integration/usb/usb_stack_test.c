// Relie le vrai observateur USB au vrai sélecteur, sans port ni rapport utilisateur.
#include "keyboard/src/os_detection/usb_auto.c"
#include "keyboard/src/os_detection/usb_observer.c"
#include <errno.h>
#include <stdio.h>

static usb_dc_ep_callback endpoints[256];
static usb_dc_status_callback native_status;
static uint8_t packet[8], base;
static uint32_t read_length = 8;
static unsigned reads, forwards, selections, status_calls;
static int read_error, callback_error, layer_error;
static bool hid_ready, reset_during_layer;

// Le contrôle de disponibilité suit le callback natif avant de notifier le sélecteur.
bool zmk_usb_is_hid_ready(void) {
    return hid_ready;
}

// La redirection conserve aussi les inscriptions ratées et les désinscriptions.
int __real_usb_dc_ep_set_callback(uint8_t endpoint, usb_dc_ep_callback callback) {
    if (callback_error)
        return callback_error;
    endpoints[endpoint] = callback;
    return 0;
}

// Chaque requête doit lire une seule fois le pilote, même lorsque l’observation échoue.
int __real_usb_dc_ep_read(uint8_t endpoint, uint8_t *bytes, uint32_t size, uint32_t *length) {
    (void)endpoint;
    reads++;
    if (bytes && size >= sizeof(packet))
        memcpy(bytes, packet, sizeof(packet));
    if (length)
        *length = read_length;
    return read_error;
}

// Stocke le callback installé par le wrapper pour simuler les événements de contrôleur.
void __real_usb_dc_set_status_callback(usb_dc_status_callback callback) {
    native_status = callback;
}

// Aucun événement filtré ne doit atteindre la keymap avant une sélection valide.
int __real_zmk_event_manager_raise(zmk_event_t *event) {
    (void)event;
    forwards++;
    return 0;
}

// Un reset concurrent invalide une sélection de couche pourtant réussie côté keymap.
int __real_zmk_keymap_layer_to(zmk_keymap_layer_id_t layer, bool locking) {
    (void)locking;
    selections++;
    if (reset_during_layer) {
        reset_during_layer = false;
        native_status(USB_DC_RESET, NULL);
    }
    if (layer_error)
        return layer_error;
    base = layer;
    return 0;
}

// Ces points sont absents du firmware normal et ne doivent pas être appelés dans ce scénario.
void splitergoll_usb_trace_reset(void) {
    assert(false);
}
void splitergoll_usb_trace_record(uint32_t ms, const uint8_t *bytes, size_t size) {
    (void)ms;
    (void)bytes;
    (void)size;
    assert(false);
}

// Le pilote natif peut mettre à jour HID avant le passage du travail différé.
static void status_changed(enum usb_dc_status_code status, const uint8_t *parameter) {
    (void)parameter;
    status_calls++;
    if (status == USB_DC_CONFIGURED)
        hid_ready = true;
    if (status == USB_DC_RESET || status == USB_DC_DISCONNECTED)
        hid_ready = false;
}

// Le callback natif délègue sa lecture existante, sans ajout de transaction USB.
static void setup(uint8_t endpoint, enum usb_dc_ep_cb_status_code status) {
    (void)status;
    uint8_t bytes[8];
    uint32_t length;
    __wrap_usb_dc_ep_read(endpoint, bytes, sizeof(bytes), &length);
}

// Le temps est simulé et chaque travail dû est exécuté une fois, sans temporisation réelle.
static void tick(uint32_t now) {
    clock_ms = now;
    for (unsigned i = 0; i < 10; i++) {
        if (!selection_work.queued || (int32_t)(now - selection_work.due) < 0)
            return;
        selection_work.queued = false;
        select_os(&selection_work.work);
    }
    assert(false);
}

// Chaque cas repart d’une nouvelle génération sans OS par défaut.
static void boot(void) {
    classifier = (struct usb_os_classifier){0};
    generation++;
    reliable = true;
    state = (struct usb_auto_state){0};
    auto_ready = 0;
    selection_work.queued = false;
    reading_setup = false;
    read_error = callback_error = layer_error = 0;
    read_length = 8;
    reads = forwards = selections = status_calls = 0;
    hid_ready = reset_during_layer = false;
    clock_ms = 0;
    __wrap_usb_dc_set_status_callback(status_changed);
    assert(__wrap_usb_dc_ep_set_callback(0, setup) == 0);
    auto_init();
    tick(0);
    assert(splitergoll_usb_auto_selected() == USB_OS_UNKNOWN);
}

// Produit seulement un SETUP public, sans contenu de frappe ni descripteur privé.
static void request(uint16_t value, uint16_t language, uint16_t length) {
    const uint8_t bytes[] = {0x80,     0x06,          value,  value >> 8,
                             language, language >> 8, length, length >> 8};
    memcpy(packet, bytes, sizeof(packet));
    unsigned before = reads;
    endpoints[0](0, USB_DC_EP_SETUP);
    assert(reads == before + 1);
}

// Le motif Mac est construit à partir de noms distincts, pas d’un ordre temporel fixe.
static void mac(void) {
    request(0x100, 0, 8);
    request(0x100, 0, 18);
    request(0x301, 0x409, 2);
    request(0x301, 0x409, 18);
    request(0x302, 0x409, 2);
    request(0x302, 0x409, 38);
}

// La saisie n’attend pas un délai arbitraire une fois les preuves et HID disponibles.
static void startup_and_cycle(void) {
    boot();
    mac();
    tick(1);
    assert(splitergoll_usb_auto_selected() == USB_OS_UNKNOWN);
    native_status(USB_DC_CONFIGURED, NULL);
    tick(2);
    assert(state.selected == USB_OS_MACOS && base == AUTO_MACOS_BASE && selections == 1);
    assert(__wrap_zmk_keymap_layer_to(AUTO_MACOS_BASE + 1, false) == 0);
    assert(activated_base == AUTO_MACOS_BASE);
    uint32_t old_generation = generation;
    native_status(USB_DC_SUSPEND, NULL);
    native_status(USB_DC_RESUME, NULL);
    tick(3);
    assert(generation == old_generation && state.selected == USB_OS_MACOS);
    selection_work.queued = false;
    native_status(USB_DC_SOF, NULL);
    assert(!selection_work.queued);
    native_status(USB_DC_DISCONNECTED, NULL);
    tick(4);
    assert(generation != old_generation && state.selected == USB_OS_UNKNOWN);
}

// Les touches ignorées attendent leur relâchement, sans release orpheline ni rejeu.
static void held_and_ignored_events(void) {
    boot();
    zmk_event_t key = {.type = 1, .position = {.position = 0, .state = true}};
    __wrap_zmk_event_manager_raise(&key);
    assert(forwards == 0);
    mac();
    native_status(USB_DC_CONFIGURED, NULL);
    tick(1);
    assert(state.selected == USB_OS_UNKNOWN);
    key.position.state = false;
    __wrap_zmk_event_manager_raise(&key);
    tick(2);
    assert(state.selected == USB_OS_MACOS && forwards == 0);
    key.position.state = true;
    __wrap_zmk_event_manager_raise(&key);
    assert(forwards == 1);
}

// Une trame courte ou un échec natif ne devient jamais une preuve d’OS.
static void failures_and_callbacks(void) {
    boot();
    read_length = 7;
    request(0x100, 0, 8);
    read_length = 8;
    mac();
    native_status(USB_DC_CONFIGURED, NULL);
    tick(1);
    assert(state.selected == USB_OS_UNKNOWN);
    native_status(USB_DC_RESET, NULL);
    tick(2);
    mac();
    native_status(USB_DC_CONFIGURED, NULL);
    layer_error = -EIO;
    tick(3);
    assert(state.selected == USB_OS_UNKNOWN);
    layer_error = 0;
    splitergoll_usb_auto_notify();
    tick(4);
    assert(state.selected == USB_OS_MACOS);
    usb_dc_ep_callback previous = setup_callback;
    callback_error = -EIO;
    assert(__wrap_usb_dc_ep_set_callback(0, setup) == -EIO && setup_callback == previous);
    callback_error = 0;
    assert(__wrap_usb_dc_ep_set_callback(0x81, setup) == 0 && endpoints[0x81] == setup);
    assert(__wrap_usb_dc_ep_set_callback(0, NULL) == 0 && endpoints[0] == NULL);
    __wrap_usb_dc_set_status_callback(NULL);
    assert(native_status == NULL);
    boot();
    mac();
    native_status(USB_DC_CONFIGURED, NULL);
    reset_during_layer = true;
    tick(5);
    assert(state.selected == USB_OS_UNKNOWN);
}

// Vérifie la chaîne observateur-classifieur-sélecteur et ses frontières natives.
int main(void) {
    startup_and_cycle();
    held_and_ignored_events();
    failures_and_callbacks();
    puts("real USB stack: callbacks, cold start, generations, held keys and failures OK");
    return 0;
}
