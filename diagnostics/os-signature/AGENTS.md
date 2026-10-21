# Étude des signatures d’OS

## Commandes publiques

make usb et make ble affichent leurs aides. usb.sh accepte capture avec un port.
ble.sh accepte capture/memory/profiles avec un port et select/forget avec port puis
index. profiles envoie S, select envoie l’index explicite, aucune action free dans
la CLI Make. Les arguments sont contrôlés avant de déléguer à capture.sh, sans nouveau
protocole ni changement des gardes série ou de la confirmation d’effacement.
Action absente : aide et succès. Action erronée ou incomplète : aide et échec.
Les anciens alias à tirets sont retirés. Le script PowerShell autonome ne change pas.

## Diagnostic Bluetooth

`make build bluetooth <keymap> ble-trace` ajoute uniquement sur la gauche le
snippet `splitergoll-ble-trace` et le module de ce dossier. Le clavier reste en
sortie Bluetooth, USB HID désactivé, sans changement de keymap, délais, protections
ou appairages. La droite est identique au normal. `auto` fonctionne aussi en Bluetooth,
depuis la mémoire de l’appareil sélectionné, indépendamment du port de diagnostic.
Le sélecteur USB et son classifieur ne sont pas compilés dans ce diagnostic.
Le protocole humain est [../../docs/detection-os-bluetooth.md](../../docs/detection-os-bluetooth.md).

`keyboard/src/os_detection/ble_observer.c` filtre le rôle local `BT_CONN_ROLE_PERIPHERAL` pour ne pas observer
la droite. Les callbacks Zephyr suivent connexion, sécurité, paramètres proposés
et négociés, MTU et fonctionnalités distantes. `BT_REMOTE_INFO` expose les données
échangées par la pile, `BT_REMOTE_VERSION` reste désactivé. Aucun GATT client
supplémentaire ne sonde l’ordinateur. Les fonctionnalités décrivent sa radio,
pas une preuve d’OS. Les callbacks ne touchent ni aux couches ni à NVS.

Le wrapper `bt_l2cap_recv` correspond exactement à la signature interne du commit
Zephyr figé. Il lit le tampon réassemblé sans le consommer, puis appelle l’original
une seule fois avec les mêmes arguments. Le lien entre `conn.c` et `l2cap.c` est
vérifié dans le désassemblage ARM, sans modification de dépendance. LTO et EATT
sont refusés, le parseur observe uniquement le canal ATT fixe `0x0004`.
Pas d’en-tête privé Zephyr importé ni de patch de ses sources.

`ble_trace_store.c/.h` est pur et testé en natif. Il conserve au plus 256 événements
append-only depuis le démarrage, sur plusieurs sessions numérotées. Les branchements
USB et lectures D ne le réinitialisent pas. Aucun payload n’est conservé : la liste
blanche extrait opcode, longueur, handles, plages, offsets et UUID 16 bits des
requêtes. Une recherche par UUID 128 bits garde sa forme mais pas cet UUID.
Les notifications, réponses, SMP et canaux autres que ATT sont ignorés. Les valeurs
des écritures, adresses et clés ne sont jamais copiées. Seul le type d’adresse est
noté. La table ATTR donne les UUID publics locaux par handle, sans leurs valeurs.
Read Multiple conserve le premier handle et le nombre, pas toute sa liste.

Le diagnostic suit jusqu’à CONFIG_BT_MAX_CONN connexions, borné à six par un
BUILD_ASSERT. Le rôle local exclut le split. bt_conn_index indexe des canaux RAM,
chacun reçoit un nouveau numéro de session à chaque connexion. Déconnexion et
réutilisation d’un index ne transfèrent jamais les preuves. Les événements de
plusieurs hôtes peuvent s’entrelacer dans le même tampon de 256 enregistrements.
L’emplacement est résolu par zmk_ble_profile_index sans exporter l’adresse. Les
erreurs natives, dont -ENODEV avant appairage, sont normalisées en -1 inconnu. Un
emplacement inconnu peut être complété après appairage, un remplacement inattendu
sur la même session invalide la capture. Aucun OS n’est déduit de l’emplacement.
Les compteurs dropped/rejected saturent et font refuser la capture par le lecteur.
Un dump copie les métadonnées de son instantané sous spinlock, puis lit les index
immuables sans tenir le verrou pendant l’envoi série. Les ajouts ultérieurs ne font
pas partie de cet instantané. Le reset physique remet la RAM à zéro, jamais NVS.

Le transport extrait dans `trace_console.c/.h` est partagé avec USBTRACE : attente
DTR, interruption TX et échéance par ligne inchangées, thread séparé du Bluetooth.
La commande D restitue BEGIN version=2, EVENT, ATTR, puis une paire HOST/OS par
session et END avec le nombre d’attributs. connected dans BEGIN compte les hôtes
actifs, de zéro à six. Les sessions sont numérotées par leur ouverture, pas par
ordre de fermeture. Les lecteurs gardent aussi la compatibilité version 1.
Chaque EVENT porte index, uptime, session, type et cinq champs hexadécimaux :

- CONNECTED : intervalle, latence, timeout, type d’adresse, emplacement + 1.
- DISCONNECTED : raison, dernier emplacement + 1, puis zéros.
  Le code zéro désigne un emplacement inconnu. En version 1 ces champs étaient zéro.
- SECURITY : niveau, erreur puis zéros.
- PARAM_REQUEST : minimum, maximum, latence, timeout, zéro.
- PARAM_UPDATED : intervalle, latence, timeout puis zéros.
- MTU : TX, RX puis zéros. FEATURES : deux mots little-endian de 32 bits puis zéros.
- ATT : opcode, longueur de PDU, puis trois champs selon l’opcode. MTU/read :
  valeur/handle. Find Information : début/fin. Read Blob/Prepare Write : handle/offset.
  Read By Type/Group et Find By Type Value : début/fin/type. Write : handle seul.
  Read Multiple : premier handle/nombre. Execute Write : zéros.

Les intervalles sont en unités de 1,25 ms et les timeouts en unités de 10 ms,
comme les API Zephyr. Les horodatages ne mesurent pas la latence physique des touches.
Le cache GATT peut supprimer toute découverte au retour d’un hôte déjà appairé.
Ne pas inventer de règle par défaut macOS, ni réutiliser l’OS de l’alimentation USB.
Attendre les captures Mac/Windows, puis comparer aussi les reconnexions avant tout
classifieur. Linux reste inconnu. Ne pas effacer de bonds pour obtenir plus d’indices.

`make ble capture <port>` utilise `capture.sh --ble`, sorties `.local/ble-traces/`.
Le même `capture.ps1` autonome accepte `-Port COMx -Ble`, sans administrateur,
sortie `windows-ble-….txt` dans le dossier courant. Les validateurs AWK et PowerShell
contrôlent indices, sessions, nombre d’attributs et absence de pertes. Ils ne font
aucune classification côté lecteur. Les lignes OS du firmware sont présentées après
validation. Les lectures série Windows antérieures sont confirmées, le nouveau
résumé reste à essayer sur matériel.
Le premier essai vise V1/Mac puis Windows, sans lecture de console anticipée :
attendre le signal de disponibilité et distinguer flash, appairage et capture.

Validation : suite native `ble_trace_store_test.c` avec `ble_trace_store.c`,
`-Ifirmware -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined`.
Elle couvre sessions, limites, compteurs saturés, tailles, offsets/UUID, exclusion
SMP/notifications et invariance du stockage quand les valeurs écrites changent.
Les trois suites USB restent pertinentes. Pas de nouvelle suite de tests d’outillage.
La lecture UART, le rôle vu par les callbacks et la reconnexion nécessitent le matériel.

Validation réalisée avant flash : quatre suites C natives passent avec ASan/UBSan.
La paire `bluetooth macos ble-trace`, la paire normale Bluetooth et `wire auto usb-trace`
compilent. Les six UF2 passent SHA-256, blocs/famille/adresses. La paire finale de
trace est identique à celle contrôlée. Couches, comportements, conditions et macros
BLE restent identiques au normal, ainsi que matrice et encodeurs. La droite trace
est identique octet pour octet au normal. Les seize UF2 manuels restent identiques
aux références, les six Devicetrees auto conformes aux profils fixes.
Les deux appels ARM de conn.c passent par le wrapper, qui délègue à l’original.
Les validateurs AWK/PowerShell acceptent LF/CRLF et refusent neuf captures altérées.
Le script PowerShell passe le parseur officiel local, sans exécution série Windows.
Syntaxe Bash/POSIX, arguments incompatibles et diff contrôlés. Aucun avertissement
nouveau dans le diagnostic, ceux du socle restent visibles. Aucun flash ni lecture
matérielle effectué, aucun conteneur de build restant, trois services préexistants
conservés. Les essais physiques Mac puis Windows restent à réaliser.

### Contrôle explicite des emplacements

La keymap existante contient BT_CLR_ALL, mais aucun BT_SEL/BT_NXT/BT_PRV. Le
premier diagnostic ne permettait donc pas le passage sûr à un second hôte. Pour
préserver la case du Mac, le nouveau firmware gauche ajoute `ble_trace_profiles.c`
et la politique pure `ble_profile_control.c/.h`, sans modifier la keymap ni les UUID.
Ce contrôle est limité au port physique du diagnostic `ble-trace`, absent du normal,
de debug et de usb-trace. La lecture D reste passive et ne choisit aucun emplacement.

Commandes UART : S lit, F choisit le premier emplacement libre, 0 à 9 sélectionnent
explicitement un index connu. La limite est dix, contrôlée au build et dans la
politique, avec cinq emplacements actuellement compilés. Le statut expose index
actif, paired, connected et délai de sauvegarde, sans adresse ni clé.
`zmk_ble_prof_select()` est le seul appel de mutation : aucune suppression de bond,
NVS directe ou modification de préférence USB n’est ajoutée. Une sélection explicite
change le profil actif, sauvegardé par ZMK après `CONFIG_ZMK_SETTINGS_SAVE_DEBOUNCE`,
actuellement 60 000 ms. Couper le courant avant ce délai peut restaurer l’ancien index,
pas effacer un bond. L’USB peut rester sur le Mac pendant toute la bascule BLE.

