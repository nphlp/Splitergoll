#!/bin/bash

# Installe le bundle déjà compilé, sans compilation, lancement ou inscription de session.
# Le dossier Applications est fourni par macOS. APP_INSTALL_DIRECTORY permet une destination explicite.
# Une mise à jour remplace uniquement Splitergoll, après contrôle de son identité et de sa fermeture.

set -euo pipefail

APP_SOURCE=$(CDPATH= cd "$(dirname "$0")" && pwd -P)
REPOSITORY=$(CDPATH= cd "$APP_SOURCE/../.." && pwd -P)
APP_BUNDLE="$REPOSITORY/.local/apps/Splitergoll.app"

# Refuse une installation incomplète ou une cible inattendue avant son remplacement.
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

[ "$#" -eq 0 ] || fail 'Utilise make app install sans argument'
[ "$(uname -s)" = Darwin ] || fail 'L’installation nécessite macOS'
command -v lsof >/dev/null || fail 'Prérequis absent : lsof'
[ -d "$APP_BUNDLE" ] || fail 'Aucun bundle compilé. Lance make app build avant make app install'

bundle_identifier=$(plutil -extract CFBundleIdentifier raw -o - "$APP_SOURCE/Info.plist")
executable=$(plutil -extract CFBundleExecutable raw -o - "$APP_SOURCE/Info.plist")

# Vérifie le bundle sans l’exécuter et refuse toute copie encore ouverte par l’utilisateur.
check_bundle() {
    local bundle=$1
    [ -d "$bundle" ] && [ ! -L "$bundle" ] || fail "Bundle inattendu : $bundle"
    local identifier
    identifier=$(plutil -extract CFBundleIdentifier raw -o - "$bundle/Contents/Info.plist")
    [ "$identifier" = "$bundle_identifier" ] || fail "Une autre application occupe $bundle"
    codesign --verify --strict "$bundle"
    if lsof -t "$bundle/Contents/MacOS/$executable" >/dev/null 2>&1; then
        fail 'Quitte Splitergoll depuis la barre des menus avant de l’installer'
    fi
}

check_bundle "$APP_BUNDLE"
if [ "${APP_INSTALL_DIRECTORY+x}" = x ]; then
    [ -n "$APP_INSTALL_DIRECTORY" ] || fail 'APP_INSTALL_DIRECTORY est vide'
    install_directory=$APP_INSTALL_DIRECTORY
else
    install_directory=$(osascript -l JavaScript -e \
        'ObjC.import("Foundation"); ObjC.unwrap($.NSFileManager.defaultManager.URLsForDirectoryInDomains($.NSApplicationDirectory, $.NSLocalDomainMask).firstObject.path);')
fi
[ -d "$install_directory" ] && [ -w "$install_directory" ] ||
    fail 'Le dossier Applications doit exister et être accessible en écriture'
install_directory=$(CDPATH= cd "$install_directory" && pwd -P)
case "$install_directory/" in
    "$APP_BUNDLE/"*) fail 'La destination ne peut pas être dans le bundle source' ;;
esac
installed_bundle="$install_directory/$(basename "$APP_BUNDLE")"
[ "$installed_bundle" != "$APP_BUNDLE" ] || fail 'La source et la destination sont identiques'
if [ -e "$installed_bundle" ] || [ -L "$installed_bundle" ]; then
    check_bundle "$installed_bundle"
fi

stage_directory=$(mktemp -d "$install_directory/.splitergoll-install.XXXXXX")
staged_bundle="$stage_directory/$(basename "$APP_BUNDLE")"
backup_bundle="$stage_directory/previous.app"

# Restaure l’ancienne app si le déplacement échoue, puis nettoie uniquement notre dossier temporaire.
cleanup() {
    local status=$?
    trap - EXIT
    if [ "$status" -ne 0 ] && [ -d "$backup_bundle" ] && [ ! -e "$installed_bundle" ]; then
        if ! mv "$backup_bundle" "$installed_bundle"; then
            printf 'Restauration impossible. Ancienne app conservée : %s\n' "$backup_bundle" >&2
            exit 1
        fi
    fi
    rm -rf -- "$stage_directory"
    exit "$status"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

ditto --norsrc "$APP_BUNDLE" "$staged_bundle"
check_bundle "$staged_bundle"
if [ -d "$installed_bundle" ]; then
    mv "$installed_bundle" "$backup_bundle"
fi
mv "$staged_bundle" "$installed_bundle"
printf 'Application installée : %s\n' "$installed_bundle"
printf '%s\n' 'Ouvre cette copie depuis Applications. Vérifie ensuite son entrée d’ouverture de session.'
