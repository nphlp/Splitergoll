#!/bin/bash

# Compile les deux moitiés dans Docker et exporte les firmwares UF2 dans dist/.
# Sans argument, compile tous les profils. Un argument choisit wire ou bluetooth.
# Appelé par make build, sans flasher le clavier.

set -eu

. "$(dirname "$0")/build-common.sh"
. "$(dirname "$0")/build-logs.sh"

[ "$#" -le 1 ] || fail 'Choisis un seul profil : make build wire ou make build bluetooth'
if [ "$#" -eq 1 ]; then
    profiles=$1
    select_profile "$profiles"
else
    profiles=$PROFILES
    for selected_profile in $profiles; do
        select_profile "$selected_profile"
    done
fi

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

# Compile et exporte les deux moitiés du profil reçu, avec son propre journal.
build_profile() {
    select_profile "$1"
    start_logging build 4
    printf 'Profil : %s\n' "$PROFILE"
    run_step 'Vérification des prérequis' preflight
    resolve_workspace
    run_step 'Contrôle du workspace préparé' require_setup
    run_step 'Compilation et contrôle des deux moitiés' run_container build --network none
    run_step 'Export des firmwares et de leurs empreintes' export_firmware
    finish_logging 'Deux firmwares compilés et vérifiés'
    printf 'Firmwares : %s\n' "$(cat "$RUN_LOG_DIRECTORY/artifacts-path")"
}

for selected_profile in $profiles; do
    build_profile "$selected_profile"
done