La politique refuse une case hors limites, un libre absent ou une bascule pendant
la saisie. La présence d’un hôte connecté n’est plus un motif de refus. Le wrapper
zmk_event_manager_raise de ce diagnostic suit les 44 positions physiques et
l’activité des keycodes/encodeurs, sans filtrer ni modifier les événements.
Un mutex récursif sérialise cette activité et la sélection native. La garde vérifie
les positions maintenues, les rapports HID clavier/consumer, les modificateurs
explicites et 750 ms depuis le dernier événement d’entrée. Ce drainage concerne
uniquement une commande de bascule, jamais les frappes ordinaires ni le classifieur.
Il couvre les comportements différés actuels. Une macro future dépassant cette
fenêtre sans événement devra faire réexaminer ce contrat. Aucune sélection différée
ou nouvelle tentative automatique : busy rend la main à l’utilisateur.
Les appels de sélection utilisent toujours zmk_ble_prof_select, sans déconnexion
forcée, suppression de clé, changement de préférence de sortie ni nouvelle keymap.
S et une sélection de l’index déjà actif n’écrivent rien. F ne choisit jamais une
case occupée. Ne pas réessayer F automatiquement après timeout, lire S pour connaître
l’état puis réutiliser un index précis après un premier appairage Windows.

Le protocole BLEPROFILE émet BEGIN version=2, SLOT dans l’ordre, RESULT avec commande/statut,
puis END. L’état peut évoluer pendant l’envoi des SLOT si l’hôte se reconnecte.
Les lecteurs vérifient les bornes, les comptes, la commande attendue et la cohérence
active/selected. Un refus busy/full/invalid/failed rend une erreur et conserve la
réponse. Les lecteurs acceptent aussi version=1, dont busy signifiait hôte connecté.
Le lecteur n’effectue jamais de nouvelle tentative de sélection.

`make ble profiles <port>` lit le statut et `make ble select <port> <index>`
sélectionne un emplacement, via `capture.sh --profile` avec status ou un index.
Le lecteur Bash n’envoie plus F. Le protocole firmware reste compatible avec les
captures historiques. Sur Windows le même fichier autonome `capture.ps1 -Port COMx -Profile
<status|free|index>` suffit, sans -Ble ni administrateur. Les sorties Mac restent
dans `.local/ble-profiles/`, Windows `windows-ble-profile-….txt` dans le dossier
courant. Les jeux PowerShell Profile, Serial et UsbEvents sont exclusifs.
Le [guide Windows](../../docs/capture-bluetooth-windows.md) précise flash gauche,
lecture de la case Mac, coupure Bluetooth Mac, alimentation PC, sélection libre,
appairage Windows, capture -Ble et retour à l’index Mac sans oublier aucun appareil.

La politique est couverte par `ble_profile_control_test.c` : 81 920 combinaisons de
commande, masque, index et état de saisie pour les cinq emplacements, plus bornes
0/10/11 et bits hors capacité. Les cinq suites C passent avec ASan/UBSan. Build BLE
réussi, droite identique au précédent, keymaps et comportements inchangés, deux UF2
contrôlés intégralement. Les validateurs acceptent LF/CRLF et refusent cinq réponses
altérées. PowerShell passe le parseur officiel local. L’utilisateur a ensuite flashé
la gauche et validé le contrôle S sur Mac, F sous Windows et le retour explicite à 0.
Les API série Windows de sélection et de capture sont donc essayées sur le PC.
Aucune sélection d’emplacement, écriture série ou opération matérielle faite par
l’agent pendant cette préparation. Aucun effacement d’appairage autorisé.

### Première observation sur V1/Mac

L’utilisateur confirme avoir flashé les deux moitiés et connecté V1 au Mac par BLE.
La lecture de gauche sur `cu.usbmodem1101`, seul port présent et libre, donne la
capture privée `.local/ble-traces/macos-2026-10-19-13-09-53-XMA99y` : 49 événements,
une session active, dropped=0, rejected=0, 52 attributs. Le lecteur se termine
normalement et l’absence de lecteur restant est vérifiée avec lsof. Aucun reset
ni flash effectué par l’agent. Cela valide ce premier parcours de capture physique.

Les 42 requêtes ATT comprennent découverte primaire, lectures par type de noms,
informations HID `2a4a`, Report Map `2a4b` avec offsets 64 puis 128, PnP `2a50`,
références de rapports `2908`, abonnements CCC et lecture des deux services batterie.
Les deux mesures de batterie ne sont pas copiées. GAP Appearance `2a01` n’est
pas lu dans cette fenêtre. La présence de PnP ou HID Info ne suffit donc pas à
conclure Windows. Ne pas extrapoler de l’absence d’Appearance une règle Mac.
La découverte observée ne prouve pas qu’aucun bond ne préexistait au flash.

La connexion commence à 62 883 ms avec intervalle 24, latence 0, timeout 72,
adresse de type public, puis passe à intervalle 12, latence 22, timeout 200.
La requête MTU demande 527, le callback annonce TX 527 / RX 65. Ne pas résumer
ces deux champs par un MTU symétrique 527 : les fragments de Map ont 64 octets.
SECURITY passe au niveau 2, erreur 0, à 64 764 ms. Les fonctionnalités sont
`f701492f / 000000e9`, des indices de contrôleur et non une identité d’OS.
Le dernier événement collecté est à 67 914 ms. Ces temps sont ceux du firmware,
pas une mesure de latence de saisie. Reconnexion Mac et captures Windows restent
à effectuer avant toute classification. Les logs bruts ne sont pas versionnés.

### Reconnexion sur V1/Mac

Après déconnexion du seul clavier par l’utilisateur, sans reset ni coupure USB,
la capture privée `macos-2026-10-19-13-31-49-Hg5XOJ` contient
77 événements, deux sessions, zéro perte/rejet et les mêmes 52 attributs.
Les 49 événements de la première capture restent strictement identiques. L’index
49 ferme la première session avec raison `0x13`. La seconde comprend 27 événements,
dont 20 requêtes ATT. Le lecteur est fermé, absence vérifiée par lsof.
L’utilisateur précise que la reconnexion était automatique et immédiate à son
échelle. La trace donne 1 054 ms entre DISCONNECTED et CONNECTED. Ne pas décrire
cet essai comme une reconnexion manuelle ni exiger un délai hors connexion.

Le `zmk/app/src/ble.c` figé explique le mécanisme : `disconnected()` soumet
`update_advertising_work` sans traiter différemment la raison `0x13`. Si la cible
active n’est plus connectée, `update_advertising()` relance `ZMK_ADV_CONN` via
`bt_le_adv_start`. Il n’y a pas de suspension après une déconnexion volontaire.
Le clavier reste périphérique BLE, le Mac réétablit la connexion. La documentation
officielle Bluetooth Behavior confirme la reconnexion immédiate du profil actif,
même quand la sortie USB est sélectionnée. L’observateur ne relance pas les connexions. Le contrôle explicite UART ajouté
ensuite peut sélectionner un autre emplacement, uniquement sur commande utilisateur.
Ne pas présenter cette politique ZMK comme une obligation Bluetooth ni une
modification de nos traces.

CONNECTED est à 1 413 784 ms, l’instantané à 1 434 280 ms : lecture 20 496 ms après
reconnexion, au-delà des dix secondes du protocole. Le dernier événement est à
1 418 805 ms, soit 5 021 ms après connexion. Le délai prévu est respecté,
aucune nouvelle capture n’est nécessaire pour ce point. Ces écarts viennent
de l’horloge du firmware, pas du temps de réponse du chat.

SECURITY niveau 2 sans erreur précède les requêtes ATT de cette session. Le Mac
demande encore MTU 527, les callbacks annoncent TX 527/RX 65 et les mêmes fonctionnalités.
Les paramètres finaux restent intervalle 12, latence 22, timeout 200.
Le Mac recherche/lit encore les UUID DIS fabricant/modèle/PnP, mais ne relit plus
HID Info `2a4a`, Report Map `2a4b` ni les références `2908`. Il réécrit les CCC HID,
redécouvre les caractéristiques batterie et relit les deux Battery Level. Appearance
`2a01` reste absent. Cette réduction est compatible avec le cache GATT, sans lecture
du cache interne du Mac. Ne pas baser une règle sur la présence obligatoire de
Report Map à chaque connexion. Il reste plusieurs indices observables, mais aucune
spécificité Mac n’est démontrée sans captures Windows puis d’autres hôtes.

### Première connexion Windows sur V1

L’utilisateur fournit `windows-ble-2026-10-20-07-19-42-da5e66ed.txt` et
`windows-ble-profile-2026-10-20-07-19-09-923108d7.txt` depuis Downloads. Les copies
privées dans `.local/ble-traces/` et `.local/ble-profiles/` conservent exactement
les octets CRLF, avec empreintes comparées et sans modification des originaux.
Les deux validateurs AWK les acceptent. Les deux lecteurs PowerShell ont terminé
normalement et créé leurs fichiers dans le dossier courant, confirmé par l’utilisateur.

Le statut Mac D8787Z montre active=0, case 0 appairée et connectée, autres libres.
F sous Windows choisit 1, sans toucher à la case 0. La capture BLE annonce une
session connectée. Au retour Mac, ZXU2Bp montre cases 0 et 1 appairées et le choix
de 0 avec changed=1, status=ok. Les deux appairages sont conservés et le sélecteur
revient bien à la case Mac. Ce dernier dump montre connected=0 pendant que Bluetooth
Mac est encore coupé, il ne constitue pas une lecture de reconnexion au Mac.
L’utilisateur indique avoir suivi la procédure dans l’ordre, sans reset de bonds.

La capture Windows contient 76 événements, 67 ATT, dropped=0, rejected=0, et les
mêmes 52 handles/UUID que les deux sessions Mac. CONNECTED est à 114 787 ms,
snapshot à 133 335 ms, soit 18 548 ms après connexion. Le dernier événement est à
120 656 ms, soit 5 869 ms après connexion. La marge de dix secondes est respectée.
SECURITY niveau 2, erreur zéro, arrive à 116 189 ms. Le MTU demandé est encore 527,
callback TX 527/RX 65 : aucune différence exploitable ici face aux deux sessions Mac.

Les différences observées sont une exploration plus large et plus de lectures
directes. Windows utilise Find By Type Value `0x06` deux fois, Read By Type pour
Included Service `0x2802` six fois, lit GAP Appearance `2a01`, Device Name `2a00`,
PPCP `2a04`, Client Supported Features `2b29`, Database Hash `2b2a` et les CCC `2902`.
Ces lectures sont absentes des captures Mac comparées, hormis la recherche par type
de Device Name sur Mac. Mac utilise des Read By Type ciblés sur Manufacturer/Model/
PnP, présents aussi à sa reconnexion. La lecture HID Info/Map/PnP est commune aux
premières connexions, les compteurs et l’ordre intégral ne sont pas des garanties.
Le parseur ne conserve pas la valeur UUID recherchée par Find By Type Value,
donc ne pas attribuer son service cible à partir du seul champ type `2800`.

