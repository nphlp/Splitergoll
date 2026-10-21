#!/bin/bash

# Regroupe capture, mémoire, sélection et effacement Bluetooth sous make ble.
# Valide l’action et ses arguments avant tout accès série. L’effacement conserve sa confirmation dédiée.

set -euo pipefail

SCRIPT_DIRECTORY=$(CDPATH= cd "$(dirname "$0")" && pwd)
HELP_SCRIPT="$SCRIPT_DIRECTORY/../../scripts/help.sh"

# Affiche les commandes Bluetooth et termine sans toucher aux connexions ou aux appairages.
usage() {
    bash "$HELP_SCRIPT" ble
    exit "${1:-0}"
}

[ "$#" -gt 0 ] || usage
case "$1" in
    help) [ "$#" -eq 1 ] && usage; usage 1 ;;
    capture|memory)
        [ "$#" -eq 2 ] || usage 1
        if [ "$1" = capture ]; then option=--ble; else option=--memory; fi
        ;;
    profiles)
        [ "$#" -eq 2 ] || usage 1
        option=--profile
        set -- "$1" "$2" status
        ;;
    select)
        [ "$#" -eq 3 ] || usage 1
        case "$3" in [0-9]) ;; *) usage 1 ;; esac
        option=--profile
        ;;
    forget)
        [ "$#" -eq 3 ] || usage 1
        case "$3" in [0-9]) ;; *) usage 1 ;; esac
        option=--forget
        ;;
    *) usage 1 ;;
esac
case "$2" in
    /dev/cu.usbmodem*|/dev/ttyACM*) ;;
    *) usage 1 ;;
esac
shift
exec bash "$SCRIPT_DIRECTORY/capture.sh" "$option" "$@"
