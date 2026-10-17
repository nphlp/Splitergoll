#!/bin/bash

# Lit les requêtes mémorisées par la centrale usb-trace, sans flash ni reset.
# Reçoit un port série explicite et conserve la capture dans .local/usb-traces/.
# macOS/Linux uniquement. Sous Windows, copier capture.ps1 dans le dossier de travail.
set -euo pipefail

DIAGNOSTIC_DIRECTORY=$(CDPATH= cd "$(dirname "$0")" && pwd)
REPOSITORY=$(CDPATH= cd "$DIAGNOSTIC_DIRECTORY/../.." && pwd)
TRACE_ROOT="$REPOSITORY/.local/usb-traces"

# Affiche une erreur et interrompt la lecture sans modifier le clavier.
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

if [ "$#" -eq 0 ]; then
    printf '%s\n' 'Utilise make usb-capture <port-série-gauche>'
    printf '%s\n' 'Exemple : make usb-capture /dev/cu.usbmodem123'
    exit 0
fi
[ "$#" -eq 1 ] || fail 'Un seul port série explicite est attendu'
port=$1
case "$(uname -s)" in
    Darwin) host=macos ;;
    Linux) host=linux ;;
    *) fail 'Sous Windows, copie capture.ps1 dans ton dossier de travail et lance-le avec PowerShell (voir docs/detection-os.md)' ;;
esac
case "$port" in
    /dev/cu.usbmodem*|/dev/ttyACM*) ;;
    *) fail 'Port attendu : /dev/cu.usbmodem… ou /dev/ttyACM…' ;;
esac
[ -c "$port" ] && [ ! -L "$port" ] || fail 'Le port série est absent ou incorrect'
command -v lsof >/dev/null || fail 'Prérequis absent : lsof, nécessaire pour vérifier que le port est libre'
if lsof -t "$port" >/dev/null 2>&1; then
    fail 'Ce port est déjà ouvert : ferme son lecteur avant de continuer'
fi

# Restaure les réglages série et ferme uniquement le descripteur ouvert par ce script.
close_port() {
    if [ -n "${serial_settings-}" ]; then
        stty "$serial_settings" <&3 2>/dev/null || true
    fi
    exec 3>&-
}

mkdir -p "$TRACE_ROOT"
capture=$(mktemp "$TRACE_ROOT/$host-$(date -u +%Y-%m-%d-%H-%M-%S)-XXXXXX")
printf '# host=%s collected_at_utc=%s\n' "$host" "$(date -u +%Y-%m-%dT%H:%M:%SZ)" > "$capture"
exec 3<> "$port"
serial_settings=$(stty -g <&3)
trap close_port EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
stty raw -echo 115200 <&3
printf 'Lecture de la centrale sur %s…\n' "$port"
printf '%s\n' 'Les requêtes du démarrage sont conservées, aucune frappe à effectuer.'
sleep 1
printf D >&3
deadline=$((SECONDS + 15))
finished=false
while [ "$SECONDS" -lt "$deadline" ]; do
    if IFS= read -r -t 1 line <&3; then
        line=${line%$'\r'}
        printf '%s\n' "$line" >> "$capture"
        if [ "$line" = 'USBTRACE END' ]; then
            finished=true
            break
        fi
    else
        sleep 0.05
    fi
done

[ "$finished" = true ] || fail "Capture incomplète ou mauvais firmware/port. Données conservées : $capture"
awk -f "$DIAGNOSTIC_DIRECTORY/check.awk" "$capture" ||
    fail "Capture refusée (pertes ou format incorrect). Données conservées : $capture"
printf 'Capture complète : %s\n' "$capture"

result=$(awk '/^USBTRACE OS / { sub(/^result=/, "", $3); print $3 }' "$capture")
case "$result" in
    macos) printf '%s\n' 'Signature USB : hypothèse macOS' ;;
    windows) printf '%s\n' 'Signature USB : hypothèse Windows' ;;
    ambiguous) printf '%s\n' 'Signature USB : indices contradictoires, résultat ambigu' ;;
    unknown) printf '%s\n' 'Signature USB : indices insuffisants, OS inconnu' ;;
    '') printf '%s\n' 'Classification absente : ancien firmware de capture' ;;
esac
active=$(awk '/^USBTRACE ACTIVE / { sub(/^result=/, "", $3); print $3 }' "$capture")
case "$active" in
    macos) printf '%s\n' 'Keymap automatique appliquée : macOS' ;;
    windows) printf '%s\n' 'Keymap automatique appliquée : Windows + gaming' ;;
    unknown) printf '%s\n' 'Keymap automatique en attente, saisie bloquée tant que l’OS reste inconnu.' ;;
    '') printf '%s\n' 'La keymap reste inchangée. Ce diagnostic ne sélectionne aucun OS.' ;;
esac
