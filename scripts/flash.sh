#!/bin/bash

# Sélectionne un UF2 local, affiche sa date et le copie après confirmation.
# Reçoit matériel, keymap, left/right et éventuellement debug, usb-trace, ble-trace ou identity, ou un UF2.
# macOS uniquement. Aucun build, reset des réglages ou flash sans confirmation.

set -euo pipefail

. "$(dirname "$0")/build-common.sh"
. "$(dirname "$0")/terminal-colors.sh"
configure_output

if [ "$#" -eq 0 ]; then
    exec bash "$SCRIPT_DIRECTORY/help.sh" flash
fi

[ "$(uname -s)" = Darwin ] || fail 'Le flash automatique est disponible sur macOS uniquement'

# Retrouve le dernier dossier du profil exact, sans mélanger normal et debug.
select_latest_firmware() {
    local candidate name latest=''

    for candidate in "$ARTIFACT_ROOT/$BUILD_VARIANT"-*; do
        [ -d "$candidate" ] && [ ! -L "$candidate" ] || continue
        name=$(basename "$candidate")
        [[ "$name" =~ ^$BUILD_VARIANT-[0-9]{4}(-[0-9]{2}){5}$ ]] || continue
        if [[ -z "$latest" || "$candidate" > "$latest" ]]; then
            latest=$candidate
        fi
    done

    [ -n "$latest" ] || fail "Aucun build trouvé pour $BUILD_VARIANT. Lance make build $PROFILE $KEYMAP_PROFILE${DEBUG_ARGUMENT-}"
    FIRMWARE="$latest/splitergoll-$BUILD_VARIANT-$SIDE.uf2"
}

# Vérifie l’origine du fichier, son empreinte et tous ses blocs avant toute copie.
check_source() {
    local directory filename entry expected actual
    directory=$(dirname "$FIRMWARE")
    filename=$(basename "$FIRMWARE")
    [ -f "$FIRMWARE" ] && [ ! -L "$FIRMWARE" ] || fail "Firmware absent ou lien symbolique : $FIRMWARE"
    [ -r "$directory/build-info.txt" ] && [ -r "$directory/SHA256SUMS" ] ||
        fail 'Le fichier doit provenir d’un build du dépôt, avec build-info.txt et SHA256SUMS'

    for entry in "hardware_profile=$HARDWARE_PROFILE" "keymap_profile=$KEYMAP_PROFILE" "build_mode=$BUILD_MODE" "variant=$BUILD_VARIANT"; do
        grep -Fxq "$entry" "$directory/build-info.txt" || fail "Métadonnées incompatibles : $entry"
    done
    expected=$(awk -v name="./$filename" '$2 == name { print $1 }' "$directory/SHA256SUMS")
    [[ "$expected" =~ ^[a-f0-9]{64}$ ]] || fail 'Empreinte absente, ambiguë ou incorrecte dans SHA256SUMS'
    actual=$(shasum -a 256 "$FIRMWARE" | cut -d ' ' -f 1)
    [ "$actual" = "$expected" ] || fail 'Le firmware ne correspond pas à son empreinte SHA-256'
    hexdump -v -e '128/4 "%u " "\n"' "$FIRMWARE" | awk -f "$SCRIPT_DIRECTORY/uf2-check.awk" ||
        fail 'Le firmware UF2 est incompatible ou incomplet'
    FIRMWARE_SHA=$actual
}

