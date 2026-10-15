# Splitergoll

## Intention

Une configuration ZMK complète, commune et compilable en local. Projet classé
`sync` avec les règles globales. Branche de travail : `unified-firmware`.

Le [futur README](docs/README.md) indexe les guides.
Il reste dans `docs/` jusqu’à la mise à jour finale. Les guides
courts couvrent [le build](docs/build-local.md), [le matériel](docs/configuration-materielle.md)
et [la keymap](docs/keymap.md). Les tâches et idées sont centralisées dans
[docs/TODO.md](docs/TODO.md), à la demande de l’utilisateur.

Tous les guides restent à plat dans `docs/`, avec un fichier par sujet et sans
date dans les noms ou les titres. Ils restent courts et utiles à l’usage.
L’AGENTS.md garde les invariants, preuves techniques et décisions utiles au prochain chantier.

## Invariants

- Deux claviers : PC Windows alimenté par deux USB, MacBook sur batteries en Bluetooth direct.
  Le XIAO BLE reste disponible pour le profil dongle historique.
- Les moitiés sont des répliques de nice!nano v2. Le Nordic des photos n’est pas utilisé.
- La keymap complète de `dongle` à `2584ca4` fait référence : conserver les 44 positions,
  accents, Shift, macros, couches et encodeurs. Les helpers, transforms et GPIO sont inchangés.
- La keymap actuelle constitue `multi`, avec les trois OS et le jeu. Avant son découpage,
  conserver une sauvegarde versionnée de la keymap, des couches, helpers et dispositions.
  Les futurs profils sont `windows` avec jeu, `macos`, `linux` et `multi`. Ce chantier reste futur.
- Les branches `-minimal` sont des archives, exclues du chantier.
- Les branches historiques restent intactes. `wire` archive `main` à `cbad9a0`,
  `wire-minimal` archive `main-minimal` à `27ed2b7`.
- Le README reste au contenu validé du commit `a1f6c35` jusqu’à sa mise à jour finale.
- Distinguer l’OS, le rôle central/périphérique et la sortie USB/Bluetooth.
- Le BLE entre moitiés reste nécessaire avec le câblage actuel. Le Bluetooth direct
  fonctionne sur le Mac avec alimentation USB et sur batterie seule. Veille et réveil
  sont testés sur des touches représentatives, toutes les touches et encodeurs restent à vérifier.
  La priorité USB est un profil futur.

## Étape active et validation

