#!/bin/bash

# Oriente les commandes vers l’app Mac actuelle ou le build Windows explicite.
# Chaque plateforme conserve son script, sans démarrage implicite de l’autre app.
set -euo pipefail
COMPANION_SOURCE=$(CDPATH= cd "$(dirname "$0")" && pwd)
if [ "$#" -eq 2 ] && [ "$2" = windows ]; then
    exec bash "$COMPANION_SOURCE/windows/app.sh" "$1"
fi
exec bash "$COMPANION_SOURCE/macos/app.sh" "$@"
