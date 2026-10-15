#!/bin/bash

# Prépare le build local : télécharge l’image Docker et les sources ZMK/Zephyr.
# Appelé par make setup, avec les caches conservés dans .local/.

set -eu

. "$(dirname "$0")/build-common.sh"
. "$(dirname "$0")/build-logs.sh"

[ "$#" -eq 0 ] || fail 'Utilise make setup sans argument'

# Prépare le workspace identifié par les versions sans écraser un dossier non géré.
prepare_workspace() {
    if [ -d "$workspace" ]; then
        test -f "$workspace/.splitergoll" ||
            fail "Workspace non géré par Splitergoll : $workspace"
        test "$(cat "$workspace/.splitergoll")" = "$identity" ||
            fail 'Le workspace ne correspond pas au build figé'
    fi

    mkdir -p "$workspace/config"
    printf '%s\n' "$identity" > "$workspace/.splitergoll"
    cp "$REPOSITORY/config/west.yml" "$workspace/config/west.yml"
}

start_logging setup 4
run_step 'Vérification des prérequis' preflight
resolve_workspace
run_step 'Préparation du workspace local' prepare_workspace
run_step 'Téléchargement de l’image Docker' docker pull "$BUILD_IMAGE"
run_step 'Téléchargement et contrôle des sources ZMK/Zephyr' run_container setup
finish_logging 'Environnement prêt'
printf 'Suite : %smake build%s\n' "$LOG_HEADING" "$LOG_RESET"
