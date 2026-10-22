#!/bin/bash

# Compile l’app native et prépare son bundle local, ou exécute ses tests Swift.
# start ouvre, build compile, install copie le bundle déjà compilé, test teste sans Bluetooth.
# Tous les caches restent dans .local/. L’app s’inscrit à l’ouverture de session lors de son lancement.

set -euo pipefail

# Chemins relatifs au checkout, comme l’outillage de build du firmware.
APP_SOURCE=$(CDPATH= cd "$(dirname "$0")" && pwd)
REPOSITORY=$(CDPATH= cd "$APP_SOURCE/../.." && pwd)
APP_CACHE="$REPOSITORY/.local/tools/macos-app"
APP_BUNDLE="$REPOSITORY/.local/apps/Splitergoll.app"
APP_ICON_CACHE="$APP_CACHE/icon"
CENTRAL_CONFIGURATION="$REPOSITORY/keyboard/boards/shields/splitergoll/splitergoll_left.conf"

# Signale un prérequis ou argument manquant avant toute compilation ou ouverture d’application.
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

# L’aide et les arguments erronés ne nécessitent ni Swift, ni cache, ni accès Bluetooth.
usage() {
    bash "$REPOSITORY/scripts/help.sh" app
    exit "${1:-0}"
}

[ "$#" -gt 0 ] || usage
[ "$#" -eq 1 ] || usage 1
case "$1" in
    start|build|install|test) action=$1 ;;
    help) usage ;;
    *) usage 1 ;;
esac
if [ "$action" = install ]; then
    exec bash "$APP_SOURCE/install.sh"
fi
[ "$(uname -s)" = Darwin ] || fail 'L’application nécessite macOS 13 ou plus récent'
command -v swift >/dev/null || fail 'Installe les outils de ligne de commande Xcode pour Swift'
mkdir -p "$APP_CACHE"
export CLANG_MODULE_CACHE_PATH="$APP_CACHE/clang-modules"

# Isole les caches SwiftPM du projet et n’utilise aucune dépendance réseau.
swift_package() {
    swift "$1" --package-path "$APP_SOURCE" --scratch-path "$APP_CACHE/build" \
        --cache-path "$APP_CACHE/packages" --config-path "$APP_CACHE/config" \
        --security-path "$APP_CACHE/security" --disable-sandbox "${@:2}"
}

if [ "$action" = test ]; then
    swift_package test -Xswiftc -warnings-as-errors
    exit 0
fi

# Refuse de remplacer le bundle d’une application active, sans fermer celle de l’utilisateur.
if [ -f "$APP_BUNDLE/Contents/MacOS/SplitergollBatteries" ] && \
    lsof -t "$APP_BUNDLE/Contents/MacOS/SplitergollBatteries" >/dev/null 2>&1; then
    fail 'Quitte Splitergoll depuis la barre des menus avant de reconstruire son bundle'
fi

keyboard_name=$(sed -n 's/^CONFIG_ZMK_KEYBOARD_NAME="\([^"]*\)"$/\1/p' "$CENTRAL_CONFIGURATION")
[ -n "$keyboard_name" ] || fail 'CONFIG_ZMK_KEYBOARD_NAME est absent de la configuration de la centrale'
case "$keyboard_name" in
    *$'\n'*) fail 'Le nom Bluetooth doit tenir sur une ligne' ;;
esac

swift_package build -c release -Xswiftc -warnings-as-errors
binary_directory=$(swift_package build -c release --show-bin-path)

# Génère l’icône à partir de son dessin vectoriel, puis embarque toutes ses tailles dans le bundle.
if [ ! -f "$APP_ICON_CACHE/AppIcon.icns" ] || \
    [ "$APP_SOURCE/Assets/RenderIcon.swift" -nt "$APP_ICON_CACHE/AppIcon.icns" ] || \
    [ "$APP_SOURCE/Assets/AppIcon.svg" -nt "$APP_ICON_CACHE/AppIcon.icns" ]; then
    mkdir -p "$APP_ICON_CACHE"
    swiftc -swift-version 6 -warnings-as-errors -parse-as-library \
        -module-cache-path "$CLANG_MODULE_CACHE_PATH" \
        "$APP_SOURCE/Assets/RenderIcon.swift" -o "$APP_ICON_CACHE/render-icon"
    "$APP_ICON_CACHE/render-icon" "$APP_SOURCE/Assets/AppIcon.svg" "$APP_ICON_CACHE/AppIcon.iconset"
    iconutil -c icns "$APP_ICON_CACHE/AppIcon.iconset" -o "$APP_ICON_CACHE/AppIcon.icns"
fi

mkdir -p "$APP_BUNDLE/Contents/MacOS" "$APP_BUNDLE/Contents/Resources"
cp "$binary_directory/SplitergollBatteries" "$APP_BUNDLE/Contents/MacOS/SplitergollBatteries"
cp "$APP_ICON_CACHE/AppIcon.icns" "$APP_BUNDLE/Contents/Resources/AppIcon.icns"
ditto --norsrc "$binary_directory/SplitergollBatteries_SplitergollBatteries.bundle" \
    "$APP_BUNDLE/Contents/Resources/SplitergollBatteries_SplitergollBatteries.bundle"
cp "$APP_SOURCE/Info.plist" "$APP_BUNDLE/Contents/Info.plist"
plutil -insert SplitergollKeyboardName -string "$keyboard_name" "$APP_BUNDLE/Contents/Info.plist"
codesign --force --sign - "$APP_BUNDLE"
codesign --verify --strict "$APP_BUNDLE"
printf 'Application prête : %s\n' "$APP_BUNDLE"

if [ "$action" = start ]; then
    open "$APP_BUNDLE"
fi
