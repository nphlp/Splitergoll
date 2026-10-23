// Exécute le vrai code d’activité ZMK avec le vrai ordonnanceur Zephyr sur ARM émulé.
#include <errno.h>
#include <splitergoll/host_preference.h>
#include <splitergoll/power_store.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/ztest.h>

static bool usb_powered = true;
static unsigned poweroffs, resumes, activity_events;
static int suspend_error;
static struct power_trace_store journal;

// Observe la demande d’extinction sans arrêter la machine de test.
static void test_poweroff(void) {
    poweroffs++;
}

// Seule la frontière d’alimentation est doublée, la condition et les délais restent réels.
#define CONFIG_USB_DEVICE_STACK 1
#define sys_poweroff test_poweroff
#include "activity.c"
#undef sys_poweroff

// Réinjecte les transitions publiées dans le stockage réel du journal.
int raise_zmk_activity_state_changed(struct zmk_activity_state_changed event) {
    activity_events++;
    zassert_true(
        power_trace_append(&journal, (struct power_trace_record){.timestamp_ms = k_uptime_get_32(),
                                                                 .kind = POWER_ACTIVITY,
                                                                 .activity = event.state,
                                                                 .usb = usb_powered ? 1 : 0,
                                                                 .value = event.state}));
    return 0;
}

// Un chargeur et un ordinateur peuvent tous deux fournir de l’énergie USB.
bool zmk_usb_is_powered(void) {
    return usb_powered;
}

// Expose une panne de suspension sans la confondre avec une extinction réussie.
int zmk_pm_suspend_devices(void) {
    return suspend_error;
}

// Compte la restauration demandée après échec de suspension.
void zmk_pm_resume_devices(void) {
    resumes++;
}

// Arrête le timer entre cas et repart d’une vraie activité physique simulée.
static void before(void *fixture) {
    ARG_UNUSED(fixture);
    k_timer_stop(&activity_timer);
    usb_powered = true;
    struct k_work_sync sync;
    k_work_flush(&activity_work, &sync);
    journal = (struct power_trace_store){0};
    poweroffs = resumes = activity_events = 0;
    suspend_error = 0;
    note_activity();
}

ZTEST(activity, test_usb_allows_idle_but_blocks_sleep) {
    k_sleep(K_MSEC(80));
    activity_work_handler(NULL);
    zassert_equal(zmk_activity_get_state(), ZMK_ACTIVITY_IDLE);
    zassert_equal(poweroffs, 0);
}

ZTEST(activity, test_battery_idle_then_poweroff) {
    usb_powered = false;
    k_sleep(K_MSEC(20));
    activity_work_handler(NULL);
    zassert_equal(zmk_activity_get_state(), ZMK_ACTIVITY_IDLE);
    zassert_equal(poweroffs, 0);
    k_sleep(K_MSEC(50));
    activity_work_handler(NULL);
    zassert_equal(zmk_activity_get_state(), ZMK_ACTIVITY_SLEEP);
    zassert_equal(poweroffs, 1);
}

ZTEST(activity, test_suspend_failure_does_not_poweroff) {
    usb_powered = false;
    suspend_error = -EIO;
    k_sleep(K_MSEC(80));
    activity_work_handler(NULL);
    zassert_equal(poweroffs, 0);
    zassert_equal(resumes, 1);
}

ZTEST(activity, test_physical_activity_restarts_deadline) {
    k_sleep(K_MSEC(20));
    activity_work_handler(NULL);
    zassert_equal(zmk_activity_get_state(), ZMK_ACTIVITY_IDLE);
    activity_event_listener(NULL);
    zassert_equal(zmk_activity_get_state(), ZMK_ACTIVITY_ACTIVE);
    activity_work_handler(NULL);
    zassert_equal(zmk_activity_get_state(), ZMK_ACTIVITY_ACTIVE);
}

static struct host_preference preference;
static bool checkpoint_done;

// Sauvegarde le choix avec le vrai moteur, sans provoquer de note_activity.
static void checkpoint(struct k_work *work) {
    ARG_UNUSED(work);
    uint8_t identity[HOST_PREFERENCE_ID_SIZE] = {1}, bytes[HOST_PREFERENCE_RECORD_SIZE];
    zassert_true(host_preference_prepare(&preference, identity, bytes));
    host_preference_saved(&preference, bytes, 0);
    checkpoint_done = true;
}
K_WORK_DELAYABLE_DEFINE(checkpoint_work, checkpoint);

// Exécute la vérification réelle d’activité depuis la même file système.
static void idle_check(struct k_work *work) {
    ARG_UNUSED(work);
    activity_work_handler(NULL);
}
K_WORK_DELAYABLE_DEFINE(idle_work, idle_check);

ZTEST(activity, test_checkpoint_runs_before_during_after_idle) {
    const uint32_t delays[] = {5, 20, 30};
    for (unsigned i = 0; i < ARRAY_SIZE(delays); i++) {
        note_activity();
        preference = (struct host_preference){0};
        checkpoint_done = false;
        uint32_t last_input = activity_last_uptime;
        k_work_schedule(&checkpoint_work, K_MSEC(delays[i]));
        k_work_schedule(&idle_work, K_MSEC(20));
        k_sleep(K_MSEC(45));
        zassert_true(checkpoint_done && preference.present);
        zassert_equal(zmk_activity_get_state(), ZMK_ACTIVITY_IDLE);
        zassert_equal(activity_last_uptime, last_input);
    }
}

ZTEST(activity, test_native_periodic_timer_enters_idle) {
    k_timer_start(&activity_timer, K_MSEC(20), K_MSEC(20));
    k_sleep(K_MSEC(50));
    k_timer_stop(&activity_timer);
    zassert_equal(zmk_activity_get_state(), ZMK_ACTIVITY_IDLE);
    zassert_true(activity_events > 0);
    zassert_equal(poweroffs, 0);
}

ZTEST_SUITE(activity, NULL, NULL, before, NULL, NULL);
