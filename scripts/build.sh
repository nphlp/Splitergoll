#!/bin/bash

# Compile les deux moitiés dans Docker et exporte les firmwares UF2 dans dist/.
# Appelé par make build après la préparation, sans flasher le clavier.

set -eu

. "$(dirname "$0")/build-common.sh"
. "$(dirname "$0")/build-logs.sh"

# Exporte une paire complète et ses empreintes seulement après les deux compilations réussies.
export_firmware() {
    timestamp=$(date +%Y-%m-%d-%H-%M-%S)
    destination="$ARTIFACT_ROOT/$PROFILE-$timestamp"
    mkdir -p "$ARTIFACT_ROOT"
    mkdir "$destination" || fail "Export refusé : impossible de créer $destination"

    for side in $SIDES; do
        name="splitergoll-$PROFILE-$side"
        cp "$workspace/build/$name/zephyr/zmk.uf2" "$destination/$name.uf2"
    done

    printf 'profile=%s\nenvironment=%s\nimage=%s\n' "$PROFILE" "$identity" "$BUILD_IMAGE" > "$destination/build-info.txt"
    (cd "$destination" && shasum -a 256 ./*.uf2 > SHA256SUMS)
    printf '%s\n' "$destination" > "$RUN_LOG_DIRECTORY/artifacts-path"
}

start_logging build 4
run_step 'Vérification des prérequis' preflight
resolve_workspace
run_step 'Contrôle du workspace préparé' require_setup
run_step 'Compilation et contrôle des deux moitiés' run_container build --network none
run_step 'Export des firmwares et de leurs empreintes' export_firmware
finish_logging 'Deux firmwares compilés et vérifiés'
printf 'Firmwares : %s\n' "$(cat "$RUN_LOG_DIRECTORY/artifacts-path")"
