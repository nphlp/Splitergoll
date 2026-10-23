#!/bin/bash

# Lit l’historique de la centrale usb-trace ou ble-trace, sans flash ni reset.
# --ble lit les traces, --profile contrôle les emplacements, --memory lit la mémoire d’OS.
# macOS/Linux uniquement. Sous Windows, copier capture.ps1 dans le dossier de travail.
set -euo pipefail

DIAGNOSTIC_DIRECTORY=$(CDPATH= cd "$(dirname "$0")" && pwd)
REPOSITORY=$(CDPATH= cd "$DIAGNOSTIC_DIRECTORY/../.." && pwd)
TRACE_ROOT="$REPOSITORY/.local/usb-traces"
protocol=USBTRACE
command_name='usb capture'
validator="$DIAGNOSTIC_DIRECTORY/check.awk"
serial_command=D
if [ "${1-}" = --power ]; then
    shift
    TRACE_ROOT="$REPOSITORY/.local/power-traces"
    protocol=POWERTRACE
    command_name='ble power'
    validator="$DIAGNOSTIC_DIRECTORY/../power/check.awk"
    serial_command=P
elif [ "${1-}" = --ble ]; then
    shift
    TRACE_ROOT="$REPOSITORY/.local/ble-traces"
    protocol=BLETRACE
    command_name='ble capture'
    validator="$DIAGNOSTIC_DIRECTORY/check-ble.awk"
elif [ "${1-}" = --profile ]; then
    shift
    TRACE_ROOT="$REPOSITORY/.local/ble-profiles"
    protocol=BLEPROFILE
    command_name='ble profiles'
    validator="$DIAGNOSTIC_DIRECTORY/check-profile.awk"
elif [ "${1-}" = --memory ]; then
    shift
    TRACE_ROOT="$REPOSITORY/.local/ble-memory"
    protocol=BLEMEM
    command_name='ble memory'
    validator="$DIAGNOSTIC_DIRECTORY/check-memory.awk"
    serial_command=M
elif [ "${1-}" = --forget ]; then
    shift
    TRACE_ROOT="$REPOSITORY/.local/ble-profiles"
    protocol=BLEFORGET
    command_name='ble forget'
    validator="$DIAGNOSTIC_DIRECTORY/check-forget.awk"
fi

# Affiche une erreur et interrompt la lecture sans modifier le clavier.
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

if [ "$#" -eq 0 ]; then
    if [ "$protocol" = BLEFORGET ]; then
        printf '%s\n' 'Utilise make ble forget <port-série-gauche> <index>. Effacement ciblé après confirmation.'
        exit 0
    fi
    if [ "$protocol" = BLEPROFILE ]; then
        printf '%s\n' 'Utilise make ble profiles <port-série-gauche> ou make ble select <port-série-gauche> <index>'
        exit 0
    fi
    printf 'Utilise make %s <port-série-gauche>\n' "$command_name"
    printf 'Exemple : make %s /dev/cu.usbmodem123\n' "$command_name"
    exit 0
fi
if [ "$protocol" = BLEFORGET ]; then
    [ "$#" -eq 2 ] || fail 'Utilise make ble forget <port-série-gauche> <index>'
    case "$2" in
        [0-9]) serial_command="!CLEAR $2" ;;
        *) fail 'Index attendu : de 0 à 9, sans sélection implicite' ;;
    esac
elif [ "$protocol" = BLEPROFILE ]; then
    [ "$#" -eq 2 ] || fail 'Utilise make ble profiles <port-série-gauche> ou make ble select <port-série-gauche> <index>'
    case "$2" in
        status) serial_command=S ;;
        [0-9]) serial_command=$2 ;;
        *) fail 'Utilise make ble profiles <port> pour lire ou make ble select <port> <index> pour sélectionner' ;;
    esac
else
    [ "$#" -eq 1 ] || fail 'Un seul port série explicite est attendu'
fi
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
if [ "$protocol" = BLEFORGET ]; then
    [ -t 0 ] || fail 'Effacement refusé sans terminal interactif'
    printf 'Effacer uniquement l’appairage Bluetooth du slot %s et son OS mémorisé ?\n' "$2"
    printf '%s\n' 'Le firmware refusera si ce slot n’est pas celui actuellement sélectionné.'
    printf 'Continue? [y/N] '
    answer=''
    IFS= read -r -s -n 1 answer || true
    printf '\n'
    case "$answer" in
        y|Y) ;;
        *) printf '%s\n' 'Annulé.'; exit 0 ;;
    esac
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
if [ "$protocol" = BLEPROFILE ]; then
    printf '%s\n' 'Contrôle explicite des emplacements Bluetooth, sans effacer les appairages.'
