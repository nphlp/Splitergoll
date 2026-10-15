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

if [ "${1-}" = build ]; then
    printf '\n%sProfils firmware disponibles%s\n\n' "$LOG_HEADING" "$LOG_RESET"
    show_command 'make build wire' 'USB vers l’ordinateur'
    show_command 'make build bluetooth' 'Bluetooth direct, y compris pendant la charge'
    show_command 'make build wire debug' 'Profil USB avec logs USB sur les deux moitiés'
    show_command 'make build bluetooth debug' 'Profil Bluetooth avec logs USB sur les deux moitiés'
    printf '\n%sKeymaps%s\n' "$LOG_HEADING" "$LOG_RESET"
    printf '%s\n' '  multi (par défaut) : trois OS avec changement au clavier'
    printf '%s\n' '  windows : Windows avec gaming'
    printf '%s\n' '  macos / linux : OS fixe, sans gaming ni changement d’OS'
    printf '\n'
    show_command 'make build wire macos' 'USB avec keymap macOS fixe'
    show_command 'make build bluetooth windows debug' 'Bluetooth, Windows avec gaming et logs USB'
    printf '\n'
    exit 0
fi

printf '\n%sSplitergoll%s\n\n' "$LOG_HEADING" "$LOG_RESET"
printf '%sFirmware%s\n' "$LOG_HEADING" "$LOG_RESET"
show_command 'make setup' 'Préparer les sources et le build local'
show_command 'make build' 'Afficher les profils firmware disponibles'
show_command 'make build wire' 'Compiler le profil USB'
show_command 'make build bluetooth' 'Compiler le profil Bluetooth direct'
show_command 'make build <profil> debug' 'Ajouter les logs USB pour le diagnostic'
show_command 'make build <profil> <keymap> [debug]' 'multi, windows avec gaming, macos ou linux'

printf '\n%sBatteries sur macOS%s\n' "$LOG_HEADING" "$LOG_RESET"
show_command 'make bat' 'Lire les deux derniers niveaux annoncés'
show_command 'make bat avg' 'Calculer une moyenne par moitié sur 30 secondes'

printf '\n%sEntretien%s\n' "$LOG_HEADING" "$LOG_RESET"
show_command 'make stop' 'Arrêter les conteneurs de build de ce checkout'
show_command 'make clear' 'Arrêter le build et effacer les fichiers locaux générés'
show_command 'make help' 'Afficher cette aide'
printf '\n'