Les fonctionnalités Windows sont `3001492f / 000000ee`, différentes de la radio
Mac mais insuffisantes pour identifier un OS sur d’autres contrôleurs.
Paramètres initiaux : intervalle 48, latence 0, timeout 960. Après les changements
observés : intervalle 12, latence 30, timeout 400, contre 12/22/200 sur notre Mac.
Les paramètres restent des indices de ce couple hôte/pile, pas une signature exclusive.
Les lectures cache/GATT et Appearance peuvent aussi exister sur Linux ou un autre
client. Aucun classifieur BLE n’est écrit à ce stade. La capture Windows déjà appairé
est le prochain test nécessaire : sélectionner l’index 1, pas F, sans nouveau flash
ni suppression d’appareil. Ne pas déclencher une capture matérielle sans son signal.

### Reconnexion Windows déjà appairé

L’utilisateur fournit les fichiers `windows-ble-2026-10-20-07-36-47-56aa0156.txt`
et `windows-ble-profile-2026-10-20-07-36-22-a565d305.txt`. Les copies privées sous
`.local/ble-traces/` et `.local/ble-profiles/` conservent exactement les octets CRLF
et les originaux restent inchangés. Les validateurs AWK BLETRACE et BLEPROFILE
(commande 1 attendue) passent. La case 1 est choisie explicitement avec changed=1,
status=ok, les cases Mac 0 et Windows 1 restent appairées, sans connexion au moment
du contrôle de profil. Aucun nouvel appairage ou effacement n’est rapporté.

La trace contient six événements, une session connectée, dropped=0, rejected=0,
et la même table locale de 52 attributs que les trois sessions précédentes.
CONNECTED est à 59 517 ms, snapshot à 76 178 ms : 16 661 ms après connexion.
Le dernier événement est à 61 101 ms, 1 584 ms après connexion. Le protocole
d’attente est respecté, pas d’indice qu’une capture plus longue manque ici.
L’absence de lectures ne correspond donc pas à une perte annoncée ou une capture
prise avant l’établissement de la sécurité.

La séquence est CONNECTED (12/30/400, adresse publique), FEATURES
`3001492f / 000000ee`, SECURITY niveau 2 sans erreur, requête MTU 527, callback
MTU TX 527/RX 65, puis Read de Battery Level `2a19` au handle 0x0012.
Il n’y a ni Appearance, ni Database Hash, ni Included Service, ni lecture HID Map,
ni nouvelle écriture CCC dans cette fenêtre. Les restaurations et caches existants
sont compatibles avec ce comportement, sans lire leur état interne sur Windows.
Les deux requêtes ATT restantes sont génériques et le MTU est identique à macOS.
Les paramètres et fonctionnalités distantes restent différents sur nos deux
machines, sans preuve de spécificité OS sur un autre contrôleur, Linux ou un autre hôte.

Cette observation invalide une règle sans mémoire exigeant les indices distinctifs
du premier Windows à chaque reconnexion. Elle ne démontre pas l’impossibilité
générale d’identifier un hôte BLE avec d’autres observations. Ne pas transformer
deux lectures ou le silence d’un hôte en Windows par défaut.
Les quatre observations permettent maintenant une décision d’architecture : enrichir
les signatures passives, ou apprendre automatiquement l’OS lorsqu’il est reconnu
puis associer cette connaissance à un appairage authentifié. Cette seconde piste
doit invalider la connaissance lors du remplacement de bond, et traiter le changement
d’OS/dual boot sur une même identité. Elle ne doit pas coder Mac=0, Windows=1,
utiliser l’OS du collecteur USB, imposer une affectation manuelle, ou transformer
une radio reconnue en preuve d’OS. Aucune mémoire d’OS ni sélection BLE automatique
n’est implémentée. Les tâches et la décision restent dans docs/TODO.md.

La mémorisation proposée est un cache de résultat heuristique, pas du machine learning.
Sa mise en œuvre peut être déterministe et testée, mais la qualité de l’identification
initiale reste indépendante. Deux machines et quatre sessions ne démontrent pas une
signature générale. Un cache peut prolonger une erreur ou devenir périmé après un
changement d’OS conservant la même identité Bluetooth. Ne pas promettre une fiabilité
élevée ou chiffrée sur ces seules captures. L’utilisateur accepte le principe d’une
mémoire automatique si la détection initiale est fiable, ce qui reste à démontrer.
Si cette architecture est retenue, prévoir l’origine de chaque décision (indices
actuels ou cache), ses preuves et la version des règles, ainsi qu’une invalidation
du seul cache OS sans supprimer les bonds. Ces exigences d’observabilité ne prouvent
pas la justesse des heuristiques et ne doivent pas imposer une affectation manuelle d’OS.

### Inspirations et décision de poursuite

La recherche porte sur le full auto sans programme compagnon. Le manuel avec mémoire
est désormais une solution de dernier recours demandée par l’utilisateur, pas le
comportement de repli implicite d’un classifieur. Un raccourci du type L1 + clic
encodeur, la restauration après redémarrage/veille et une éventuelle mémoire par hôte
restent à concevoir. Les profils de build évoqués ne sont pas ajoutés. Le suivi reste
dans docs/TODO.md, sans dupliquer une liste de tâches ici.

Sources examinées à leurs commits précis, sans exécuter ni intégrer leur code :

- `cormoran/zmk-feature-os-detection` à `3052679f645b8fc1997275c2eaa6b86bdd55ca01`,
  `src/os_detection_ble.c`, `src/os_detection_settings.c`, `Kconfig` et captures.
  Le code actuel utilise le callback public d’autorisation GATT, accepte toutes les
  lectures, puis attend 1 000 ms de silence sur les caractéristiques surveillées.
  Il restaure éventuellement un résultat persistant par index à la connexion.
  Cela corrige le classement transitoire à chaque lecture décrit comme encore ouvert
  dans ses captures, mais ne résout pas la qualité des règles. PnP + Appearance donne
  Windows, y compris pour sa capture Linux. Report Map + HID Info sans Appearance
  donne Android. Notre premier Mac lit précisément Map (handle 0026) et Info (0024),
  sans Appearance : cette règle produirait Android, par lecture des conditions,
  sans exécution du module tiers. Le repli macOS reste ambigu. La configuration de
  sonde ANCS/AMS existe, mais ce fichier ne lance aucune découverte client et ne
  positionne jamais `ancs_or_ams_present`. Le cache lu ici est indexé par emplacement,
  ce qui ne suffit pas à démontrer une invalidation sur remplacement de bond.
  Licence MIT. Ne pas importer son fork ZMK/Studio ni ses dépendances pour cette étude.
- `darkmentorllc/Blue2thprinting` à `a76b485e12cd506daf1899f49cce69d19c3d7ad7` :
  `Analysis/TME/TME_SMP.py` décode les capacités IO, OOB, AuthReq, taille maximale
  de clé et masques de distribution demandés, pas seulement les lectures GATT.
  `TME_LLCP.py` expose version/sous-version, fabricant et fonctionnalités de radio.
  Ces catégories inspirent des observations supplémentaires, sans prouver une
  signature d’OS ni fournir un composant embarqué directement réutilisable.
  Son LICENSE.md ne fournit pas de licence permissive et mentionne un brevet.
  Le code public n’est pas à assimiler à un module libre intégrable. Aucun code copié.
- `furiousMAC/continuity` à `d9fa98bddafbfa3baafc84e37a417d78a0598546` : documentation
  et dissections des annonces Apple, licence GPL-2.0. Les travaux associés montrent
  des signatures de type/version d’OS sur les appareils étudiés. Notre inférence :
  lier une annonce à la connexion authentifiée reste indispensable, sinon un iPhone
  voisin pourrait être pris pour l’hôte. Aucun scan supplémentaire lancé.
- `mentaldesk/oskey` à `78658510090f60a63cbaf551145c66ad722d2980` : sélection manuelle
  dans `behaviors/behavior_os_selector.c`, licence MIT. Son README confirme l’absence
  de stockage flash. Ce module ne résout ni notre auto-détection ni le repli persistant
  demandé, mais illustre la séparation sélection d’OS / bindings.

Les sondes GATT actives restent une piste distincte : ANCS vise iOS et Apple précise
que sa présence n’est pas garantie. Son absence ne prouve donc ni Windows ni macOS.
Ne pas confondre sonder les services de l’hôte et observer ses lectures du clavier.
La référence est la spécification officielle Apple ANCS. Les métadonnées SMP sont
une piste d’enrichissement, pas une nouvelle collecte déjà autorisée ou implémentée.
Toute instrumentation devrait limiter explicitement opcodes/champs non secrets et
être testée, en préservant le protocole et tous les appairages existants.

La comparaison sémantique des requêtes existantes donne une piste plus précise que
la seule présence de lectures HID : le Mac recherche successivement fabricant
`2a29`, modèle `2a24` et PnP `2a50` par Read By Type (`0x08`) dans le service DIS.
Ce motif positif est présent dans sa première capture et sa reconnexion. Windows
lit les mêmes valeurs directement par handles, mais sa première capture ne fait
pas ces trois recherches ciblées. Il effectue aussi une écriture Client Supported
Features (`2b29`), des lectures Database Hash (`2b2a`), Appearance (`2a01`) et des
recherches de services inclus (`2802`). Sa reconnexion n’a aucun de ces indices.
Ce sont des candidats de règles, pas des signatures générales validées. Les handles,
comptages exacts et délais ne doivent pas devenir des constantes de classification.
Étudier d’abord ces séquences positives sur les captures conservées, avec résultat
inconnu si les preuves manquent et ambigu si elles se contredisent. Les comparer à
Linux et à d’autres hôtes reste nécessaire avant toute promesse de généralisation.
Ces observations ont ensuite servi au prototype hors ligne ci-dessous. Aucun cache
persistant ou nouveau diagnostic SMP n’est ajouté.

### Classifieur BLE et intégration au diagnostic

