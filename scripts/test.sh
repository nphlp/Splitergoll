#!/bin/bash

# Compile et exécute les moteurs C et journaux avec ASan/UBSan, sans clavier ni Docker.
# Les exécutables restent dans .local/tests, chaque exécution possède son propre dossier.
set -euo pipefail

SCRIPT_DIRECTORY=$(CDPATH= cd "$(dirname "$0")" && pwd)
REPOSITORY=$(CDPATH= cd "$SCRIPT_DIRECTORY/.." && pwd)
command -v clang >/dev/null || { printf '%s\n' 'Prérequis absent : clang'; exit 1; }
mkdir -p "$REPOSITORY/.local/tests"
TEST_DIRECTORY=$(mktemp -d "$REPOSITORY/.local/tests/native-XXXXXX")
SUITE_COUNT=0
cd "$REPOSITORY"

# Vérifie un comportement avec le vrai moteur C et un test déterministe sans attente réelle.
run_suite() {
    local name=$1 test=$2
    shift 2
    clang -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
        -I. -Ikeyboard/include -Ikeyboard/config/keymaps -Idiagnostics/os-signature/firmware \
        -Idiagnostics/power/firmware/include "$@" "$test" -o "$TEST_DIRECTORY/$name"
    if ! "$TEST_DIRECTORY/$name" "$TEST_DIRECTORY" > "$TEST_DIRECTORY/$name.log" 2>&1; then
        cat "$TEST_DIRECTORY/$name.log"
        return 1
    fi
    tail -n 1 "$TEST_DIRECTORY/$name.log"
    SUITE_COUNT=$((SUITE_COUNT + 1))
}

run_suite usb_os_classifier keyboard/tests/usb_os_classifier_test.c keyboard/src/os_detection/usb_os_classifier.c
run_suite usb_auto_state keyboard/tests/usb_auto_state_test.c keyboard/src/os_detection/usb_auto_state.c
run_suite ble_os_classifier keyboard/tests/ble_os_classifier_test.c keyboard/src/os_detection/ble_os_classifier.c
run_suite ble_os_memory keyboard/tests/ble_os_memory_test.c keyboard/src/os_detection/ble_os_memory.c
run_suite ble_auto_state keyboard/tests/ble_auto_state_test.c keyboard/src/os_detection/ble_auto_state.c keyboard/src/os_detection/usb_auto_state.c
run_suite ble_device_state keyboard/tests/ble_device_state_test.c keyboard/src/os_detection/ble_device_state.c keyboard/src/os_detection/ble_auto_state.c keyboard/src/os_detection/usb_auto_state.c
run_suite host_preference keyboard/tests/host_preference_test.c keyboard/src/connections/host_preference.c
run_suite host_identity keyboard/tests/host_identity_test.c keyboard/src/identity/host_identity.c
run_suite usb_priority keyboard/tests/usb_priority_test.c keyboard/src/connections/usb_priority_state.c
run_suite usb_priority_adapter keyboard/tests/integration/identity/priority_test.c \
    -DCONFIG_SPLITERGOLL_USB_PRIORITY=1 -DCONFIG_ZMK_USB=1 -DCONFIG_ZMK_POINTING=1 \
    -Ikeyboard/tests/integration/identity/stubs keyboard/src/identity/host_identity.c keyboard/src/connections/usb_priority_state.c
run_suite host_identity_adapter keyboard/tests/integration/identity/adapter_test.c \
    -Ikeyboard/tests/integration/identity/stubs keyboard/src/identity/host_identity.c
run_suite host_window_policy keyboard/tests/host_window_policy_test.c keyboard/src/connections/host_window_policy.c
run_suite ble_pairing_state keyboard/tests/ble_pairing_state_test.c keyboard/src/connections/ble_pairing_state.c
run_suite ble_reconnect_state keyboard/tests/ble_reconnect_state_test.c keyboard/src/connections/ble_reconnect_state.c keyboard/src/connections/host_window_policy.c
run_suite usb_trace_store diagnostics/os-signature/tests/usb_trace_store_test.c diagnostics/os-signature/firmware/usb_trace_store.c
run_suite ble_trace_store diagnostics/os-signature/tests/ble_trace_store_test.c diagnostics/os-signature/firmware/ble_trace_store.c keyboard/src/os_detection/ble_att.c
run_suite ble_profile_control diagnostics/os-signature/tests/ble_profile_control_test.c diagnostics/os-signature/firmware/ble_profile_control.c
run_suite power_store diagnostics/power/tests/power_store_test.c diagnostics/power/firmware/power_store.c