elif [ "$protocol" = BLEFORGET ]; then
    printf 'Effacement ciblé du slot %s, sans toucher aux autres appairages.\n' "$2"
elif [ "$protocol" = BLETRACE ]; then
    printf '%s\n' 'Les connexions Bluetooth depuis le démarrage sont conservées. Aucune frappe à effectuer.'
elif [ "$protocol" = BLEMEM ]; then
    printf '%s\n' 'Lecture de la mémoire d’OS. Aucun changement de keymap, de sélection ou d’appairage.'
elif [ "$protocol" = POWERTRACE ]; then
    printf '%s\n' 'Historique RAM d’activité et de sauvegarde. Aucun reset ni changement de destination.'
else
    printf '%s\n' 'Les requêtes du démarrage sont conservées, aucune frappe à effectuer.'
fi
sleep 1
if [ "$protocol" = BLEFORGET ]; then
    printf '%s\n' "$serial_command" >&3
else
    printf '%s' "$serial_command" >&3
fi
deadline=$((SECONDS + 15))
finished=false
while [ "$SECONDS" -lt "$deadline" ]; do
    if IFS= read -r -t 1 line <&3; then
        line=${line%$'\r'}
        printf '%s\n' "$line" >> "$capture"
        if { [ "$protocol" = USBTRACE ] && [ "$line" = 'USBTRACE END' ]; } ||
           { [ "$protocol" = BLETRACE ] && [[ "$line" == 'BLETRACE END attrs='* ]]; } ||
           { [ "$protocol" = BLEMEM ] && [ "$line" = 'BLEMEM END' ]; } ||
           { [ "$protocol" = BLEFORGET ] && [ "$line" = 'BLEFORGET END' ]; } ||
           { [ "$protocol" = POWERTRACE ] && [ "$line" = 'POWERTRACE END' ]; } ||
           { [ "$protocol" = BLEPROFILE ] && [ "$line" = 'BLEPROFILE END' ]; }; then
            finished=true
            break
        fi
    else
        sleep 0.05
    fi
done

[ "$finished" = true ] || fail "Capture incomplète ou mauvais firmware/port. Données conservées : $capture"
awk -v command="$serial_command" -f "$validator" "$capture" ||
    fail "Capture refusée (pertes ou format incorrect). Données conservées : $capture"
printf 'Capture complète : %s\n' "$capture"
if [ "$protocol" = POWERTRACE ]; then
    cat "$capture"
    printf '%s\n' 'activity : 0 actif, 1 inactif, 2 sommeil. usb : 0 absent, 1 alimentation, 2 énuméré.'
    printf '%s\n' 'BOOT identifie ce démarrage. Le journal RAM disparaît après reset ou sommeil profond.'
    exit 0
fi
if [ "$protocol" = BLEFORGET ]; then
    cat "$capture"
    grep -q ' status=ok ' "$capture" || fail 'Effacement refusé ou échoué. Aucun nouvel essai automatique.'
    exit 0
fi
if [ "$protocol" = BLEMEM ]; then
    cat "$capture"
    printf '%s\n' 'stored = OS enregistré, state=saved = sauvegarde confirmée et identité vérifiée.'
    printf '%s\n' 'measured = dernière hypothèse de l’acquisition initiale de ce démarrage.'
    exit 0
fi
if [ "$protocol" = BLEPROFILE ]; then
    cat "$capture"
    grep -q ' status=ok ' "$capture" || fail 'Sélection refusée : busy = saisie en cours (ancien firmware : hôte connecté)'
    exit 0
fi
if [ "$protocol" = BLETRACE ]; then
    awk '
        /^BLETRACE HOST / {
            split($3, session, "="); split($4, profile, "="); split($5, connected, "=")
            suffix[session[2]] = sprintf(", emplacement %s, %s", profile[2] == -1 ? "inconnu" : profile[2], connected[2] ? "connecté" : "déconnecté")
        }
        /^BLETRACE OS / {
            split($3, session, "="); split($4, result, "=")
            label = "indices insuffisants, OS inconnu"
            if (result[2] == "macos") label = "hypothèse macOS"
            else if (result[2] == "windows") label = "hypothèse Windows"
            else if (result[2] == "ambiguous") label = "indices contradictoires, résultat ambigu"
            printf "Signature Bluetooth, session %s%s : %s\n", session[2], suffix[session[2]], label
            reports++
        }
        END { if (!reports) print "Classification absente : ancien firmware de capture" }
    ' "$capture"
    printf '%s\n' 'Signatures de session uniquement. Pour l’OS mémorisé, utilise make ble memory <port>.'
    exit 0
fi

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