`keyboard/src/os_detection/ble_os_classifier.c` et son interface dans
`keyboard/include/splitergoll/` sont purs, sans Zephyr, allocation, lecture de valeurs
GATT, sélection de couche, profil ou écriture NVS. Initialement testé hors ligne,
il est maintenant ajouté à CMake uniquement sous SPLITERGOLL_BLE_TRACE. Le dump
rejoue son instantané dans un second passage après les ATTR, dans le thread USB.
Les callbacks ne calculent aucun OS. Les handles des lectures
et écritures directes sont résolus avec l’API publique `bt_gatt_foreach_attr` sur
le handle exact, sans lire les valeurs. Un handle absent rend la session non fiable.

`ble_os_observe()` consomme les métadonnées de `ble_trace_record`, avec l’UUID 16 bits
résolu dans la table ATTR pour une lecture/écriture par handle. Zéro signifie non
résolu, sans deviner à partir des handles historiques. Read By Type porte son UUID
dans le record lui-même. Une nouvelle connexion efface les preuves de la précédente.
Les numéros de sessions ne désignent pas des OS, et les attributs ne désignent pas
des emplacements d’appairage. Le chiffrement niveau 2 ou plus est exigé pour conclure,
sans le confondre avec une authentification MITM ou une preuve de bond.

Preuves expérimentales et bits de diagnostic :

- `0001` : Read By Type fabricant `2a29`, modèle `2a24`, PnP `2a50`, dans cet ordre
  sur une même plage non vide. Les requêtes sans rapport peuvent s’intercaler.
- `0002` : lecture directe des trois UUID DIS, après le motif précédent, dans cette
  même plage. Répéter un seul UUID ne remplace pas les autres. `0003` donne macOS.
- `0004` : Write Request d’un octet sur Client Supported Features `2b29`.
- `0008` : Read Request sur Database Hash `2b2a`.
- `0010` : Read Request sur Appearance `2a01`.
- `0020` : recherche Read By Type des services inclus `2802`.
- `0040` : Read By Type de Database Hash `2b2a` sur toute la base GATT `1..ffff`,
  puis lecture directe de cet UUID, puis un nouveau Read By Type identique.
  Les trois indices communs `001c` sont nécessaires, avec `0020` ou `0040` :
  `003c` reconnaît la découverte complète et `005c` la découverte partielle.

Un motif partiel donne inconnu. Les motifs complets des deux OS donnent ambigu
(`003f` pour l’ancienne famille Windows, `005f` pour la nouvelle, `007f` si les deux).
Une capture non fiable (perte/rejet ou table non résolue), une session incohérente,
des champs utilisés invalides, une connexion sans chiffrement ou une erreur de
sécurité interdisent la conclusion. La table non résolue doit être traitée comme
non fiable par le futur adaptateur. Les paramètres radio, MTU, valeurs de batterie,
horodatages et métadonnées host du lecteur ne sont pas des preuves.
La décision est recalculée sur les preuves disponibles : elle peut changer si
des indices tardifs arrivent. Pas de temporisation, latch ou mémorisation automatique.
Cette fonction ne définit pas quand un futur sélecteur pourrait figer une décision.
Linux peut partager ces opérations avec Windows : aucune exclusivité entre OS ni
fiabilité générale n’est démontrée. Les règles décrivent nos observations Mac/Windows.

Validation : `keyboard/tests/ble_os_classifier_test.c`, compilé avec le C du classifieur et
`-Ifirmware -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined`, assertions
actives. Les extraits anonymes correspondent aux requêtes utiles, sans captures
brutes ni identifiants de machine versionnés. Sont couverts motifs complets,
préfixes et preuve manquante, ordre/plage DIS, répétitions, handles déplacés,
UUID non résolus, incohérences, chiffrement et erreurs, ambiguïté dans les deux
ordres, reconnexion sans découverte et remise à zéro entre sessions.
Les six suites natives USB/BLE passent, sans avertissement du nouveau C.

Le pilote temporaire `.local/validation/ble-classifier/replay.c` résout les handles
depuis les ATTR et rejoue les événements originaux dans le même C, par session,
après validation de leur cadrage par check-ble.awk. Les cinq fichiers privés donnent :

- Mac XMA99y : session 1 macos `0003`.
- Mac Hg5XOJ : sessions 1 et 2 macos `0003`. Sa session 1 est la même que XMA99y,
  pas une observation indépendante supplémentaire.
- Windows da5e66ed : session 1 windows `003c`.
- Windows 56aa0156 : session 1 unknown `0000`, sans réutiliser une connaissance antérieure.
- Mac xZhWJD : sessions 1, 2 et 3 macos `0003`.

Les 292 préfixes valides de ces fichiers ne donnent aucune hypothèse contraire à
l’hôte attendu : inconnu reste permis tant que les preuves sont incomplètes.
Inverser le champ host ne change aucun résultat. Perte ou rejet simulé rend la
capture irrecevable et le résultat inconnu. Retirer Appearance du vrai Windows
donne inconnu. Un fichier tronqué est refusé par le validateur. Les sources privées
conservent leurs octets et empreintes. Aucun port série, conteneur, flash ou nouveau
build n’est lancé pendant cette validation hors ligne.

L’intégration ajoute, après les ATTR et avant END, deux lignes par session :
`BLETRACE HOST session=1 profile=0 connected=1`, puis
`BLETRACE OS session=1 result=macos evidence=0003`. HOST donne le dernier emplacement
connu (-1 si inconnu) et l’état de la session à l’instantané, indépendamment de sa
sélection pour les frappes. Les sessions closes et les sessions encore actives sont
toutes représentées, sans changer le nombre de champs EVENT/ATTR ni leurs comptes.
Les lecteurs acceptent zéro ligne OS pour les anciens UF2 v1. En v1, dès qu’une
ligne existe, les résultats sont exigés pour toutes les sessions. En v2, les paires
HOST/OS sont obligatoires pour les sessions 1 à N dans l’ordre, sans doublon,
attribut tardif ou résultat mal formé. Les états HOST doivent correspondre aux
ouvertures/fermetures observées, et l’emplacement aux métadonnées connues. La keymap
reste fixe, même si une hypothèse est disponible. Ces résultats ne sont pas la future mémoire de production.

Le second passage reprend le même count et les indicateurs de perte/rejet du début
du dump. Les index sont immuables. Les ajouts pendant l’envoi ne changent donc pas
son instantané. Les six canaux de connexion sont copiés sous le même verrou pour
stabiliser les associations HOST. Le dump rejoue chaque session dans un classifieur
neuf, en filtrant son numéro : au plus 256 x 256 examens, uniquement à la demande
USB. Aucun calcul GATT, UART ou OS n’est fait sous spinlock. Chaque session garde
sa sécurité et la fiabilité locale de résolution. Aucun tableau de 256 résultats
n’est ajouté à la pile du thread de 2 Kio.
Un échec UART ou de lecture d’un record interrompt sans END pour rendre la capture
irrecevable. Les gardes du build imposent ble_os_observe/ble_os_result dans la map
gauche de trace uniquement.

Validation d’intégration : six suites natives ASan/UBSan vertes, syntaxe shell et
PowerShell contrôlée. Un harnais temporaire sous `.local/validation/ble-integration/`
compile le vrai `ble_trace.c` avec des doubles des API Zephyr/UART/GATT. Les cinq
captures rejouées conservent tous leurs EVENT/ATTR et produisent les mêmes huit
résultats de sessions que le prototype. Le harnais vérifie aussi absence d’UART
sous spinlock, ajouts pendant le dump, handle introuvable et échec de transmission.
Les validateurs AWK et PowerShell acceptent/refusent les mêmes 27 cas LF/CRLF,
anciens et nouveaux formats, lignes manquantes, doubles ou désordonnées, session
hors limites, résumés mal formés et captures tronquées/perdues. Le parseur
PowerShell local ne valide pas les API série Windows ou Windows PowerShell 5.1.
Les paires bluetooth macos ble-trace, bluetooth macos normal et wire auto usb-trace
compilent en 16, 14 et 10 secondes. Les six UF2 passent les gardes de rôles/logs,
SHA-256 et partitionnement. Les quatre UF2 normaux/USB et la droite BLE trace sont
identiques à leurs références. Les Devicetrees des deux moitiés Bluetooth conservent
keymap, behaviors, macros et conditional_layers. Aucun avertissement nouveau :
Docker insecure et KSCAN restent visibles dans ces compilations incrémentales.
Le nouveau diagnostic est dans `dist/firmware/bluetooth-macos-ble-trace-2026-10-20-12-24-14/`.
Tous les conteneurs de validation ont disparu, les trois services préexistants
sont conservés. Ces validations n’ont effectué ni flash, ni lecture série,
ni changement d’emplacement ou effacement de bond.

Après flash de gauche par l’utilisateur et son signal de disponibilité, la capture
réelle `.local/ble-traces/macos-2026-10-20-11-08-37-oTzqIp` confirme macos `0003`
dans le dump du firmware. Elle contient 27 événements, dont 20 ATT, 52 attributs,
une session connectée, sécurité niveau 2 sans erreur, zéro perte/rejet.
Le motif DIS est présent, sans effacement ni nouvel appairage demandé.
USB gauche sur Mac, USB droit sur PC Windows pour alimentation : cette topologie
ne change pas l’hôte BLE observé. Le port est refermé et libre selon lsof.
La reconnexion Windows avec ce nouveau firmware reste à capturer. Ne pas présenter
cet essai Mac comme une répétition de premier appairage ou une validation multi-machines.

### Validation du diagnostic à plusieurs hôtes

Les six suites natives USB/BLE passent avec ASan/UBSan et -Wall -Wextra -Werror.
Les tests existants couvrent maintenant les hôtes entrelacés, déconnexion et
réutilisation d’un canal, emplacement inconnu puis résolu, attribution incohérente,
bornes des canaux, capacité et compteurs saturés. Le classifieur garde des preuves
et des erreurs de sécurité indépendantes. La politique couvre aussi les positions
de couche, HID actif, drainage à 749/750 ms, horloge rebouclée et position invalide.

Sous `.local/validation/ble-multihost/`, les vrais adaptateurs C sont compilés avec
des doubles Zephyr/UART/GATT. Les callbacks excluent le split et gardent les deux
hôtes. Le retour natif -ENODEV avant appairage devient bien un emplacement inconnu,
résolu ensuite sans perte. Six captures privées et trois scénarios synthétiques
passent le vrai dump, dont deux signatures simultanées, réutilisation d’une connexion
et preuves complémentaires réparties entre hôtes qui doivent rester inconnus.
Les sources conservent leurs octets. L’instantané reste stable pendant de nouveaux
callbacks, un handle manquant donne inconnu et une erreur UART supprime END.
L’adaptateur de sélection réel passe les cas de deux hôtes connectés, maintien,
relâchement, drainage, modificateur explicite, consumer actif, événements imbriqués
et erreur native. Les événements sont transmis et aucun UART n’est utilisé sous mutex.

