#!/bin/bash

# Compile les deux moitiés dans Docker et exporte les firmwares UF2 dans dist/.
# Sans argument, affiche l’aide. Le matériel et la keymap sont obligatoires, dans cet ordre.
# debug ajoute les logs USB, usb-trace et ble-trace étudient la connexion de la centrale.
# identity expose le diagnostic USB vendor et BLE, sans changer la saisie Bluetooth.
# Appelé par make build, sans flasher le clavier.

set -eu

. "$(dirname "$0")/build-common.sh"
. "$(dirname "$0")/build-logs.sh"

if [ "$#" -eq 0 ]; then
    exec bash "$SCRIPT_DIRECTORY/help.sh" build
fi

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    bash "$SCRIPT_DIRECTORY/help.sh" build >&2
    fail 'Précise le matériel puis la keymap : make build <wire|bluetooth> <auto|multi|windows|macos|linux> [debug|usb-trace|ble-trace|identity]'
fi

# Exporte une paire complète et ses empreintes seulement après les deux compilations réussies.
export_firmware() {
    timestamp=$(date +%Y-%m-%d-%H-%M-%S)
    destination="$ARTIFACT_ROOT/$BUILD_VARIANT-$timestamp"
    mkdir -p "$ARTIFACT_ROOT"
    mkdir "$destination" || fail "Export refusé : impossible de créer $destination"

    for side in $SIDES; do
        name="splitergoll-$BUILD_VARIANT-$side"
        cp "$workspace/$BUILD_CACHE_ROOT/$name/zephyr/zmk.uf2" "$destination/$name.uf2"
    done

    printf 'profile=%s\nhardware_profile=%s\nkeymap_profile=%s\nbuild_mode=%s\nvariant=%s\nenvironment=%s\nimage=%s\n' \
        "$PROFILE" "$HARDWARE_PROFILE" "$KEYMAP_PROFILE" "$BUILD_MODE" "$BUILD_VARIANT" "$identity" "$BUILD_IMAGE" > "$destination/build-info.txt"

    (cd "$destination" && shasum -a 256 ./*.uf2 > SHA256SUMS)
    printf '%s\n' "$destination" > "$RUN_LOG_DIRECTORY/artifacts-path"
}

# Compile le matériel et la keymap reçus, en mode normal ou debug, avec leur propre journal.
build_profile() {
    select_profile "$1"
    select_keymap "$2"
    select_build_mode "$3"
    start_logging build 4
    printf 'Profil : %s\n' "$BUILD_VARIANT"
    if [ "$BUILD_MODE" = debug ]; then
        printf '%s\n' 'Diagnostic : logs USB activés sur les deux moitiés'
    elif [ "$BUILD_MODE" = usb-trace ]; then
        if [ "$KEYMAP_PROFILE" = auto ]; then
            printf '%s\n' 'Auto USB : sélection macOS / Windows et trace de contrôle sur la gauche'
        else
            printf '%s\n' 'Diagnostic : signature USB étudiée sur la gauche, hypothèse OS sans changement de keymap'
        fi
    elif [ "$BUILD_MODE" = ble-trace ]; then
        if [ "$KEYMAP_PROFILE" = auto ]; then
            printf '%s\n' 'Auto Bluetooth : OS mémorisé de l’appareil sélectionné et trace de contrôle sur la gauche'
        else
            printf '%s\n' 'Diagnostic : échanges de l’hôte Bluetooth sur la gauche, sans sélection automatique d’OS'
        fi
    elif [ "$BUILD_MODE" = identity ]; then
        printf '%s\n' 'Diagnostic : identité USB vendor et BLE chiffré, saisie toujours en Bluetooth'
    fi
    run_step 'Vérification des prérequis' preflight
    resolve_workspace
    run_step 'Contrôle du workspace préparé' require_setup
    run_step 'Compilation et contrôle des deux moitiés' run_container build --network none
    run_step 'Export des firmwares et de leurs empreintes' export_firmware
    finish_logging 'Deux firmwares compilés et vérifiés'
    printf 'Firmwares : %s\n' "$(cat "$RUN_LOG_DIRECTORY/artifacts-path")"
}

profile=$1
keymap=$2
mode=normal

if [ "$#" -eq 3 ]; then
    case "$3" in
        debug|usb-trace|ble-trace|identity) mode=$3 ;;
        *) fail 'L’argument final doit être debug, usb-trace, ble-trace ou identity' ;;
    esac
fi

build_profile "$profile" "$keymap" "$mode"
