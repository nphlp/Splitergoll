#!/bin/sh

# Définit les constantes et fonctions partagées du build local.
# Ce fichier est chargé par les autres scripts, il ne se lance pas directement.

# Chemins relatifs au dépôt et image officielle figée.
SCRIPT_DIRECTORY=$(CDPATH= cd "$(dirname "$0")" && pwd)
REPOSITORY=$(CDPATH= cd "$SCRIPT_DIRECTORY/.." && pwd)
BUILD_IMAGE='zmkfirmware/zmk-build-arm:3.5@sha256:0979a8de325e3d822e48bd8d1d9efaee2984ad3750253119f920a342f0ce5cde'
LOCAL_ROOT="$REPOSITORY/.local"
OUTPUT_ROOT="$REPOSITORY/dist"
LOG_ROOT="$LOCAL_ROOT/logs"
WORKSPACE_ROOT="$LOCAL_ROOT/zmk"
ARTIFACT_ROOT="$OUTPUT_ROOT/firmware"
CONTAINER_SOURCE='/work/splitergoll'
CONTAINER_WORKSPACE='/work/zmk'
BOARD='nice_nano_v2'
PROFILE='cable-multi'
SIDES='left right'

# Affiche une erreur et arrête la commande pour éviter de poursuivre dans un état incorrect.
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

# Vérifie que Docker est installé et que son moteur répond avant toute action.
require_docker() {
    command -v docker >/dev/null || fail 'Prérequis absent : docker'
    docker info >/dev/null
}

# Vérifie les outils requis, le digest de l’image et l’accès au moteur Docker.
preflight() {
    require_docker
    command -v shasum >/dev/null || fail 'Prérequis absent : shasum'
    printf '%s\n' "$BUILD_IMAGE" | grep -Eq '@sha256:[a-f0-9]{64}$' ||
        fail 'Image Docker non figée par digest'
}

# Arrête uniquement les conteneurs de build étiquetés avec le chemin de ce checkout.
stop_containers() {
    containers=$(docker ps -q --filter "label=splitergoll.build.repository=$REPOSITORY")

    if [ -z "$containers" ]; then
        printf '%s\n' 'Aucun conteneur de build actif pour ce checkout'
        return
    fi

    for container in $containers; do
        docker stop "$container"
    done
}

# Refuse le nettoyage si un conteneur restant utilise le dépôt ou ses fichiers locaux.
require_idle_checkout() {
    active_containers=$(docker ps -q)

    for container in $active_containers; do
        mounts=$(docker inspect --format '{{range .Mounts}}{{println .Source}}{{end}}' "$container")

        printf '%s\n' "$mounts" | while IFS= read -r source; do
            case "$source" in
                "$REPOSITORY"|"$LOCAL_ROOT"|"$LOCAL_ROOT/"*|"$OUTPUT_ROOT"|"$OUTPUT_ROOT/"*)
                    fail "Nettoyage refusé : le conteneur $container utilise encore ce checkout"
                    ;;
            esac
        done
    done
}

# Isole le cache selon le manifeste et l’image pour éviter de mélanger les versions.
resolve_workspace() {
    test -r "$REPOSITORY/config/west.yml" || fail 'Manifeste ZMK absent'
    identity=$(
        { cat "$REPOSITORY/config/west.yml"; printf '%s' "$BUILD_IMAGE"; } |
            shasum -a 256 | cut -d ' ' -f 1
    )
    prefix=$(printf '%s' "$identity" | cut -c 1-16)
    workspace="$WORKSPACE_ROOT/$prefix"
}

# Vérifie que le workspace est préparé et correspond à la configuration attendue.
require_setup() {
    test -f "$workspace/.splitergoll" || fail 'Lance make setup avant make build'
    test "$(cat "$workspace/.splitergoll")" = "$identity" ||
        fail 'Le workspace ne correspond pas au build figé'
    test -d "$workspace/.west" || fail 'Préparation incomplète : relance make setup'
    cmp "$REPOSITORY/config/west.yml" "$workspace/config/west.yml" ||
        fail 'Le manifeste local a changé : relance make setup'
}

# Lance l’action dans un conteneur éphémère, avec le dépôt en lecture seule.
# Les arguments après l’action sont transmis à Docker, par exemple pour couper le réseau.
run_container() {
    action=$1
    shift

    docker run --rm --init "$@" \
        --label "splitergoll.build.repository=$REPOSITORY" \
        --mount "type=bind,source=$REPOSITORY,target=$CONTAINER_SOURCE,readonly" \
        --mount "type=bind,source=$workspace,target=$CONTAINER_WORKSPACE" \
        --workdir "$CONTAINER_WORKSPACE" \
        --entrypoint /bin/sh "$BUILD_IMAGE" \
        "$CONTAINER_SOURCE/scripts/container.sh" "$action"
}
