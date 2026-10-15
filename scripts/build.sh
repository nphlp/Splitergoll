#!/bin/bash

# Compile les deux moitiés dans Docker et exporte les firmwares UF2 dans dist/.
# Sans argument, affiche les profils disponibles. Un argument choisit le profil à compiler.
# L’argument debug ajoute les logs USB au profil choisi.
# Appelé par make build, sans flasher le clavier.

set -eu

. "$(dirname "$0")/build-common.sh"
. "$(dirname "$0")/build-logs.sh"

[ "$#" -le 2 ] || fail 'Utilise make build wire ou bluetooth, avec debug en option'
if [ "$#" -eq 0 ]; then
    exec bash "$SCRIPT_DIRECTORY/help.sh" build
fi
if [ "$#" -eq 2 ] && [ "$2" != debug ]; then
    fail 'Le seul second argument accepté est debug'
fi

# Exporte une paire complète et ses empreintes seulement après les deux compilations réussies.
export_firmware() {
    timestamp=$(date +%Y-%m-%d-%H-%M-%S)
    destination="$ARTIFACT_ROOT/$BUILD_VARIANT-$timestamp"
    mkdir -p "$ARTIFACT_ROOT"
    mkdir "$destination" || fail "Export refusé : impossible de créer $destination"

    for side in $SIDES; do
        name="splitergoll-$BUILD_VARIANT-$side"
        cp "$workspace/build/$name/zephyr/zmk.uf2" "$destination/$name.uf2"
    done

    printf 'profile=%s\nhardware_profile=%s\nbuild_mode=%s\nvariant=%s\nenvironment=%s\nimage=%s\n' \
        "$PROFILE" "$HARDWARE_PROFILE" "$BUILD_MODE" "$BUILD_VARIANT" "$identity" "$BUILD_IMAGE" > "$destination/build-info.txt"

    (cd "$destination" && shasum -a 256 ./*.uf2 > SHA256SUMS)
    printf '%s\n' "$destination" > "$RUN_LOG_DIRECTORY/artifacts-path"
}

# Compile et exporte le profil reçu en mode normal ou debug, avec son propre journal.
build_profile() {
    select_profile "$1"
    select_build_mode "$2"
    start_logging build 4
    printf 'Profil : %s\n' "$BUILD_VARIANT"
    if [ "$BUILD_MODE" = debug ]; then
        printf '%s\n' 'Diagnostic : logs USB activés sur les deux moitiés'
    fi
    run_step 'Vérification des prérequis' preflight
    resolve_workspace
    run_step 'Contrôle du workspace préparé' require_setup
    run_step 'Compilation et contrôle des deux moitiés' run_container build --network none
    run_step 'Export des firmwares et de leurs empreintes' export_firmware
    finish_logging 'Deux firmwares compilés et vérifiés'
    printf 'Firmwares : %s\n' "$(cat "$RUN_LOG_DIRECTORY/artifacts-path")"
}

build_profile "$1" "${2-normal}"
