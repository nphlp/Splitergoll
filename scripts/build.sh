#!/bin/sh

# Compile les deux moitiés dans Docker et exporte les firmwares UF2 dans dist/.
# Appelé par make build après la préparation, sans flasher le clavier.

set -eu

. "$(dirname "$0")/build-common.sh"

preflight
resolve_workspace
require_setup
run_container build --network none

# Exporter une paire complète seulement après la réussite des deux compilations.
artifacts="$ARTIFACT_ROOT/$PROFILE"
mkdir -p "$artifacts"
destination=$(mktemp -d "$artifacts/$(date -u +%Y%m%dT%H%M%SZ)-XXXXXX")

for side in $SIDES; do
    name="splitergoll-$PROFILE-$side"
    cp "$workspace/build/$name/zephyr/zmk.uf2" "$destination/$name.uf2"
done

printf 'environment=%s\nimage=%s\n' "$identity" "$BUILD_IMAGE" > "$destination/build-info.txt"
(cd "$destination" && shasum -a 256 ./*.uf2 > SHA256SUMS)
printf '%s\n' "Deux firmwares compilés et vérifiés : $destination"
