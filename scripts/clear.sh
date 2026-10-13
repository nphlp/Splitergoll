#!/bin/sh

# Arrête le build et supprime les sources, caches et firmwares locaux de toutes les versions.
# Appelé par make clear. Il faudra relancer make setup avant le prochain build.

set -eu

. "$(dirname "$0")/build-common.sh"

test -f "$REPOSITORY/Makefile" && test -f "$REPOSITORY/config/west.yml" ||
    fail 'Nettoyage refusé : racine du dépôt non reconnue'
test "$LOCAL_ROOT" = "$REPOSITORY/.local" && test "$OUTPUT_ROOT" = "$REPOSITORY/dist" ||
    fail 'Nettoyage refusé : chemins inattendus'

for directory in "$LOCAL_ROOT" "$OUTPUT_ROOT"; do
    test ! -L "$directory" || fail "Nettoyage refusé : lien symbolique $directory"
done

require_docker
stop_containers
require_idle_checkout

printf 'Suppression de %s et %s\n' "$LOCAL_ROOT" "$OUTPUT_ROOT"
rm -rf -- "$LOCAL_ROOT" "$OUTPUT_ROOT"
printf '%s\n' 'Fichiers locaux supprimés. Relance make setup avant make build'