AWK et PowerShell acceptent/refusent les mêmes 59 cas BLETRACE et 27 cas BLEPROFILE,
avec compatibilité v1/v2, LF/CRLF, sessions/états/emplacements incohérents, pertes,
troncatures et résultats manquants. Le BOM et le parseur PowerShell sont contrôlés,
sans prétendre à une nouvelle exécution série Windows ou PowerShell 5.1.

La paire finale est dans `dist/firmware/bluetooth-macos-ble-trace-2026-10-20-14-39-01/`,
compilée et exportée en 15 secondes. Les gardes UF2, rôles, logs et symboles passent.
Le stockage utilise 0x2044 octets de RAM, dont 8192 pour les 256 événements.
La gauche utilise 75 148 octets de RAM sur 256 Kio. Les deux builds Bluetooth normaux
et les deux UF2 wire auto usb-trace sont identiques aux références précédentes,
ainsi que la droite BLE trace. Aucun fichier clavier, keymap, configuration,
manifeste ou préférence de sortie n’est modifié. Aucun nouvel avertissement C,
les avertissements connus Docker insecure et KSCAN restent visibles.
Un premier export a échoué parce que le script de contrôle avait été édité pendant
sa lecture par le shell. Les exports suivants, fichiers stabilisés, ont réussi.
Ces validations de préparation n’ont effectué ni accès série, ni sélection
d’emplacement, ni effacement de bond. Le flash et les essais réels sont décrits ci-dessous.
Les conteneurs de compilation ont terminé, seuls les trois services préexistants
restent actifs après vérification.

Après confirmation du flash gauche, S donne le statut v2 dans
`.local/ble-profiles/macos-2026-10-20-12-53-54-h499vM` : active=0, les emplacements
0 et 1 appairés et connectés, changed=0. D donne
`.local/ble-traces/macos-2026-10-20-12-54-03-tHNELK` : 32 événements, deux sessions
simultanées, zéro perte/rejet et 52 attributs. Windows se connecte le premier en
session 1, emplacement 1 : cinq événements, une seule requête ATT de MTU,
sécurité niveau 2 sans erreur, unknown `0000`. Le Mac est en session 2, emplacement
0 : 27 événements, dont 20 ATT, sécurité niveau 2 sans erreur, macos `0003`.
Cela confirme les deux connexions et la séparation de leurs échanges sur matériel,
pas une détection initiale Windows. Les deux ports de lecture sont refermés et libres.
Aucune sélection n’a été envoyée pendant ces premières lectures, la destination reste Mac.

Après accord sur un aller-retour sans frappe, l’agent lit S (jywaEY), sélectionne 1
(r8Vn20), confirme par S (BaNhnn), lit D (Y99bVL), sélectionne 0 (XRMDus), puis
confirme par S (I1kvj2). Les cinq réponses BLEPROFILE sont dans `.local/ble-profiles/`,
avec le préfixe `macos-2026-10-20-12-58-`. Les deux sélections donnent status=ok,
changed=1, error=0. Tous les statuts gardent les emplacements 0 et 1 connectés.
La trace `.local/ble-traces/macos-2026-10-20-12-58-56-Y99bVL` conserve les mêmes
32 événements et résultats, sans déconnexion, perte ou rejet. La bascule seule
n’a donc apporté aucun nouvel indice d’OS dans cette capture. Une clause finally
prévoit le retour à 0 même si une vérification intermédiaire échoue, sans réessai
automatique. Le statut final confirme active=0, changed=0. Le port est refermé
et libre selon lsof. Aucune touche n’a été synthétisée par l’agent et aucune clé
supprimée. Le sélecteur natif peut envoyer ses rapports vides de relâchement.
Ce test valide les commandes de sélection et leurs statuts, pas
la réception effective des lettres ni la garde pendant une touche maintenue.

### Connexion Mac sans saisie avec Windows encore sélectionné

Après le signalement utilisateur, lecture explicitement autorisée de S puis D,
sans action de sélection : `.local/ble-profiles/macos-2026-10-20-08-15-31-ZobXBG`
donne active=1, slot 0 paired=1 connected=1, slot 1 paired=1 connected=0,
status=ok changed=0. L’utilisateur voit `Splitergoll_Left` connecté sur le Mac.
La trace `.local/ble-traces/macos-2026-10-20-08-15-45-xZhWJD` est complète :
84 événements, trois sessions, aucune perte/rejet et sécurité niveau 2 sans erreur
sur la session active. Les deux lecteurs sont fermés, absence vérifiée par lsof.
ZMK `endpoints.c` vérifie `zmk_ble_active_profile_is_connected()` pour choisir BLE,
et `hog.c` transmet via `zmk_ble_active_profile_conn()` : un hôte connecté mais
non sélectionné ne reçoit donc pas les frappes. Ne pas attribuer ce cas à la
détection d’OS, qui n’est pas implémentée dans ce build.
Le garde du diagnostic refuse toute sélection différente lorsqu’un hôte est
connecté, même si cet hôte occupe précisément l’emplacement demandé. La reprise
proposée est donc Mac Bluetooth coupé, sélection explicite de 0, Bluetooth réactivé,
puis contrôle de saisie par l’utilisateur. L’utilisateur confirme ensuite que le
clavier fonctionne à nouveau. Aucun nouveau flash ni effacement d’appairage.
La politique de destination automatique si un autre hôte connu se reconnecte reste
un sujet distinct de la reconnaissance d’OS. Ne pas la changer implicitement.

### Acquisition initiale et mémoire, prototype actuel

Le moteur commun est désormais `keyboard/src/os_detection/ble_acquisition.c`,
avec le backend `ble_os_settings.c`. Le diagnostic `ble_trace_memory.c` présente
seulement son instantané. `BLETRACE` v2 et `BLEPROFILE` v2 restent compatibles.
Une keymap fixe reste fixe, `bluetooth auto` applique l’OS mémorisé. D rejoue le tampon historique.
La commande M expose `BLEMEM` v1, via `make ble memory <port>` : measured, stored,
origin, state, learning, evidence et error, sans identité ni empreinte privée.
`stored` est inconnu tant que l’identité n’est pas vérifiée ou l’écriture réussie.
`measured` concerne seulement l’acquisition initiale du démarrage courant.
Après reboot il peut être inconnu alors que stored retrouve l’OS sauvegardé.
D, S et M sont passifs. Leur ouverture n’efface rien et ne déclenche aucun apprentissage.

Un worker distinct consomme 64 messages transitoires bornés. Les callbacks gardent
une référence à la connexion et copient uniquement les métadonnées admises, jamais
les payloads. Chaque référence est libérée, même si la file refuse un message.
Un classifieur par index de connexion isole les hôtes. Le parseur ATT est partagé
avec le stockage historique, mais son tampon de 256 événements ne borne pas
l’apprentissage. Une saturation de la file de travail invalide l’apprentissage de
ce démarrage et retire les choix initiaux concernés. Les caches des reconnexions
ne sont pas redétectés. Ce défaut doit être visible, pas interprété comme Windows.

Seul pairing_complete avec bond active fresh. Avant ce signal, les preuves restent
provisoires. Une association renouvelée sur une connexion existante retire les
anciennes preuves. L’identité est vérifiée puis l’acquisition inconnue sauvegardée,
ensuite le premier motif suffisant est écrit. Pas de timer de stabilisation.
Une contradiction ou erreur de sécurité tardive pendant cette connexion retire
le choix, sans reprendre un autre OS arbitraire. Les reconnexions chargent le cache
sans le réécrire, même si leurs traces de recherche suggèrent autre chose.
Le backend et les invariants de persistance sont décrits dans l’AGENTS du module clavier.

L’effacement de test est distinct et explicite : trame `!CLEAR <index>\n`,
`BLEFORGET` v1, client `make ble forget <port> <index>` avec confirmation y/N.
Le parseur retient toute la trame, y compris invalide ou saturée : ses chiffres ne
peuvent pas devenir des commandes de sélection. Refuser index hors plage, index
non actif, saisie tenue/HID actif/drainage et slot vide. Après appel, vérifier le
slot ouvert et l’absence du bond ciblé avec bt_foreach_bond. Appeler uniquement
`zmk_ble_clear_bonds()` pour le slot actif demandé, jamais clear_all/settings_reset.
La notification bond_deleted supprime seulement son association dans notre namespace.
L’utilisateur a exécuté l’effacement ciblé pour les essais Mac puis Windows.
Le résultat Windows conservé dans jZlMfX confirme le seul index 1, status ok et error 0.
Tout effacement réel par l’agent reste soumis à un accord explicite sur le slot.
Le script PowerShell reste un collecteur de traces/profils, sans commande d’effacement.

La suite pure mémoire et les six suites existantes passent avec ASan/UBSan.
Le harnais local du vrai worker/backend vérifie reconnexions, reboot, remplacement,
ambiguïté, erreurs NVS et saturation. Le harnais de sélection vérifie aussi la cible
exacte de l’effacement et les refus. Les captures existantes gardent les mêmes
hypothèses et les 59 cas du protocole BLE passent. L’acquisition initiale Mac et
sa reconnexion sont confirmées ci-dessous. Le premier nouvel appairage Windows
avait donné inconnu avec une découverte partielle. Le correctif et sa validation
matérielle sont détaillés ci-dessous. La restauration après reset est maintenant
confirmée sur les deux appareils. Veille profonde V2 et autres machines restent à tester.

Sur V1, après l’effacement ciblé proposé du slot 0 et le nouvel appairage Mac
réalisés par l’utilisateur, la mémoire annonce measured=macos, stored=macos,
origin=initial, state=saved, learning=1, evidence=0003, error=0, dropped=0.
Le statut garde le Mac actif en 0 et Windows connecté en 1. D conserve 90 événements,
sans perte/rejet : la nouvelle session Mac 5 est sécurisée niveau 2 sans erreur
et donne macos 0003. Deux sessions transitoires non appairées sont closes, sans
motif suffisant. Cette lecture valide l’apprentissage et le retour positif du
backend NVS, pas encore sa restauration après coupure ou sa réutilisation sans
nouvelle détection. Fichiers privés : profil wmdqyC, mémoire TAGUdz, trace WaFfDw.
Les trois lectures sont passives et le port est refermé, absence vérifiée avec lsof.