Build local et profil `wire` : gauche centrale USB, droite périphérique BLE,
alimentation USB, sans veille profonde ni reporting batterie.
Profil `bluetooth` : mêmes rôles, sortie Bluetooth directe de la gauche, USB clavier
désactivé même pendant la charge, veille après 15 minutes et reporting batterie activés.
Le profil aux délais courts a été supprimé après validation matérielle. Les deux
profils disponibles sont `wire` et `bluetooth`. L’option transversale `debug` ajoute
le snippet natif `zmk-usb-logging` à chaque moitié, sans toucher aux timings, rôles
ou keymaps. `BUILD_MODE` vaut `normal` ou `debug`, `BUILD_VARIANT` nomme le profil
avec suffixe `-debug` uniquement en diagnostic. Caches et artefacts sont séparés,
et les métadonnées indiquent le mode. Valider les quatre paires de builds et conserver
les contrôles d’absence de reset dans tous les modes. Le logging doit être présent
en debug et absent en normal. Le guide court est [logs-usb.md](docs/logs-usb.md).
Avec l’ancien firmware aux délais courts, l’utilisateur confirme une reprise immédiate après 10 secondes
d’inactivité et une déconnexion après une minute, suivie d’une reconnexion en environ
3 secondes sur appui, sans reset. Son essai séparé confirme le réveil local gauche
puis droite : le premier appui n’écrit rien, le second écrit après reconnexion.
Dans l’ordre inverse, la droite se réveille mais ne transmet rien tant que la gauche
dort. Réveiller la gauche permet à la droite de se reconnecter. Le réveil depuis la
droite seule n’est donc pas fonctionnel quand la centrale est en veille profonde.
Ne pas prétendre que toutes les touches ou les encodeurs ont été testés.
Le choix macOS revient à Windows après une veille profonde, confirmé par l’utilisateur.
Dans ZMK 0.3.0, `activity.c` appelle `sys_poweroff()`, l’état des couches et la couche
par défaut sont des variables initialisées à zéro dans `keymap.c`. `&to` ne sauvegarde
pas cet état. Windows correspond à la couche initiale zéro. Les appairages NVS sont
distincts et conservés. La persistance du choix OS ou son initialisation au build
restent à définir lors du découpage, sans modifier la keymap avant sa sauvegarde.
Le menu Bluetooth macOS affiche un seul niveau (63 % lors de l’essai).
Un lecteur temporaire Swift/CoreBluetooth a identifié deux services `180F` et
deux caractéristiques `2A19`. Les descripteurs `2904` indiquent `0x0106` (principale)
et `0x0108` (auxiliaire), le descripteur `2901` du proxy indique `Peripheral 0`.
Six lectures par service donnent gauche 63 % et droite 62 %. Le proxy et la liaison
de mesure droite sont donc validés. Les notifications ne sont pas testées, le lecteur
effectue uniquement des lectures et s’arrête après 30 secondes. Il réutilise le clavier
déjà connecté et ferme sa connexion cliente, sans modifier les appairages.
Le diagnostic initial reste dans `.local/diagnostic/`. La lecture durable est
versionnée dans `scripts/batteries.sh`, `batteries.swift` et `batteries-info.plist`.
`make bat` retourne dès que deux valeurs identifiées sont reçues. `make bat avg`
démarre une fenêtre de 30 secondes après réception de la première paire, relit
toutes les 5 secondes et calcule une moyenne arithmétique par moitié, sans combiner
les deux batteries. Ce sont les valeurs annoncées, pas des mesures ADC forcées :
le profil normal les actualise toutes les 60 secondes. La première connexion et
lecture est limitée à 15 secondes, les erreurs rendent un statut non nul.
Le lecteur nécessite macOS et Swift, compile automatiquement dans
`.local/tools/batteries/` si ses sources ou métadonnées changent, sans Docker.
Son nom cible est lu dans la configuration de la centrale, sans nom par défaut inventé.
L’accès Bluetooth demande la permission macOS, et hors sandbox pour l’agent.
Ne modifier ni les appairages ni les abonnements aux notifications pour lire.
Valider la compilation Swift, la lecture réelle et les erreurs de CLI, sans nouvelle
suite de tests d’outillage. Un affichage permanent dans la barre des menus reste futur.
Les deux commandes sont validées sur le Mac avec USB de charge branchés : lecture
rapide gauche/droite 100 % en moins d’une seconde, puis moyennes 100,0 % sur six
lectures par moitié. Compilation Swift sans avertissement, syntaxe Bash macOS et
validation des commandes compatibles et incompatibles contrôlées.
Essai matériel sur le Mac : les deux moitiés fonctionnent avec les UF2 Bluetooth
normaux, confirmé par l’utilisateur. Les USB étaient branchés pour l’alimentation,
la saisie reste en Bluetooth. L’utilisateur confirme aussi la saisie sur batterie
seule, sans USB ni dongle. Les résultats de veille sont décrits ci-dessus.
Toutes les touches et les encodeurs restent à valider séparément.
Après les essais, l’utilisateur confirme avoir remis les deux UF2 `bluetooth`
normaux avec proxy batterie, sans reset des réglages. Les deux USB restent branchés
pour charger. Inactivité 30 secondes, veille profonde 15 minutes et mesure batterie
60 secondes sont les paramètres compilés. Ne pas présenter le délai de 15 minutes
comme un nouvel essai matériel : l’alimentation USB bloque actuellement cette veille.

