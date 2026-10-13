#!/bin/sh

# Définit les constantes et fonctions partagées du build local.
# Ce fichier est chargé par les autres scripts, il ne se lance pas directement.

# Chemins relatifs au dépôt et image officielle figée.
SCRIPT_DIRECTORY=$(CDPATH= cd "$(dirname "$0")" && pwd)
REPOSITORY=$(CDPATH= cd "$SCRIPT_DIRECTORY/.." && pwd)
BUILD_IMAGE='zmkfirmware/zmk-build-arm:3.5@sha256:0979a8de325e3d822e48bd8d1d9efaee2984ad3750253119f920a342f0ce5cde'
WORKSPACE_ROOT="$REPOSITORY/.local/zmk"
ARTIFACT_ROOT="$REPOSITORY/dist/firmware"
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

# Vérifie les outils requis, le digest de l’image et l’accès au moteur Docker.
preflight() {
    for command in docker shasum; do
        command -v "$command" >/dev/null || fail "Prérequis absent : $command"
    done

    printf '%s\n' "$BUILD_IMAGE" | grep -Eq '@sha256:[a-f0-9]{64}$' ||
        fail 'Image Docker non figée par digest'
    docker info >/dev/null
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
        --mount "type=bind,source=$REPOSITORY,target=$CONTAINER_SOURCE,readonly" \
        --mount "type=bind,source=$workspace,target=$CONTAINER_WORKSPACE" \
        --workdir "$CONTAINER_WORKSPACE" \
        --entrypoint /bin/sh "$BUILD_IMAGE" \
        "$CONTAINER_SOURCE/scripts/container.sh" "$action"
}
