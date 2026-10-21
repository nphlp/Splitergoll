#!/bin/sh

# Exécute les commandes west dans le conteneur et contrôle les dépendances.
# Reçoit setup ou build, le profil matériel, la keymap et le mode de diagnostic.
# Vérifie les firmwares compilés avant leur export.

set -eu

. "$(dirname "$0")/build-common.sh"
cd "$CONTAINER_WORKSPACE"

# Vérifie que chaque dépendance est au commit prévu et ne contient aucune modification suivie.
check_dependencies() {
    west list -f '{name} {revision} {path}' > .west/projects.lock

    while read -r name revision path; do
        [ "$name" != manifest ] || continue
        printf 'Contrôle de la dépendance : %s\n' "$name"
        current=$(git -C "$path" rev-parse HEAD)
        changes=$(git -C "$path" status --porcelain --untracked-files=no)

        if [ "$current" != "$revision" ] || [ -n "$changes" ]; then
            fail "Dépendance incorrecte ou modifiée : $name"
        fi
        printf '[OK] %s au commit prévu\n' "$name"
    done < .west/projects.lock
}

# Contrôle le bootloader, les rôles, la sortie et l’alimentation, puis la présence de l’UF2.
check_firmware() {
    side=$1
    directory=$2
    configuration="$directory/zephyr/.config"

    grep -Fxq "KEYMAP_FILE:STRING=$KEYMAP_FILE" "$directory/CMakeCache.txt" ||
        fail 'La keymap compilée ne correspond pas à celle demandée'

    for option in \
        CONFIG_ZMK_BOARD_COMPAT=y \
        CONFIG_RETENTION_BOOT_MODE=y \
        CONFIG_ZMK_BOOTMODE_TO_MAGIC_VALUE_MAPPER=y \
        CONFIG_ZMK_BOOTMODE_BOOTLOADER_MAGIC_VALUE=0x57 \
        CONFIG_FLASH_LOAD_OFFSET=0x26000 \
        CONFIG_FLASH_LOAD_SIZE=0xc6000; do
        grep -Fxq "$option" "$configuration" || fail "Configuration nice!nano ou bootloader incorrecte : $option"
    done

    grep -qx 'CONFIG_ZMK_SPLIT=y' "$configuration" || fail 'Split désactivé'
    grep -qx 'CONFIG_ZMK_BLE=y' "$configuration" || fail 'BLE interne désactivé'
    if grep -Eq '^(CONFIG_ZMK_SETTINGS_RESET_ON_START|CONFIG_ZMK_BLE_CLEAR_BONDS_ON_START)=y$' "$configuration"; then
        fail 'Firmware de clavier incorrect : effacement des appairages activé'
    fi

    if [ "$BUILD_MODE" = debug ]; then
        grep -qx 'CONFIG_ZMK_USB_LOGGING=y' "$configuration" || fail 'Logs USB de diagnostic absents'
        grep -qx 'CONFIG_LOG=y' "$configuration" || fail 'Système de logs désactivé'
        grep -qx 'CONFIG_UART_CONSOLE=y' "$configuration" || fail 'Console UART désactivée'
        grep -qx 'CONFIG_USB_CDC_ACM=y' "$configuration" || fail 'Port série USB absent'
        grep -q 'snippet_zmk_usb_logging_uart' "$directory/zephyr/zephyr.dts" || fail 'Console USB de diagnostic absente du matériel compilé'
    elif grep -qx 'CONFIG_ZMK_USB_LOGGING=y' "$configuration"; then
        fail 'Logs USB activés dans un firmware normal'
    fi

    if [ "$BUILD_MODE" = usb-trace ] && [ "$side" = left ]; then
        grep -qx 'CONFIG_SPLITERGOLL_USB_TRACE=y' "$configuration" || fail 'Trace USB absente de la centrale'
        grep -qx 'CONFIG_USB_CDC_ACM=y' "$configuration" || fail 'Port de lecture USB absent'
        grep -q 'splitergoll_usb_trace_uart' "$directory/zephyr/zephyr.dts" || fail 'Port dédié absent du matériel'
        grep -Eq '0x[0-9a-f]+[[:space:]]+__wrap_usb_dc_ep_read$' "$directory/zephyr/zmk.map" || fail 'Observation des lectures USB absente du firmware'
        grep -Eq '0x[0-9a-f]+[[:space:]]+__wrap_usb_dc_ep_set_callback$' "$directory/zephyr/zmk.map" || fail 'Observation des SETUP absente du firmware'
        grep -Eq '0x[0-9a-f]+[[:space:]]+usb_os_result$' "$directory/zephyr/zmk.map" || fail 'Classifieur USB absent du diagnostic'
        grep -Eq '0x[0-9a-f]+[[:space:]]+__wrap_usb_dc_set_status_callback$' "$directory/zephyr/zmk.map" || fail 'Séparation des cycles USB absente du diagnostic'
    elif grep -qx 'CONFIG_SPLITERGOLL_USB_TRACE=y' "$configuration"; then
        fail 'Diagnostic USB présent hors de la centrale usb-trace'
    fi

    if [ "$BUILD_MODE" = ble-trace ] && [ "$side" = left ]; then
        grep -qx 'CONFIG_SPLITERGOLL_BLE_TRACE=y' "$configuration" || fail 'Trace BLE absente de la centrale'
        grep -qx 'CONFIG_SPLITERGOLL_BLE_MEMORY=y' "$configuration" || fail 'Mémoire OS BLE absente du diagnostic'
        grep -qx 'CONFIG_USB_CDC_ACM=y' "$configuration" || fail 'Port de lecture BLE absent'
        grep -qx 'CONFIG_BT_REMOTE_INFO=y' "$configuration" || fail 'Informations BLE distantes absentes'
        grep -q 'splitergoll_ble_trace_uart' "$directory/zephyr/zephyr.dts" || fail 'Port dédié BLE absent du matériel'
        grep -Eq '0x[0-9a-f]+[[:space:]]+__wrap_bt_l2cap_recv$' "$directory/zephyr/zmk.map" || fail 'Observation ATT absente'
        for symbol in ble_os_observe ble_os_result __wrap_zmk_event_manager_raise ble_os_settings_bind ble_memory_assign; do
            grep -Eq "0x[0-9a-f]+[[:space:]]+$symbol\$" "$directory/zephyr/zmk.map" || fail "Composant BLE absent du diagnostic : $symbol"
        done
        if grep -qx 'CONFIG_BT_REMOTE_VERSION=y' "$configuration"; then
            fail 'Le diagnostic BLE ne doit pas demander la version du contrôleur distant'
        fi
    elif grep -qx 'CONFIG_SPLITERGOLL_BLE_TRACE=y' "$configuration"; then
        fail 'Diagnostic BLE présent hors de la centrale ble-trace'
    fi
    if [ "$side" = left ] && { [ "$BUILD_MODE" = ble-trace ] || { [ "$PROFILE" = bluetooth ] && [ "$KEYMAP_PROFILE" = auto ]; }; }; then
        grep -qx 'CONFIG_SPLITERGOLL_BLE_MEMORY=y' "$configuration" || fail 'Mémoire OS BLE absente'
        grep -qx 'CONFIG_SPLITERGOLL_BLE_OBSERVER=y' "$configuration" || fail 'Observateur BLE absent'
    elif grep -Eq '^CONFIG_SPLITERGOLL_BLE_(MEMORY|OBSERVER)=y$' "$configuration"; then
        fail 'Mémoire ou observation BLE active hors du profil demandé'
    fi

    if [ "$KEYMAP_PROFILE" = auto ] && [ "$side" = left ]; then
        grep -qx 'CONFIG_SPLITERGOLL_OS_AUTO=y' "$configuration" || fail 'Sélection automatique absente'
        for symbol in __wrap_zmk_event_manager_raise __wrap_zmk_keymap_layer_to; do
            grep -Eq "0x[0-9a-f]+[[:space:]]+$symbol$" "$directory/zephyr/zmk.map" ||
                fail "Garde de sélection automatique absente : $symbol"
        done
    elif grep -qx 'CONFIG_SPLITERGOLL_OS_AUTO=y' "$configuration"; then
        fail 'Sélection automatique activée hors du profil auto central'
    fi

    if [ "$side" = left ] && { [ "$BUILD_MODE" = usb-trace ] || { [ "$PROFILE" = wire ] && [ "$KEYMAP_PROFILE" = auto ]; }; }; then
        grep -qx 'CONFIG_SPLITERGOLL_USB_OBSERVER=y' "$configuration" || fail 'Observateur USB absent'
    elif grep -qx 'CONFIG_SPLITERGOLL_USB_OBSERVER=y' "$configuration"; then
        fail 'Observation USB présente dans un firmware qui ne la demande pas'
    fi

    if [ "$PROFILE" = bluetooth ] && [ "$KEYMAP_PROFILE" = auto ] && [ "$side" = left ]; then
        grep -qx 'CONFIG_SPLITERGOLL_BLE_AUTO=y' "$configuration" || fail 'Sélecteur de keymap BLE absent'
        grep -qx 'CONFIG_SPLITERGOLL_BLE_RECONNECT=y' "$configuration" || fail 'Fenêtre de reconnexion BLE absente'
        grep -qx 'CONFIG_BT_FILTER_ACCEPT_LIST=y' "$configuration" || fail 'Filtre des hôtes connus absent'
        grep -qx 'CONFIG_BT_SMP_APP_PAIRING_ACCEPT=y' "$configuration" || fail 'Contrôle des nouveaux appairages absent'
        for symbol in splitergoll_ble_acquisition_snapshot ble_auto_can_select __wrap_bt_l2cap_recv splitergoll_ble_auto_next_device; do
            grep -Eq "0x[0-9a-f]+[[:space:]]+$symbol$" "$directory/zephyr/zmk.map" || fail "Composant auto BLE absent : $symbol"
        done
        for symbol in __wrap_bt_le_adv_start __wrap_bt_le_adv_stop __wrap_bt_conn_auth_cb_register ble_reconnect_apply; do
            grep -Eq "0x[0-9a-f]+[[:space:]]+$symbol$" "$directory/zephyr/zmk.map" || fail "Contrôle de reconnexion absent : $symbol"
        done
        grep -q 'device-modifier-position = < 0x28 >' "$directory/zephyr/zephyr.dts" || fail 'Position L1 du raccourci absente'
        grep -q 'device-next-position = < 0x2b >' "$directory/zephyr/zephyr.dts" || fail 'Position encodeur du raccourci absente'
    elif grep -qx 'CONFIG_SPLITERGOLL_BLE_AUTO=y' "$configuration"; then
        fail 'Sélecteur BLE présent hors du profil auto bluetooth central'
    elif grep -qx 'CONFIG_SPLITERGOLL_BLE_RECONNECT=y' "$configuration"; then
        fail 'Reconnexion connue présente hors du profil auto bluetooth central'
    fi

    if [ "$side" = left ]; then
        grep -qx 'CONFIG_ZMK_SPLIT_ROLE_CENTRAL=y' "$configuration" || fail 'Gauche non centrale'
    elif grep -Eq '^(CONFIG_ZMK_SPLIT_ROLE_CENTRAL|CONFIG_ZMK_USB)=y$' "$configuration"; then
        fail 'Rôle incorrect pour la droite'
    fi

    case "$PROFILE" in
        wire)
            if [ "$side" = left ]; then
                grep -qx 'CONFIG_ZMK_USB=y' "$configuration" || fail 'Sortie USB gauche absente'
            fi
            if grep -Eq '^(CONFIG_ZMK_SLEEP|CONFIG_ZMK_BATTERY_REPORTING)=y$' "$configuration"; then
                fail 'Veille ou reporting batterie activé dans le profil wire'
            fi
            if grep -Eq '^CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_(FETCHING|PROXY)=y$' "$configuration"; then
                fail 'Remontée des batteries activée dans le profil wire'
            fi
            ;;
        bluetooth)
            if grep -qx 'CONFIG_ZMK_USB=y' "$configuration"; then
                fail 'Sortie clavier USB activée dans le profil bluetooth'
            fi
            grep -qx 'CONFIG_ZMK_SLEEP=y' "$configuration" || fail 'Veille Bluetooth désactivée'
            grep -qx 'CONFIG_ZMK_IDLE_TIMEOUT=30000' "$configuration" || fail 'Délai d’inactivité normal incorrect'
            grep -qx 'CONFIG_ZMK_IDLE_SLEEP_TIMEOUT=900000' "$configuration" || fail 'Délai de veille normal incorrect'
            grep -qx 'CONFIG_ZMK_BATTERY_REPORT_INTERVAL=60' "$configuration" || fail 'Intervalle batterie normal incorrect'
            grep -qx 'CONFIG_ZMK_BATTERY_REPORTING=y' "$configuration" || fail 'Reporting batterie désactivé'
            grep -qx 'CONFIG_PM_DEVICE=y' "$configuration" || fail 'Gestion d’alimentation désactivée'
            sed -n '/kscan0: kscan {/,/};/p' "$directory/zephyr/zephyr.dts" |
                grep -q 'wakeup-source;' || fail 'Matrice non déclarée comme source de réveil'
            if [ "$side" = left ]; then
                grep -qx 'CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y' "$configuration" ||
                    fail 'Récupération de la batterie droite désactivée'
                grep -qx 'CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_PROXY=y' "$configuration" ||
                    fail 'Transmission de la batterie droite désactivée'
            elif grep -Eq '^CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_(FETCHING|PROXY)=y$' "$configuration"; then
                fail 'Récupération ou proxy batterie activé sur la droite'
            fi
            ;;
    esac
    test -s "$directory/zephyr/zmk.uf2" || fail "UF2 absent : $side"
}

