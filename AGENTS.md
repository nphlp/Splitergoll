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
Les fonctionnalités Bluetooth sont décrites par petits sujets : sélection de
l’ordinateur, mémoire/correction de l’OS, diagnostic et méthodes de capture.
Mettre ces guides à jour avec chaque implémentation, sans y recopier les preuves
techniques. Distinguer clairement comportement prévu, implémenté et testé.

## Invariants

- `PC Windows` : fixe assemblé par l’utilisateur, à domicile, pour le jeu et les vidéos/séries.
- `MacBook Pro M4 Max` : ordinateur de travail et de développement.
- `Clavier V1` : clavier à domicile, sans batterie, alimenté par deux USB en profil
  `wire`. Habituellement utilisé sur le PC Windows, également sur le Mac en télétravail.
- `Clavier V2` : clavier au travail, une batterie par moitié, utilisé en Bluetooth
  direct sur le Mac. Le XIAO BLE reste disponible pour le profil dongle historique.
- Les moitiés sont des répliques de nice!nano v2. Le Nordic des photos n’est pas utilisé.
- Aucune LED externe n’est installée, seules celles des contrôleurs sont présentes.
  Ne pas activer d’indicateur LED à l’occasion de la migration.
- La keymap complète de `dongle` à `2584ca4` fait référence : conserver les 44 positions,
  accents, Shift, macros, couches et encodeurs de `multi`. Les helpers, transforms et GPIO sont inchangés.
- L’archive unique avant découpage est conservée dans l’historique Git, par exemple
  à `df55c35` sous `archives/keymap-multi/` : keymap, couches, helpers, trois
  dispositions et `SHA256SUMS`. L’utilisateur a retiré `archives/` du checkout.
  Les six sources archivées restent identiques à `e8bbfc1`. Aucun profil actif ne les inclut.
  Ne pas remettre de keymap historique ni ses helpers dans le dossier du shield.
  Les builds locaux choisissent toujours `keyboard/config/keymaps/<profil>.keymap`
  avec `KEYMAP_FILE`, sans utiliser la sélection automatique du shield.
- Les quatre keymaps manuelles sont `multi`, `windows` avec jeu, `macos` et `linux`.
  `auto` ajoute un profil expérimental macOS / Windows avec jeu : signature USB
  en wire, mémoire d’OS de l’appareil sélectionné en Bluetooth.
  `multi` garde ses quinze couches et son changement d’OS. Les trois profils fixes
  démarrent sur leur OS en couche zéro, sans bascule OS. Windows a sept couches,
  macOS et Linux cinq, sans gaming. Les retours du pavé et du jeu ciblent leur base.
  Le clic d’encodeur droit avec L1 est sans action dans les profils fixes.
  Le pavé macOS utilise Command/Control et les symboles de son OS.
- `keyboard/config/keymaps/common/behaviors.dtsi` partage les comportements historiques.
  `common/base-layer.dtsi` porte les 44 positions de la base commune aux quatre
  keymaps. Chaque appelant définit `BASE_CTRL`, `BASE_ALT`, `BASE_GUI`, `BASE_DOT`,
  `BASE_BSLS` et `BASE_OS`. `BASE_LAYER(bindings, sensors)` conserve le nom du nœud
  `WINDOWS_layer` dans `multi` et `BASE_layer` dans les profils fixes. Tous ces
  paramètres sont retirés après l’inclusion pour éviter toute fuite vers les autres couches.
  Les couches Windows et macOS sont dans `keymaps/layers/*.dtsi`, partagées avec `auto`.
  Leurs fichiers `.keymap` sélectionnent les numéros et comportements fixes.
  Les autres couches des profils fixes explicitent leurs affectations, après résolution
  des touches transparentes des adaptations OS. Ne pas garder une couche Windows
  active comme base cachée de macOS ou Linux. `multi.keymap` compose directement
  les quinze couches avec la base et les comportements partagés.
  Les helpers actifs, les numéros de couches `multi` et les dispositions sont
  dans `common/function.h`, `common/multi-layers.h` et `common/french_layout/`.
  Faire évoluer ces copies actives, jamais les fichiers de la sauvegarde historique.
- Chaque couche des keymaps fixes conserve un schéma commenté au même format
  que `multi` : 44 positions, clics et deux actions de rotation des encodeurs.
  Les légendes suivent les bindings réels, `·` indique une action héritée et `∅`
  aucune action. Adapter le schéma dans le même changement que les affectations.
  Aligner les bindings gauche à gauche et droite à droite sous le schéma, comme
  dans `multi`, avec les retraits des rangées et des pouces. La base garde son
  schéma concret dans chaque profil et son schéma paramétré dans le fichier commun.
- `.vscode/settings.json` garde les commentaires en style normal pour lire les
  schémas, via les réglages TextMate et sémantiques du workspace, sans fixer de thème.
  Les fragments `keyboard/config/keymaps/common/*.dtsi` utilisent la coloration `cpp`,
  adaptée à leurs appels de macros. Les fichiers Devicetree matériels gardent
  leur langage natif. Cette association ne change pas le build.
- Les branches `-minimal` sont des archives, exclues du chantier.
- Le snapshot des 32 firmwares ZMK 0.3.0 est conservé dans l’historique Git au
  commit `7e7be6d`. À la demande de l’utilisateur, `archives/firmware/zmk-0.3.0/`
  est retiré du checkout après les essais positifs de V1. Cela ne valide pas V2
  ni tous les autres profils. Le guide `docs/firmwares-stables.md` explique sa
  récupération sans changer de branche. Les métadonnées et empreintes restent
  dans ce commit, sans log privé, NVS ou appairage. L’archive de la keymap est
  distincte et reste également récupérable dans l’historique Git.
- Les branches historiques restent intactes. `wire` archive `main` à `cbad9a0`,
  `wire-minimal` archive `main-minimal` à `27ed2b7`.
- Sur `unified-firmware`, l’historique antérieur à la refonte est regroupé dans
  le commit initial `6cbbfe8`. Les commits de refonte sont conservés avec de nouveaux
  identifiants. Cette racine n’a pas d’ancêtre commun avec `main` ou `dongle`.
  La promotion vers `main` reste une opération distincte à valider.
