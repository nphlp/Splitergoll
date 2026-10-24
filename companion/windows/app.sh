#!/bin/bash

# Compile l’exécutable portable Windows ou teste son noyau depuis le checkout.
# Ne lance aucune app, aucun accès Bluetooth et aucun installateur.
set -euo pipefail

APP_SOURCE=$(CDPATH= cd "$(dirname "$0")" && pwd)
REPOSITORY=$(CDPATH= cd "$APP_SOURCE/../.." && pwd)
APP_CACHE="$REPOSITORY/.local/tools/windows-app"
CENTRAL_CONFIGURATION="$REPOSITORY/keyboard/boards/shields/splitergoll/splitergoll_left.conf"
DOTNET_COMMAND=${DOTNET_COMMAND:-}
if [ -z "$DOTNET_COMMAND" ]; then
    DOTNET_COMMAND=$(command -v dotnet || true)
    if [ -z "$DOTNET_COMMAND" ] && [ -x "$REPOSITORY/.local/windows-sdk/sdk/dotnet" ]; then
        DOTNET_COMMAND="$REPOSITORY/.local/windows-sdk/sdk/dotnet"
    fi
fi

# Refuse les actions implicites et explique les prérequis sans modifier le système.
usage() {
    printf '%s\n' 'Application Windows' '  make app build windows   Compiler le portable win-x64' '  make app test windows    Tester le noyau sans Windows ni Bluetooth' '  SDK .NET 10 requis sur PATH, ou indiqué par DOTNET_COMMAND.'
    exit "${1:-0}"
}

[ "$#" -eq 1 ] || usage 1
case "$1" in build|test) action=$1 ;; *) usage 1 ;; esac
[ -n "$DOTNET_COMMAND" ] && command -v "$DOTNET_COMMAND" >/dev/null || { printf '%s\n' 'SDK .NET 10 introuvable. Installe-le ou renseigne DOTNET_COMMAND.' >&2; exit 1; }
mkdir -p "$APP_CACHE"
export DOTNET_CLI_HOME="$APP_CACHE/cli"
export NUGET_HTTP_CACHE_PATH="$APP_CACHE/http-cache"
export NUGET_PLUGINS_CACHE_PATH="$APP_CACHE/plugin-cache"
export DOTNET_CLI_TELEMETRY_OPTOUT=1
export DOTNET_GENERATE_ASPNET_CERTIFICATE=false
export DOTNET_NOLOGO=1
export DOTNET_CLI_WORKLOAD_UPDATE_NOTIFY_DISABLE=true
sdk_version=$("$DOTNET_COMMAND" --version)
case "$sdk_version" in *-*) printf '%s\n' 'Les SDK de préversion ne sont pas utilisés pour cette application.' >&2; exit 1 ;; esac
case "$sdk_version" in 10.0.*) ;; *) printf '%s\n' 'Cette application nécessite un SDK .NET 10 stable.' >&2; exit 1 ;; esac

keyboard_name=$(sed -n 's/^CONFIG_ZMK_KEYBOARD_NAME="\([^"]*\)"$/\1/p' "$CENTRAL_CONFIGURATION")
[ -n "$keyboard_name" ] || { printf '%s\n' 'CONFIG_ZMK_KEYBOARD_NAME est absent.' >&2; exit 1; }
case "$keyboard_name" in *$'\n'*|*';'*|*'%'*) printf '%s\n' 'Nom Bluetooth incompatible avec les paramètres MSBuild.' >&2; exit 1 ;; esac

if [ "$action" = test ]; then
    "$DOTNET_COMMAND" run --project "$APP_SOURCE/BatteryCore.Tests/BatteryCore.Tests.csproj" --configuration Release
    "$DOTNET_COMMAND" build "$APP_SOURCE/UiTests/UiTests.csproj" --configuration Release --runtime win-x64 "-p:SplitergollKeyboardName=$keyboard_name"
    if [ "${OS:-}" = Windows_NT ]; then
        exec "$DOTNET_COMMAND" run --project "$APP_SOURCE/UiTests/UiTests.csproj" --configuration Release --runtime win-x64 --no-build "-p:SplitergollKeyboardName=$keyboard_name"
    fi
    printf '%s\n' 'SKIP : tests UI compilés, leur exécution exige Windows (WinForms natif).'
    exit 0
fi

mkdir -p "$REPOSITORY/dist/apps"
destination=$(mktemp -d "$REPOSITORY/dist/apps/windows-$(date -u +%Y-%m-%d-%H-%M-%S)-XXXXXX")
"$DOTNET_COMMAND" publish "$APP_SOURCE/Splitergoll/Splitergoll.csproj" --configuration Release \
    --runtime win-x64 --self-contained true --output "$destination" \
    "-p:SplitergollKeyboardName=$keyboard_name"
[ -f "$destination/Splitergoll.exe" ] || { printf '%s\n' 'Le build n’a pas produit Splitergoll.exe.' >&2; exit 1; }
if command -v shasum >/dev/null; then
    (cd "$destination" && shasum -a 256 Splitergoll.exe > SHA256SUMS)
elif command -v sha256sum >/dev/null; then
    (cd "$destination" && sha256sum Splitergoll.exe > SHA256SUMS)
else
    printf '%s\n' 'Installe shasum ou sha256sum pour contrôler le portable.' >&2; exit 1
fi
cat > "$destination/LIRE-MOI.txt" <<'GUIDE'
Splitergoll pour Windows, première version de test non signée

Copier Splitergoll.exe dans un dossier stable, puis double-cliquer pour lancer.
L’app reste près de l’horloge. Un clic ouvre les batteries, un clic droit ouvre le menu.
L’icône peut se trouver sous la flèche des icônes masquées de Windows.

Pour commencer sans Bluetooth : Splitergoll.exe --demo
Ou utiliser Démonstration sans Bluetooth dans le menu de l’icône.
Les niveaux 90/10 sont fictifs en démonstration, clairement signalée dans le panneau.

Pour les niveaux réels : V2 doit être déjà appairé et connecté à Windows.
Aucun flash, nouvel appairage, reset ou changement de destination de saisie par l’app.
Zéro ou valeur absente donne un tiret. Cent donne un éclair de charge possible.

Mise à jour : Quitter l’app, remplacer le fichier, relancer. Pas de réinstallation.
Le choix du clavier reste dans le dossier de données utilisateur de Splitergoll.
Lancer avec Windows est facultatif et se règle dans le menu.

SmartScreen peut avertir pour ce build non signé, y compris après une mise à jour.
La signature ou la réputation n’est pas modifiée et aucune protection n’est désactivée.
Si Smart App Control ou une politique bloque l’exécution, conserver le message exact.

Contrôle de l’exécutable avec PowerShell :
Get-FileHash .\Splitergoll.exe -Algorithm SHA256
Comparer à SHA256SUMS fourni avec ce build.
GUIDE
printf 'Application portable : %s\n' "$destination/Splitergoll.exe"
printf '%s\n' 'Windows : quitter la version active, remplacer le fichier et relancer.' 'Démonstration sans Bluetooth : Splitergoll.exe --demo' 'Exécutable non signé. Aucune protection Windows modifiée.'