Un appairage incohérent bloquait la droite : clé restaurée, renouvellement refusé,
erreur de sécurité 4. Le firmware et le scan des touches étaient corrects.
Le reset officiel a été flashé par l’utilisateur sur les deux moitiés, avec
effacement NVS confirmé dans les logs, puis les UF2 normaux remis droite et gauche.
Le nouvel appairage avec le Mac a résolu le blocage, sans modification de keymap
ni des protections Bluetooth. Vérifier le firmware et les logs avant de changer
des options radio ou de répéter des resets. Garder les deux moitiés sous le reset
jusqu’au contrôle des deux effacements, puis remettre les firmwares.
Les artefacts temporaires de diagnostic et de reset restent dans `.local/flash/`.
La [procédure d’appairage](docs/appairage-bluetooth.md) reste lisible pour l’utilisateur.
Les preuves techniques sont conservées ci-dessous.
Sur le clavier Mac, l’interrupteur coupe la batterie mais pas l’alimentation USB.
Une moitié réellement hors tension a son USB débranché et sa batterie coupée.

Le reset nice!nano v2 est indépendant du profil matériel. Les builds reset,
gauche et droite utilisent la même partition NVS à `0xEC000`, sur 32 Kio.
Le binaire temporaire reste dans `.local/flash/`, à la demande de l’utilisateur.

Les nouveaux builds Bluetooth activent récupération et proxy de batterie droite
sur la centrale via `bluetooth-left.conf`. La droite conserve son service de mesure.
Le service principal expose la gauche, l’auxiliaire la droite. L’affichage de deux
valeurs par le menu natif macOS n’est pas observé, malgré deux mesures lues via CoreBluetooth,
voir [batteries.md](docs/batteries.md).
Les profils `wire` ne récupèrent ni ne transmettent ces mesures.

La matrice `kscan0` porte `wakeup-source` pour permettre un réveil par les touches.
Ne pas activer la veille dans le profil câble. Le profil Bluetooth active `ZMK_SLEEP`.
Vérifier sur le matériel chaque touche, la première frappe et la reconnexion.
ZMK 0.3.0 bloque la veille profonde lorsqu’une alimentation USB est détectée,
quel que soit le profil de sortie. Les délais partent de la dernière activité et
sont contrôlés toutes les secondes. Les moitiés ont des activités indépendantes.
Avec les deux en veille profonde, la droite ne peut pas réveiller la centrale.
Décider après les tests si la centrale doit rester en inactivité pour permettre
une reprise depuis la droite, sans promettre ce comportement avec sa radio coupée.
Le guide `docs/veille.md` distingue les capacités de ZMK des essais matériels.
`docs/flash-firmware.md` couvre le flash et le bootloader, `docs/appairage-bluetooth.md`
couvre les clés de connexion et le reset.
Le mécanisme bootloader déprécié reste nécessaire à ZMK 0.3.0 : son remplacement
demande l’API de rétention, l’adaptation du comportement reset et des codes UF2.
Ne pas désactiver `NRF_STORE_REBOOT_TYPE_GPREGRET` pour faire disparaître le warning.