- Le README reste au contenu validé du commit `6f89015` jusqu’à sa mise à jour finale.
- Distinguer l’OS, le rôle central/périphérique et la sortie USB/Bluetooth.
- Le BLE entre moitiés reste nécessaire avec le câblage actuel. Le Bluetooth direct
  fonctionne sur le Mac avec alimentation USB et sur batterie seule. Veille et réveil
  sont testés sur des touches représentatives, toutes les touches et encodeurs restent à vérifier.
  La priorité USB est une amélioration différée après l’auto-détection d’OS,
  pour viser une adaptation automatique de la keymap et de la sortie USB/Bluetooth.

## Étape active et validation

Build local et profil `wire` : gauche centrale USB, droite périphérique BLE,
alimentation USB, sans veille profonde ni reporting batterie.
Le Clavier V1 sans batterie est utilisé sur le MacBook Pro M4 Max en télétravail.
Sur le firmware migré vers Zephyr 4.1, l’utilisateur confirme sur V1 en `wire macos`
la saisie, Shift, les encodeurs, copier/coller, la déconnexion/reconnexion et l’accès
aux couches. Le triple appui droit active le pavé numérique. Le protocole distingue
désormais les maintiens de couches des tap-dances, avec gestes et résultats précis.
L’accès au bootloader par L1 + L2 puis clic sur l’encodeur gauche et le retour
par reset sont confirmés par l’utilisateur. Les cas détaillés restent à valider. Les autres validations
matérielles rapportées ci-dessous concernent ZMK 0.3.0.
L’utilisateur confirme ensuite le verrouillage Shift (distinct de Caps Lock),
les accents essayés dont le triple `à/â/À`, l’ordre des frappes alternées entre
moitiés et une latence ressentie imperceptible, sans mesure instrumentée.
Son signalement `ëè` sur trois appuis de F avec L2 correspond à la configuration
historique : `eg_et` a deux actions (`&kp _EG`, `&fr_et`), pas trois. Le second
appui atteint le maximum, le troisième commence une nouvelle séquence.
Les bindings compilés et le délai 250 ms sont identiques dans les deux workspaces.
Le code exécutable du moteur tap-dance est inchangé, seules les déclarations
const des tableaux/configurations diffèrent. Le délai repart au dernier appui,
jusqu’à résolution ou nombre maximal atteint. Ne pas augmenter les délais ni
changer les macros d’accents pour corriger ce résultat attendu.
Les repères du protocole sont corrigés : Y secondaire donne `=`, pas `7`.
L’encodeur droit est vertical en secondaire et horizontal en tertiaire.
Le flash `wire windows` (gaming inclus) et son fonctionnement sur V1 sont confirmés
par l’utilisateur. Cela ne valide pas chaque touche ou couche de jeu.
Le fonctionnement avec `wire macos` normal est également confirmé,
via `make flash wire macos right` puis `left` : flash et fonctionnement des deux
moitiés confirmés par l’utilisateur. Le parcours complet des touches reste à faire.
Avec la précédente variante de diagnostic `multi`, le clic d’encodeur
affichant l’OS paraît plus rapide qu’avec son ancien firmware. Un écart gauche/droite
a initialement été ressenti sur V1 en debug, puis était devenu imperceptible en normal.
Le diagnostic court est clos à la demande de l’utilisateur : en `wire macos debug`,
aucun écart net lecteurs fermés, impression des deux moitiés un peu plus lentes
lecteurs ouverts, puis comparaison incertaine après fermeture. Aucun écart significatif
n’est reproduit. Ne pas conclure à une latence nulle ni à une cause radio ou logging.
V1 fonctionne désormais en `wire macos` normal sur le firmware migré, selon les
premiers essais ci-dessus. Aucun nouveau test de latence instrumenté n’est réalisé.

Deux captures de 90 secondes valident la lecture des consoles et la réception des
événements de droite sur la gauche, sans avertissement, erreur, déconnexion ou perte
de logs signalée dans ces fenêtres. Aucun paramètre BLE négocié n’y est journalisé.
La première capture n’était pas contrôlée, avec huit notifications et huit événements
distants traités. La seconde a commencé après le signal de disponibilité de l’utilisateur.
Les fichiers bruts restent privés dans `.local/logs/`. Les lecteurs sont fermés,
absence vérifiée avec `lsof`. Sur ces connexions, `cu.usbmodem101` est la droite
(événements périphériques) et `cu.usbmodem1101` la gauche (réceptions BLE).
Réidentifier les ports après un changement de branchement.

