# Étude des signatures d’OS

Ce dossier contient toute l’instrumentation d’étude, séparée du clavier dans
`../../keyboard/`. Les lecteurs sont `capture.sh` et `capture.ps1`, le validateur
est `check.awk`. Le firmware, son snippet et ses tests restent dans leurs sous-dossiers.
La commande publique `make usb-capture` reste à la racine du dépôt.

Le chantier actif est le diagnostic passif USB, puis BLE. V1 sans batterie peut
tester les deux transports, avec alimentation USB. Ne pas confondre la source
d’alimentation et l’hôte Bluetooth. Aucun OS n’est encore classé automatiquement.

`make build wire <keymap> usb-trace` et le même argument final de `make flash`
isolent caches, artefacts et métadonnées du normal et de `debug`. `usb-trace`
est refusé en matériel `bluetooth`. La droite reste normale, la gauche ajoute
le snippet local `splitergoll-usb-trace`. Son CDC est indépendant de la console
ZMK, sans logs de frappes, clé BLE, modification de keymap ou effacement NVS.
La keymap doit rester identique entre les captures comparées.

Le module local déclaré dans `firmware/zephyr/module.yml` compile `firmware/` uniquement
avec `SPLITERGOLL_USB_TRACE`. Les dépendances figées restent intactes. Deux wrappers
de liens encadrent `usb_dc_ep_set_callback` et `usb_dc_ep_read`, en transmettant
les arguments et statuts originaux. Seuls les en-têtes SETUP de l’endpoint zéro OUT
sont copiés après lecture. Aucun payload de contrôle ni rapport HID n’est conservé.
La pile nouvelle, LTO et les SoC autres que nRF52840 sont refusés par CMake.

Le tampon append-only contient 256 requêtes, garde les premières et compte les
pertes et tailles incorrectes avec saturation. Le pilote lit les SETUP complets
de huit octets, avec une longueur de retour NULL dans la pile figée actuelle.
Le verrou protège uniquement les copies, jamais l’envoi série. La commande D
restitue un instantané avec cadrage BEGIN/SETUP/END et compteurs, sans effacement.
L’envoi vérifie DTR et borne l’attente par ligne. Un redémarrage efface le tampon.

`make usb-capture <port>` lit sur macOS/Linux, `capture.ps1 -Port COMx`
sur Windows. Les ports sont explicites, les captures restent ignorées dans
`.local/usb-traces/`. Les lecteurs ont une échéance et refusent les pertes,
troncatures et indices incohérents. Aucun service permanent ni flash implicite.
Les horodatages sont ceux du traitement firmware, pas une mesure de latence complète.

Le CDC modifie la signature du périphérique et le lecteur ajoute des requêtes.
Les mesures instrumentées ne prouvent pas encore une classification sur le firmware
normal. Capturer plusieurs démarrages par OS, puis Linux, hubs, reconnexion et réveil.
Ne pas transformer une heuristique Mac/Windows en garantie générale.

