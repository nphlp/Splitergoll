# Étude des signatures d’OS

Ce dossier contient toute l’instrumentation d’étude, séparée du clavier dans
`../../keyboard/`. Les lecteurs sont `capture.sh` et `capture.ps1`, le validateur
est `check.awk`. Le firmware, son snippet et ses tests restent dans leurs sous-dossiers.
La commande publique `make usb-capture` reste à la racine du dépôt.

Le chantier actif est le diagnostic passif USB, puis BLE. V1 sans batterie peut
tester les deux transports, avec alimentation USB. Ne pas confondre la source
d’alimentation et l’hôte Bluetooth. Une keymap fixe garde ses couches, `wire auto` applique la décision via le module clavier.

`make build wire <keymap> usb-trace` et le même argument final de `make flash`
isolent caches, artefacts et métadonnées du normal et de `debug`. `usb-trace`
est refusé en matériel `bluetooth`. La droite reste normale, la gauche ajoute
le snippet local `splitergoll-usb-trace`. Son CDC est indépendant de la console
ZMK, sans logs de frappes, clé BLE ou effacement NVS. La sélection de couches dépend
du choix explicite `auto`, pas de l’activation du diagnostic.
La keymap doit rester identique entre les captures comparées.

Le module local déclaré dans `firmware/zephyr/module.yml` compile `firmware/` uniquement
avec `SPLITERGOLL_USB_TRACE`. Les dépendances figées restent intactes. L’observateur partagé est compilé par le
module `keyboard`. Trois wrappers
de liens encadrent `usb_dc_ep_set_callback`, `usb_dc_ep_read` et
`usb_dc_set_status_callback`, en transmettant
les arguments et statuts originaux. Seuls les en-têtes SETUP de l’endpoint zéro OUT
sont copiés après lecture. Aucun payload de contrôle ni rapport HID n’est conservé.
La pile nouvelle, LTO et les SoC autres que nRF52840 sont refusés par CMake.

Le tampon append-only contient 256 requêtes, garde les premières et compte les
pertes et tailles incorrectes avec saturation. Le pilote lit les SETUP complets
de huit octets, avec une longueur de retour NULL dans la pile figée actuelle.
Le verrou protège uniquement les copies, jamais l’envoi série. La commande D
restitue un instantané avec cadrage BEGIN/SETUP/END et compteurs, sans effacement.
L’envoi vérifie DTR et borne l’attente par ligne. Un reset du bus ou une déconnexion efface les index et compteurs du cycle précédent.

`make usb-capture <port>` lit sur macOS/Linux, `capture.ps1 -Port COMx`
sur Windows. Les ports sont explicites. Bash conserve les captures dans
`.local/usb-traces/`. PowerShell est un fichier autonome à copier dans un dossier
quelconque : aucun `$PSScriptRoot`, dépôt ou fichier voisin n’est nécessaire. Il écrit
son `.txt` directement dans le dossier courant PowerShell (`Get-Location`), avec un
chemin absolu basé sur `ProviderPath`, et refuse un provider autre que FileSystem.
Ne pas utiliser le répertoire courant .NET, qui peut différer de celui de PowerShell.
Ne pas créer d’arborescence ni déplacer la destination vers le parent du script.
Les noms Windows sont ignorés par Git si la commande est lancée dans le dépôt.
Les lecteurs ont une échéance et refusent les pertes,
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
des phandles. Les appels vers les wrappers de lecture et endpoint sont vérifiés dans le désassemblage
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
Les profils fixes restent passifs. Le même classifieur sert maintenant à `auto`,
qui garde une attente explicite pour inconnu/ambigu.

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

## Classifieur USB