# Lit la date du dossier de build, dans le même fuseau local que les exports.
read_build_date() {
    local name timestamp month
    local months=(janvier février mars avril mai juin juillet août septembre octobre novembre décembre)
    name=$(basename "$(dirname "$FIRMWARE")")
    [[ "$name" =~ ^$BUILD_VARIANT-[0-9]{4}(-[0-9]{2}){5}$ ]] || fail 'Le fichier doit rester dans son dossier de build horodaté'
    timestamp=${name#"$BUILD_VARIANT"-}
    BUILD_EPOCH=$(date -j -f '%Y-%m-%d-%H-%M-%S' "$timestamp" '+%s' 2>/dev/null) || fail 'Date du build illisible'
    [ "$(date -r "$BUILD_EPOCH" '+%Y-%m-%d-%H-%M-%S')" = "$timestamp" ] || fail 'Date du build invalide'
    month=$(date -r "$BUILD_EPOCH" '+%m')
    BUILD_DATE=$(date -r "$BUILD_EPOCH" "+%d ${months[$((10#$month - 1))]} %Y à %H h %M min %S s %Z")
}

# Formate l’âge du build, ou signale une date future plutôt que de la masquer.
describe_build_age() {
    local age prefix='il y a'
    age=$(( $(date '+%s') - BUILD_EPOCH ))
    if [ "$age" -lt 0 ]; then
        age=$(( -age ))
        prefix='dans'
    fi
    if [ "$age" -ge 86400 ]; then
        printf '%s %d j %d h' "$prefix" "$((age / 86400))" "$((age % 86400 / 3600))"
    elif [ "$age" -ge 3600 ]; then
        printf '%s %d h %d min' "$prefix" "$((age / 3600))" "$((age % 3600 / 60))"
    elif [ "$age" -ge 60 ]; then
        printf '%s %d min %d s' "$prefix" "$((age / 60))" "$((age % 60))"
    else
        printf '%s %d s' "$prefix" "$age"
    fi
}

# Présente le chemin du firmware relativement au dépôt, sans dépendance supplémentaire.
relative_firmware_path() {
    local base="$REPOSITORY" prefix=''

    while [[ "$FIRMWARE" != "$base/"* ]]; do
        base=${base%/*}
        prefix="../$prefix"
    done

    printf '%s%s' "$prefix" "${FIRMWARE#"$base/"}"
}

# Présente le profil dans l’ordre des arguments, puis la date et le fichier du build.
show_firmware_summary() {
    local keymap_label side_label logging_label

    case "$KEYMAP_PROFILE" in
        auto) keymap_label='auto (macOS / Windows + gaming)' ;;
        multi) keymap_label='multi (Windows, macOS, Linux)' ;;
        windows) keymap_label='Windows + gaming' ;;
        macos) keymap_label='macOS' ;;
        linux) keymap_label='Linux' ;;
    esac
    case "$SIDE" in
        left) side_label='gauche (left)' ;;
        right) side_label='droite (right)' ;;
    esac
    case "$BUILD_MODE" in
        debug) logging_label='activés (debug)' ;;
        usb-trace) logging_label='trace des requêtes USB (centrale gauche)' ;;
        ble-trace) logging_label='trace de la connexion Bluetooth (centrale gauche)' ;;
        identity) logging_label='désactivés, canal d’identité USB/Bluetooth actif sur la gauche' ;;
        usb-priority) logging_label='désactivés, priorité USB pour le même ordinateur déclaré' ;;
        normal) logging_label='désactivés (normal)' ;;
    esac

    printf '\n%sProfil sélectionné%s\n' "$LOG_HEADING" "$LOG_RESET"
    printf '  Matériel : %s\n' "$HARDWARE_PROFILE"
    printf '  Keymap   : %s\n' "$keymap_label"
    printf '  Côté     : %s\n' "$side_label"
    printf '  Logs USB : %s\n' "$logging_label"

    printf '\n  Build : %s\n' "$BUILD_DATE"
    printf '          %s%s%s\n' "$LOG_DIM" "$(describe_build_age)" "$LOG_RESET"
    printf '\n  Fichier : %s\n' "$(relative_firmware_path)"
}

# Lit un champ du volume interrogé avec diskutil, sans exécuter ses métadonnées.
volume_field() {
    printf '%s' "$VOLUME_INFO" | plutil -extract "$1" raw -o - -
}

# Vérifie que le chemin correspond toujours à un volume FAT monté sur un disque USB.
check_volume() {
    [ -d "$BOOTLOADER_VOLUME" ] && [ ! -L "$BOOTLOADER_VOLUME" ] || fail 'Le volume bootloader a disparu'
    VOLUME_INFO=$(diskutil info -plist "$BOOTLOADER_VOLUME") || fail 'Impossible de vérifier le volume bootloader'
    [ "$(volume_field MountPoint)" = "$BOOTLOADER_VOLUME" ] || fail 'Le chemin ne désigne pas un volume monté'
    [ "$(volume_field BusProtocol)" = USB ] && [ "$(volume_field FilesystemType)" = msdos ] ||
        fail 'Le bootloader doit être un volume FAT sur USB'
    [ "$(volume_field WritableVolume)" = true ] || fail 'Le volume bootloader n’est pas accessible en écriture'
    [ "$(tr -d '\r' < "$BOOTLOADER_VOLUME/INFO_UF2.TXT")" = "$BOOTLOADER_INFO" ] ||
        fail 'L’identité du bootloader a changé'
}

# Refuse l’absence ou l’ambiguïté des bootloaders nice!nano, sans déduire leur côté.
find_bootloader() {
    local info board count=0
    for info in "$BOOTLOADER_VOLUME_ROOT"/*/INFO_UF2.TXT; do
        [ -f "$info" ] || continue
        board=$(sed -n 's/^Board-ID: *//p' "$info" | tr -d '\r')
        [ "$board" = "$BOOTLOADER_BOARD_ID" ] || continue
        count=$((count + 1))
        BOOTLOADER_VOLUME=$(dirname "$info")
    done
    [ "$count" -gt 0 ] || fail "Aucun bootloader nice!nano trouvé. Fais un double appui sur reset de la moitié $SIDE, puis relance la commande"
    [ "$count" -eq 1 ] || fail 'Plusieurs bootloaders nice!nano détectés. Garde uniquement la moitié à flasher en bootloader'
    BOOTLOADER_INFO=$(tr -d '\r' < "$BOOTLOADER_VOLUME/INFO_UF2.TXT")
    check_volume
    BOOTLOADER_DEVICE=$(volume_field DeviceIdentifier)
}

# Lit une confirmation d’une seule touche, puis recontrôle le fichier et le volume.
confirm_and_flash() {
    local answer actual
    [ -t 0 ] || fail 'Confirmation interactive requise. Relance la commande depuis un terminal'
    if ! IFS= read -r -s -n 1 -p 'Continue? [y/N] ' answer; then
        answer=''
    fi
    printf '\n'
    case "$answer" in
        y|Y) ;;
        *) printf '%s\n' 'Flash annulé, aucun fichier copié'; return ;;
    esac

    actual=$(shasum -a 256 "$FIRMWARE" | cut -d ' ' -f 1)
    [ "$actual" = "$FIRMWARE_SHA" ] || fail 'Le firmware a changé depuis la confirmation'
    check_volume
    [ "$(volume_field DeviceIdentifier)" = "$BOOTLOADER_DEVICE" ] || fail 'Le disque USB a changé depuis la confirmation'
    printf '\n%sCopie du firmware…%s\n' "$LOG_HEADING" "$LOG_RESET"
    cp -X "$FIRMWARE" "$BOOTLOADER_VOLUME/$(basename "$FIRMWARE")" ||
        fail 'Copie signalée en erreur. Le résultat du flash reste à vérifier, aucun nouvel essai automatique'
    printf '%sCopie terminée.%s Le bootloader peut redémarrer et disparaître.\n' "$LOG_SUCCESS" "$LOG_RESET"
    printf '%s\n' 'Vérifie ensuite la saisie du clavier pour confirmer le flash.'
}

