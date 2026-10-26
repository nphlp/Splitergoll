#!/bin/bash

# Affiche les aides par famille. Aucun accès Docker, Bluetooth ou USB.
# make help présente les groupes, les autres scripts demandent leur aide dédiée.

set -eu

. "$(dirname "$0")/terminal-colors.sh"
configure_output

# Affiche une commande et sa description dans deux colonnes lisibles.
show_command() {
    printf '  %s%-38s%s %s\n' "$LOG_SUCCESS" "$1" "$LOG_RESET" "$2"
}

# Explique les deux choix obligatoires, l’option debug et des commandes complètes.
show_build_help() {
    printf '\n%sCompiler un firmware%s\n\n' "$LOG_HEADING" "$LOG_RESET"
    printf '  %smake build <matériel> <keymap> [debug|usb-trace|ble-trace|identity]%s\n' "$LOG_SUCCESS" "$LOG_RESET"
    printf '%s\n' '  Matériel puis keymap obligatoires, aucun choix par défaut.'

    printf '\n%s1. Matériel (obligatoire)%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'wire' 'USB vers l’ordinateur, BLE entre les moitiés'
    show_command 'bluetooth' 'Bluetooth direct, y compris pendant la charge USB'

    printf '\n%s2. Keymap (obligatoire)%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'auto' 'Sélection macOS / Windows avec gaming (USB ou mémoire BLE, expérimental)'
    show_command 'multi' 'Windows, macOS et Linux, avec changement au clavier et gaming'
    show_command 'windows' 'Windows avec gaming inclus'
    show_command 'macos' 'macOS fixe, sans gaming'
    show_command 'linux' 'Linux fixe, sans gaming'

    printf '\n%sOption finale%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'debug' 'Ajouter les logs USB sur les deux moitiés'
    show_command 'usb-trace' 'Observer les requêtes USB sur la gauche (wire uniquement)'
    show_command 'ble-trace' 'Observer la connexion BLE sur la gauche (bluetooth uniquement)'
    show_command 'identity' 'Comparer USB/BLE avec l’app (bluetooth auto uniquement, sans bascule)'

    printf '\n%sExemples%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make build wire auto' 'USB avec sélection automatique macOS / Windows'
    show_command 'make build bluetooth auto' 'Bluetooth avec OS mémorisé par appareil sélectionné'
    show_command 'make build wire macos' 'USB avec keymap macOS fixe'
    show_command 'make build bluetooth multi' 'Bluetooth avec changement d’OS au clavier'
    show_command 'make build bluetooth windows debug' 'Bluetooth, Windows avec gaming et logs USB'
    show_command 'make build bluetooth macos ble-trace' 'Étudier la signature Bluetooth, sans sélection automatique'
    printf '\n'
}

# Présente la sélection du build et la confirmation requise avant un flash USB.
show_flash_help() {
    printf '\n%sFlasher un firmware (macOS)%s\n\n' "$LOG_HEADING" "$LOG_RESET"
    printf '  %smake flash <matériel> <keymap> <left|right> [debug|usb-trace|ble-trace|identity]%s\n' "$LOG_SUCCESS" "$LOG_RESET"
    printf '  %smake flash <chemin.uf2>%s\n\n' "$LOG_SUCCESS" "$LOG_RESET"
    printf '%s\n' '  Matériel : wire ou bluetooth. Keymap : auto, multi, windows, macos ou linux.'
    printf '%s\n' '  Sans chemin, choisit le dernier build exact. Le diagnostic doit être explicite.'
    printf '%s\n' '  Affiche la date complète et l’ancienneté, puis demande confirmation avant copie.'
    printf '%s\n' '  Mettre uniquement la moitié choisie en bootloader par un double appui sur reset.'
    printf '\n%sExemples%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make flash wire macos right' 'Dernier build USB avec keymap macOS, moitié droite'
    show_command 'make flash bluetooth multi left debug' 'Dernier build Bluetooth multi avec logs, moitié gauche'
    printf '\n'
}

# Présente les actions du lecteur ponctuel des deux batteries.
show_bat_help() {
    printf '\n%sBatteries sur macOS%s\n\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make bat' 'Afficher cette aide'
    show_command 'make bat read' 'Lire immédiatement les deux niveaux annoncés'
    show_command 'make bat avg' 'Calculer une moyenne par moitié sur 30 secondes'
    printf '\n'
}