L’historique de session situe le premier signalement V1 au 15 octobre, avant toute
lecture de sa console par l’agent retrouvée dans cette session. Les lectures droite
puis gauche du 14 octobre concernaient l’appairage de V2. L’hypothèse d’un firmware
ou lecteur de logs seulement à droite lors du premier ressenti V1 n’est pas vérifiée.
L’essai droite seule proposé ensuite n’a pas été lancé, le diagnostic ayant été clos.
Le debug est indépendant de la sortie USB/Bluetooth. Pour une éventuelle reprise,
attendre le signal de disponibilité avant une capture et annoncer ouverture/fermeture.
Le parcours complet des touches, encodeurs et couches reste à valider selon le
[protocole wire](docs/tests-wire.md).
L’ordre retenu est : diagnostic rapide de latence sur V1 et le Mac, migration
ZMK/Zephyr, puis diagnostic d’auto-détection d’OS et autres améliorations du backlog.
Le [guide de latence](docs/latence.md) distingue diagnostic et mesure complète.
La [préparation de migration](docs/migration-zmk.md) conserve les versions,
ruptures et validations. La recherche officielle du 16 octobre 2026 confirme
que ZMK 0.3.0 reste la dernière release stable, publiée le 1er août 2025.
La PR officielle #3024 prépare 0.4.0 mais reste ouverte. Ne pas présenter sa date
automatique de changelog comme une date de publication.
Le snapshot `main` examiné est `4dd95d7840eb3d5a3f59bb29fbf1ab054715ef85`,
committé le 9 octobre 2026, avec 195 commits depuis 0.3.0, dont de la documentation
et de la maintenance. La migration utilise ce commit et Zephyr
`10ba6d0cb38bc3d258775d27982f707599320085` sur `v4.1.0+zmk-fixes`.
Le snapshot stable a été committé et publié à `7e7be6d` avant toute migration.
Les seize paires du snapshot ont été compilées en 187 secondes et passent les
contrôles de profils. Les 32 fichiers archivés passent SHA-256, les gardes UF2
et la sélection par chemin de flash, sans écriture USB. Le wire macos normal
est identique octet pour octet au build précédent `wire-macos-2026-10-16-11-56-08`.
Le snapshot occupe environ 14 Mio. Cela ne valide pas physiquement les 32 UF2.
Les avertissements de cette compilation stable étaient Docker insecure et GPREGRET déprécié.
Les conteneurs éphémères ont disparu, les trois services préexistants sont conservés.
Le [guide des firmwares stables](docs/firmwares-stables.md) décrit le retour arrière.
La cible nice!nano v2 est `nice_nano@2.0.0//zmk`. L’image officielle 4.1 est figée
au digest `sha256:edb1c953438c6f720ddb79c3762f3972013b7fbbaf4fff3592fc869983e7afc5`.
Le setup a contrôlé les 45 dépendances actives en 194 secondes. Le manifeste
résout les imports officiels avec West et fige 68 projets, groupes facultatifs
et babblesim désactivés. Le nouveau workspace est `.local/zmk/25d3fa3b79b2ce10/`,
celui de la 0.3 reste intact. Aucun patch n’est appliqué aux dépendances.
Les 32 UF2 migrés compilent et passent les gardes de profils, de rôles et de logs.
`container.sh` impose la variante ZMK, la rétention de boot, le mapper de code
UF2 `0x57`, l’offset `0x26000` et la taille d’application `0xc6000`.
Les 32 arbres compilés ont été comparés à ceux du snapshot stable : `/keymap`,
`/conditional_layers`, `/macros`, ordre des comportements et leurs propriétés
sont identiques, sauf la nouvelle propriété `bootloader` attendue sur `&bootloader`.
La matrice, son transform, les deux encodeurs, leurs capteurs et `vbatt` sont identiques.
Les partitions restent `0x26000` / `0xc6000` pour l’application et `0xEC000` /
`0x8000` pour NVS. Les deux nœuds de rétention et leurs références chosen sont vérifiés.
Les empreintes, blocs, famille nRF52840 et adresses des 32 UF2 sont vérifiés.
Les validations temporaires sont dans `.local/migration/`, sans nouvelle suite d’outillage.
Tous les conteneurs de validation ont disparu, les trois services préexistants sont conservés.
La rupture des couches est indépendante de Zephyr : `&to` et `&tog` activent
`locking`, empêchant un comportement non verrouillant de désactiver leur couche.
`common/behaviors.dtsi` retire cette propriété de `&to` et `&tog` pour conserver
le comportement stable, sans modifier les affectations. Leur absence est contrôlée
dans les arbres compilés. Un harnais temporaire utilisant les fonctions C réelles
des deux versions compare 245 952 triplets d’actions d’activation, désactivation,
toggle et to sur 5, 7 et 15 couches. Les états sont identiques sans verrouillage.
Ce contrôle ne simule pas la file d’événements, la temporisation des tap-dances
ou le matériel. Les tap-dances, retours de pavé, gaming et bascules OS restent à essayer.
L’avertissement GPREGRET déprécié a disparu. Le symbole de compatibilité
`NRF_STORE_REBOOT_TYPE_GPREGRET` reste à y dans ZMK, mais le chemin reset
compilé utilise `RETENTION_BOOT_MODE`, pas l’ancienne implémentation Zephyr.
Deux avertissements du socle restent visibles : KSCAN déprécié et qualification
`const` perdue à l’initialisation des tap-dances (`behavior_tap_dance.c:269`).
Le tableau de bindings est const, le pointeur de configuration ne l’est pas,
et les fonctions press/release copient les bindings avant invocation.
Ne pas masquer le diagnostic. Le remplacement de KSCAN par input et le correctif
const amont restent des chantiers séparés, sans fork ou patch implicite de ZMK.
La préférence de sortie NVS migre de `endpoints/preferred` à `endpoints/preferred2`,
avec suppression de l’ancienne clé. Un retour à 0.3.0 peut donc perdre cette
préférence, sans que ce constat prouve un effacement des appairages Bluetooth.
La demande native de détection d’OS #2553 reste ouverte. Les changements examinés
n’apportent pas cette fonction. La pile USB utilisée reste `USB_DEVICE_STACK`,
le remplacement par la nouvelle pile et le retrait de kscan ne font pas partie
de cette migration ciblée. Ne pas ajouter de module OS tiers pendant ce lot.
Dans ZMK 0.3.0, le périphérique transmet position/état sans timestamp d’origine,
puis `split/central.c` recrée l’événement avec `k_uptime_get()` sur la centrale.
Les horloges des moitiés ne sont pas synchronisées. Ne pas soustraire leurs logs
pour annoncer un délai BLE, ni utiliser l’arrivée des lignes sur le Mac comme
horodatage de l’appui : le logging debug est différé et peut perturber les timings.
Les logs d’une même centrale permettent d’observer une portion du traitement,
pas le trajet complet depuis le contact physique jusqu’à la réception par macOS.
Profil `bluetooth` : mêmes rôles, sortie Bluetooth directe de la gauche, USB clavier
désactivé même pendant la charge, veille après 15 minutes et reporting batterie activés.
Le profil aux délais courts a été supprimé après validation matérielle. Les deux
profils disponibles sont `wire` et `bluetooth`. L’option transversale `debug` ajoute
le snippet natif `zmk-usb-logging` à chaque moitié, sans toucher aux timings, rôles
ou keymaps. `BUILD_MODE` vaut `normal`, `debug`, `usb-trace` ou `ble-trace`. `BUILD_VARIANT` nomme
matériel et keymap avec suffixe `-debug`, `-usb-trace` ou `-ble-trace` pour ces diagnostics. Caches et artefacts sont séparés,
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
distincts et conservés. Ce comportement reste celui de `multi`. Les profils fixes
mettent leur propre OS en couche zéro, sans état OS à sauvegarder. Le matériel
reste à tester pour ces nouveaux profils.
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
Sur le Clavier V2, l’interrupteur coupe la batterie mais pas l’alimentation USB.
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
Le build utilise le bootloader par rétention de la variante nice!nano ZMK et
conserve le code UF2 `0x57`. Le bootloader installé sur les contrôleurs n’est pas
mis à jour. L’accès par `&bootloader` est confirmé sur V1 après migration.
Le retour par reset est confirmé. Tester encore `&sys_reset` et le double appui
physique avant de considérer le parcours matériel complet validé.
Le snapshot 0.3 dans l’historique Git garde son ancien mécanisme.