Depuis ce dossier, tests du code embarqué : compiler `tests/usb_trace_store_test.c` avec
`firmware/usb_trace_store.c`, `-Ifirmware -std=c11 -Wall -Wextra -Werror`, puis
exécuter les assertions. Les validations d’outillage restent syntaxe, préflights,
cadrage de captures synthétiques et builds réels, sans nouvelle suite d’outillage.
La variante `wire macos usb-trace` et les quatre paires macOS wire/Bluetooth,
normal/debug compilent et passent les gardes. Les lecteurs sont validés par trois
captures physiques Mac et trois Windows sur V1 après flash utilisateur. PowerShell
est exécuté par l’utilisateur sur le PC, pas par l’agent sur le Mac.
Aucun flash ou commit n’est implicite dans ce chantier.
Les huit UF2 macOS usuels sont identiques octet pour octet aux builds précédents.
Les quatre sous-arbres keymap, behaviors, conditional_layers et macros de la
variante trace sont identiques au normal sur les deux moitiés, après normalisation
des phandles. Les appels vers les deux wrappers sont vérifiés dans le désassemblage
ARM, pas seulement par leur présence dans le code source. Dix UF2 passent les
contrôles SHA-256 et des blocs. Le stockage passe les tests C avec ASan/UBSan.
Les préflights normal, debug et usb-trace de flash, par profil et chemin explicite,
sont vérifiés avec stdin fermé, sans écriture USB. Les conteneurs éphémères ont disparu.
Les trois captures privées de `.local/usb-traces/`, suffixes `zCkuMW`, `YTIKT9`
et `mzhaXU`, contiennent chacune 24 SETUP, dropped=0 et rejected=0. Les paramètres
et l’ordre sont strictement identiques, hors horodatages. La séquence standard
commence entre 608 et 614 ms après démarrage. Le device descriptor est demandé sur 8 puis 18
octets, les strings sur 2 puis leur longueur, la configuration sur 9 puis 100,
le report HID sur 176 et les langues deux fois sur 255 octets.
Les 9 requêtes suivantes sont CDC sur l’interface zéro, à l’ouverture du lecteur :
SET_CONTROL_LINE_STATE (0x22), puis huit SET_LINE_CODING (0x20).
Le lecteur a donc récupéré les échanges précoces sans être ouvert au démarrage.
Ne pas considérer ces observations comme une signature exclusive de macOS.
Les trois captures Windows fournies dans Downloads sont copiées dans le dossier
ignoré `.local/usb-traces/`, sans modifier les originaux : suffixes `acc35a52`,
`9bc2d113`, `45ccd398`. Elles contiennent 57, 58 et 57 requêtes, aucun rejet ni perte.
Les première et troisième séquences sont identiques hors temps. La deuxième
répète sa première demande de descripteur périphérique sur 64 octets une fois de plus,
puis rejoint exactement la même séquence. Ne pas utiliser un nombre fixe de requêtes
ou un ordre intégral rigide pour classer. Les 28 requêtes CDC de chaque capture
sont sur l’interface zéro : quatre pendant l’initialisation du pilote, puis 24
à l’ouverture du lecteur. Elles sont exclues de la comparaison des indices HID/standard.
Hors CDC, Windows compte 29, 30 et 29 requêtes contre 15 sous macOS.
Les indices présents dans les trois essais de chaque machine sont : premier
GET_DESCRIPTOR device demandé sur 8 octets sous macOS, 64 sous Windows,
GET_DESCRIPTOR device qualifier (0x0600) sur 10 octets sous Windows uniquement,
report HID (0x2200) demandé sur 176 contre 240 octets, HID SET_IDLE (0x21/0x0a,
interface 2) présent sous Windows et absent des captures Mac. Ce sont des demandes,
pas des réponses validées ni une preuve de fonctionnalité supportée.
Les six captures instrumentées permettent de distinguer ces deux hôtes dans cet
échantillon. Linux, d’autres versions d’OS, hubs et descripteurs normaux restent
inconnus. Ne pas transformer l’absence d’un indice en certitude de l’autre OS.
Un classificateur futur doit rester diagnostique d’abord, gérer inconnu/ambigu et
les répétitions. Aucun changement de keymap n’est activé par cette analyse.

Le lecteur PowerShell est validé sur Windows avec `-ExecutionPolicy Bypass`,
limité à cette invocation, pas une modification de politique persistante ou une
raison d’exiger un terminal administrateur. Aucun réglage Windows n’est changé par
l’agent. Le script source est désormais UTF-8 avec BOM, conforme à Windows PowerShell
5.1, pour corriger `Capture complÃ¨te`. Les captures restent UTF-8 sans BOM.
Le changement d’encodage seul ne change pas la logique, son affichage doit être
revalidé sur le PC. L’autorisation web Microsoft couvre les documentations
PowerShell sur politiques d’exécution et encodage pour ce diagnostic.
Le validateur AWK accepte les fins de ligne LF et CRLF, afin de contrôler directement
les fichiers Windows sans conversion ni modification des originaux.
Le message Make indiquant que le port est à jour est sans rapport avec le succès
de la collecte : le chemin du périphérique est aussi traité comme une cible Make.

