#!/bin/sh

# Prépare le build local : télécharge l’image Docker et les sources ZMK/Zephyr.
# Appelé par make setup, avec les caches conservés dans .local/.

set -eu

. "$(dirname "$0")/build-common.sh"

preflight
resolve_workspace

if [ -d "$workspace" ]; then
    test -f "$workspace/.splitergoll" ||
        fail "Workspace non géré par Splitergoll : $workspace"
    test "$(cat "$workspace/.splitergoll")" = "$identity" ||
        fail 'Le workspace ne correspond pas au build figé'
fi

mkdir -p "$workspace/config"
printf '%s\n' "$identity" > "$workspace/.splitergoll"
cp "$REPOSITORY/config/west.yml" "$workspace/config/west.yml"

docker pull "$BUILD_IMAGE"
run_container setup
printf '%s\n' 'Environnement prêt. Lance make build'