# Présente le démarrage, la compilation et les tests de l’app native.
show_app_help() {
    printf '\n%sApplications%s\n\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make app' 'Afficher cette aide'
    show_command 'make app start' 'Compiler et démarrer l’app dans la barre des menus'
    show_command 'make app build' 'Compiler et signer le bundle sans le lancer'
    show_command 'make app install' 'Quitter l’app, installer le bundle compilé et le lancer'
    show_command 'make app test' 'Tester la logique sans accès Bluetooth'
    printf '\n'
    show_command 'make app build windows' 'Compiler le portable Windows sans le lancer'
    show_command 'make app test windows' 'Tester sa logique sans Windows ni Bluetooth'
    printf '\n'
}

# Explique comment identifier le port série de la gauche, sans ouvrir de connexion.
show_port_help() {
    printf '\n%sTrouver le port de la gauche%s\n' "$LOG_HEADING" "$LOG_RESET"
    printf '%s\n' '  Brancher la gauche en USB avec un firmware de diagnostic, hors bootloader.'
    printf '%s\n' '  Utiliser usb-trace pour make usb, ble-trace pour make ble.'
    printf '\n'
    show_command 'ls /dev/cu.usbmodem*' 'Lister les ports sur macOS'
    show_command 'ls /dev/ttyACM*' 'Lister les ports sur Linux'
    printf '\n'
    printf '%s\n' '  Reporter le nom trouvé à la place de <port> dans les commandes USB ou BLE.'
    printf '%s\n' '  Si plusieurs ports apparaissent, comparer la liste avant et après branchement de la gauche.'
    printf '\n'
}

# Présente la lecture USB, avec un port explicite et sans effet sur le firmware.
show_usb_help() {
    printf '\n%sDiagnostic USB%s\n\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make usb' 'Afficher cette aide'
    show_command 'make usb capture <port>' 'Lire les requêtes de la centrale wire usb-trace'
    printf '\n'
    printf '%s\n' '  Pour trouver le port de la gauche : make port.'
    printf '\n'
}

# Présente les lectures et les changements explicites de profil Bluetooth.
show_ble_help() {
    printf '\n%sDiagnostic Bluetooth%s\n\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make ble' 'Afficher cette aide'
    show_command 'make ble capture <port>' 'Lire les échanges Bluetooth de la centrale ble-trace'
    show_command 'make ble memory <port>' 'Lire l’OS mémorisé et l’état de sauvegarde'
    show_command 'make ble power <port>' 'Lire le journal RAM du diagnostic d’activité dédié'
    show_command 'make ble profiles <port>' 'Afficher les emplacements appairés'
    show_command 'make ble select <port> <index>' 'Sélectionner un emplacement'
    show_command 'make ble forget <port> <index>' 'Effacer un appairage après confirmation'
    printf '\n'
    printf '%s\n' '  Pour trouver le port de la gauche : make port.'
    printf '\n'
}

# Garde l’aide générale compacte, avec les mêmes familles que le Makefile.
show_help() {
    printf '\n%sSplitergoll%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make help' 'Afficher cette aide'
    printf '\n%sFirmware%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make setup' 'Préparer les sources et le build local'
    show_command 'make build' 'Afficher les matériels et keymaps disponibles'
    show_command 'make flash' 'Afficher les choix de flash'
    show_command 'make stop' 'Arrêter les conteneurs de build de ce checkout'
    show_command 'make clear' 'Arrêter le build et effacer .local/ et dist/'
    printf '\n%sBatteries et applications%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make bat' 'Afficher les commandes de lecture des batteries'
    show_command 'make app' 'Afficher les commandes des apps Mac et Windows'
    printf '\n%sDebug%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make port' 'Afficher la méthode pour trouver le port du clavier'
    show_command 'make usb' 'Afficher les commandes de diagnostic USB'
    show_command 'make ble' 'Afficher les commandes de diagnostic Bluetooth'
    printf '\n'
}

[ "$#" -le 1 ] || { show_help; exit 1; }
case "${1-}" in
    '') show_help ;;
    build) show_build_help ;;
    flash) show_flash_help ;;
    bat) show_bat_help ;;
    app) show_app_help ;;
    port) show_port_help ;;
    usb) show_usb_help ;;
    ble) show_ble_help ;;
    *) show_help; exit 1 ;;
esac
