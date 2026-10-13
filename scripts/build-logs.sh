#!/bin/bash

# Affiche les étapes du setup et du build avec leurs logs en direct et leur durée.
# Chargé par les scripts hôtes. En terminal interactif, chaque étape se replie après réussite.

LOG_WINDOW_LINES=8

# Active une palette sobre uniquement dans un terminal compatible, sauf avec NO_COLOR.
configure_output() {
    LOG_HEADING='' LOG_SUCCESS='' LOG_WARNING='' LOG_ERROR='' LOG_DIM='' LOG_RESET=''
    if [ -t 1 ] && [ "${TERM:-dumb}" != dumb ] && [ -z "${NO_COLOR:-}" ]; then
        LOG_HEADING=$'\033[1;36m'
        LOG_SUCCESS=$'\033[32m'
        LOG_WARNING=$'\033[33m'
        LOG_ERROR=$'\033[31m'
        LOG_DIM=$'\033[90m'
        LOG_RESET=$'\033[0m'
    fi
}

# Formate une durée en secondes pour afficher des minutes et secondes lisibles.
format_duration() {
    local duration=$1
    if [ "$duration" -lt 60 ]; then
        printf '%d s' "$duration"
    else
        printf '%d min %02d s' "$((duration / 60))" "$((duration % 60))"
    fi
}

# Crée un journal distinct pour cette exécution et initialise le suivi des étapes.
start_logging() {
    local action=$1
    RUN_STEP_COUNT=$2
    RUN_STEP=0
    RUN_STARTED=$(date +%s)
    configure_output
    mkdir -p "$LOG_ROOT"
    RUN_LOG_DIRECTORY=$(mktemp -d "$LOG_ROOT/$action-$(date -u +%Y%m%dT%H%M%SZ)-XXXXXX")
    printf '\n%sSplitergoll / %s%s\n' "$LOG_HEADING" "$action" "$LOG_RESET"
}

# Exécute une commande sans reconstruire ses arguments, conserve son statut et ses logs.
# En terminal interactif, les huit dernières lignes se replient en une ligne de validation.
run_step() {
    local title=$1
    shift
    RUN_STEP=$((RUN_STEP + 1))

    local started logfile statusfile status elapsed columns shown=0 line warning
    local -a window=() statuses=()
    started=$(date +%s)
    logfile="$RUN_LOG_DIRECTORY/$(printf '%02d' "$RUN_STEP").log"
    statusfile="$logfile.exit"
    printf '\n'

    if [ -t 1 ] && [ "${TERM:-dumb}" != dumb ]; then
        columns=$(tput cols 2>/dev/null) || columns=80
        printf '\033[s%s[%s/%s] %s%s\n' "$LOG_HEADING" "$RUN_STEP" "$RUN_STEP_COUNT" "$title" "$LOG_RESET"

        while IFS= read -r line || [ -n "$line" ]; do
            window+=("$line")
            if [ "${#window[@]}" -gt "$LOG_WINDOW_LINES" ]; then
                window=("${window[@]: -LOG_WINDOW_LINES}")
            fi
            if [ "$shown" -gt 0 ]; then printf '\033[%dA' "$shown"; fi
            for line in "${window[@]}"; do
                printf '\033[2K%s%s%s\n' "$LOG_DIM" "${line:0:$((columns - 1))}" "$LOG_RESET"
            done
            shown=${#window[@]}
        done < <(
            set +e
            (set -e; "$@") 2>&1 | tee "$logfile" | tr '\r' '\n'
            statuses=("${PIPESTATUS[@]}")
            status=${statuses[0]}
            for elapsed in "${statuses[@]}"; do
                if [ "$elapsed" -ne 0 ]; then status=$elapsed; break; fi
            done
            printf '%s\n' "$status" > "$statusfile"
        )
        printf '\033[u\033[J'
        status=1
        if [ -s "$statusfile" ]; then status=$(cat "$statusfile"); fi
    else
        printf '[%s/%s] %s\n' "$RUN_STEP" "$RUN_STEP_COUNT" "$title"
        set +e
        (set -e; "$@") 2>&1 | tee "$logfile"
        statuses=("${PIPESTATUS[@]}")
        set -e
        status=${statuses[0]}
        for elapsed in "${statuses[@]}"; do
            if [ "$elapsed" -ne 0 ]; then status=$elapsed; break; fi
        done
        printf '%s\n' "$status" > "$statusfile"
    fi

    elapsed=$(( $(date +%s) - started ))
    printf '%s\t%s\t%s\t%s\n' "$RUN_STEP" "$elapsed" "$status" "$title" >> "$RUN_LOG_DIRECTORY/durations.tsv"

    if [ "$status" -eq 0 ]; then
        printf '%s[OK %s/%s]%s %s %s(%s)%s\n' "$LOG_SUCCESS" "$RUN_STEP" "$RUN_STEP_COUNT" "$LOG_RESET" "$title" "$LOG_DIM" "$(format_duration "$elapsed")" "$LOG_RESET"
        if [ -t 1 ] && [ "${TERM:-dumb}" != dumb ]; then
            while IFS= read -r warning; do
                printf '  %s%s%s\n' "$LOG_WARNING" "$warning" "$LOG_RESET"
            done < <(grep -Ei '(^|[[:space:]])warning(:|[[:space:]])' "$logfile" || :)
        fi
    else
        printf '%s[ÉCHEC %s/%s]%s %s (%s)\n' "$LOG_ERROR" "$RUN_STEP" "$RUN_STEP_COUNT" "$LOG_RESET" "$title" "$(format_duration "$elapsed")" >&2
        if [ -t 1 ] && [ "${TERM:-dumb}" != dumb ]; then cat "$logfile"; fi
        printf 'Journal : %s\n' "$logfile" >&2
    fi
    return "$status"
}

# Affiche la durée totale et le chemin des journaux après toutes les étapes réussies.
finish_logging() {
    local elapsed=$(( $(date +%s) - RUN_STARTED ))
    printf '\n%s%s%s en %s\n%sLogs complets : %s%s\n' "$LOG_SUCCESS" "$1" "$LOG_RESET" "$(format_duration "$elapsed")" "$LOG_DIM" "$RUN_LOG_DIRECTORY" "$LOG_RESET"
}
