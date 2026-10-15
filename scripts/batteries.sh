#!/bin/bash

# Prépare le lecteur CoreBluetooth natif et affiche les deux batteries sur macOS.
# Sans argument : lecture immédiate. avg : moyenne des lectures pendant 30 secondes.

set -eu

. "$(dirname "$0")/build-common.sh"

# Le cache compilé est local au dépôt et sera recréé automatiquement après make clear.
BATTERY_CACHE="$LOCAL_ROOT/tools/batteries"
BATTERY_BINARY="$BATTERY_CACHE/batteries"
BATTERY_SOURCE="$SCRIPT_DIRECTORY/batteries.swift"
BATTERY_INFO="$SCRIPT_DIRECTORY/batteries-info.plist"
CENTRAL_CONFIGURATION="$REPOSITORY/boards/shields/splitergoll/splitergoll_left.conf"

[ "$#" -le 1 ] || fail 'Utilise make bat ou make bat avg'
case "${1-}" in
    '') mode=snapshot ;;
    avg) mode=average ;;
    *) fail 'Utilise make bat ou make bat avg' ;;
esac

[ "$(uname -s)" = Darwin ] || fail 'La lecture des batteries utilise CoreBluetooth et nécessite macOS'
command -v swiftc >/dev/null || fail 'Prérequis absent : installe les outils de ligne de commande Xcode pour Swift'

# Lit le nom annoncé dans la configuration de la centrale, sans valeur inventée.
resolve_keyboard_name() {
    test -r "$CENTRAL_CONFIGURATION" || fail 'Configuration de la centrale absente'
    keyboard_name=$(sed -n 's/^CONFIG_ZMK_KEYBOARD_NAME="\([^"]*\)"$/\1/p' "$CENTRAL_CONFIGURATION")
    [ -n "$keyboard_name" ] || fail 'CONFIG_ZMK_KEYBOARD_NAME est absent de la configuration de la centrale'
    case "$keyboard_name" in
        *$'\n'*) fail 'Le nom Bluetooth doit tenir sur une ligne' ;;
    esac
}

# Compile seulement si le lecteur ou ses métadonnées ont changé, sans Docker.
prepare_reader() {
    mkdir -p "$BATTERY_CACHE"

    if [ ! -x "$BATTERY_BINARY" ] || [ "$BATTERY_SOURCE" -nt "$BATTERY_BINARY" ] || [ "$BATTERY_INFO" -nt "$BATTERY_BINARY" ]; then
        printf '%s\n' 'Préparation du lecteur batterie natif…'
        swiftc -warnings-as-errors -module-cache-path "$BATTERY_CACHE/swift-cache" \
            "$BATTERY_SOURCE" \
            -Xlinker -sectcreate -Xlinker __TEXT -Xlinker __info_plist -Xlinker "$BATTERY_INFO" \
            -o "$BATTERY_BINARY"
    fi
}

resolve_keyboard_name
prepare_reader
exec "$BATTERY_BINARY" "$keyboard_name" "$mode"
