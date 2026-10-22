#!/bin/bash

# Prépare le bundle déjà compilé, quitte les copies concernées et ouvre la nouvelle installation.
# Le dossier Applications est fourni par macOS. APP_INSTALL_DIRECTORY permet une destination explicite.
# Une mise à jour remplace uniquement Splitergoll, après contrôle et fermeture normale, sans forcer.

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

# Vérifie le bundle sans l’exécuter, avant toute demande de fermeture.
check_bundle() {
    local bundle=$1
    [ -d "$bundle" ] && [ ! -L "$bundle" ] || fail "Bundle inattendu : $bundle"
    local identifier
    identifier=$(plutil -extract CFBundleIdentifier raw -o - "$bundle/Contents/Info.plist")
    [ "$identifier" = "$bundle_identifier" ] || fail "Une autre application occupe $bundle"
    codesign --verify --strict "$bundle"
}

# Attend une fermeture normale des seules copies source et destination, jamais un autre programme.
quit_app() {
    osascript -l JavaScript - "$bundle_identifier" "$APP_BUNDLE" "$installed_bundle" <<'JXA'
// Demande une fermeture normale et borne l’attente sans utiliser de terminaison forcée.
function run(args) {
    ObjC.import('AppKit');
    const copies = ObjC.unwrap($.NSWorkspace.sharedWorkspace.runningApplications).filter(function(app) {
        if (ObjC.unwrap(app.bundleIdentifier) !== args[0]) return false;
        const path = ObjC.unwrap(app.bundleURL.URLByResolvingSymlinksInPath.path);
        if (path !== args[1] && path !== args[2]) {
            throw new Error('Une autre copie de Splitergoll est ouverte. Quitte-la avant l’installation.');
        }
        return true;
    });
    copies.forEach(function(app) {
        if (!app.terminate) throw new Error('Splitergoll a refusé la demande de fermeture.');
    });
    const deadline = Date.now() + 10000;
    while (copies.some(function(app) { return !app.terminated; })) {
        if (Date.now() >= deadline) throw new Error('Splitergoll reste ouvert. Installation annulée, aucune fermeture forcée.');
        $.NSRunLoop.currentRunLoop.runUntilDate($.NSDate.dateWithTimeIntervalSinceNow(0.1));
    }
}
JXA
}

# Une app rouverte pendant la préparation ne doit pas être remplacée pendant son exécution.
check_closed() {
    if lsof -t "$1/Contents/MacOS/$executable" >/dev/null 2>&1; then
        fail 'Splitergoll a été rouvert pendant l’installation. Relance make app install'
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
printf '%s\n' 'Fermeture de Splitergoll si nécessaire…'
quit_app
check_closed "$APP_BUNDLE"
if [ -d "$installed_bundle" ]; then
    check_closed "$installed_bundle"
fi
if [ -d "$installed_bundle" ]; then
    mv "$installed_bundle" "$backup_bundle"
fi
mv "$staged_bundle" "$installed_bundle"
printf 'Application installée : %s\n' "$installed_bundle"
open "$installed_bundle"
printf '%s\n' 'Application lancée depuis sa copie installée.'