Après déconnexion/reconnexion Mac sans oubli par l’utilisateur, le slot 0 conserve
stored=macos, origin=initial, state=saved, evidence=0003 et error=0, avec learning=0
et dropped=0. Le statut garde le Mac actif et les deux hôtes connectés. D ajoute
la session Mac 6, chiffrée niveau 2 sans erreur, pour 118 événements sans perte/rejet.
measured=macos reste la dernière acquisition initiale de ce démarrage, pas une
nouvelle décision de reconnexion. Ce test valide la réutilisation sans apprentissage,
pas encore la restauration NVS après redémarrage. Captures privées : profil 0XdU6d,
mémoire KsYdlW, trace RNuCY6. Le lecteur est fermé et lsof ne trouve aucun accès au port.

Le nouvel appairage Windows est confirmé par origin=initial et learning=1 en slot 1,
mais measured et stored restent unknown. state=saved et error=0 confirment la
sauvegarde du résultat inconnu, pas l’identification de Windows. Le cache Mac en 0
reste macos/saved/learning=0. Les deux hôtes sont connectés et Windows est actif
lors de la lecture. Fichiers privés : profil 9Gpjdn, mémoire vQXjve, trace P67lbi.
L’utilisateur confirme ensuite la réception de ses frappes sur Windows avec la
keymap macOS fixe et annonce son retour volontaire au Mac. L’agent ne change
aucune sélection et referme tous les lecteurs.

La capture P67lbi contient 174 événements, neuf sessions, sans perte/rejet. La
session Windows 9 contient 49 événements dont 40 ATT, avec sécurité 2 sans erreur.
La lecture arrive 76 539 ms après la connexion, le dernier événement après 5 871 ms.
Le résultat n’est pas dû à une lecture avant les dix secondes du protocole.
Face au premier essai Windows à 67 ATT, cette variante conserve l’écriture
Client Supported Features, la lecture Database Hash et la lecture Appearance,
mais aucune recherche Included Services 2802. Elle recherche trois fois 2b2a par
Read By Type et lit des handles connus sans refaire la découverte complète.
La réutilisation d’un cache est compatible avec ces échanges, pas prouvée par
une inspection de l’état interne Windows. Ne pas attribuer cet échec à une mauvaise
manipulation de l’utilisateur ni déclarer l’acquisition initiale universellement fiable.

Le rejeu du vrai dump C reproduit unknown/evidence 001c : il manque le bit 0020
au masque Windows 003c. Un ajout synthétique, uniquement hors matériel, de la
requête Included Services fait passer ce rejeu à windows/003c. Cela isole la règle
bloquante, sans démontrer qu’il faut l’assouplir sans autre critère. Les sorties sont
dans .local/validation/ble-memory/windows-repair*. Le evidence=0000 de BLEMEM est
celui du marqueur inconnu sauvegardé au début, car l’OS n’a pas changé ensuite.
BLETRACE donne les indices réellement collectés. Évaluer une variante de signature
avec tests positifs/négatifs avant tout nouveau flash ou appairage. Aucun changement
des règles, firmware ou appairages par l’agent pendant ce diagnostic.

### Correctif de la découverte partielle Windows

Le masque Windows conserve les trois preuves communes `001c` et exige l’une des
deux découvertes positives : Included Services `0020` ou cycle Database Hash `0040`.
Ce cycle exige un Read By Type de `2b2a` sur `1..ffff`, sa lecture directe résolue
par UUID, puis un nouveau Read By Type identique, dans cet ordre. Les opérations
sans rapport peuvent s’intercaler. Les trois autres preuves restent indépendantes
de cet ordre. Répéter une seule phase ne complète pas le cycle. Ni délai, ni nombre
de requêtes arbitraire, ni handle local, ni radio ou slot n’est une preuve d’OS.
Le champ RAM hash_step est remis à zéro avec la session. Les gardes de sécurité,
fiabilité et contradiction restent inchangées. Le protocole de capture garde son
format v2 et ses quatre chiffres hexadécimaux, sans changement des lecteurs.

Le test extrait de P67lbi échoue avant le correctif et passe après. La suite couvre
les cinq opérations nécessaires, leurs 120 permutations (40 reconnues avec une
sonde de chaque côté de la lecture), répétitions, handles décalés, UUID voisins,
plages partielles, opérations ou tailles incorrectes, chiffrement et erreurs,
sessions successives, hôtes simultanés et contradictions dans les deux ordres.
Les sept suites C passent avec ASan/UBSan. Le vrai dump C rejoue 12 captures et
35 sessions : seule P67lbi/session 9 change, de unknown/001c à windows/005c.
Les 49 préfixes de cette session restent inconnus jusqu’au cycle complet,
puis Windows à partir de l’événement 41. Les sources privées sont inchangées.
Les neuf fixtures et 59 contrôles de protocole existants passent aussi.

Le harnais du vrai worker et du backend rejoue les 49 événements de Windows avec
pairing_complete après SECURITY. Il confirme acquisition, sauvegarde 005c, puis
restauration après reboot simulé sans nouveau GATT ni écriture. L’enregistrement
Mac est préservé. Un ancien unknown reçoit aussi ce motif complet en reconnexion
dans le harnais et reste unknown, sans écriture : aucun réapprentissage implicite.
Les preuves et rapports de validation sont privés dans .local/validation/ble-windows-partial/.
Ces rejeux ne prouvent pas la reconnaissance de Linux ou d’autres machines.

L’extension est additive : les anciennes décisions connues restent acceptées et
le format mémoire/règles 1 est conservé. Le cache Mac n’est pas invalidé par le
correctif. L’ancien unknown Windows n’est pas transformé : un nouvel appairage
ciblé sera nécessaire à sa validation matérielle. Aucun effacement, capture USB,
changement de destination ou flash n’est réalisé par l’agent dans ce correctif.

Le build corrigé est bluetooth-macos-ble-trace-2026-10-20-16-40-47. Les deux UF2
passent SHA-256 et le validateur existant des blocs, famille et adresses. La droite
est identique à la version déjà flashée, seule la gauche change. Les gardes des rôles,
keymap, logging et absence de settings_reset passent. RAM gauche 83 060 octets sur
256 Kio, flash 266 664 octets sur 792 Kio. Aucun avertissement de notre C, KSCAN et
le drapeau Docker restent ceux du socle. Le conteneur éphémère a disparu et les
trois services partagés sont conservés. Ni environnement, Dockerfile, Compose,
dépendance, keymap ou configuration radio n’est modifié par ce correctif.
Après le flash corrigé de gauche par l’utilisateur, M retrouve le Mac du slot 0 :
measured unknown, stored macos, origin initial, state saved, learning 0, evidence
0003 et error 0. L’ancien unknown Windows est conservé, sans nouvel apprentissage.
Les deux hôtes sont connectés, Mac actif. Captures privées : profil bRhqqh,
mémoire BPQCrq. Cela confirme la restauration du cache Mac après flash/redémarrage.

L’utilisateur refait uniquement l’appairage Windows du slot 1 puis attend dix
secondes. D conserve 109 événements et huit sessions, sans perte ni rejet. La
session Windows 8 donne windows/005c, Mac 2 reste macos/0003. M confirme Windows
measured windows, stored windows, origin initial, state saved, learning 1, evidence
005c et error 0, dropped 0. Le cache Mac reste saved, learning 0, error 0. Les deux
hôtes sont connectés, Windows actif. Captures privées : trace QCs5Zg, mémoire
rZcbHF, profil 8irl51. Les lecteurs sont refermés, lsof ne trouve aucun accès au
port. Ce résultat valide le correctif et sa sauvegarde sur V1, pas sa généralisation
à d’autres machines. La restauration est contrôlée ensuite après reset.
L’utilisateur annonce ensuite sa sélection explicite du Mac et libère le PC.
Ne pas demander de nouvelles manipulations Windows pour contrôler la restauration
du cache : les lectures peuvent rester sur l’USB gauche au Mac.

Après reset bref de gauche par l’utilisateur, M retrouve Mac stored macos, origin
initial, state saved, learning 0, evidence 0003, error 0, dropped 0. Windows reste
origin initial, evidence 005c, state unverified, measured/stored unknown, learning 0
et error 0. S confirme Mac 0 actif et connecté, Windows 1 appairé mais déconnecté.
L’enregistrement Windows a été chargé, mais son identité n’a pas encore été liée
à une connexion de ce démarrage. Ne pas conclure à une perte ni à une restauration
utilisable tant qu’il ne s’est pas reconnecté. Captures privées : mémoire H7oFqn,
profil 7hoMez. Lecteurs fermés, absence de lecteur vérifiée avec lsof. Aucune
sélection modifiée par l’agent. Cette première lecture valide la restauration Mac,
la reconnexion Windows est contrôlée ensuite, sans nouvel effacement ou appairage.

Après sélection de Windows par l’utilisateur, S confirme actif 1 et deux hôtes
connectés. M retrouve stored macos/0003 en 0 et stored windows/005c en 1, tous deux
origin initial, state saved, measured unknown, learning 0, error 0 et dropped 0.
D conserve 31 événements et deux sessions, sans perte ni rejet, chiffrées niveau 2
sans erreur. La session Windows 2 reste unknown/0000, la session Mac 1 macos/0003.
Le cache Windows a donc été restauré et son identité vérifiée sans réapprentissage,
malgré une découverte insuffisante. Les lettres tapées par l’utilisateur ne changent
pas ce résultat : aucune frappe n’est collectée et le test ne mesure pas la latence.
Captures privées : profil uwFmIB, mémoire ve2fv7, trace fIvlsk. Le retour explicite
au Mac est exécuté par l’agent comme annoncé : profil 6VOvNR, actif 0, deux hôtes
toujours connectés. Lecteurs fermés, lsof sans accès au port.

Ne pas imposer dix secondes avant chaque lecture de mémoire. C’était une marge
pour comparer les captures initiales, pas un timeout du classifieur. Lire après
connexion, distinguer résultat provisoire de découverte et OS restauré. Cela ne
mesure pas le délai entre la reconnexion et la disponibilité réelle du cache.

### Architecture BLE souhaitée après le prototype