if [ "$#" -eq 1 ]; then
    FIRMWARE=$1
    filename=$(basename "$FIRMWARE")
    [[ "$filename" =~ ^splitergoll-(wire|bluetooth)-(auto|multi|windows|macos|linux)(-(debug|usb-trace|ble-trace|identity|usb-priority))?-(left|right)\.uf2$ ]] ||
        fail 'Précise un UF2 généré par ce dépôt, ou make flash <matériel> <keymap> <left|right> [debug|usb-trace|ble-trace|identity|usb-priority]'
    profile=${BASH_REMATCH[1]}
    keymap=${BASH_REMATCH[2]}
    mode=${BASH_REMATCH[4]}
    SIDE=${BASH_REMATCH[5]}
    select_profile "$profile"
    select_keymap "$keymap"
    select_build_mode "${mode:-normal}"
    directory=$(CDPATH= cd "$(dirname "$FIRMWARE")" && pwd) || fail 'Dossier du firmware absent'
    FIRMWARE="$directory/$filename"
else
    [ "$#" -eq 3 ] || [ "$#" -eq 4 ] || fail 'Utilise make flash <matériel> <keymap> <left|right> [debug|usb-trace|ble-trace|identity|usb-priority]'
    select_profile "$1"
    select_keymap "$2"
    SIDE=$3
    case "$SIDE" in left|right) ;; *) fail 'Moitié attendue : left ou right' ;; esac
    mode=normal
    if [ "$#" -eq 4 ]; then
        case "$4" in
            debug|usb-trace|ble-trace|identity|usb-priority) mode=$4 ;;
            *) fail 'Utilise debug, usb-trace, ble-trace, identity ou usb-priority après la moitié' ;;
        esac
        DEBUG_ARGUMENT=" $mode"
    fi
    select_build_mode "$mode"
    select_latest_firmware
fi

check_source
read_build_date
show_firmware_summary
find_bootloader
printf '  %sBootloader : %s%s\n' "$LOG_DIM" "$BOOTLOADER_VOLUME" "$LOG_RESET"
printf '\n%sLe disque ne distingue pas gauche et droite.%s\n' "$LOG_WARNING" "$LOG_RESET"
printf '%s\n\n' 'Seule la moitié choisie doit être en bootloader.'
confirm_and_flash