Le Makefile expose `help`, `setup`, `build <matériel> <keymap> [debug|usb-trace|ble-trace]`,
`flash`, `bat`, `bat avg`, `usb-capture <port>`, `ble-capture <port>`, `ble-profile <port> <status|free|index>`, `stop` et `clear`, délégués aux scripts dédiés. Le matériel (`wire` ou `bluetooth`)
puis la keymap (`auto`, `multi`, `windows`, `macos` ou `linux`) sont obligatoires, sans défaut.
Windows inclut toujours le gaming. L’argument final facultatif choisit `debug`
pour les logs USB, `usb-trace` pour la connexion wire ou `ble-trace` pour l’hôte Bluetooth.
`make build wire` et `make build wire debug` échouent avant Docker.
Les arguments sont validés avant les journaux et les actions Docker.
Ni le passage au conteneur ni son point d’entrée ne remplacent une keymap absente
par `multi`. `setup` ne dépend pas de la keymap.
Chaque commande a sa propre cible et son appel de script dans le Makefile.
À la demande de l’utilisateur, le Makefile appelle directement chaque script avec
ses arguments, sans validateur central ni variable d’environnement intermédiaire.
`ARGS` retire le nom de la cible et protège chaque mot pour le shell. Chaque script
vérifie son nombre d’arguments et ses valeurs avant toute action. `setup`, `stop`
et `clear` n’acceptent aucun argument. `help.sh` accepte les périmètres internes `build`
et `flash`, l’aide générale reste `make help`. La règle `%` ne fait rien : Make voit
les arguments comme d’autres cibles, déjà traitées par le script de la commande.
Un mot inconnu seul est donc sans action. Ne pas ajouter de dispatch central ni
de cibles individuelles pour chaque valeur d’argument.
`make build` seul affiche les profils disponibles via `help.sh build`, sans Docker
ni préparation requise. L’aide présente l’ordre des deux choix obligatoires,
les valeurs disponibles et des commandes complètes. Les deux moitiés du seul
choix demandé sont compilées.
`help.sh` affiche l’aide groupée, avec la palette partagée de `terminal-colors.sh`.
Les constantes partagées sont en tête de `scripts/build-common.sh`. `container.sh`
exécute `west` et vérifie les firmwares. Aucun `.env` n’est nécessaire.

`make flash <matériel> <keymap> <left|right> [debug|usb-trace|ble-trace]` choisit le dernier dossier
horodaté du profil exact, sans rabattement normal/debug ni retour à un build plus
ancien si le dernier est incomplet. `make flash <chemin.uf2>` accepte un UF2 du
dépôt resté dans son dossier horodaté, avec métadonnées et empreintes.
`flash.sh` présente matériel, keymap, côté puis activation des logs USB,
dans le même ordre que les arguments de la commande.
L’affichage est compact sous un seul titre, sans sections pour le build ou le bootloader.
Il affiche date absolue et âge sur deux lignes, avec les mois en français sans
dépendre des locales installées et l’heure au format `11 h 50 min 08 s`.
Une ligne vide précède le chemin UF2 relatif au dépôt et la ligne de bootloader
avant confirmation. Le SHA-256 reste contrôlé en interne.
La date du dossier
est locale, sans fuseau embarqué. Windows inclut toujours le gaming.
Le flash est limité à macOS, sans Docker. Le script vérifie les métadonnées, SHA-256
et chaque bloc avec `uf2-check.awk` : famille nRF52840, blocs de 256 octets, adresses
de l’application nice!nano v2 de `0x26000` à la partition NVS `0xEC000` exclue.
Ces bornes devront être revues lors d’une migration du partitionnement.
La détection utilise `/Volumes/*/INFO_UF2.TXT`, le Board-ID historique observé
`nRF52840-nicenano`, puis `diskutil` pour vérifier un volume FAT USB monté et inscriptible.
Le bootloader ne révèle pas le côté : seule la moitié choisie doit être en bootloader.
Refuser zéro ou plusieurs cibles compatibles. Une seule confirmation interactive
est demandée, avec annulation par défaut et sans option d’approbation automatique.
La question `Continue? [y/N]` lit une seule touche sans écho, sans attendre Entrée.
Seuls `y` ou `Y` confirment, toute autre touche ou fin d’entrée annule.
Après accord, recontrôler le SHA et l’identité du disque avant `cp -X`, sans effacement
des réglages, éjection forcée ou nouvel essai automatique en cas d’erreur.
Une copie réussie n’est pas une validation de saisie. Aucun flash matériel implicite
pendant les validations de code.
Les gardes sont contrôlés sans écriture USB : sélection par profil et par chemin
sur des UF2 réels, absence de build et de bootloader, refus des arguments invalides
et de neuf variantes UF2 malformées transmises en mémoire. Aucun nouveau fichier
de tests d’outillage n’est ajouté. L’utilisateur confirme le parcours réel et le
fonctionnement des deux moitiés avec `make flash wire macos right` puis `left`.
Le chemin UF2 explicite, debug, l’annulation et les autres OS restent à tester sur matériel.

