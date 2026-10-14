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
  fonctionne sur le Mac avec alimentation USB et sur batterie seule. La veille reste à tester.
  La priorité USB est un profil futur.

## Étape active et validation

Build local et profil `wire` : gauche centrale USB, droite périphérique BLE,
alimentation USB, sans veille profonde ni reporting batterie.
Profil `bluetooth` : mêmes rôles, sortie Bluetooth directe de la gauche, USB clavier
désactivé même pendant la charge, veille après 15 minutes et reporting batterie activés.
Essai matériel sur le Mac : les deux moitiés fonctionnent avec les UF2 Bluetooth
normaux, confirmé par l’utilisateur. Les USB étaient branchés pour l’alimentation,
la saisie reste en Bluetooth. L’utilisateur confirme aussi la saisie sur batterie
seule, sans USB ni dongle. Veille, première frappe et encodeurs restent à valider séparément.

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

Le Mac affiche actuellement la mesure de batterie de la gauche. Le build ne récupère
pas celle de droite. ZMK 0.3.0 dispose des options de récupération et proxy sur la
centrale, l’affichage de deux valeurs par macOS n’est pas garanti. L’étude est dans
[batteries.md](docs/batteries.md), sans option appliquée pour l’instant.

La matrice `kscan0` porte `wakeup-source` pour permettre un réveil par les touches.
Ne pas activer la veille dans le profil câble. Le profil Bluetooth active `ZMK_SLEEP`.
Vérifier sur le matériel chaque touche, la première frappe et la reconnexion.
Le guide `docs/veille.md` distingue les capacités de ZMK des essais matériels.
`docs/flash-firmware.md` couvre le flash et le bootloader, `docs/appairage-bluetooth.md`
couvre les clés de connexion et le reset.
Le mécanisme bootloader déprécié reste nécessaire à ZMK 0.3.0 : son remplacement
demande l’API de rétention, l’adaptation du comportement reset et des codes UF2.
Ne pas désactiver `NRF_STORE_REBOOT_TYPE_GPREGRET` pour faire disparaître le warning.

Le Makefile expose `setup`, `build wire`, `build bluetooth`, `stop` et `clear`,
délégués aux scripts dédiés. `make build` compile `wire`, puis `bluetooth`.
Un profil explicite limite la compilation. Les deux cibles marqueurs sont déclarées
séparément pour permettre cette syntaxe, un seul profil explicite est accepté.
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
moitiés fixent leurs rôles. `config/profiles/<profil>.conf` est chargé en dernier avec
`EXTRA_CONF_FILE`. Le profil est validé côté hôte et transmis comme argument au
conteneur, puis validé à nouveau. Les caches CMake sont séparés par profil et moitié.
La matrice `build.yaml` couvre les quatre UF2 des deux profils, avec les mêmes fichiers
de profil que le build local. Elle utilise `GITHUB_WORKSPACE` fourni par GitHub.

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
isolé dans `build-logs.sh`. Les autres scripts restent compatibles `sh`.
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
