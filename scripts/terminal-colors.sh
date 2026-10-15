#!/bin/bash

# Partage les couleurs de l’aide et des journaux, sans codes ANSI hors terminal.
# Ce fichier est chargé par les scripts Bash qui affichent des messages.

# Active une palette sobre dans un terminal compatible, sauf si NO_COLOR est défini.
configure_output() {
    LOG_HEADING='' LOG_SUCCESS='' LOG_WARNING='' LOG_ERROR='' LOG_DIM='' LOG_RESET=''

    if [ -t 1 ] && [ "${TERM:-dumb}" != dumb ] && [ "${NO_COLOR+x}" != x ]; then
        LOG_HEADING=$'\033[1;36m'
        LOG_SUCCESS=$'\033[32m'
        LOG_WARNING=$'\033[33m'
        LOG_ERROR=$'\033[31m'
        LOG_DIM=$'\033[90m'
        LOG_RESET=$'\033[0m'
    fi
}
