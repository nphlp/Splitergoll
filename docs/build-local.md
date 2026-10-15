# Build local

Le Makefile lance Docker, puis `west`, l’outil de build de ZMK/Zephyr.
CMake prépare la compilation et Ninja utilise le compilateur ARM pour produire
les fichiers UF2. GitHub Actions n’est pas nécessaire.

## Utilisation

Prérequis : Make, Bash, Docker démarré, `shasum` et un terminal macOS/Linux.
Sur Windows, utiliser WSL avec Docker. La validation Windows reste à faire.

```sh
make setup
make build bluetooth
```

- `setup` télécharge l’image et les sources figées.
- `build wire` compile les deux moitiés pour une sortie USB à gauche.
- `build bluetooth` compile les deux moitiés pour une sortie Bluetooth directe.
- `build wire debug` et `build bluetooth debug` ajoutent les logs USB, voir [logs-usb.md](logs-usb.md).
- `build` sans profil affiche les profils disponibles, sans compilation ni Docker.
- `stop` arrête les conteneurs de build de ce checkout, en conservant les fichiers.
- `clear` les arrête puis supprime `.local/` et `dist/` pour toutes les versions.
- `make -n build bluetooth` affiche les commandes sans les exécuter.
- `make help` affiche l’aide colorée. Les commandes batterie sont décrites dans [batteries.md](batteries.md).

Après le premier `setup`, choisir le profil à compiler tant que les versions restent les mêmes.
Un seul `setup` prépare les deux profils. Lancer la compilation
avec `make build wire` ou `make build bluetooth`.
Chaque profil conserve son propre dossier de sortie et ses journaux.
Après `make clear`, recommencer par `make setup`. L’image Docker reste disponible.
Le nettoyage refuse les liens symboliques et tout conteneur restant qui utilise le dépôt.

Chaque action Docker utilise un conteneur éphémère, supprimé à sa sortie grâce à `--rm`.
Chaque build explicite compile les deux moitiés du profil choisi.
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

Chaque commande a sa propre cible dans le Makefile et appelle directement son script.
Les mots après la commande sont ses arguments, vérifiés dans le script :

- `scripts/help.sh` et `terminal-colors.sh` : aide colorée et palette partagée avec les journaux.
- `scripts/build-common.sh` : constantes de chemins et d’image, contrôles et lancement Docker.
- `scripts/build-logs.sh` : affichage des étapes, durées et journaux complets.
- `scripts/setup.sh` : préparation des dépendances.
- `scripts/build.sh` : compilation et récupération des UF2.
- `scripts/container.sh` : commandes `west` et contrôles dans le conteneur.
- `scripts/stop.sh` : arrêt des conteneurs de build du checkout.
- `scripts/clear.sh` : arrêt et suppression des fichiers locaux générés.

Les chemins locaux sont calculés depuis le dépôt. Aucun `.env` n’est nécessaire.
`make build bluetooth` appelle `build.sh bluetooth`, `make bat avg` appelle
`batteries.sh avg`. Les scripts refusent les arguments qu’ils ne prennent pas en charge.

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
Le build charge aussi `config/profiles/<profil matériel>-<moitié>.conf` lorsqu’il
existe. `bluetooth-left.conf` active la récupération et le proxy de batterie sur
la centrale seulement. L’option `debug` ajoute le snippet natif `zmk-usb-logging`
au même profil. Elle a ses propres caches et sorties, sans changer les délais.
`make build` seul affiche l’aide. Chaque compilation demande un profil explicite.
`dongle` sera ajouté plus tard.

## Résultat

Chaque build crée un dossier `dist/firmware/<profil>-AAAA-MM-JJ-HH-MM-SS/`,
avec un horodatage utilisant l’heure locale de la machine.
Le dossier contient :

- `splitergoll-<profil>-left.uf2`
- `splitergoll-<profil>-right.uf2`
- `SHA256SUMS` : empreintes des deux firmwares
- `build-info.txt` : profil et identifiant de l’environnement utilisé

Avec `debug`, le nom du profil porte le suffixe `-debug` dans le dossier et les UF2,
par exemple `bluetooth-debug-AAAA-MM-JJ-HH-MM-SS/` et
`splitergoll-bluetooth-debug-left.uf2`. `build-info.txt` distingue le profil matériel,
le mode de compilation et la variante. Les builds normaux gardent leurs noms.

Si deux exports utilisent la même seconde, le second échoue sans écraser le premier.
Relancer la même commande de build avec son profil pour obtenir un nouvel horodatage.

Les caches sont séparés par version, profil et moitié. Les dépendances modifiées
bloquent le build. Les rôles USB/BLE sont contrôlés après compilation.
Le build ne flashe rien et ne supprime aucun appairage.

Validation : `make setup` et les builds explicites passent sur le Mac ARM après le découpage
en scripts. Les deux profils produisent chacun deux UF2 vérifiés. La saisie Bluetooth
des deux moitiés est confirmée sur le Mac, avec alimentation USB et sur batterie seule.
Le profil d’essai confirme un réveil sans reset sur chaque moitié, avec un premier
appui sans saisie. Toutes les touches, les encodeurs et le profil `wire` restent à tester.
Les profils normaux compilent, avec les délais et le service auxiliaire batterie
contrôlés. Les deux mesures ont aussi été lues sur le Mac. Le profil permanent
aux délais courts est retiré après les essais.
Les quatre paires normales/debug compilent et leurs options sont contrôlées.
La lecture des nouvelles consoles reste à tester sur le clavier.
Ces contrôles ne remplacent pas les essais matériels restants.
Le nettoyage effectué avec `clear` a été vérifié avant une préparation complète.
Le setup passe ensuite en mode normal et interactif, avec les 39 dépendances vérifiées.
L’arrêt avec `stop` pendant un build reste à valider.
Voir [le matériel](configuration-materielle.md), [la keymap](keymap.md) et
[les tâches restantes](TODO.md).