# Les vrais adaptateurs utilisent uniquement les doubles du domaine testé.
run_suite ble_selector keyboard/tests/integration/selector/ble_selector_test.c \
    -Ikeyboard/tests/integration/selector/stubs \
    keyboard/src/os_detection/ble_device_state.c keyboard/src/os_detection/ble_auto_state.c \
    keyboard/src/os_detection/usb_auto_state.c keyboard/src/connections/host_window_policy.c \
    keyboard/src/connections/host_preference.c
run_suite usb_priority_selector keyboard/tests/integration/selector/ble_selector_test.c \
    -DCONFIG_SPLITERGOLL_USB_PRIORITY=1 -Ikeyboard/tests/integration/selector/stubs \
    keyboard/src/os_detection/ble_device_state.c keyboard/src/os_detection/ble_auto_state.c \
    keyboard/src/os_detection/usb_auto_state.c keyboard/src/connections/host_window_policy.c \
    keyboard/src/connections/host_preference.c
run_suite radio_session keyboard/tests/integration/radio/radio_session_test.c \
    -Ikeyboard/tests/integration/radio/stubs keyboard/src/connections/ble_reconnect_state.c \
    keyboard/src/connections/ble_pairing_state.c keyboard/src/connections/host_window_policy.c
run_suite pairing keyboard/tests/integration/radio/pairing_test.c \
    -Ikeyboard/tests/integration/radio/stubs keyboard/src/connections/ble_reconnect_state.c \
    keyboard/src/connections/ble_pairing_state.c keyboard/src/connections/host_window_policy.c
run_suite preference keyboard/tests/integration/memory/preference_test.c \
    -Ikeyboard/tests/integration/memory/stubs keyboard/src/connections/host_preference.c
run_suite preference_instrumented keyboard/tests/integration/memory/preference_test.c \
    -DCONFIG_SPLITERGOLL_POWER_TRACE=1 -Ikeyboard/tests/integration/memory/stubs \
    keyboard/src/connections/host_preference.c
run_suite acquisition keyboard/tests/integration/memory/acquisition_test.c \
    -Ikeyboard/tests/integration/memory/stubs keyboard/src/os_detection/ble_os_memory.c \
    keyboard/src/os_detection/ble_os_classifier.c diagnostics/os-signature/firmware/ble_trace_store.c \
    keyboard/src/os_detection/ble_att.c
run_suite power_adapter diagnostics/power/tests/integration/adapter_test.c \
    -Idiagnostics/power/tests/integration/stubs diagnostics/power/firmware/power_store.c
run_suite usb_stack keyboard/tests/integration/usb/usb_stack_test.c \
    -Ikeyboard/tests/integration/usb/stubs keyboard/src/os_detection/usb_os_classifier.c \
    keyboard/src/os_detection/usb_auto_state.c
run_suite ble_observer keyboard/tests/integration/observer/ble_observer_test.c \
    -Ikeyboard/tests/integration/observer/stubs keyboard/src/os_detection/ble_observer.c \
    keyboard/src/os_detection/ble_att.c
run_suite wire_usb_only keyboard/tests/integration/wire/wire_usb_only_test.c \
    -Ikeyboard/tests/integration/wire/stubs \
    -DCONFIG_ZMK_USB=1 -DCONFIG_ZMK_SPLIT_ROLE_CENTRAL=1 -DCONFIG_ZMK_SPLIT_BLE=1 \
    -DCONFIG_BT_EXT_ADV=0 -DCONFIG_LTO=0 -DCONFIG_SPLITERGOLL_BLE_RECONNECT=0
printf '\n%s suites C passent avec ASan/UBSan.\n' "$SUITE_COUNT"
printf 'Résultats : %s\n' "$TEST_DIRECTORY"

# Valide les formats échangés, sans charger les fonctions de capture série.
command -v python3 >/dev/null || { printf '%s\n' 'Prérequis absent : python3'; exit 1; }
protocol_arguments=()
if [ -n "${POWERSHELL-}" ]; then protocol_arguments+=(--powershell "$POWERSHELL"); fi
python3 diagnostics/os-signature/tests/protocol_test.py --output "$TEST_DIRECTORY/protocols" "${protocol_arguments[@]}"