Le Makefile expose `help`, `setup`, `build wire`, `build bluetooth`, `bat`, `bat avg`,
`stop` et `clear`, délégués aux scripts dédiés. Le second argument `debug` active
les logs USB du profil choisi, sans cibles Make supplémentaires.
Chaque commande a sa propre cible et son appel de script dans le Makefile.
À la demande de l’utilisateur, le Makefile appelle directement chaque script avec
ses arguments, sans validateur central ni variable d’environnement intermédiaire.
`ARGS` retire le nom de la cible et protège chaque mot pour le shell. Chaque script
vérifie son nombre d’arguments et ses valeurs avant toute action. `setup`, `stop`
et `clear` n’acceptent aucun argument. `help.sh` accepte le périmètre interne `build`
pour afficher les profils, l’aide générale reste `make help`. La règle `%` ne fait rien : Make voit
les arguments comme d’autres cibles, déjà traitées par le script de la commande.
Un mot inconnu seul est donc sans action. Ne pas ajouter de dispatch central ni
de cibles individuelles pour chaque valeur d’argument.
`make build` seul affiche les profils disponibles via `help.sh build`, sans Docker
ni préparation requise. La compilation demande un profil explicite : `wire`,
`bluetooth`, avec `debug` en option. Les deux moitiés du seul profil choisi sont compilées.
`help.sh` affiche l’aide groupée, avec la palette partagée de `terminal-colors.sh`.
Les constantes partagées sont en tête de `scripts/build-common.sh`. `container.sh`
exécute `west` et vérifie les firmwares. Aucun `.env` n’est nécessaire.
ZMK 0.3.0 et les
59 projets résolus sont figés dans `config/west.yml`. Les workspaces sont séparés
par empreinte de l’environnement, les caches CMake par moitié.
Les sorties sont dans `dist/firmware/<profil>-AAAA-MM-JJ-HH-MM-SS/`, en heure locale,
sans suffixe aléatoire. Refuser un dossier existant plutôt que l’écraser.
Les UF2 sont nommés `splitergoll-<profil>-left.uf2` et `splitergoll-<profil>-right.uf2`.
`dongle` nommera un futur profil, il n’est pas encore compilable.

`config/splitergoll.conf` contient les options communes. Les fichiers `.conf` des
moitiés fixent leurs rôles. `EXTRA_CONF_FILE` charge le profil matériel, puis une
configuration de moitié optionnelle. Le snippet USB logging est ajouté avec
`west build -S zmk-usb-logging` en mode debug uniquement.
Le profil est validé côté hôte et dans le conteneur, les caches sont séparés par profil
et moitié. Les contrôles vérifient les délais, la mesure et le proxy gauche, ainsi
que l’absence de reset d’appairage dans tous les UF2 clavier. Les contrôles imposent
les logs USB et leur console CDC en debug, et leur absence en normal.
Les huit UF2 normaux/debug compilent et passent ces contrôles. Empreintes SHA-256,
famille nRF52840 et adresses de firmware sont vérifiées. Les consoles de ces nouveaux
builds restent à tester sur le matériel, sans flash implicite. Les conteneurs de
validation ont disparu, les trois services préexistants sont conservés.
La matrice `build.yaml` couvre les quatre UF2 normaux. Sa liste de fichiers CMake
est entre guillemets pour que le point-virgule ne soit pas interprété par le shell.
Elle utilise `GITHUB_WORKSPACE` fourni par GitHub.

Valider la syntaxe Make/shell, les erreurs de sélection, le manifeste, la conservation
de la keymap et les deux builds réels. Pas de nouveaux tests automatisés d’outillage
par défaut. Les essais
matériels restent distincts. Aucun flash implicite. Les conteneurs sont éphémères,
le dépôt est monté en lecture seule et la compilation n’a pas de réseau.
Stabiliser les scripts avant le lancement et ne pas modifier les fichiers montés
pendant leur exécution : le montage en lecture seule n’isole pas les changements hôtes.

Les conteneurs portent le label `splitergoll.build.repository` avec le chemin du
checkout. `stop` ne cible que ce label. `clear` doit réussir cet arrêt avant de
supprimer `.local/` et `dist/`, toutes versions comprises. Refuser les chemins
inattendus, les liens symboliques et les conteneurs restants qui utilisent le dépôt.
L’image Docker partagée est conservée.
Ne pas exécuter le nettoyage pour valider les scripts sans accord explicite.

Chaque fonction des scripts doit avoir un commentaire court en français au-dessus,
qui explique son rôle et les arguments lorsque leur sens n’est pas évident.
Chaque script doit aussi expliquer son rôle dans un commentaire en tête de fichier.

