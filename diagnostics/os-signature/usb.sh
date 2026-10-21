#!/bin/bash

# Regroupe les commandes de diagnostic USB. Sans action valide, affiche l’aide avant tout accès série.
# make usb capture <port> lit la centrale, sans flash ni changement de firmware.

set -euo pipefail

SCRIPT_DIRECTORY=$(CDPATH= cd "$(dirname "$0")" && pwd)
HELP_SCRIPT="$SCRIPT_DIRECTORY/../../scripts/help.sh"

# Affiche les commandes USB et termine sans ouvrir de port ni créer de capture.
usage() {
    bash "$HELP_SCRIPT" usb
    exit "${1:-0}"
}

[ "$#" -gt 0 ] || usage
case "$1" in
    help) [ "$#" -eq 1 ] && usage; usage 1 ;;
    capture)
        [ "$#" -eq 2 ] || usage 1
        case "$2" in
            /dev/cu.usbmodem*|/dev/ttyACM*) ;;
            *) usage 1 ;;
        esac
        shift
        exec bash "$SCRIPT_DIRECTORY/capture.sh" "$@"
        ;;
    *) usage 1 ;;
esac