Le snapshot ZMK `4dd95d7840eb`, Zephyr 4.1 et les
68 projets résolus sont figés dans `keyboard/west.yml`. Les workspaces sont séparés
par empreinte de l’environnement, les caches CMake par moitié.
Les sorties sont dans `dist/firmware/<matériel>-<keymap>[-debug]-AAAA-MM-JJ-HH-MM-SS/`, en heure locale,
sans suffixe aléatoire. Refuser un dossier existant plutôt que l’écraser.
Les UF2 sont nommés `splitergoll-<matériel>-<keymap>[-debug]-<moitié>.uf2`.
Les caches sont séparés selon les mêmes choix, les métadonnées indiquent
`keymap_profile`. Les anciens artefacts ne sont ni renommés ni supprimés.
`dongle` nommera un futur profil, il n’est pas encore compilable.

`keyboard/config/splitergoll.conf` contient les options communes. Les fichiers `.conf` des
moitiés fixent leurs rôles. `EXTRA_CONF_FILE` charge le profil matériel, puis une
configuration de moitié optionnelle. Le snippet USB logging est ajouté avec
`west build -S zmk-usb-logging` en mode debug uniquement.
Le profil et la keymap sont validés côté hôte et dans le conteneur. `KEYMAP_FILE`
est transmis explicitement à CMake et contrôlé dans son cache avant export.
Les contrôles vérifient les délais, la mesure et le proxy gauche, ainsi
que l’absence de reset d’appairage dans tous les UF2 clavier. Les contrôles imposent
les logs USB et leur console CDC en debug, et leur absence en normal.
Les seize UF2 normaux (quatre keymaps, deux matériels et deux moitiés) et les quatre
UF2 `wire macos debug` / `bluetooth windows debug` compilent et passent ces contrôles.
Les devicetrees compilés des deux moitiés sont comparés à la référence précédente :
`multi` conserve intégralement `/keymap`, `/conditional_layers`, `/behaviors` et
`/macros`, ainsi que l’ordre des couches. Les profils fixes ont 44 bindings et
deux encodeurs par couche, des transitions valides et les affectations héritées
attendues. Le pavé macOS est adapté explicitement, les autres différences sont
le retrait de la bascule OS, du gaming hors Windows et le changement des numéros
de couche. Ces contrôles sont réalisés avec le parseur devicetree de Zephyr,
sans nouvelle suite de tests d’outillage. Les essais physiques restent nécessaires.
Le partage de la base est revalidé sur les mêmes vingt firmwares : les devicetrees
complets et l’ordre des couches sont identiques avant/après, hors numéros de phandle.
La séparation de la sauvegarde est contrôlée de la même manière sur les vingt
firmwares. Leurs fichiers `zephyr.dts.d` chargent une keymap active de `keyboard/config/keymaps/`
et ne référencent ni `splitergoll.keymap` ni ses helpers historiques. L’archive
conservée dans Git et ses cinq headers sont identiques à `e8bbfc1`. Son déplacement hors du shield
est revalidé avec les deux moitiés de `wire multi` et `bluetooth macos`.
Empreintes SHA-256, famille nRF52840 et adresses de firmware sont vérifiées.
Les consoles de ces nouveaux
builds restent à tester sur le matériel, sans flash implicite. Les conteneurs de
validation ont disparu, les trois services préexistants sont conservés.
L’utilisateur a supprimé `.github/` : aucun workflow GitHub Actions ne reste dans
le checkout. L’ancienne matrice `build.yaml` est également retirée, aucun script
local ne l’utilisait. Le build local fonctionne indépendamment.
Ne pas recréer le workflow, sa matrice ou l’archive supprimés implicitement.

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
isolé dans `build-logs.sh`. `help.sh`, `flash.sh`, `batteries.sh` et
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
et l’alimentation. Ses trois architectures doivent préciser l’état réel : saisie
câble confirmée sur le Mac avec le clavier PC sans batterie, dongle historique
pas encore réintégré au build, Bluetooth
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

## Organisation du code

Le manifeste source reste à `keyboard/west.yml`. Le setup prépare un workspace
isolé dans `.local/`, avec sa copie du manifeste. Le build compile `zmk/app` et
transmet explicitement la configuration, la keymap et le module local `keyboard`.
L’ancienne initialisation du workflow GitHub ne fait plus partie du parcours actif.

`keyboard/` regroupe le code du clavier : `config/` pour keymaps et profils,
`west.yml` pour les dépendances, `boards/` pour les shields et `zephyr/module.yml`
pour leur découverte.
Les sources, GPIO et affectations ne changent pas lors de ce rangement.
Le module est passé explicitement par `ZMK_EXTRA_MODULES` dans les builds locaux
et la matrice manuelle.

`diagnostics/os-signature/` regroupe l’instrumentation d’étude : firmware de
capture, snippet, lecteurs Bash/PowerShell, validateur et tests du stockage.
L’observateur et le classifieur communs appartiennent désormais au code du clavier,
dans `keyboard/src/os_detection/`, leurs interfaces dans `keyboard/include/splitergoll/`
et leurs tests dans `keyboard/tests/`. Son
[AGENTS.md](diagnostics/os-signature/AGENTS.md) conserve les mécanismes et preuves.
Son module est ajouté seulement à la gauche de la variante `usb-trace`.
Les scripts de build, flash et batteries restent dans `scripts/`.
Les captures privées macOS/Linux restent dans `.local/usb-traces/`, les guides humains
dans `docs/`. Le lecteur Windows est autonome : copier seulement `capture.ps1` dans
un dossier quelconque, l’exécuter depuis ce dossier, et récupérer le `.txt` directement
dans le dossier courant. Aucune arborescence de dépôt imposée. Les captures Windows
sont ignorées par Git si elles sont créées dans le checkout.

Les caches CMake du nouveau rangement sont sous `build/keyboard/` dans le workspace
existant. Ne pas réutiliser l’ancien ZMK_CONFIG mémorisé ni supprimer les anciens
caches. Le manifeste et l’image n’ont pas changé, un nouveau setup n’est pas requis.

