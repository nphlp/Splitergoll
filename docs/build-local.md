# Build local

Le Makefile lance Docker, puis `west`, l’outil de build de ZMK/Zephyr.
CMake prépare la compilation et Ninja utilise le compilateur ARM pour produire
les fichiers UF2. GitHub Actions n’est pas nécessaire.

## Utilisation

Prérequis : Make, Bash, Docker démarré, `shasum` et un terminal macOS/Linux.
Sur Windows, utiliser WSL avec Docker. La validation Windows reste à faire.

```sh
make setup
make build
```

- `setup` télécharge l’image et les sources figées.
- `build` compile les deux moitiés dans un conteneur sans réseau.
- `stop` arrête les conteneurs de build de ce checkout, en conservant les fichiers.
- `clear` les arrête puis supprime `.local/` et `dist/` pour toutes les versions.
- `make -n build` affiche les commandes sans les exécuter.

Après le premier `setup`, `make build` suffit tant que les versions restent les mêmes.
Après `make clear`, recommencer par `make setup`. L’image Docker reste disponible.
Le nettoyage refuse les liens symboliques et tout conteneur restant qui utilise le dépôt.

Chaque commande utilise un conteneur éphémère, supprimé à sa sortie grâce à `--rm`.
Les logs restent visibles dans le terminal. Un label identifie le checkout pour
que `stop` cible ses seuls conteneurs de build.

Les quatre étapes affichent leur durée. En terminal interactif, les huit dernières
lignes défilent puis se replient après réussite. En cas d’échec, la sortie complète
est affichée. Sans terminal interactif, tous les logs défilent normalement.
Les journaux complets et les durées restent dans `.local/logs/` jusqu’au prochain `clear`.
Les validations sont vertes, les avertissements jaunes et les erreurs rouges.
Les couleurs sont désactivées sans terminal compatible ou avec `NO_COLOR=1`.

Si Docker affiche `DOCKER_INSECURE_NO_IPTABLES_RAW`, ce réglage vient de son moteur.
Il désactive certaines protections réseau iptables, pas la vérification des sources.
L’avertissement reste visible. Aucun port n’est publié par les conteneurs Splitergoll.
Voir [l’explication des mainteneurs Docker](https://github.com/moby/moby/pull/49621).
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
- `scripts/build-logs.sh` : affichage des étapes, durées et journaux complets.
- `scripts/setup.sh` : préparation des dépendances.
- `scripts/build.sh` : compilation et récupération des UF2.
- `scripts/container.sh` : commandes `west` et contrôles dans le conteneur.
- `scripts/stop.sh` : arrêt des conteneurs de build du checkout.
- `scripts/clear.sh` : arrêt et suppression des fichiers locaux générés.

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
Le nettoyage effectué avec `clear` a été vérifié avant une préparation complète.
Le setup passe ensuite en mode normal et interactif, avec les 39 dépendances vérifiées.
L’arrêt avec `stop` pendant un build reste à valider.
Voir [le matériel](configuration-materielle.md), [la keymap](keymap.md) et
[les tâches restantes](TODO.md).
