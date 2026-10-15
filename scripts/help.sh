#!/bin/bash

# Affiche les commandes du dépôt, regroupées par usage, avec des couleurs facultatives.
# Appelé par make help, ou avec build pour afficher uniquement les profils disponibles.

set -eu

[ "$#" -le 1 ] || { printf '%s\n' 'Utilise make help sans argument' >&2; exit 1; }
case "${1-}" in
    ''|build) ;;
    *) printf '%s\n' 'Utilise make help sans argument' >&2; exit 1 ;;
esac

. "$(dirname "$0")/terminal-colors.sh"
configure_output

# Affiche une commande et sa description dans deux colonnes lisibles.
show_command() {
    printf '  %s%-38s%s %s\n' "$LOG_SUCCESS" "$1" "$LOG_RESET" "$2"
}

# Explique les deux choix obligatoires, l’option debug et des commandes complètes.
show_build_help() {
    printf '\n%sCompiler un firmware%s\n\n' "$LOG_HEADING" "$LOG_RESET"
    printf '  %smake build <matériel> <keymap> [debug]%s\n' "$LOG_SUCCESS" "$LOG_RESET"
    printf '%s\n' '  Matériel puis keymap obligatoires, aucun choix par défaut.'

    printf '\n%s1. Matériel (obligatoire)%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'wire' 'USB vers l’ordinateur, BLE entre les moitiés'
    show_command 'bluetooth' 'Bluetooth direct, y compris pendant la charge USB'

    printf '\n%s2. Keymap (obligatoire)%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'multi' 'Windows, macOS et Linux, avec changement au clavier et gaming'
    show_command 'windows' 'Windows avec gaming inclus'
    show_command 'macos' 'macOS fixe, sans gaming'
    show_command 'linux' 'Linux fixe, sans gaming'

    printf '\n%sOption finale%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'debug' 'Ajouter les logs USB sur les deux moitiés'

    printf '\n%sExemples%s\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make build wire macos' 'USB avec keymap macOS fixe'
    show_command 'make build bluetooth multi' 'Bluetooth avec changement d’OS au clavier'
    show_command 'make build bluetooth windows debug' 'Bluetooth, Windows avec gaming et logs USB'
    printf '\n'
}

if [ "${1-}" = build ]; then
    show_build_help
    exit 0
fi

printf '\n%sSplitergoll%s\n\n' "$LOG_HEADING" "$LOG_RESET"
show_command 'make setup' 'Préparer les sources et le build local'
show_command 'make build' 'Afficher les choix de compilation'
show_build_help

printf '\n%sBatteries sur macOS%s\n' "$LOG_HEADING" "$LOG_RESET"
show_command 'make bat' 'Lire les deux derniers niveaux annoncés'
show_command 'make bat avg' 'Calculer une moyenne par moitié sur 30 secondes'

printf '\n%sEntretien%s\n' "$LOG_HEADING" "$LOG_RESET"
show_command 'make stop' 'Arrêter les conteneurs de build de ce checkout'
show_command 'make clear' 'Arrêter le build et effacer les fichiers locaux générés'
show_command 'make help' 'Afficher cette aide'
printf '\n'