Après rangement, les huit paires normales, les paires `wire macos debug`,
`bluetooth windows debug` et `wire macos usb-trace` compilent. Les 22 UF2 sont
identiques octet pour octet aux précédents. Les arbres compilés conservent keymaps,
comportements, conditions, macros, matrice, encodeurs et batterie. Les sources du
clavier et le manifeste conservent leurs empreintes. Le stockage passe ASan/UBSan,
les six captures existantes passent le validateur déplacé. L’initialisation du
workflow manuel est vérifiée hors réseau avec West, pas exécutée sur GitHub.
Les conteneurs éphémères sont supprimés, les trois services partagés sont conservés.

Le firmware expérimental `wire auto` est implémenté. Il compose la base neutre et les
sept couches Windows puis les cinq macOS depuis les mêmes fragments que les profils
fixes. Il ne dépend pas de `multi`. Les motifs USB ont été confirmés sur V1, macOS,
Windows puis retour Mac, sans perte/rejet sur les deux dernières captures.
L’utilisateur confirme ensuite sur V1 le nom macOS par l’encodeur droit sur le Mac,
Windows sur le PC, l’entrée en gaming et le retour Windows, ainsi que le fonctionnement
habituel sur Mac après flash des deux moitiés. Cela confirme la sélection effective
avec la variante instrumentée de l’essai. L’utilisateur a ensuite flashé V1 avec
`wire auto` normal, sans `usb-trace`. Il confirme maintenant le fonctionnement sur
macOS et aucune saisie sur Windows. La capture ETW et son rejeu révèlent ensuite
un motif insuffisant, sans donner directement l’état interne du firmware.
Les descripteurs compilés diffèrent : classe/sous-classe/protocole `00/00/00` en normal,
`ef/02/01` en trace, avec les mêmes VID/PID et les mêmes index de noms. Le motif
Windows sur quatre octets est absent de la capture normale, avec des demandes de
noms sur 255 puis 262 octets. Ne pas attribuer son origine à un pilote précis sans preuve.
`capture.ps1 -UsbEvents` prépare une capture ETW hôte sans port COM ni modification
du firmware. Fichier autonome, sorties à la racine du dossier courant, PowerShell
administrateur requis pour ce mode uniquement. Le guide est
[docs/capture-usb-windows.md](docs/capture-usb-windows.md). Le correctif ajoute le motif
255 puis 262 du même index/langue, plus deux noms distincts lus sur 255, sans
Windows par défaut. Les veto, gardes d’appuis et keymaps restent inchangés.
Les trois suites C passent et les dix captures précédentes gardent leur OS.
Les 22 demandes ETW reproduisent inconnu avant correction, Windows après, y compris
sans la sonde initiale 64. Après flash du correctif, l’utilisateur confirme sur V1
la saisie et la reconnaissance Windows puis macOS, en débranchant/rebranchant
le même firmware normal entre les deux hôtes, sans reflash intermédiaire.
L’utilisateur confirme ensuite les layouts et le gaming en usage normal.
Les essais de veille, reset du bus sans coupure et autres ordinateurs restent distincts.
L’utilisateur signale ensuite un démarrage irrégulier sur Mac, reproduit même en
Mac vers Mac : le premier aller-retour réussi ne valide donc pas la fiabilité.
Mesurer un branchement sans frappe avant de modifier le code. Les appuis pendant
l’attente repoussent le garde de 750 ms, sans preuve que ce soit la cause observée.
Pas de persistance Windows démontrée. Le diagnostic détaillé est dans la spec
`keyboard/src/os_detection/AGENTS.md`.
L’essai suivant avec cinq secondes sans frappe fonctionne du premier coup,
ce qui concorde avec l’attente repoussée par les positions ignorées. Ce n’est
pas une corruption de signature et les cinq secondes ne sont pas un délai codé.
L’utilisateur confirme ensuite le fonctionnement des layouts et du gaming sur
Mac et Windows. Ces usages sont validés par son retour, le chantier porte sur
le délai au branchement. L’optimisation retire l’attente initiale de 750 ms et
son réarmement par les positions/rotations ignorées. Le relâchement complet reste
requis, la marge de 750 ms est conservée après un cycle actif et pour les keycodes
différés. Cette première optimisation conservait 250 ms de stabilisation et un
contrôle toutes les 20 ms. Le nouveau
démarrage est ressenti nettement plus rapide après essai utilisateur, sans mesure
chronométrée. Les keymaps et les descripteurs sont inchangés.
La première optimisation est publiée à `c83c146`. La seconde retire les 250 ms et
le contrôle périodique : les requêtes et statuts USB réveillent directement la
sélection, ainsi que les événements pendant l’attente. Le classifieur n’utilise
plus d’horloge. La sélection reste hors callback USB, après disponibilité HID et
relâchement complet. Le seul minuteur est le reste de la marge des anciens
comportements après reset. Les contradictions avant sélection restent refusées,
celles arrivées après le choix figé peuvent ne pas être prises en compte.
L’utilisateur confirme après flash l’aller-retour Mac → Windows → Mac : les noms
affichés par le clic d’encodeur correspondent à chaque OS et le démarrage est
ressenti très rapide à chaque branchement. Aucun chronométrage. Les cas de touche
maintenue, reset du bus sans coupure et réveil restent distincts.
L’utilisateur confirme finalement
le son de connexion Windows, mais pas de port COM4 : cette absence est attendue
sans CDC. L’ETW confirme l’énumération terminée sans erreur globale, avec 2 075 événements
et zéro perte déclarée. Les champs utiles sont lisibles malgré un avertissement
de schéma Tracerpt. Le script utilise désormais son mode de décodage `-lr`, encore
à essayer sur le PC. Aucun besoin de refaire la capture pour le correctif actuel. Le script
PowerShell a eu deux chaînes cassées par des apostrophes typographiques, corrigées
après reproduction exacte avec le parseur officiel local. Les regex ne changent pas.
Les invariants et tests du nouveau moteur sont dans
[keyboard/src/os_detection/AGENTS.md](keyboard/src/os_detection/AGENTS.md).
Le guide humain est [docs/keymap-auto.md](docs/keymap-auto.md).
`auto` est refusé avec Bluetooth. `usb-trace` reste passif avec une keymap fixe,
mais expose aussi la sélection réellement appliquée avec `auto`.
Lorsque `auto` couvrira les usages et sera validé, l’utilisateur souhaite retirer
`multi` des profils actifs tout en le conservant comme archive autonome. Ce retrait
n’est pas effectué maintenant, il reste suivi dans `docs/TODO.md`.

