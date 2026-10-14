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
- `build wire` compile les deux moitiés pour une sortie USB à gauche.
- `build bluetooth` compile les deux moitiés pour une sortie Bluetooth directe.
- `build` sans profil compile `wire`, puis `bluetooth`, soit quatre UF2.
- `stop` arrête les conteneurs de build de ce checkout, en conservant les fichiers.
- `clear` les arrête puis supprime `.local/` et `dist/` pour toutes les versions.
- `make -n build bluetooth` affiche les commandes sans les exécuter.

Après le premier `setup`, `make build` suffit tant que les versions restent les mêmes.
Un seul `setup` prépare les deux profils. Pour limiter le build, choisir un profil
avec `make build wire` ou `make build bluetooth`.
Chaque profil conserve son propre dossier de sortie et ses journaux.
Après `make clear`, recommencer par `make setup`. L’image Docker reste disponible.
Le nettoyage refuse les liens symboliques et tout conteneur restant qui utilise le dépôt.

Chaque action Docker utilise un conteneur éphémère, supprimé à sa sortie grâce à `--rm`.
`make build` lance les deux compilations successivement, une par profil.
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
Le diagnostic sur le moteur OrbStack n’a pas identifié de réglage exposé pour
corriger ce drapeau. Ce point reste ouvert côté moteur, sans modification système.

L’avertissement `NRF_STORE_REBOOT_TYPE_GPREGRET` vient de ZMK 0.3.0, qui utilise
encore ce mécanisme pour la touche bootloader. Le supprimer demande une adaptation
du firmware. Voir [le bootloader](flash-firmware.md#touche-bootloader).

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

## Profils matériels

| Profil | Sortie de la gauche | Veille profonde | Batterie |
| --- | --- | --- | --- |
| `wire` | USB vers l’ordinateur | Désactivée | Reporting désactivé |
| `bluetooth` | Bluetooth direct, même pendant la charge USB | Après 15 minutes sur batterie | Reporting activé |

Dans les deux cas, la droite communique avec la gauche en BLE. La gauche applique
la même keymap de `dongle`. `config/splitergoll.conf` contient le socle commun,
les fichiers des moitiés fixent leurs rôles, et `config/profiles/<profil>.conf`
est ajouté en dernier via `EXTRA_CONF_FILE`. Le nom du profil est transmis
au script Docker comme argument. Les versions ZMK et l’image restent identiques.
`dongle` sera ajouté plus tard.

## Résultat

Chaque build crée un dossier `dist/firmware/<profil>-AAAA-MM-JJ-HH-MM-SS/`,
avec un horodatage utilisant l’heure locale de la machine.
Le dossier contient :

- `splitergoll-<profil>-left.uf2`
- `splitergoll-<profil>-right.uf2`
- `SHA256SUMS` : empreintes des deux firmwares
- `build-info.txt` : profil et identifiant de l’environnement utilisé

Si deux exports utilisent la même seconde, le second échoue sans écraser le premier.
Relancer `make build` pour obtenir un nouvel horodatage.

Les caches sont séparés par version, profil et moitié. Les dépendances modifiées
bloquent le build. Les rôles USB/BLE sont contrôlés après compilation.
Le build ne flashe rien et ne supprime aucun appairage.

Validation : `make setup` et `make build` passent sur le Mac ARM après le découpage
en scripts. Les deux profils produisent chacun deux UF2 vérifiés. La saisie Bluetooth
des deux moitiés est confirmée sur le Mac, avec alimentation USB et sur batterie seule.
Réveil et première frappe restent à tester, ainsi que le profil `wire`.
Le nettoyage effectué avec `clear` a été vérifié avant une préparation complète.
Le setup passe ensuite en mode normal et interactif, avec les 39 dépendances vérifiées.
L’arrêt avec `stop` pendant un build reste à valider.
Voir [le matériel](configuration-materielle.md), [la keymap](keymap.md) et
[les tâches restantes](TODO.md).