Après flash gauche du prototype mémoire par l’utilisateur, les lectures S, M et D
sur V1 confirment deux hôtes appairés/connectés : actif 0 (Mac) et emplacement 1
(Windows). Les 32 événements sont complets, sans perte/rejet, les deux sessions
chiffrées au niveau 2 sans erreur. Mac donne macos `0003`, Windows unknown `0000`.
La mémoire indique cinq emplacements empty, origin none, learning 0, error 0,
sans perte de file. Cet état est attendu pour les anciens appairages et ne valide
pas encore une sauvegarde initiale. Fichiers privés : profil `guJ3NL`, mémoire
`1zT3tf`, trace `LjRfwR` sous leurs dossiers `.local/`. Le port est refermé,
absence de lecteur vérifiée avec lsof. Aucun profil changé ni appairage effacé.

L’utilisateur demande de développer et tester la mémoire avant de solliciter de
nouveaux appairages, afin de regrouper les manipulations sur le PC Windows utilisé
par une autre personne. Préparer d’abord les tests sans matériel et le diagnostic
de l’OS mesuré/mémorisé avec état de sauvegarde. Après le prochain flash gauche,
regrouper les essais des deux OS : premier appairage, persistance, reconnexion et
redémarrage. Ce changement d’ordre ne valide pas les signatures initiales et
n’autorise aucun effacement des appairages actuels. La keymap reste fixe pendant
la validation de mémoire, sa sélection automatique constitue une étape suivante.

L’utilisateur souhaite maintenant associer l’OS reconnu initialement à l’appairage,
et sélectionner automatiquement l’unique ordinateur connecté. Si plusieurs hôtes
sont connectés, conserver le destinataire actuel et proposer une bascule explicite.
Les raccourcis restent un chantier différé. Ce cadrage ne vaut pas implémentation
dans les firmwares ni validation de la qualité de reconnaissance sur d’autres hôtes.

Le ZMK figé fournit `zmk_ble_profile_is_connected`, `zmk_ble_profile_index`, les
identités de profils et `zmk_ble_prof_select`. Le cache doit suivre l’identité
d’appairage et son remplacement, pas seulement l’index. Une réassociation du même
appareil peut conserver son adresse : la comparaison d’adresses seule ne suffit
pas à invalider l’ancien OS. Ne pas exporter les adresses ou clés dans les logs.
Prévoir le schéma/version des règles, les preuves et l’origine actuelle/cache de
la décision. L’utilisateur précise désormais que la production ne relance aucune
détection aux reconnexions : elle charge exclusivement l’OS enregistré. Les traces
de recherche peuvent toujours observer les échanges sans modifier cet OS.
Une correction manuelle de l’OS du destinataire devient un mécanisme normal du
mode automatique, distinct du profil entièrement manuel de dernier recours.
Elle est persistante et prioritaire, y compris après flash conservant NVS. Un OS
connu n’est jamais remplacé automatiquement lors d’une reconnexion. Un OS initial
inconnu doit pouvoir être corrigé sans effacer le bond. Les raccourcis restent à
choisir et doivent être accessibles même si aucune keymap d’OS n’est encore active.
Détection initiale inconnue, dual boot et changement d’OS relèvent alors de la
correction explicite, ou d’une suppression volontaire du lien suivie d’un appairage.
Les appairages actuels n’ont aucun cache OS, il faudra acquérir des indices avant
de les renseigner. Une reconnexion Windows sans découverte n’en fournit pas.

Dans `ble.c`, `update_advertising()` choisit NONE lorsque le profil actif est
connecté. Le sélecteur seul ne garantit donc pas que deux ordinateurs voisins
établiront simultanément une liaison. La configuration du dernier build trace
gauche autorise BT_MAX_CONN=6 et BT_MAX_PAIRED=6, dont un périphérique split,
mais la capacité configurée n’est pas un essai de deux hôtes simultanés.
Prévoir la stratégie d’annonces et l’exception de sélection/appairage volontaire :
sinon un retour automatique vers l’ancien hôte connecté peut empêcher l’arrivée
du second. Le diagnostic sépare désormais les observations de plusieurs hôtes, sans changer
la stratégie native d’annonces. Une sélection explicite d’un profil déconnecté
relance les annonces ZMK, tout en gardant l’hôte précédent connecté. Cela doit
encore être vérifié sur nos deux ordinateurs, sans promettre leur reconnexion
s’ils ne tentent pas eux-mêmes de reprendre la liaison.

L’utilisateur souhaite lire les traces Mac et Windows depuis le Mac et sélectionner
l’un ou l’autre sans couper leur Bluetooth. Ce support est préparé dans le seul
diagnostic et nécessite un nouveau flash gauche. La lecture reste passive. Une
sélection est une action distincte et explicite qui change la destination des frappes,
sous garde de relâchement et drainage. Elle ne déclenche aucune nouvelle découverte
GATT et ne mémorise aucun OS. Tester sur matériel le statut, la connexion du second
hôte, les deux sens de bascule, l’absence de frappes sur l’autre ordinateur et le
refus pendant un maintien avant de conclure. Le firmware normal n’est pas modifié.

La sélection automatique attend un hôte connu et une liaison sécurisée, exclut la
moitié droite et respecte les relâchements avant changement de destination/couches.
Une deuxième connexion ne vole pas la saisie à l’hôte actif. Le raccourci de destination
devra rester utilisable même si l’OS n’a pas encore été identifié.
Les tâches sont centralisées dans docs/TODO.md. Aucun code, réglage de connexion,
préférence de sortie ou appairage n’est modifié pour ce cadrage.

### Fenêtre de reprise après réveil

L’utilisateur propose 5 secondes pour retrouver le destinataire précédant le sommeil.
Conserver deux notions distinctes : destination active en RAM et dernier choix durable.
Au redémarrage/réveil avec reconnexion, si le précédent A n’est pas disponible et que
B est le premier hôte connu utilisable, sélectionner B immédiatement, mais garder A
comme préférence pendant la fenêtre. L’instant zéro est la première connexion hôte
utilisable et sécurisée, pas la mise sous tension, l’ouverture du lecteur USB ou
une connexion de la moitié droite. Les connexions suivantes ne repoussent pas l’échéance.

Si A devient utilisable avant l’échéance, restaurer A. Sinon, B devient le dernier
choix durable à l’échéance, à condition d’être toujours disponible. Un A arrivant
après l’échéance ne reprend pas la saisie. Une sélection manuelle annule la fenêtre
et gagne immédiatement comme intention de destination, avec application soumise
aux gardes de relâchement. Ne pas assimiler tout réveil d’inactivité à un reboot :
si les connexions sont conservées, ne pas recommencer la politique de reconnexion.

L’utilisateur précise que taper sur B ne constitue pas un choix de destination :
B fonctionne immédiatement, mais A reprend la sélection s’il revient dans les
5 secondes, même si B a déjà reçu des frappes. La saisie ne ferme ni ne prolonge
la fenêtre, et ne mémorise pas B comme préférence à elle seule. Les frappes déjà
envoyées restent chez B, sans rejeu chez A. Seul un choix manuel d’appareil annule
cette priorité temporaire. La sûreté des relâchements reste requise.
Si la transition est différée pour relâcher
des touches, la candidature de A dépend de son instant de disponibilité, pas de la
date d’exécution tardive du travail. Tester avant/à/après l’échéance, bascule manuelle,
déconnexion de B avant échéance, A absent, redémarrage avec un seul hôte et absence
de boucle entre destinations. L’égalité exacte avec l’échéance reste à préciser
dans la politique testée. Pas de sauvegarde NVS à chaque événement ou frappe.
La durée de 5 secondes est une proposition utilisateur à confirmer sur le matériel.
La veille profonde réelle nécessite V2 sur batterie, l’alimentation USB de V1 bloque
actuellement ce parcours. Les redémarrages et les traces ne remplacent pas cet essai.

### Collecteur USB Mac et hôte Bluetooth Windows

Le code observe `bt_conn` de rôle périphérique hôte, indépendamment de la console
UART USB. L’USB peut rester sur le Mac pendant la connexion/appairage BLE Windows.
Le Bluetooth du Mac peut être coupé sans perdre la lecture série USB. Cela évite
les transferts du script et des captures Windows. Le nouveau protocole humain est
`../../docs/capture-bluetooth-mac.md`, l’alternative PowerShell reste disponible.
Le champ host et le préfixe macos décrivent l’OS du collecteur, jamais l’hôte BLE.
Consigner l’hôte testé séparément, sans modifier les captures brutes ni utiliser
cette annotation comme entrée du classifieur. Le diagnostic v2 sépare les sessions
des hôtes simultanés. Aucune nouvelle capture n’est ouverte pendant cette préparation.

Rejouer les cinq fichiers privés et leurs préfixes valide le C sur ces données,
pas de nouveaux échantillons indépendants ni une fiabilité générale de premier
appairage. Le premier Windows a été observé dans une case libre. La première trace
Mac ne prouve pas un appairage neuf. Pour reproduire des premiers appairages, prévoir
un effacement ciblé des deux côtés du seul lien testé avec accord explicite, jamais
settings_reset ou BT_CLR_ALL. S/F/index ne permettent actuellement pas de supprimer
un seul bond depuis le lecteur. Un reset physique ou un nouveau flash ne remplace
pas cet effacement. Préparer le mécanisme sûr avant de donner des commandes de reset.

## Diagnostic USB existant

Ce dossier contient toute l’instrumentation d’étude, séparée du clavier dans
`../../keyboard/`. Les lecteurs sont `capture.sh` et `capture.ps1`, le validateur
est `check.awk`. Le firmware, son snippet et ses tests restent dans leurs sous-dossiers.
La commande publique `make usb capture` reste à la racine du dépôt.

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

`make usb capture <port>` lit sur macOS/Linux, `capture.ps1 -Port COMx`
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
ni une probabilité. Les timestamps restent dans les records pour leur chronologie,
mais le classifieur n’utilise plus d’horloge ni de délai de stabilisation.

Les états par chaîne sont bornés à 16 couples index/langue. Un motif exige le
sondage puis une relecture du même couple. La lecture de l’index zéro décrit les
langues, elle est exclue. Les répétitions ou traductions d’un même index ne comptent
pas comme plusieurs noms indépendants. macOS exige des motifs 2 puis complet sur
deux noms distincts. Windows exige un motif 4 puis complet ou 255 puis 262 du même
couple index/langue, et des lectures sur 255 pour deux noms distincts. Le second
motif est celui du firmware HID normal, confirmé dans la capture ETW côté PC.
Les sondes device sur 8/64 sont complémentaires, jamais
obligatoires. Si présentes, elles peuvent contredire un motif de noms et le rendre ambigu. Sur nos trois captures Windows, seul le produit a le motif 4 puis 24,
plusieurs fois : ne pas exiger deux noms différents pour ce motif-là.

