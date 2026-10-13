# Build local

Le Makefile lance Docker, puis `west`, l’outil de build de ZMK/Zephyr.
CMake prépare la compilation et Ninja utilise le compilateur ARM pour produire
les fichiers UF2. GitHub Actions n’est pas nécessaire.

## Utilisation

Prérequis : Make, Docker démarré, `shasum` et un terminal macOS/Linux.
Sur Windows, utiliser WSL avec Docker. La validation Windows reste à faire.

```sh
make setup
make build
```

- `setup` télécharge l’image et les sources figées.
- `build` compile les deux moitiés dans un conteneur sans réseau.
- `make -n build` affiche les commandes sans les exécuter.

Les conteneurs sont éphémères et les logs restent visibles dans le terminal.
Le dépôt est monté en lecture seule. Les dépendances et caches sont dans `.local/`.
ZMK 0.3.0, Zephyr et leurs dépendances sont verrouillés dans `config/west.yml`.
L’image officielle est figée par son digest dans `scripts/build-common.sh`.

Ces hashes identifient les commits et l’image exacts pour retrouver le même
environnement à chaque build. La version ZMK retenue correspond à `v0.3.0`.
Le manifeste détaille les 59 projets normalement chargés par les imports
ZMK/Zephyr, afin de verrouiller aussi les dépendances indirectes.

## Organisation

Le Makefile expose les commandes, les scripts contiennent leur logique :

- `scripts/build-common.sh` : constantes de chemins et d’image, contrôles et lancement Docker.
- `scripts/setup.sh` : préparation des dépendances.
- `scripts/build.sh` : compilation et récupération des UF2.
- `scripts/container.sh` : commandes `west` et contrôles dans le conteneur.

Les chemins locaux sont calculés depuis le dépôt. Aucun `.env` n’est nécessaire.

## Résultat

Un nouveau dossier sous `dist/firmware/cable-multi/` contient :

- `splitergoll-cable-multi-left.uf2`
- `splitergoll-cable-multi-right.uf2`
- les empreintes SHA-256 et l’identifiant de l’environnement utilisé

Les caches sont séparés par version et par moitié. Les dépendances modifiées
bloquent le build. Les rôles USB/BLE sont contrôlés après compilation.
Le build ne flashe rien et ne supprime aucun appairage.

Validation : `make setup` et `make build` passent sur le Mac ARM après le découpage
en scripts. Les deux UF2 et les rôles sont vérifiés. Il reste l’essai matériel.
Voir [le matériel](configuration-materielle.md), [la keymap](keymap.md) et
[les tâches restantes](TODO.md).