case "${1-}" in
    setup)
        if [ ! -d .west ]; then west init -l config; fi
        west update --narrow --fetch-opt=--filter=blob:none --fetch-opt=--progress
        check_dependencies
        ;;
    build)
        select_profile "${2-}"
        select_keymap "${3-}"
        select_build_mode "${4-normal}"
        check_dependencies
        west zephyr-export

        for side in $SIDES; do
            select_side_configuration "$side"
            # Un cache neuf évite l’ancien ZMK_CONFIG mémorisé par CMake, sans le supprimer.
            directory="$BUILD_CACHE_ROOT/splitergoll-$BUILD_VARIANT-$side"
            modules="$KEYBOARD_ROOT"
            # Le raccourci ne change que la centrale bluetooth auto, pas les profils fixes ou wire.
            dts_cppflags=''
            if [ "$PROFILE" = bluetooth ] && [ "$KEYMAP_PROFILE" = auto ] && [ "$side" = left ]; then
                dts_cppflags='-DSPLITERGOLL_BLE_SHORTCUTS'
            fi
            set --
            if [ "$BUILD_MODE" = debug ]; then
                set -- -S zmk-usb-logging
            elif [ "$BUILD_MODE" = usb-trace ] && [ "$side" = left ]; then
                set -- -S splitergoll-usb-trace
                modules="$modules;$OS_DIAGNOSTIC_ROOT/firmware"
            elif [ "$BUILD_MODE" = ble-trace ] && [ "$side" = left ]; then
                set -- -S splitergoll-ble-trace
                modules="$modules;$OS_DIAGNOSTIC_ROOT/firmware"
            fi
            west build -p auto -s zmk/app -d "$directory" -b "$BOARD" "$@" -- \
                -DSHIELD="splitergoll_$side" \
                -DZMK_CONFIG="$KEYBOARD_CONFIGURATION" \
                -DZMK_EXTRA_MODULES="$modules" \
                -DSNIPPET_ROOT="$OS_DIAGNOSTIC_ROOT" \
                -DKEYMAP_FILE:STRING="$KEYMAP_FILE" \
                -DDTS_EXTRA_CPPFLAGS:STRING="$dts_cppflags" \
                -DEXTRA_CONF_FILE="$FIRMWARE_CONFIGURATION"
            check_firmware "$side" "$directory"
        done
        ;;
    *) fail 'Action attendue : setup ou build' ;;
esac
