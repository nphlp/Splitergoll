#!/bin/sh

# Exécute les commandes west dans le conteneur et contrôle les dépendances.
# Reçoit setup ou build des scripts hôtes, puis vérifie les firmwares compilés.

set -eu

. "$(dirname "$0")/build-common.sh"
cd "$CONTAINER_WORKSPACE"

# Vérifie que chaque dépendance est au commit prévu et ne contient aucune modification suivie.
check_dependencies() {
    west list -f '{name} {revision} {path}' > .west/projects.lock

    while read -r name revision path; do
        [ "$name" != manifest ] || continue
        printf 'Contrôle de la dépendance : %s\n' "$name"
        current=$(git -C "$path" rev-parse HEAD)
        changes=$(git -C "$path" status --porcelain --untracked-files=no)

        if [ "$current" != "$revision" ] || [ -n "$changes" ]; then
            fail "Dépendance incorrecte ou modifiée : $name"
        fi
        printf '[OK] %s au commit prévu\n' "$name"
    done < .west/projects.lock
}

# Contrôle les rôles USB/BLE, les options du profil câble et la présence du fichier UF2.
check_firmware() {
    side=$1
    directory=$2
    configuration="$directory/zephyr/.config"

    grep -qx 'CONFIG_ZMK_SPLIT=y' "$configuration" || fail 'Split désactivé'
    grep -qx 'CONFIG_ZMK_BLE=y' "$configuration" || fail 'BLE interne désactivé'

    if [ "$side" = left ]; then
        grep -qx 'CONFIG_ZMK_SPLIT_ROLE_CENTRAL=y' "$configuration" || fail 'Gauche non centrale'
        grep -qx 'CONFIG_ZMK_USB=y' "$configuration" || fail 'Sortie USB gauche absente'
    elif grep -Eq '^(CONFIG_ZMK_SPLIT_ROLE_CENTRAL|CONFIG_ZMK_USB)=y$' "$configuration"; then
        fail 'Rôle incorrect pour la droite'
    fi

    if grep -Eq '^(CONFIG_ZMK_SLEEP|CONFIG_ZMK_BATTERY_REPORTING)=y$' "$configuration"; then
        fail 'Veille ou reporting batterie activé dans le profil câble'
    fi
    test -s "$directory/zephyr/zmk.uf2" || fail "UF2 absent : $side"
}

case "${1-}" in
    setup)
        if [ ! -d .west ]; then west init -l config; fi
        west update --narrow --fetch-opt=--filter=blob:none --fetch-opt=--progress
        check_dependencies
        ;;
    build)
        check_dependencies
        west zephyr-export

        for side in $SIDES; do
            directory="build/splitergoll-$PROFILE-$side"
            west build -p auto -s zmk/app -d "$directory" -b "$BOARD" -- \
                -DSHIELD="splitergoll_$side" \
                -DZMK_CONFIG="$REPOSITORY/config" \
                -DZMK_EXTRA_MODULES="$REPOSITORY"
            check_firmware "$side" "$directory"
        done
        ;;
    *) fail 'Action attendue : setup ou build' ;;
esac