Les scripts hôtes `setup.sh` et `build.sh` utilisent Bash pour l’affichage repliable,
isolé dans `build-logs.sh`. `help.sh`, `batteries.sh` et
`terminal-colors.sh` utilisent également Bash. Les autres scripts restent compatibles `sh`.
Conserver le statut réel des commandes et tous leurs logs dans `.local/logs/`.
En terminal interactif, replier uniquement les étapes réussies et garder les
avertissements visibles. Sans TTY, diffuser la sortie complète sans codes de curseur.
Afficher la durée par étape et totale. Ne pas masquer les avertissements du moteur Docker.
Utiliser des couleurs sobres en terminal compatible, respecter `NO_COLOR` et
conserver une sortie lisible sans couleur. Afficher le chemin des logs une seule fois.

Exception locale autorisée par l’utilisateur : les chemins de cet outillage et
l’image figée peuvent être des constantes dans le script commun. Les chemins
locaux sont relatifs au dépôt, aucun chemin personnel absolu n’est inscrit.
Cette exception remplace ici la règle globale des variables d’environnement,
sans l’étendre aux autres zones.

`docs/keymap.md` reprend les sept schémas du README, avec les légendes alignées
sur les bindings de `dongle`. Ils montrent la base Windows et les couches communes,
les adaptations OS sont résumées séparément. Les points indiquent les bindings
transparents, `∅` les touches sans action. Garder les schémas cohérents avec la source.
Ne pas modifier le README actuel avant la promotion finale de l’index.

`docs/configuration-materielle.md` distingue le matériel, le trajet des touches
et l’alimentation. Ses trois architectures doivent préciser l’état réel : câble
compilé, dongle utilisé sur le Mac mais pas encore réintégré au build, Bluetooth
direct compilé et saisie des deux moitiés confirmée avec USB et sur batterie seule.
Ne pas présenter les capacités natives ZMK ou les essais restants comme déjà validés.

## Repères techniques du diagnostic Bluetooth

Le `CURRENT.UF2` de droite correspondait aux 677 blocs du build normal attendu.
La matrice détectait les appuis et relâchements, notamment à la position 29.
Les rôles compilés étaient corrects. La liaison interne restait au niveau 1,
avec `Security failed ... err 4` et `Link is not encrypted`.

Dans Zephyr 3.5, l’erreur 4 est `BT_SECURITY_ERR_AUTH_REQUIREMENT` et le motif
distant `0x03` indiquait un refus par la droite. Une clé était restaurée pour la
gauche actuelle, qui demandait un nouvel appairage Just Works. La clé utilisait
Secure Connections et un chiffrement de 16 octets. Avec
`CONFIG_BT_SMP_ALLOW_UNAUTH_OVERWRITE` désactivé, `update_keys_check()` refuse son
remplacement. Cette protection a été conservée. L’erreur 4 seule ne prouve pas
un ancien appairage, le moment où les clés sont devenues incohérentes reste inconnu.

Le reset et les firmwares utilisent `storage_partition` à `0xEC000`, taille
`0x8000`, avec huit secteurs NVS de 4096 octets. L’initialisation flash a la
priorité 50 et l’effacement la priorité 60. Le reset officiel appelle
`flash_area_erase()` sur la partition entière, avant le code applicatif.
Les deux captures montraient le début de l’effacement, puis `data wra: 0, 0`,
sans erreur d’effacement observée. Certaines lignes de démarrage étaient partielles.

Le snippet `zmk-usb-logging` ajoutait la console USB sans sortie clavier USB.
`BT_SMP_LOG_LEVEL_DBG` et `BT_KEYS_LOG_LEVEL_DBG` précisaient la négociation.
Les journaux privés restent dans `.local/logs/` et peuvent contenir des clés.
Ne jamais en copier les valeurs dans Git. Les firmwares normaux sans logs sont installés.

## TODO

Voir [docs/TODO.md](docs/TODO.md), source unique du suivi.

## Ideas / backlog

Consigner les idées dans [docs/TODO.md](docs/TODO.md), sans dupliquer les listes.