## Diagnostic Bluetooth

Le diagnostic actif est maintenant Bluetooth sur V1, d’abord Mac puis Windows.
`ble-trace` ajoute un port série USB dédié sur la gauche Bluetooth, sans sortie
clavier USB ni sélection automatique. Les échanges précoces restent en RAM jusqu’au
reset physique, indépendamment de l’ouverture du lecteur. La droite reste normale.
La spec est dans [diagnostics/os-signature/AGENTS.md](diagnostics/os-signature/AGENTS.md)
et le guide court dans [docs/detection-os-bluetooth.md](docs/detection-os-bluetooth.md).
Ne pas confondre hôte Bluetooth et ordinateur d’alimentation USB, ni déduire un OS
de sa radio seule. Les captures réelles et reconnexions doivent précéder toute règle.
Le premier flash des deux moitiés et la connexion BLE V1/Mac sont confirmés par
l’utilisateur. La capture gauche donne 49 événements, zéro perte/rejet, découverte
HID et sécurité niveau 2 sans erreur. La reconnexion Mac sans reset ni coupure USB
est ensuite capturée : 27 événements, dont 20 requêtes ATT contre 42 au départ,
sans relecture de la description HID. Zéro perte/rejet. La lecture est faite
20,5 secondes après reconnexion, les dix secondes du protocole sont respectées.
L’utilisateur précise que la reconnexion était automatique, avec 1 054 ms entre
déconnexion et connexion dans la trace. Ces premières captures utilisaient un
diagnostic limité à un hôte, isolé en coupant temporairement le Bluetooth des autres.
Le lecteur est fermé. La première capture Windows est ensuite fournie : 76 événements,
67 requêtes ATT, zéro perte/rejet, même table GATT de 52 attributs. Le MTU est identique
au Mac, Windows lit en plus Appearance et des attributs de cache GATT.
La reconnexion Windows déjà appairé est ensuite capturée sans nouveau flash :
six événements, deux requêtes ATT génériques (MTU et batterie), aucune perte/rejet.
Les lectures distinctives du premier appairage disparaissent. Les paramètres/radios
diffèrent sur nos deux hôtes mais ne prouvent pas un OS universel. Ne pas ajouter
Windows par absence d’indices. Le prochain choix est d’enrichir l’observation ou
d’étudier une mémoire automatique d’OS appris par appairage authentifié, avec gestion
des remplacements et changements d’OS. Le classifieur C pur dans
Le classifieur BLE est maintenant dans `keyboard/src/os_detection/ble_os_classifier.c`,
son interface dans `keyboard/include/splitergoll/`. Le dump calcule une hypothèse par session après les
ATTR, dans le thread USB, sans ajouter de calcul dans les callbacks radio.
Les lecteurs acceptent les anciennes captures et les nouvelles lignes `BLETRACE OS`.
Après flash de gauche par l’utilisateur, la lecture réelle sur V1/Mac confirme
macos `0003` : 27 événements, sécurité niveau 2 sans erreur, zéro perte/rejet.
Le port USB est refermé. L’essai suivant utilise le nouveau diagnostic v2.
La gauche est alimentée et lue par le Mac, la droite est alimentée par le PC Windows.
Le nouveau diagnostic v2 sépare maintenant les sessions simultanées et leur état
par emplacement, avec un tampon partagé toujours borné à 256 événements. La commande
USB permet la sélection explicite d’un autre hôte connecté sans couper les Bluetooth.
Elle refuse une bascule pendant un maintien, un rapport HID actif ou les 750 ms
suivant un événement de saisie. Après flash gauche, S et D observent réellement
les deux hôtes connectés : Mac actif en 0, Windows connecté en 1, 32 événements
sans perte/rejet. La session Windows ne fournit qu’une requête de MTU et reste
inconnue, la session Mac donne macos `0003`. L’aller-retour 0 → 1 → 0 est ensuite
confirmé par les statuts, les deux hôtes restant connectés, sans frappe générée.
Le retour final au Mac est confirmé. L’utilisateur confirme ensuite la réception
de frappes sur Windows avec la keymap macOS fixe. La garde de maintien reste à
tester sur matériel. Ni keymap, ni préférence
de sortie, ni appairage, ni firmware normal ne changent dans ce lot.
Le rejeu des cinq fichiers existants reconnaît les sessions Mac et la découverte
initiale Windows, mais laisse sa reconnexion limitée au MTU/batterie inconnue.
La mémoire validée est maintenant partagée par `bluetooth auto` et le diagnostic
gauche. Le sélecteur de keymap BLE est préparé, son essai matériel reste à faire.
La sélection automatique de destination BLE reste un chantier distinct.
Le fonctionnement et les essais sont décrits dans [memoire-os.md](docs/memoire-os.md)
et dans les AGENTS des modules clavier et diagnostic.
L’utilisateur a désormais flashé ce build : acquisition initiale et reconnexion
Mac sont confirmées, avec macos sauvegardé sans réapprentissage à la reconnexion.
Le nouvel appairage Windows avait fourni trois des quatre indices et enregistré
unknown sans erreur. Le correctif accepte maintenant un motif positif de découverte
partielle : sondage du hash GATT, lecture directe, nouveau sondage, en plus des
trois indices communs. Les tests et le rejeu de sa capture donnent Windows.
Après flash corrigé de gauche, le Mac retrouve stored=macos, state=saved et
learning=0, sans nouvel appairage. Le nouvel appairage Windows confirme ensuite
measured=windows, stored=windows, origin=initial, state=saved, evidence=005c et
error=0, sans perte. Aucun ancien unknown n’est réinterprété aux reconnexions.
Après reset puis reconnexion de Windows, les deux emplacements retrouvent leur
OS en state=saved, learning=0, error=0 et dropped=0. Le diagnostic de session
Windows reste inconnu, mais son cache restitue windows : la restauration sans
réapprentissage est validée sur ces deux appareils. Le retour final au Mac est
confirmé, les deux hôtes restant connectés et le lecteur fermé. La veille profonde
sur V2 et les autres machines ne sont pas validées par cet essai.

