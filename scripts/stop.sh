#!/bin/sh

# Arrête les conteneurs de build de ce checkout sans supprimer ses fichiers.
# Appelé par make stop, notamment si un setup ou un build reste actif.

set -eu

. "$(dirname "$0")/build-common.sh"

require_docker
stop_containers