Le code est propre à Splitergoll. Les références sont la
[documentation QMK](https://docs.qmk.fm/features/os_detection),
[FingerprintUSBHost](https://github.com/keyboardio/FingerprintUSBHost) et les
[captures Cormoran](https://github.com/cormoran/zmk-feature-os-detection/blob/3052679f645b8fc1997275c2eaa6b86bdd55ca01/docs/fingerprints.md).
Les motifs publics et nos six captures servent de données, aucun code QMK ni module
externe n’est intégré. Ne pas activer BOS, modifier bcdUSB ni ajouter un fork ZMK
pour ce diagnostic. Les tailles HID 176/240 ne participent plus à la décision.

`../../keyboard/src/os_detection/usb_os_classifier.c` et son interface dans
`../../keyboard/include/splitergoll/` sont du C indépendant de Zephyr.
Le diagnostic rejoue les records au début de la demande D. L’observateur partagé
met également à jour un état borné à la réception des SETUP pour le firmware auto. Ses bits décrivent des demandes, pas leurs réponses
ni une probabilité. L’horloge de stabilisation est celle du firmware au début de
l’instantané, jamais l’heure d’arrivée des lignes ni la fin de l’envoi UART.

Les états par chaîne sont bornés à 16 couples index/langue. Un motif exige le
sondage puis une relecture du même couple. La lecture de l’index zéro décrit les
langues, elle est exclue. Les répétitions ou traductions d’un même index ne comptent
pas comme plusieurs noms indépendants. macOS exige des motifs 2 puis complet sur
deux noms distincts. Windows exige un motif 4 puis complet et des lectures sur 255
pour deux noms distincts. Les sondes device sur 8/64 sont complémentaires, jamais
obligatoires. Si présentes, elles peuvent contredire un motif de noms et le rendre ambigu. Sur nos trois captures Windows, seul le produit a le motif 4 puis 24,
plusieurs fois : ne pas exiger deux noms différents pour ce motif-là.

Le qualifier reste un indice affiché, sans décider de l’OS. CDC et HID ne contribuent
ni à l’OS ni à sa stabilisation. Les sondages contradictoires ou les deux familles de
motifs donnent ambigu. Un motif incomplet, une perte/rejet ou un dépassement de
capacité donne inconnu. Aucun OS par défaut et aucune règle Linux ne sont inventés.
250 ms sans nouvelle demande utile sont nécessaires, avec soustraction uint32 pour
le passage de l’horloge par zéro. Ce délai ne bloque pas la saisie.

Le callback de statut remet count/dropped/rejected à zéro sur USB_DC_RESET et
USB_DC_DISCONNECTED et change la génération. Il transmet l’événement original à
Zephyr, y compris les paramètres, sans modifier suspend/resume ni demander un reboot.
Un dump qui change de génération est interrompu sans END pour être refusé par le
lecteur, plutôt que mélanger des records de deux cycles. Les anciens octets ne sont
plus lisibles par les index, sans memset du tableau dans le callback. Le contrôle
de génération protège les lectures et la fin du dump. Aucun événement de couche
n’est produit par le stockage. La sélection propre à `auto` est détaillée dans
`../../keyboard/src/os_detection/AGENTS.md`. Les cycles sans coupure restent à tester.

La ligne facultative `USBTRACE OS result=<macos|windows|unknown|ambiguous> evidence=<4 hex>`
précède END. Les lecteurs acceptent les captures version 1 sans cette ligne, signalent
son absence et ne recalculent pas la décision en Bash ou PowerShell. Les captures
privées restent intactes. Les lecteurs Bash/PowerShell et leurs accents sont confirmés sur les deux hôtes
par l’utilisateur. Aucun runtime PowerShell n’est disponible sur ce Mac.

Le stockage garde son test dans `tests/`, avec `-Ifirmware`. Le classifieur et son
test sont dans `../../keyboard/`, avec `-I../../keyboard/include`. Les deux commandes
emploient `-std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined` et assertions actives. Le classifieur
couvre motifs, ordre, répétitions, langues, perte, ambiguïté, débordement de capacité,
perturbations CDC/HID, délai et horloge 32 bits. Le stockage vérifie aussi la séparation
des générations, dont une lecture ancienne après remplacement de l’index zéro.
Les six captures se rejouent avec le même C : trois macos 0005 et trois windows 003a.
Leurs résultats sont locaux à ces deux machines, sans garantie de généralité.

Validation de cette version : les deux suites C passent avec ASan/UBSan. Le rejeu
garde les octets, timestamps et uptime_ms d’origine des six captures. `wire macos`
et `wire macos usb-trace` compilent et passent les gardes, SHA-256 et le contrôle des
quatre UF2. Les deux UF2 normaux et la droite trace restent identiques aux précédents.
Keymap, conditions, comportements et macros compilés sont identiques. La garde de
build exige aussi le wrapper de statut. Les essais matériels sont décrits ci-dessous.
Les conteneurs de validation ont disparu et les trois services partagés sont conservés.

## Validation sur V1 et sonde initiale facultative

L’utilisateur a flashé le diagnostic sur V1. La capture Mac p5lqtj contient
24 requêtes et annonce macos 0005. La capture Windows 59f3534e fournie dans Downloads
est copiée sous .local/usb-traces sans modifier l’original, avec 53 requêtes, zéro
perte et zéro rejet. Elle annonce unknown 0038 avant correction. Tous les motifs
Windows sont présents : sondage produit 4 puis 24, lectures sur 255 de plusieurs
noms, qualifier sur 10. Son premier device est demandé sur 18, aucun 64 n’est visible.
Le délai de stabilisation est largement écoulé. Le veto était la condition obligatoire
de première lecture sur 64, pas un échec du lecteur ou des communications du clavier.

Le reset par cycle peut retirer une phase de sondage antérieure. Sans journal de
statut dans cette capture, ne pas présenter cette origine comme prouvée : on sait
seulement que la sonde 64 n’est pas dans le cycle collecté. Les motifs de noms
suffisent désormais, les sondes 8/64 restent des indices et des veto si contradictoires.
Le test de régression échoue sur l’ancien C, puis passe après correction. Il couvre
Windows et macOS commençant sur 18, le délai, les pertes et une sonde contraire tardive.
Le rejeu C des huit captures donne quatre macOS, trois Windows 003a et un Windows 0038.
La reconnaissance Windows corrigée est confirmée après nouveau flash par l’utilisateur.

Le build correctif `wire macos usb-trace` passe en 11 secondes. Les deux UF2 passent
les gardes, SHA-256 et les bornes mémoire. Droite et firmwares normaux sont inchangés.
Les deux suites C passent avec ASan/UBSan. Le conteneur éphémère a disparu et les
trois services partagés sont conservés. Les copies UF2 sont effectuées par l’utilisateur.

L’utilisateur fournit ensuite la capture Windows 97b7bcbd dans son message. Elle est
conservée dans .local/usb-traces en LF telle que reçue, pas comme copie binaire du
fichier Windows. Son cadrage est valide : 53 SETUP, zéro perte/rejet, puis
result=windows evidence=0038. Le rejeu avec le C réel confirme ce résultat.
Le device commence encore sur 18. Les motifs et paramètres des SETUP sont identiques
à 59f3534e, hors timestamps, mais le firmware annonce cette fois Windows.
Le retour au Mac avec ce même firmware corrigé est confirmé par la capture hLZ5V6 :
24 SETUP, dropped=0, rejected=0, result=macos evidence=0005. Le validateur et le rejeu
C la confirment. Les paramètres et l’ordre des requêtes sont identiques à p5lqtj,
hors timestamps. L’aller-retour Mac/Windows/Mac est donc validé sur V1. Le départ
Mac utilisait le diagnostic avant correction, Windows et le retour Mac le même
firmware corrigé sans reflash entre les deux. Cela ne valide pas un reset de bus
sans coupure d’alimentation, les autres ordinateurs, Linux, BLE ou le fonctionnement sans CDC.

## Capture d’un firmware auto

Le même `usb-trace` fonctionne avec `make build wire auto usb-trace`. Une ligne
facultative `USBTRACE ACTIVE result=<macos|windows|unknown>` suit OS et précède END.
Elle expose la sélection réellement appliquée pour le cycle actuel, pas l’hypothèse
recalculée lors du dump. Les deux résultats peuvent diverger si des indices tardifs
arrivent après la sélection figée, ou si une touche maintenue retarde son application.
Les lecteurs acceptent les anciens formats sans ACTIVE, refusent doublons et ordre
invalide, et n’affirment plus que toute capture laisse les couches inchangées.
Le lecteur PowerShell modifié reste à essayer sur le PC, aucun runtime n’est disponible ici.

## Usage Windows autonome

Le script canonique reste `capture.ps1` ici, sa copie peut se placer à la racine de
n’importe quel dossier de travail. La commande utilisateur est
`powershell -NoProfile -ExecutionPolicy Bypass -File .\capture.ps1 -Port COMx`.
Le fichier de capture est créé dans le dossier courant, sans sous-dossier. Son nom
garde la date UTC, le suffixe aléatoire et le contenu UTF-8 sans BOM. Le source garde
son BOM pour Windows PowerShell 5.1. Aucun module, fichier de validation voisin,
mode administrateur ou modification persistante de la politique n’est demandé.
Le changement de destination est contrôlé par revue du diff. Pas de runtime
PowerShell disponible sur le Mac, donc la prochaine capture Windows doit confirmer
le nouveau chemin en conditions réelles. Aucun test automatisé d’outillage ajouté.