La direction BLE demandée est désormais : sélectionner l’unique ordinateur connu
connecté, conserver le destinataire et permettre une bascule explicite si plusieurs
sont connectés, puis charger l’OS reconnu et mémorisé pour cet appairage. Une
reconnexion ne relance aucune détection et charge exclusivement l’OS enregistré.
Un raccourci permettra de corriger durablement l’OS de l’appareil sélectionné.
Après redémarrage/veille profonde, le premier hôte peut recevoir immédiatement la
saisie, mais une fenêtre proposée de 5 secondes permet de restaurer le précédent
destinataire s’il revient, même si l’utilisateur a déjà tapé sur le premier hôte.
Taper ne ferme ni ne prolonge cette fenêtre. Sinon le nouveau devient la préférence mémorisée.
La bascule manuelle annule cette fenêtre. Les annonces nécessaires au second hôte,
l’invalidation d’appairage et la politique de reprise restent à implémenter/tester.
Les guides courts sont [selection-appareil.md](docs/selection-appareil.md) et
[memoire-os.md](docs/memoire-os.md), avec les cas limites dans la spec du diagnostic.

L’utilisateur accepte une détection initiale suivie d’une mémoire automatique si la
première reconnaissance est fiable. Le full auto reste prioritaire. La correction
manuelle d’OS en fait partie, le profil entièrement manuel reste un dernier recours.
La mémoire est préparée dans le diagnostic, la sélection de couches et la correction
par raccourci restent à faire. Les noms des profils restent à définir. La recherche
de code public est synthétisée dans le guide Bluetooth, avec commits et limites dans
la spec du diagnostic. Les règles BLE de Cormoran ne conviennent pas directement :
elles classeraient notre première capture Mac comme Android. Les métadonnées publiques
SMP sont une piste supplémentaire à évaluer, sans collecte de clés ni nouvelle sonde.


Le passage Windows nécessite un second emplacement d’appairage. La keymap initiale
n’a pas de sélecteur Bluetooth. Le diagnostic gauche ajoute donc `ble-profile`
(status, free ou index), sans BT_CLR ni modification de touches. Seul l’index actif
change via l’API native, sauvegardé après 60 secondes, les bonds restent intacts.
La sélection Windows se fait sur le PC après déplacement des USB et coupure du
Bluetooth Mac. Le guide [capture-bluetooth-windows.md](docs/capture-bluetooth-windows.md)
donne le retour à la case Mac sans nouvel appairage. Le contrôle est testé en natif,
compilé puis flashé par l’utilisateur. Ses sorties confirment Mac en case 0, choix
libre 1 sur Windows, puis retour à 0 avec les deux cases toujours appairées.
La sélection et le lecteur PowerShell sont donc validés sur les deux ordinateurs.
Pour reconnecter Windows, réutiliser 1 explicitement, jamais `free` après l’appairage.
Les prochains essais peuvent garder les USB sur le Mac pendant la liaison BLE
Windows. Le [guide de capture Mac](docs/capture-bluetooth-mac.md) évite PowerShell
et les transferts de fichiers. Le champ host de la capture désigne seulement le collecteur.

## Bluetooth auto, première intégration

La mémoire validée est publiée au commit 11f7a87. Le lot suivant active
`make build bluetooth auto` et ses variantes debug/ble-trace. Le moteur BLE commun
est regroupé dans keyboard, le diagnostic n’en garde que la présentation et
l’historique optionnel. La keymap partage exactement les treize couches de wire auto.
Les identités et le format de mémoire sont compatibles avec les appairages actuels.
Une connexion sécurisée, une mémoire vérifiée et une génération radio à jour sont
requises avant la saisie, sans OS implicite ni délai de détection initial.
Les gardes de touches et comportements différés sont partagées avec l’auto USB.

Huit suites C, le worker/backend réels en harnais et douze captures / 43 sessions
passent. Les six paires de build et douze UF2 sont contrôlés. Wire auto et le
Bluetooth macOS fixe normal restent identiques aux versions validées. Les couches
des trois variantes auto BLE sont identiques à wire auto. Le guide court est
[keymap-auto-bluetooth.md](docs/keymap-auto-bluetooth.md). L’utilisateur confirme
le flash gauche. Les lectures passives retrouvent Mac actif/connecté, stored macos,
state saved, learning 0 et error 0, sans perte. Windows reste appairé/déconnecté,
son cache chargé mais unverified. La saisie, l’affichage d’OS et les transitions
de cette keymap BLE restent à confirmer. La droite est identique à celle installée.
Aucun firmware normal ne doit être présenté comme testé sur matériel sur la base
de ces contrôles logiciels.

L’utilisateur demande ensuite de préparer le raccourci pendant que le PC est occupé.
L1 + clic encodeur droit dans bluetooth auto sélectionne uniquement l’hôte connecté
suivant. Les emplacements déconnectés ou vides sont ignorés, contrairement au BT_NXT
natif qui parcourt tous les profils. La demande attend les relâchements, un rapport
HID vide et le garde des comportements différés. Un chemin physique de secours garde
ce geste accessible avec OS inconnu ou hôte indisponible, sans laisser passer les
autres frappes. Le clic simple continue d’indiquer l’OS. Aucun changement dans wire,
multi ou les profils fixes. Le choix manuel forcé d’OS reste différé.
La politique et le pilote sont dans keyboard, pas dans le diagnostic.
Les annonces natives s’arrêtent quand l’hôte actif est connecté : reconnecter un
second hôte et automatiser le choix seront le prochain chantier. Le raccourci ne
réveille pas les appareils déconnectés. Ne pas basculer vers le PC pendant qu’il
est occupé, ni présenter les tests logiciels comme un essai physique de ce geste.

## Ideas / backlog

Consigner les idées dans [docs/TODO.md](docs/TODO.md), sans dupliquer les listes.
