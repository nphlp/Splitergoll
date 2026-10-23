#!/bin/bash

# Exécute le vrai noyau Zephyr et activity.c dans QEMU ARM, sans accès au clavier.
# Réutilise uniquement l’image et les sources figées préparées par make setup.
set -euo pipefail
. "$(dirname "$0")/build-common.sh"
select_profile bluetooth
select_keymap auto
select_build_mode ble-trace
preflight
resolve_workspace
require_setup
mkdir -p "$LOCAL_ROOT/tests"
TEST_DIRECTORY=$(mktemp -d "$LOCAL_ROOT/tests/zephyr-XXXXXX")
TEST_CACHE="build/tests/$(basename "$TEST_DIRECTORY")"
printf 'Tests Zephyr sous QEMU. Logs : %s\n' "$TEST_DIRECTORY"

docker run --rm --init --network none \
    --label "splitergoll.build.repository=$REPOSITORY" \
    --mount "type=bind,source=$REPOSITORY,target=$CONTAINER_SOURCE,readonly" \
    --mount "type=bind,source=$workspace,target=$CONTAINER_WORKSPACE" \
    --workdir "$CONTAINER_WORKSPACE" --entrypoint /bin/sh "$BUILD_IMAGE" \
    -c 'set -eu
        west zephyr-export
        west build -p always -b qemu_cortex_m3 -s "$1/keyboard/tests/zephyr" -d "$2" -- \
            -DZMK_APP_SOURCE="$3/zmk/app"
        python3 "$1/scripts/test-qemu.py" --timeout 30 -- west build -d "$2" -t run' \
    test-zephyr "$CONTAINER_SOURCE" "$TEST_CACHE" "$CONTAINER_WORKSPACE" \
    2>&1 | tee "$TEST_DIRECTORY/qemu.log"
