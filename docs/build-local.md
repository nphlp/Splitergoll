# Build local

Le Makefile lance Docker, puis `west`, l’outil de build de ZMK/Zephyr.
CMake prépare la compilation et Ninja utilise le compilateur ARM pour produire
les fichiers UF2. GitHub Actions n’est pas nécessaire.
Le workflow GitHub Actions est uniquement manuel : aucun build n’est lancé lors
d’un push ou d’une pull request. Sa matrice est conservée en complément du build local.

## Utilisation

Prérequis : Make, Bash, Docker démarré, `shasum` et un terminal macOS/Linux.
Sur Windows, utiliser WSL avec Docker. La validation Windows reste à faire.

Commande : `make build <matériel> <keymap> [debug]`.
Le matériel et la keymap sont obligatoires, dans cet ordre. Aucun choix par défaut.

```sh
make setup
make build wire macos
make build bluetooth windows debug
```

- `setup` télécharge l’image et les sources figées.
- Le matériel `wire` choisit une sortie USB à gauche, `bluetooth` une sortie Bluetooth directe.
- La keymap doit être `multi`, `windows`, `macos` ou `linux`.
- `windows` comprend le gaming. Les profils macOS et Linux restent sur leur OS.
- Ajouter `debug` après la keymap active les logs USB, voir [logs-usb.md](logs-usb.md).
- `build` seul affiche l’aide, sans compilation ni Docker. Un choix incomplet ou invalide échoue avant Docker.
- `stop` arrête les conteneurs de build de ce checkout, en conservant les fichiers.
- `clear` les arrête puis supprime `.local/` et `dist/` pour toutes les versions.
- `make -n build bluetooth multi` affiche les commandes sans les exécuter.
- `make help` affiche l’aide colorée. Les commandes batterie sont décrites dans [batteries.md](batteries.md).

Après le premier `setup`, choisir le profil à compiler tant que les versions restent les mêmes.
Un seul `setup` prépare les deux profils. Lancer la compilation
avec `make build wire macos` ou `make build bluetooth windows`, par exemple.
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
- `scripts/flash.sh` et `uf2-check.awk` : sélection, contrôle et copie d’un UF2 après confirmation sur macOS, voir [le flash](flash-firmware.md).
- `scripts/container.sh` : commandes `west` et contrôles dans le conteneur.
- `scripts/stop.sh` : arrêt des conteneurs de build du checkout.
- `scripts/clear.sh` : arrêt et suppression des fichiers locaux générés.

Les chemins locaux sont calculés depuis le dépôt. Aucun `.env` n’est nécessaire.
`make build bluetooth multi` appelle `build.sh bluetooth multi`, `make bat avg` appelle
`batteries.sh avg`. Les scripts refusent les arguments qu’ils ne prennent pas en charge.

## Profils matériels

| Profil | Sortie de la gauche | Veille profonde | Batterie |
| --- | --- | --- | --- |
| `wire` | USB vers l’ordinateur | Désactivée | Reporting désactivé |
| `bluetooth` | Bluetooth direct, même pendant la charge USB | Après 15 minutes sur batterie | Reporting activé |

Dans les deux cas, la droite communique avec la gauche en BLE. Le choix de keymap
est indépendant : `multi` conserve la référence de `dongle`, les autres profils
gardent un seul OS. `config/splitergoll.conf` contient le socle commun,
les fichiers des moitiés fixent leurs rôles, et `config/profiles/<profil>.conf`
est ajouté en dernier via `EXTRA_CONF_FILE`. Le nom du profil est transmis
au script Docker comme argument. Les versions ZMK et l’image restent identiques.
Le build charge aussi `config/profiles/<profil matériel>-<moitié>.conf` lorsqu’il
existe. `bluetooth-left.conf` active la récupération et le proxy de batterie sur
la centrale seulement. L’option `debug` ajoute le snippet natif `zmk-usb-logging`
au même profil. Elle a ses propres caches et sorties, sans changer les délais.
`KEYMAP_FILE` désigne `config/keymaps/<keymap>.keymap` dans ZMK. La commande vérifie
que CMake a bien utilisé ce fichier avant d’exporter les UF2.
`make build` seul affiche l’aide. Chaque compilation demande un matériel et une keymap explicites.
`dongle` sera ajouté plus tard.

## Résultat

Chaque build crée un dossier `dist/firmware/<matériel>-<keymap>-AAAA-MM-JJ-HH-MM-SS/`,
avec un horodatage utilisant l’heure locale de la machine.
Le dossier contient :

- `splitergoll-<matériel>-<keymap>-left.uf2`
- `splitergoll-<matériel>-<keymap>-right.uf2`
- `SHA256SUMS` : empreintes des deux firmwares
- `build-info.txt` : matériel, keymap, diagnostic et identifiant de l’environnement utilisé

Avec `debug`, le nom du profil porte le suffixe `-debug` dans le dossier et les UF2,
par exemple `bluetooth-macos-debug-AAAA-MM-JJ-HH-MM-SS/` et
`splitergoll-bluetooth-macos-debug-left.uf2`. Le nom inclut toujours la keymap,
y compris `multi`, pour éviter de confondre les firmwares.

Si deux exports utilisent la même seconde, le second échoue sans écraser le premier.
Relancer la même commande de build avec son profil pour obtenir un nouvel horodatage.

Les caches sont séparés par version, matériel, keymap, diagnostic et moitié. Les dépendances modifiées
bloquent le build. Les rôles USB/BLE sont contrôlés après compilation.
Le build ne flashe rien et ne supprime aucun appairage.

Validation : `make setup` et les builds explicites passent sur le Mac ARM après le découpage
en scripts. Les deux profils produisent chacun deux UF2 vérifiés. La saisie Bluetooth
des deux moitiés est confirmée sur le Mac, avec alimentation USB et sur batterie seule.
Le profil d’essai confirme un réveil sans reset sur chaque moitié, avec un premier
appui sans saisie. La saisie USB `wire` sur le Mac est aussi confirmée avec la
keymap `multi`. Le flash automatique et le fonctionnement des deux moitiés avec
`wire macos` sont aussi confirmés. Les essais complets des touches et encodeurs,
Windows et les autres keymaps fixes restent à faire.
Les profils normaux compilent, avec les délais et le service auxiliaire batterie
contrôlés. Les deux mesures ont aussi été lues sur le Mac. Le profil permanent
aux délais courts est retiré après les essais.
Les quatre keymaps compilent pour les deux matériels, avec comparaison des
affectations compilées. `multi` conserve ses couches, macros et encodeurs.
Avec deux variantes debug représentatives, vingt UF2 sont contrôlés : empreintes,
options, métadonnées et adresses. Les avertissements connus restent ceux du moteur
Docker et du bootloader ZMK 0.3.0.
La lecture des nouvelles consoles reste à tester sur le clavier.
Ces contrôles ne remplacent pas les essais matériels restants.
Le nettoyage effectué avec `clear` a été vérifié avant une préparation complète.
Le setup passe ensuite en mode normal et interactif, avec les 39 dépendances vérifiées.
L’arrêt avec `stop` pendant un build reste à valider.
Voir [le matériel](configuration-materielle.md), [la keymap](keymap.md) et
[les tâches restantes](TODO.md).