Le qualifier reste un indice affiché, sans décider de l’OS. CDC et HID ne contribuent
pas à l’OS. Les sondages contradictoires ou les deux familles de
motifs donnent ambigu. Un motif incomplet, une perte/rejet ou un dépassement de
capacité donne inconnu. Aucun OS par défaut et aucune règle Linux ne sont inventés.
L’hypothèse est disponible dès que les motifs sont suffisants. Le délai de 250 ms
est retiré du diagnostic comme de l’auto. Les timestamps de capture restent
présents dans le protocole et ne mesurent pas le temps complet de démarrage.

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
par l’utilisateur. Le lecteur doit toujours être validé sur Windows pour ses API natives.

Le stockage garde son test dans `tests/`, avec `-Ifirmware`. Le classifieur et son
test sont dans `../../keyboard/`, avec `-I../../keyboard/include`. Les deux commandes
emploient `-std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined` et assertions actives. Le classifieur
couvre motifs, ordre, répétitions, langues, perte, ambiguïté, débordement de capacité,
perturbations CDC/HID et disponibilité immédiate du motif complet. Le stockage vérifie aussi la séparation
des générations, dont une lecture ancienne après remplacement de l’index zéro.
Les six captures initiales se rejouent avec le même C : trois Mac et trois Windows.
L’ancienne evidence Windows 003a devient 00ba avec le motif 255 puis 262, sans
changer leur résultat. Les cinq captures Mac conservent evidence 0005.
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
Le lecteur PowerShell modifié reste à essayer sur le PC, les API Windows ne sont pas exécutables ici.

## Usage Windows autonome

Le script canonique reste `capture.ps1` ici, sa copie peut se placer à la racine de
n’importe quel dossier de travail. La commande utilisateur est
`powershell -NoProfile -ExecutionPolicy Bypass -File .\capture.ps1 -Port COMx`.
Le fichier de capture est créé dans le dossier courant, sans sous-dossier. Son nom
garde la date UTC, le suffixe aléatoire et le contenu UTF-8 sans BOM. Le source garde
son BOM pour Windows PowerShell 5.1. Aucun module, fichier de validation voisin,
mode administrateur ou modification persistante de la politique n’est demandé.
Le changement de destination est contrôlé par revue du diff et le script passe le
parseur PowerShell local. La prochaine capture Windows doit confirmer le nouveau
chemin en conditions réelles. Aucun test automatisé d’outillage ajouté.

## USB normal sur Windows

L’utilisateur confirme macOS fonctionnel et zéro saisie Windows avec `wire auto`
normal, malgré la réussite avec CDC. Les octets classe/sous-classe/protocole sont
`00/00/00` dans l’ELF normal et `ef/02/01` dans l’ELF trace. Le classement inconnu est
une piste, pas un résultat lu sur le matériel. Le mode COM ne peut observer ce firmware.

Le même script autonome a deux jeux de paramètres exclusifs : `-Port COMx` (série)
et `-UsbEvents` (ETW hôte). Le second exige un administrateur et les programmes natifs
Logman/Tracerpt. Il démarre une session `SplitergollUsb-<GUID>` directement avec `-ets`,
ajoute USBXHCI/UCX/USBHUB3 avec `Default,HeadersBusTrace`, puis affiche le signal de
branchement. Capture de vingt secondes, arrêt de sa seule session dans `finally`.
Aucun collecteur planifié persistant, aucune modification de pilote, de firmware
ou de classifieur, aucune collecte lancée par l’agent. La commande d’arrêt manuel
est affichée pour une fermeture brutale du processus. Les noms de sortie sont uniques,
les fichiers préexistants sont refusés et chaque code de sortie natif est contrôlé.
Les outils sont résolus dans le répertoire système Windows. Le nom ETL direct ou
numéroté par Logman est accepté seulement pour ce préfixe unique, avec exactement
un fichier attendu, sans choisir arbitrairement parmi plusieurs segments.

Les sorties ETL, XML et résumé TXT sont directement dans le dossier courant. Garder
le brut, examiner le résumé pour les pertes, puis les en-têtes et l’énumération dans
le XML. HeadersBusTrace exclut les paquets de données, mais les événements peuvent
décrire les autres périphériques USB, donc rester privés. Ne pas conclure à une
règle prête sans données ni annoncer une sélection interne à partir d’ETW seulement.
Les providers retenus ciblent la pile USB 3 de Windows 10/11. Un contrôleur USB 2
historique nécessiterait une collecte adaptée, sans rabattement implicite.

Sources : [capture ETW Microsoft](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/how-to-capture-a-usb-event-trace),
[Logman](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/logman-start-stop),
[Tracerpt](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/tracerpt).
Le parseur PowerShell local est disponible dans `.local/tools/powershell-parser/`.
Pas de PC Windows accessible à l’agent : parsing et gardes Mac validés, exécution
ETW réelle confirmée ensuite par l’utilisateur : démarrage, arrêt et trois sorties
dans son dossier courant. Pas de nouvelle suite automatisée d’outillage.

## Syntaxe et guillemets PowerShell

Les apostrophes typographiques sont aussi des délimiteurs PowerShell. Deux chaînes
avec ce caractère non échappé produisaient les quatre ParserError transmis par
l’utilisateur : MissingTypename aux lignes 156/167 et UnexpectedToken aux lignes
199/200, avant toute collecte. Reproduction exacte avec le parseur officiel 7.6.7,
puis zéro erreur après reformulation des deux messages. Aucun changement de regex
USBTRACE nécessaire. Garder les délimiteurs ASCII et reformuler ou échapper les
apostrophes dans les chaînes, y compris les caractères typographiques.

L’archive portable officielle macOS arm64 est uniquement dans `.local/`, contrôlée
contre le SHA-256 publié `50dde99bb066adacbb9a748182d09eb9beff25119dc3b12e85c84b6b6b1c5da9`.
Aucune installation globale. `Parser.ParseFile` valide le fichier sans lancer ETW.
Les lancements `-Port COM4` et `-UsbEvents` refusent explicitement macOS, le mélange
des deux paramètres est refusé par le binder. Cela ne valide ni Windows PowerShell
5.1 ni Logman/Tracerpt en conditions réelles. Refaire le parsing à chaque modification
PS, sans nouveau framework de tests d’outillage.

L’utilisateur confirme le son de connexion Windows avec le firmware normal.
Il ne voit pas de COM4 : absence attendue puisque CDC est désactivé. Chercher le
clavier sous les catégories Claviers ou Périphériques d’interface utilisateur,
ainsi que les erreurs éventuelles sous Contrôleurs de bus USB. Le son ne prouve
ni un HID opérationnel ni le résultat du classifieur. La capture utilise `-UsbEvents`,
sans numéro COM.

La capture privée `5663dfa5` donne 2 075 événements XML, autant que le résumé, sans
perte déclarée. Tracerpt signale des événements hors schéma : cela ne garantit pas
que tous les champs sont décodés, malgré le compte complet. Les 22 SETUP dispatchés
de notre cible sont lisibles, leurs terminaisons appariées aussi. L’énumération
termine à NTSTATUS 0. Deux requêtes sont refusées : qualifier et SET_IDLE. Ce refus
de SET_IDLE correspond au socle Zephyr sans USB_DEVICE_SOF, pas à une modification
de notre classifieur. Aucune lecture de la sélection interne ne peut être déduite
d’ETW. La suite du diagnostic et le motif 255 puis 262 sont dans la spec clavier.
Le script utilise désormais `tracerpt -lr` pour le décodage au mieux des événements
hors schéma, selon Microsoft. Ce mode de conversion reste à essayer sur le PC,
sans promettre la disparition de l’avertissement ni une récupération exhaustive.
Le brut ETL et XML restent privés, seules les 22 demandes anonymes sont reprises
dans le test du classifieur. Aucun besoin de nouvelle collecte pour ce correctif.

## Diagnostic partagé avec Bluetooth auto

Le nouveau contrôleur de reconnexion connue ajoute BLEPROFILE v3 dans bluetooth
auto ble-trace. Une ligne WINDOW précède RESULT : ready, remaining_ms, advertising,
known, connected et error. Aucun identifiant privé. Les lecteurs AWK et PowerShell
acceptent v1/v2 sans ligne et v3 avec exactement une ligne, des masques bornés et
un délai au plus 10000 ms. Une lecture S n’ouvre aucune fenêtre et ne mute aucun slot.
Le profil fixe ble-trace reste v2 et conserve l’appairage ZMK natif. Dans auto,
sélectionner un slot libre ne permet pas encore un nouvel appairage, le contrôleur
refuse les nouveaux bonds. Voir le contrat keyboard/src/connections/AGENTS.md.
Validation : 42 réponses positives/négatives communes AWK/PowerShell, syntaxe PS
et BOM UTF-8 conservés. Le témoin bluetooth macos ble-trace v2 conserve exactement
ses deux UF2. Le nouveau auto ble-trace v3 compile, sans essai matériel à ce stade.

La collecte active et la mémoire appartiennent maintenant au clavier, voir son
AGENTS. ble_trace.c garde seulement le tampon historique et son dump, ble_trace_memory.c
seulement le format BLEMEM. Le classifieur et son test sont déplacés avec git mv.
Le stockage historique garde son propre test, lié au décodeur ATT partagé.
Le hook de diagnostic peut recevoir un en-tête valide dont l’UUID local est impossible
à résoudre : conserver cet en-tête, et rendre la classification irrecevable au rejeu.
Une résolution impossible ne doit pas être confondue avec un paquet ATT malformé.
Le moteur d’acquisition reçoit séparément la fiabilité de résolution.

Avec bluetooth auto ble-trace, il n’y a qu’un wrapper d’événements, celui du clavier.
Le contrôle USB verrouille le même mutex, utilise ses gardes d’activité et refuse
la sélection/effacement pendant une saisie. Avec une keymap fixe, les gardes
historiques du diagnostic restent actives. Les formats D/S/M et l’effacement ciblé
ne changent pas. Les lecteurs ne doivent pas annoncer que toute keymap reste fixe :
la lecture est passive, mais le sélecteur auto agit indépendamment depuis la mémoire.
