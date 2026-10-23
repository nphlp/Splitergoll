# Sélection automatique de l’OS

## Intention et périmètre

Implémentation propre à Splitergoll, sans module tiers ni modification de ZMK/Zephyr.
`auto` est expérimental : Windows avec gaming et macOS, sans profil Linux.
Le wire utilise la signature USB. Le Bluetooth utilise l’OS mémorisé de l’appareil
sélectionné. La bascule entre hôtes connectés et leurs OS est confirmée sur V1.
La sélection automatique générale d’appareil et le choix manuel forcé d’OS restent
différés. L’adoption du seul nouvel hôte est maintenant préparée après appairage,
avec les mêmes gardes d’entrée et sans détourner un autre hôte connecté.
Les signatures de nos deux hôtes et la sélection de couches sont confirmées avec
CDC sur V1. L’utilisateur voit macOS sur le Mac et Windows sur le PC via l’encodeur
droit, confirme gaming puis retour Windows et le fonctionnement habituel sur Mac.
L’utilisateur a ensuite flashé `wire auto` normal sur V1. Il confirme macOS fonctionnel
et aucune saisie sur Windows. La capture ETW reproduit un classement inconnu par
absence du motif de noms sur 4 octets. Le nouveau motif 255 puis 262 corrige le rejeu,
et l’utilisateur confirme après flash la saisie et la reconnaissance Windows puis
macOS sur V1, par débranchement/rebranchement et sans reflash entre les deux hôtes.
Il signale ensuite des démarrages irréguliers sur Mac, même de Mac vers Mac.
L’utilisateur précise ensuite que les layouts sur Mac et Windows et le gaming
fonctionnent correctement en usage normal. Ne pas lui redemander de revalider ce
fonctionnement comme un chantier ouvert. Le démarrage sur événements est ensuite
confirmé sur Mac → Windows → Mac, avec le bon nom d’OS à l’encodeur et un ressenti
très rapide. Les touches maintenues, resets et reprise restent des essais distincts.
Aucun délai matériel précis n’est encore mesuré.
Aucun OS par défaut n’est ajouté.
Aucun flash implicite.
Les quatre profils manuels et l’archive de `multi` restent disponibles et inchangés.

## Organisation

- `usb_os_classifier.c` : motifs purs et bornés, partagés avec le diagnostic.
- `usb_observer.c` : les trois wrappers du pilote USB historique. Seuls les SETUP
  complets de l’endpoint zéro OUT sont observés, les arguments/résultats/statuts natifs
  sont délégués. Aucun payload HID, BOS ou descripteur supplémentaire en mode normal.
- `usb_auto_state.c` : politique pure d’attente et de relâchement des 44 positions.
- `usb_auto.c` : adaptation ZMK, sérialisation par mutex récursif, sélection des couches.
- Interfaces : `../../include/splitergoll/`. Tests natifs : `../../tests/`.
- Instrumentation de capture et lecteurs : `../../../diagnostics/os-signature/`.

Le module clavier conserve board_root, ajoute dts_root, CMake et Kconfig. Le nœud
`compatible = "splitergoll,os-auto"` active le sélecteur sur la centrale du transport choisi.
L’observateur USB est compilé pour wire auto ou la centrale usb-trace. Les autres
firmwares n’ajoutent pas de code embarqué. La nouvelle pile USB, LTO et les SoC autres
que nRF52840 sont refusés. Les dépendances, l’image et les partitions sont inchangées.

## Couches

`auto.keymap` assemble une couche neutre 0 sans action, Windows 1 à 7 et macOS 8 à 12.
Les numéros sont partagés avec C dans `config/keymaps/common/auto-layers.h`.
`layers/windows.dtsi` et `layers/macos.dtsi` gardent leurs schémas et bindings exacts,
inclus aussi par les profils fixes. Les noms de nœuds sont préfixés dans auto.
Les tap-dances L1/L2 macOS ciblent leur propre groupe, les autres comportements sont
communs. Chaque groupe conserve sa combinaison primaire + secondaire → tertiaire.
En USB la couche neutre ne fournit aucun raccourci de secours. En BLE, le filtre
reconnaît L1 + clic encodeur droit pour revenir à un hôte connecté même sans OS
disponible. Le bootloader physique reste accessible dans les deux cas.

Contrairement à la base zéro d’un profil fixe, une base de groupe serait retirée
par `&to NUMPAD` ou `&to GAMING`. Le wrapper local `zmk_keymap_layer_to` réactive donc
la base de l’OS déjà choisi après chaque transition. Le retour du pavé et du jeu cible
cette même base. Pas de sélection manuelle d’OS et pas de couche Windows cachée sous
macOS. La couche zéro neutre reste présente selon les invariants du moteur ZMK.

## Cycle et sûreté des appuis

Un reset ou une déconnexion USB vide les indices et change la génération sous spinlock.
Suspend/resume ne les efface pas. Aucun callback USB n’active de couche. Les SETUP
et changements de statut réveillent un travail sur la file système, avec mutex
partagé par la saisie. La notification de statut vient après le callback natif,
pour que la disponibilité HID soit à jour. Les SOF ne réveillent pas le sélecteur.
Le classifieur ignore CDC/HID, n’utilise plus d’horloge et ne suppose jamais Windows
par défaut. Les motifs et limites sont détaillés dans la spec du diagnostic.

La sélection exige aussi HID prêt et zéro touche physique maintenue. À froid,
`wait_for_behaviors` vaut false : aucun comportement de l’ancien cycle n’est à
terminer, donc pas de pause supplémentaire. Un reset après sélection, une position
admise, une rotation admise ou un keycode arme le garde des comportements différés.
Il exige alors 750 ms depuis la dernière activité susceptible de produire un
comportement. Ce délai dépasse les tap-dances 250 ms, le hold-tap 200 ms et l’attente
500 ms des macros actuelles. Réexaminer ce garde si ces comportements changent.
Les positions et rotations ignorées ne le réarment pas, leurs touches restent
suivies jusqu’au relâchement. Un keycode différé, même bloqué, le réarme, et ses
releases passent toujours. La décision reste figée jusqu’au cycle suivant, même
si un motif tardif change l’hypothèse. Aucune écriture NVS.

Le wrapper `zmk_event_manager_raise` filtre les appuis physiques avant les listeners
ZMK, donc avant les tap-dances et les modificateurs. Tant que l’OS est inconnu, appuis
et rotations sont ignorés sans rejeu. Un appui ignoré ne transmet pas de release orpheline.
Les touches admises avant un reset gardent leur release, même pendant l’attente. Une
touche déjà tenue doit être relâchée avant sélection. Les keycodes pressés par un ancien
comportement différé sont également bloqués pendant l’attente, leurs releases passent.
Les transitions ne sont pas déclenchées en plein raccourci. Ne pas supprimer ces gardes
pour rendre une signature inconnue utilisable en lui attribuant un OS arbitraire.

L’état des couches repart de la base reconnue à chaque cycle. Cela ne constitue pas
une sauvegarde du pavé, du gaming ou d’un verrouillage Shift. L’absence de capture CDC
normale se vérifie dans les configurations compilées, pas par le ressenti utilisateur.
La capture auto ajoute ACTIVE pour distinguer hypothèse et groupe effectivement appliqué.

## Validation

Depuis la racine du dépôt : compiler chaque source C pure avec son test homonyme,
`-Ikeyboard/include -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined`, puis
exécuter le binaire. Classifieur : motifs, répétitions/langues, sonde initiale absente,
indices contradictoires, overflow et disponibilité immédiate des motifs. État : HID absent, inconnu/ambigu,
appuis ignorés, touche maintenue, reset pendant raccourci, releases, doublons et compteurs.
Le stockage du diagnostic conserve sa suite avec `-Idiagnostics/os-signature/firmware`.

Construire les trois paires `wire auto`, `wire auto debug`, `wire auto usb-trace`.
Contrôler les appels ARM vers les wrappers, les gardes de rôles/UF2, et les couches
compilées contre les profils fixes après traduction des numéros. Comparer les huit
paires normales existantes à leurs binaires et Devicetrees précédents. Ne pas remplacer
les essais matériels par ces contrôles. Aucun test supplémentaire des wrappers Bash.

Les layouts Mac/Windows et le gaming sont validés par le retour utilisateur en
normal. Essais restants : démarrage optimisé, touche tenue, reset du bus et réveil,
avec le firmware normal. Linux, autres versions d’OS,
hubs, KVM et BIOS sont inconnus et peuvent donner une mauvaise reconnaissance.
Le Bluetooth utilise ses propres observations et sa mémoire, l’USB d’alimentation ne révèle pas son hôte.
Les tâches restent centralisées dans `docs/TODO.md`.

Validation réalisée : les trois suites C (état, classifieur, stockage) passent avec
ASan/UBSan. Les dix captures privées restent correctement classées après déplacement
du code. Les six arbres auto ont les mêmes 44 affectations et encodeurs par couche
que les profils fixes, avec seulement les destinations traduites. Les tap-dances et
les deux combinaisons de couches ciblent leurs groupes. Les appels ARM des événements
position/keycode/capteur passent par le filtre, `&to` par la restauration de base et
les trois API du pilote par l’observateur. Les trois paires auto, les huit paires
manuelles normales et la paire fixe usb-trace compilent et passent les gardes.
Les 24 UF2 passent SHA-256 et le validateur de blocs. Les seize UF2 manuels restent
identiques octet pour octet aux précédents, ainsi que leurs quatre sous-arbres de
keymap/comportements/macros/conditions. Les gardes du flash auto sont vérifiés sans
écriture USB. Aucun flash matériel effectué par l’agent. Les conteneurs éphémères ont disparu,
les trois services partagés sont conservés. Les avertissements KSCAN/tap-dance et le
drapeau moteur Docker restent ceux du socle, sans nouveau diagnostic dans notre C.

## Échec Windows sans CDC

Le normal et la trace utilisent les mêmes VID/PID et indices de chaînes, mais leur
descripteur périphérique compilé diffère aux octets 4 à 6 : `00/00/00` contre `ef/02/01`.
La trace active `USB_COMPOSITE_DEVICE` et `USB_CDC_ACM`. Le normal n’active aucun des
deux. Le source Zephyr figé `subsys/usb/device/usb_descriptor.c` et les octets de
`common_desc` des deux ELF confirment ce changement.

La capture Windows privée `5663dfa5` contient 2 075 événements et zéro perte annoncée,
avec le même compte dans XML et résumé. La cible est identifiée par VID/PID,
descripteurs et handle ETW, jamais par un numéro de port supposé. L’énumération
termine avec NTSTATUS 0. Les 22 SETUP dispatchés n’ont aucun sondage de nom sur 4.
Le produit est demandé sur 255 puis 262 octets, trois fois sur 262 au total.
Le rejeu C avant correction donne inconnu, evidence 0032 ou 0030 sans la sonde 64.
Cela reproduit un motif insuffisant, sans lire directement l’état du firmware.

Le classifieur conserve le motif Windows 4 puis complet et ajoute le motif 255 puis
262 du même index et de la même langue, dans cet ordre, bit `USB_OS_STRINGS_262`
0x80. Les deux chemins exigent toujours au moins deux index de noms lus sur 255.
La taille 262 est une taille demandée, jamais la longueur du nom réellement reçu.
Aucun numéro de nom, langue, longueur de produit ou identifiant de machine n’est
codé dans la règle. Linux n’est toujours pas reconnu. Les veto 8/64, conflits Mac,
pertes et capacité restent actifs. Une application sur un autre OS
peut reproduire ces requêtes : il s’agit toujours d’une heuristique expérimentale.

Le test natif reprend les 22 en-têtes anonymes, avec et sans la sonde initiale.
Il échoue avant le correctif et passe après. Les cas négatifs couvrent 262 seul,
nom unique répété ou traduit, mauvais index/langue/ordre, tailles voisines,
requêtes non standard, langues index zéro et contradictions avec Mac ou sonde 8.
Les trois suites natives passent avec ASan/UBSan. Le rejeu des dix captures
précédentes garde cinq Mac et cinq Windows. Les deux rejeux ETW donnent Windows,
evidence 00b2 / 00b0. Les timestamps de ces rejeux sont synthétiques : aucune
mesure de délai ou de latence n’en est déduite. Aucun flash implicite.

Les trois paires corrigées auto normal/debug/usb-trace compilent et passent les
gardes et SHA-256. Leurs six arbres restent conformes aux profils fixes. Les seize
UF2 manuels et leurs sous-arbres de keymap restent identiques aux références.
La droite normale est identique au build précédent : seul un reflash gauche est
nécessaire pour cet essai. Les conteneurs éphémères ont disparu, les trois services
préexistants sont conservés. Aucun Dockerfile, Compose, variable d’environnement,
partition, keymap ou appairage n’est modifié. L’utilisateur confirme ensuite la
saisie et la détection Windows puis macOS avec le même firmware normal, en
débranchant/rebranchant V1 entre les hôtes, sans reflash intermédiaire. Il confirme
ensuite les layouts et le gaming en usage normal. Les essais de veille, reset du
bus sans coupure, Linux et autres ordinateurs restent distincts.

Un nouveau retour utilisateur signale un délai irrégulier sur Mac après changement
d’hôte, puis le reproduit aussi en Mac vers Mac. Ne pas conclure à une persistance
Windows. Le choix n’est pas sauvegardé en NVS, le callback efface les indices sur
RESET/DISCONNECTED, et le démarrage RAM initialise l’état à zéro. Cela vérifie le
chemin prévu, pas les événements effectivement reçus sur le branchement en échec.
Avant optimisation, la sélection attendait signature stabilisée, HID prêt, aucune
touche tenue et 750 ms depuis la dernière activité. Chaque appui/relâchement ignoré
remettait ce dernier délai à zéro. Le premier essai contrôlé est Mac vers Mac, USB gauche débranché puis
rebranché, cinq secondes sans toucher touches/encodeurs, puis une touche gauche.
Mesurer avant de changer les heuristiques ou les gardes. Si la saisie reste bloquée,
garder le branchement en place pour distinguer présence USB et attente interne.
CDC peut changer la signature : son ajout seul ne prouve pas ce cas normal.

L’essai utilisateur avec cinq secondes sans frappe fonctionne du premier coup.
Cela concorde avec le réarmement des 750 ms par chaque position ignorée, sans
prouver la cause de chaque branchement lent. Les frappes ne modifient pas les
indices du classifieur : elles retardent la politique de sélection. Les cinq
secondes sont une marge de protocole, pas un nouveau délai firmware. Étudier
la distinction entre activité admise et appuis ignorés avant d’alléger ce garde,
en conservant touche maintenue, releases d’ancien cycle et comportements différés.
Ce constat motive l’optimisation suivante, sans changer les heuristiques d’OS.

## Démarrage optimisé

`USB_AUTO_DRAIN_MS` conserve les 750 ms uniquement pour vider un ancien comportement,
pas comme attente initiale. Le filtre délègue désormais positions, capteurs et
keycodes à la politique pure, afin de tester la distinction admis/ignoré réellement
utilisée dans ZMK. Les positions ignorées ne créent pas de comportement ni de
release orpheline, mais une touche encore tenue bloque toujours la sélection.
Le relâchement d’une position admise avant reset et tout keycode différé repoussent
la marge de sécurité. Un reset répété pendant cette marge la réarme également.
Cette première optimisation conservait encore la stabilisation de 250 ms et le
contrôle périodique de 20 ms, retirés dans l’étape événementielle ci-dessous.

Le test de non-report par des frappes ignorées échoue avec l’ancien code et passe
avec l’optimisation. La suite couvre aussi sélection sans attente à froid, touches
tenues, rotations ignorées, reset après un cycle actif, reset répété, anciens
relâchements, keycodes différés, inconnus/ambiguïtés, HID absent et horloge 32 bits.
Aucun changement de keymap, de descripteur USB, de dépendance ou de partition.
L’utilisateur rapporte ensuite un démarrage nettement plus rapide avec cette
optimisation, sans mesure chronométrée. Le maintien d’une touche, les resets du
bus et la reprise ne sont pas validés par ce ressenti.

Validation de l’optimisation : trois suites C avec ASan/UBSan, trois paires auto
normal/debug/usb-trace compilées et six UF2 contrôlés. Les six arbres auto gardent
les mêmes couches que les profils fixes, les seize UF2 manuels restent identiques
aux références. Les dix captures de signatures gardent leur résultat. Les droites
sont identiques aux versions précédentes : seul le flash gauche est nécessaire.
Aucun avertissement nouveau dans notre code, KSCAN et le drapeau Docker restent
ceux du socle. Les trois conteneurs de build sont supprimés automatiquement,
les trois services préexistants sont conservés. Aucun flash matériel effectué.

## Sélection sur événements

La première optimisation est publiée à `c83c146`. L’étape suivante retire le délai
de 250 ms du classifieur et ses paramètres d’horloge. Les mêmes motifs sont évalués
dès chaque nouvelle requête, y compris dans le dump du diagnostic. Les timestamps
restent uniquement dans les records et le protocole de capture.

Le travail de sélection est réveillé par SETUP, statut USB et événement physique
ou keycode tant que le choix est inconnu. Les notifications sont faites hors
spinlock, sans activation de couche ni attente de mutex depuis le pilote USB.
`auto_ready`, atomique, empêche son exécution avant SYS_INIT du sélecteur.
L’initialisation réveille une fois le travail pour couvrir les événements reçus
plus tôt. Les notifications se regroupent avec `k_work_reschedule(K_NO_WAIT)`.
Une touche encore tenue attend son relâchement, HID absent attend son statut,
un motif incomplet attend une requête : pas de boucle périodique.

Seule la récupération d’anciens comportements utilise une échéance : le travail
programme exactement `usb_auto_drain_remaining()`, borné à 750 ms, lorsqu’aucun
autre garde ne bloque. `k_work_schedule` dans le travail préserve une notification
immédiate déjà en file. Le code figé de Zephyr `kernel/work.c:1023` accepte la
programmation d’un travail en cours s’il n’a pas déjà une exécution en attente.
Les notifications ultérieures peuvent avancer cette échéance. L’horloge uint32
et les relâchements de l’ancien cycle gardent leurs protections.

Après activation de couche, génération et hypothèse sont revérifiées avant de
publier le choix. Une contradiction après publication ne le change pas avant le
prochain cycle, comme auparavant. Retirer la marge augmente la fenêtre possible
d’une contradiction tardive : c’est le compromis explicite du choix au plus tôt,
sans promesse universelle de reconnaissance. Aucun OS implicite ni nouveau motif.

Le test de résultat immédiat échoue avant retrait du délai et passe après. Le
test de stabilisation devenu sans objet est remplacé par des préfixes incomplets,
la dernière requête complétant Mac/Windows et une contradiction après hypothèse.
La politique teste aussi le reste exact du garde, dont 1 ms avant échéance et le
débordement de l’horloge. Le rejeu de chaque préfixe des dix captures reste inconnu
ou du bon OS, puis finit sur cinq Mac et cinq Windows. Les deux rejeux ETW anonymes
restent Windows. Leurs timestamps synthétiques ne sont pas des mesures de vitesse.
Les files USB et système ne sont pas simulées par ces tests natifs. L’utilisateur
confirme ensuite après flash Mac → Windows → Mac, à chaque fois très rapide, avec
les noms d’OS corrects au clic d’encodeur. Ce retour valide les branchements
essayés, sans mesure chronométrée ni validation implicite des touches maintenues,
resets de bus ou réveils.

Les trois suites natives passent avec ASan/UBSan. Les trois paires auto et la paire
fixe `wire macos usb-trace` compilent, les huit UF2 passent les gardes et SHA-256.
Cette dernière vérifie le diagnostic sans sélecteur auto après changement d’API.
Les six arbres auto restent conformes aux profils fixes et les seize UF2 manuels
normaux restent identiques aux références. Les droites auto sont inchangées.
Le harnais privé de rejeu est `.local/validation/usb-events/replay.c`. Ses résultats
sur des captures anciennes ne mesurent pas la vitesse du nouveau firmware.
Les conteneurs de validation ont disparu, les trois services partagés sont conservés.

## Mémoire BLE du diagnostic

`ble_os_memory.c` est la politique pure, `ble_os_settings.c` le backend NVS.
Les interfaces et tests suivent les dossiers clavier existants. La fonction
`SPLITERGOLL_BLE_MEMORY` est activée par le diagnostic gauche `ble-trace` ou par
`bluetooth auto` gauche, dans tous ses modes. Les profils fixes normaux ne changent pas.

L’enregistrement `splitergoll/ble-os/<index>` fait 40 octets : schéma 1, règles 1,
OS, origine, preuves, réserves et empreinte SHA-256 opaque. L’empreinte porte sur
l’identité stable et le blob natif `bt/keys` lu par `settings_load_subtree_direct`.
Le blob n’est jamais copié dans notre NVS ou les captures. Les temporaires de clés
sont effacés après hachage. Aucun en-tête privé Zephyr ni patch amont n’est utilisé.
Ce chemin de stockage appartient au socle figé : réexaminer à la prochaine migration.
Les gardes exigent SC-only, sans signing ni vieillissement automatique des clés,
pour que le blob persistant reste stable aux reconnexions. SHA-256 vient de mbedTLS
déjà présent, sans nouvelle dépendance ni option cryptographique dépréciée.

Dans bluetooth auto, le contrôleur de connexions livre explicitement cette
notification après attribution et sauvegarde du slot réservé. Le callback direct
pairing_complete de mémoire y est désactivé, pour éviter une acquisition prématurée
ou en double. Les profils fixes gardent le callback statique paired natif. Le format
NVS, les signatures et les règles de classification ne changent pas.

Une notification de nouvel appairage invalide même une empreinte identique.
Un bond remplacé pendant un autre firmware change son empreinte et perd l’ancien
OS. Un ancien bond sans cache reste inconnu. Un format/règles inconnus ou une
identité non vérifiable ne fournit aucun OS. La correction manuelle est prévue et
prioritaire dans la politique pure, mais aucun geste ni commande de correction
n’est encore raccordé au firmware. Ne pas présenter ce mécanisme comme livré.

Le worker attend un sémaphore ouvert après la restauration des réglages. Les
lectures de diagnostic avant cette ouverture ne lisent pas les tableaux en cours
de chargement. Les accès NVS sont exécutés par le worker dédié du diagnostic, jamais dans les
callbacks radio ou la lecture USB. Aucune écriture périodique ni réessai infini.
Une erreur conserve l’état sale et le diagnostic n’annonce pas cet OS comme stocké.
Une coupure avant sauvegarde réussie ne garantit pas la conservation du résultat.
L’identité est revérifiée aux connexions. `bluetooth auto` applique l’OS mémorisé
aux couches, les diagnostics construits avec une keymap fixe n’en changent pas.

Validation : suite `keyboard/tests/ble_os_memory_test.c` avec ASan/UBSan, plus
harnais local exécutant le vrai backend et le vrai worker avec doubles settings/BLE.
Couvrir apprentissage initial, héritage absent, reconnexion sans apprentissage,
reboot, remplacement du bond à adresse identique, ambiguïté tardive, erreurs NVS,
suppression ciblée, surcharge et lecture USB passive. Le SHA du harnais est un
double, la primitive cryptographique réelle est celle du build embarqué. Cela ne
simule pas les files/interruptions du SoC ni une vraie coupure pendant NVS.
Les essais physiques suivent docs/tests-memoire-os.md, après flash et accord ciblé.

Les builds de contrôle `bluetooth macos` et `wire auto usb-trace` passent : leurs
quatre UF2 sont identiques aux références précédentes. Le build mémoire final est
`bluetooth-macos-ble-trace-2026-10-20-15-52-38`, avec droite identique au diagnostic
déjà flashé, six empreintes finales/de contrôle vérifiées et gardes UF2 des scripts
passées. La gauche utilise 83 036 octets de RAM sur 256 Kio et 266 568 octets de
flash sur la partition de 792 Kio. Les logs finaux ne signalent que KSCAN côté
compilation et le drapeau Docker existant. Aucun avertissement de notre code.
Les conteneurs de validation ont disparu et les trois services partagés sont conservés.
Aucun flash, lecture série ou effacement matériel réalisé par l’agent pendant ce lot.

L’utilisateur a ensuite flashé la gauche et refait l’appairage Mac du slot 0.
La lecture matérielle confirme origin initial, stored macos, state saved et error 0,
sans perte de file. Windows reste connecté en 1, sans OS acquis. La reconnexion Mac
sans oubli confirme ensuite stored macos, state saved, learning 0 et error 0.
Le nouvel appairage Windows confirme origin initial et sauvegarde réussie du
résultat unknown. La découverte partielle est désormais couverte par un motif
supplémentaire du classifieur diagnostique : sonde du hash GATT, lecture directe,
nouvelle sonde, avec les trois indices Windows communs. Le rejeu donne Windows.
Le harnais du worker/backend vérifie sa sauvegarde 005c et la restauration simulée,
sans écraser le Mac ni réinterpréter les anciens unknown lors d’une reconnexion.
Le format mémoire/règles 1 reste compatible avec les anciennes décisions connues.
Après flash corrigé de gauche, le Mac retrouve stored macos, state saved et
learning 0 sans nouvel appairage. L’acquisition Windows refaite par l’utilisateur
confirme stored windows, origin initial, state saved, evidence 005c et error 0,
sans perte de file. Après reset puis reconnexion Windows, les deux caches sont
restaurés en state saved, learning 0, error 0 et dropped 0. Windows n’offre pas de
signature suffisante sur cette reconnexion, mais stored windows est retrouvé sans
apprentissage : le chemin NVS et son contrôle d’identité sont validés sur V1 et
ces deux appareils. La veille profonde V2 reste à tester. Les captures et étapes
sont suivies dans l’AGENTS du diagnostic et docs/TODO.md.

## Keymap automatique Bluetooth

`SPLITERGOLL_OS_AUTO` choisit un seul adaptateur selon ZMK_USB : usb_auto.c ou
ble_auto.c. Aucun observateur USB dans le firmware Bluetooth normal. Les deux
adaptateurs utilisent les mêmes treize couches et les mêmes fichiers Windows/macOS.
La première intégration conserve les affectations de `auto.keymap`. Le lot suivant
ajoute le raccourci de changement d’appareil, le choix manuel forcé d’OS reste reporté.

Le moteur auparavant diagnostique est regroupé dans keyboard : ble_events.h et
ble_att.c pour les métadonnées ATT admises, ble_os_classifier.c pour les règles,
ble_observer.c pour les callbacks et l’unique wrapper bt_l2cap_recv, ble_acquisition.c
pour le worker et son état. Les noms BLE_TRACE des métadonnées sont conservés pour
le protocole et les tests existants, sans imposer d’historique en fonctionnement normal.
Le diagnostic reçoit ces observations via un hook optionnel. Aucune copie de valeurs,
adresses, clés, rapports HID ni trace RAM historique dans le firmware normal.
Les indices, le format mémoire 1 et les identités NVS sont inchangés.

L’instantané d’acquisition expose une connexion courante distincte de l’OS sauvegardé.
Le sélecteur exige : appareil sélectionné connecté, chiffrement courant au moins L2,
session sécurisée, transport d’acquisition fiable, cache présent/vérifié/propre/sans erreur,
et OS pris en charge. Aucun OS depuis l’USB de charge, ni reclassification en reconnexion.
Une génération atomique par index de connexion change dès les notifications radio
connexion/sécurité/déconnexion et lors d’un nouvel appairage. L’instantané du worker
doit porter cette même génération, sinon aucun appui n’est admis. Cela ferme la
fenêtre où une ancienne mémoire vérifiée pourrait précéder le traitement de la
nouvelle connexion. Ce garde est testé dans la politique pure et les harnais.
Inconnu, ambigu ou Linux reste bloqué. Une erreur tardive ou un cache invalidé retire
la sélection. Aucun profil Linux automatique ou mode de secours arbitraire.

ble_auto_state.c invalide le groupe lors d’un changement d’appareil, de session,
d’OS ou de disponibilité. Il réutilise usb_auto_state.c, politique pure indépendante
du transport malgré son nom historique : appuis admis/ignorés, relâchements et
750 ms après un ancien comportement. Aucun délai à froid ni polling.
Les événements d’acquisition et de profil réveillent un travail sur événements.
Les positions, encodeurs et keycodes sont filtrés avant les comportements ZMK.
Le rapport HID doit aussi être vide avant une transition. `&to` garde la base de
l’OS sous le pavé ou le jeu, comme en wire auto. Les commandes de diagnostic
partagent le mutex récursif du filtre, sans second wrapper d’événements en auto.

Validation attendue : huit suites C strictes avec ASan/UBSan, harnais du vrai
worker/backend après déplacement, rejeu des captures et callbacks de l’observateur.
Comparer les quatre sous-arbres des couches aux builds auto USB, vérifier les rôles,
les symboles, les options normal/debug/ble-trace et les UF2. Conserver les essais
physiques comme étape distincte, sans effacement ni flash automatique. Vérifier Mac,
Windows, gaming et retour, reconnexion, appuis maintenus, keymap normale sans console.

Validation réalisée pour ce lot : huit suites natives avec -Wall/-Wextra/-Werror,
ASan et UBSan. Le harnais du vrai worker/backend conserve apprentissage, inconnus,
restauration, remplacement, ambiguïté, erreurs NVS et suppression ciblée. Il vérifie
également qu’un instantané est périmé jusqu’au traitement de la génération radio.
Le harnais des callbacks réels vérifie deux hôtes, exclusion de la moitié droite,
reconnexion, mise à jour du profil et séparation des fiabilités ATT/UUID. Douze
captures valides, 43 sessions, donnent les mêmes classifications après déplacement.
Un fichier incomplet est explicitement exclu des captures de référence.

Les trois paires bluetooth auto normal/debug/ble-trace et les contrôles wire auto,
bluetooth macos normal et bluetooth macos ble-trace passent les gardes de build.
Les douze UF2 passent SHA-256, famille, blocs et bornes de l’application. Les treize
couches, macros, conditions et comportements des trois auto BLE sont identiques à
wire auto. Les deux UF2 wire auto restent identiques au build du 18 octobre, les
deux bluetooth macos normaux à ceux du prototype mémoire avant ce lot.
Le normal auto BLE n’expose ni HID USB, CDC, console UART, observateur USB ou
historique BLE. L’observateur et la mémoire seuls sont nécessaires en normal.
La RAM gauche est de 69 596 octets / 256 Kio en normal et 83 836 avec ble-trace,
la flash de 265 152 / 792 Kio et 272 684 respectivement. Aucun avertissement dans
notre C, les diagnostics KSCAN/tap-dance et Docker restent ceux du socle.

Le build final pour le premier essai est bluetooth-auto-ble-trace-2026-10-20-17-30-04,
le normal bluetooth-auto-2026-10-20-17-30-13 et le debug USB à 17-30-24. La droite
ble-trace a exactement l’empreinte du firmware installé par l’utilisateur, seul
le flash gauche est nécessaire. Aucun flash, changement de sélection ou effacement
matériel par l’agent dans ce lot. Les conteneurs éphémères ont disparu,
les trois services préexistants sont conservés. Aucune dépendance, image, partition,
clé NVS, variable d’environnement, Dockerfile ou Compose n’est modifié.
Les preuves temporaires restent dans .local/validation/ble-auto/.

L’utilisateur confirme ensuite son flash gauche avant de demander le commit.
S annonce Mac 0 actif/connecté, Windows 1 appairé/déconnecté. M retrouve Mac
stored macos, origin initial, state saved, learning 0, evidence 0003, error 0 et
dropped 0. Windows reste origin initial, evidence 005c, state unverified et error 0,
sans connexion de ce démarrage pour revérifier son identité. Lectures passives
seulement, port fermé et lsof sans lecteur. Captures privées : profil 3MR9Gs,
mémoire l9dFX6. Cela confirme la restauration du cache Mac après ce flash, pas
encore la saisie, le nom d’OS à l’encodeur ou les transitions de bluetooth auto.

## Raccourci de destination connectée

`ble_device_state.c` porte la politique pure, `ble_device_behavior.c` le comportement
ZMK sans paramètre. `common/ble-devices.dtsi` l’instancie seulement pour la centrale
bluetooth auto, grâce à `DTS_EXTRA_CPPFLAGS=-DSPLITERGOLL_BLE_SHORTCUTS` injecté par
container.sh. La valeur vide est explicitement passée aux autres builds pour éviter
une fuite du cache. Les fragments Windows/macOS définissent HOST_NEXT à none hors
de ce contexte et retirent leurs paramètres locaux après inclusion.
Seules les positions 43 des deux couches primaires changent. La base et son affichage
d’OS restent identiques. Depuis la base, maintenir L1 puis cliquer à droite demande
l’hôte connecté suivant, dans l’ordre circulaire des profils ZMK.

Le masque parcourt les profils hôtes, jamais les connexions de la moitié droite.
Les cases vides et déconnectées sont ignorées. Une seule connexion déjà sélectionnée
ne produit aucune action. Une demande répétée avance la cible en attente, deux
demandes sur deux hôtes annulent donc la bascule. La demande conserve l’adresse
native en RAM pour refuser une case remplacée avant application, sans l’exporter.
Une sélection externe sur une autre case annule aussi la demande, même si elle
revient à la case initiale avant l’exécution du travail.

Le mutex récursif est commun au filtre, au comportement et au travail. L’application
exige aucune position tenue ou admise en attente de relâchement, un rapport HID vide
et un drainage terminé.
Le dernier relâchement admis arme les 750 ms déjà utilisées par le moteur automatique.
Les keycodes différés les réarment. L’état est ensuite mis au repos avant l’appel
natif zmk_ble_prof_select, pour ne pas drainer une deuxième fois. La sélection NVS
reste celle de ZMK avec son délai natif de 60 secondes. Aucune écriture d’OS ou de bond.

Quand l’OS est indisponible, le filtre reconnaît le pouce L1 physique et le clic
encodeur avant les comportements. Les positions 40/43 sont déclarées dans os_auto,
vérifiées à la compilation et correspondent aux schémas de base. Les doublons et
relâchements ne demandent aucune bascule, les autres frappes restent filtrées.
Ce secours permet de revenir à un hôte connecté sans supposer Windows par défaut.

Validation logicielle : neuf suites natives strictes avec ASan/UBSan, dont la nouvelle
politique (masques exhaustifs jusqu’à huit hôtes, limite seize, attente/annulation,
double demande, secours et drainage au débordement d’horloge). Un harnais temporaire
compile le vrai ble_auto.c avec callbacks ZMK simulés pour vérifier le câblage des
événements, l’attente réelle du travail, les releases, la cible déconnectée/remplacée,
le retour depuis un OS inconnu et la priorité d’une commande externe. Il ne simule
ni le moteur tap-dance complet ni la radio. L’essai utilisateur décrit ci-dessous
confirme ensuite le geste entre les deux hôtes.
Les preuves de ce lot sont dans .local/validation/ble-shortcut/.

Les trois paires bluetooth auto normal/debug/ble-trace et les deux paires témoins
wire auto / bluetooth macos compilent et passent les gardes. Les dix UF2 passent
SHA-256, famille, blocs et bornes de l’application. Dans les trois arbres auto BLE
gauches, seuls les bindings 43 des couches primaires Windows/macOS changent, avec
le nouveau comportement et les positions de secours. Toutes les autres touches,
rotations, macros, conditions et comportements sont identiques au wire auto.
Les trois droites et les quatre UF2 témoins restent identiques octet pour octet
aux exports d’avant ce lot. Seule la gauche est à reflasher pour tester le geste.
Aucun avertissement dans notre C. KSCAN, tap-dance const et le drapeau moteur Docker
restent les diagnostics du socle. Les conteneurs éphémères ont disparu, les trois
services préexistants sont conservés. Aucun flash, capture ou changement de destination
matériel dans ce lot. Ni dépendance, partition, mémoire OS, Dockerfile, Compose ou
variable d’environnement modifiés.

Validation matérielle rapportée après flash : l’utilisateur confirme la bascule
d’appareil par raccourci et la bonne association macOS / Windows sur V1 ble-trace.
Les lectures avant cet essai retrouvaient les deux hôtes connectés, Mac en slot 0,
Windows en slot 1, stored macos/windows, state saved, learning 0, error 0 et dropped 0.
Windows a été reconnecté par sélection USB explicite de 1, sans effacement.
Sa session diagnostique était inconnue mais son OS restauré était bien Windows,
ce qui vérifie la séparation entre nouvelle classification et mémoire de reconnexion.
Preuves privées : profil kPdudc, mémoire uv2SpA, trace Utbnd9.

L’attente ressentie d’environ deux secondes n’est pas une mesure. La politique
attend 750 ms depuis la dernière position/keycode/rotation admise, avec relâchements
et rapport vide. Les événements différés peuvent repousser cette échéance.
Le test ne fournit pas d’horodatages du geste ou de disponibilité, ne pas déduire
une latence BLE ni promettre qu’une simple baisse du garde suffit. L’optimisation
de ce délai est au TODO, les timings firmware restent inchangés.

La lecture postérieure XPCc1v voit Mac 0 actif/connecté, Windows 1 appairé/déconnecté.
La trace 6PVA43 annonce uptime 41 755 ms, une seule session Mac sécurisée L2 et
aucune perte. L’utilisateur confirme avoir volontairement débranché/rebranché la
gauche après l’essai : cet historique n’est donc plus celui des bascules précédentes,
et ne démontre aucun crash. Ne pas perdre la validation rapportée du raccourci.

La demande suivante porte sur la reconnexion des deux hôtes après ce redémarrage.
Le ZMK figé utilise update_advertising() dans app/src/ble.c : désir NONE lorsque
l’hôte actif est connecté, CONN lorsque le slot actif est libre ou déconnecté.
Son callback connected remet advertising_status à NONE avant de réévaluer ce désir.
CONFIG_BT_MAX_CONN vaut 6, MAX_PAIRED vaut 6, soit cinq profils hôtes et la droite.
Le défaut n’est donc pas le nombre de connexions. La documentation officielle
Bluetooth Behavior confirme cette politique et les connexions simultanées.

La politique future est maintenant cadrée par les fenêtres de trente secondes décrites
ci-dessous, plutôt que par des annonces infinies si un hôte reste absent. Préserver
la sécurité native, le contrôle des nouvelles associations et les profils fixes,
sans sélectionner celui qui arrive pendant l’usage, sauf recherche explicite armée
par le raccourci dans le cas d’une seule destination.
Le choix automatique d’hôte et sa fenêtre de retour restent séparés. Le point
d’intégration doit respecter l’état d’annonces déjà piloté par ZMK : ne pas lancer
un second gestionnaire concurrent ni patcher implicitement la dépendance figée.
Tester la politique pure, les appels réels et les deux ordres de reconnexion avant
de déclarer cette automatisation validée. La pile hôte initie la connexion, aucune
annonce ne garantit une reconnexion instantanée d’un ordinateur qui ne cherche pas.

## Direction future USB + BLE

Le lot courant ne réalise pas ce mode mixte. Il ouvre trente secondes de reconnexion
et de nouvel appairage au démarrage et via le raccourci, dans bluetooth auto.
Le contrat radio est dans ../connections/AGENTS.md. Le sélecteur conserve ses gardes,
avec adoption possible du seul nouvel hôte si le choix courant n’a pas changé.
Un autre hôte connecté interdit cette adoption. La recherche avec bascule sur la première arrivée est raccordée. La décision utilise
les hôtes sécurisés dont l’OS mémorisé est utilisable. Le format de mémoire d’OS et les identités restent inchangés.

Le cadrage utilisateur passe du slot BLE à une destination regroupant appareil,
liaisons disponibles et OS. Priorité USB au BLE pour un appareil associé, pas
priorité USB globale. Le cycle doit garder Mac BLE accessible lorsque Windows USB
est branché et masquer Windows BLE seulement si son identité commune est établie.
Le classifieur USB expose un OS et ses indices, aucune identité physique d’hôte.
La liaison USB vers le bond BLE reste un problème de conception ouvert, l’OS seul
ne permet aucune déduplication fiable entre deux machines du même système.

L’architecture actuelle choisit un seul adaptateur auto selon ZMK_USB, et BLE_AUTO
exige !ZMK_USB. Un firmware mixte demandera donc une orchestration commune des deux
observateurs et un seul filtre d’entrée, une destination et une keymap cohérentes.
Ne pas simplement activer les deux sélecteurs indépendants ni faire du statut USB
d’alimentation une destination HID. Les contrôles d’identité mémoire BLE restent requis.
Les bonds, le format NVS d’OS et les profils de build existants sont inchangés dans
ce cadrage. Définir d’abord ouverture/fermeture des nouvelles associations en séparant
annonces de reconnexion et acceptation d’appairage. Spécifications humaines et TODO
dans docs/gestion-appairage.md et docs/destinations-saisie.md.

Le dernier cadrage utilisateur propose annonces et nouveaux appairages ouverts
pendant trente secondes au démarrage/reset, au retour Gaming USB vers Windows et via
L1 + clic encodeur droit.
À l’échéance, conserver les connexions et arrêter les annonces. Dans la fenêtre,
accepter les nouveaux appareils seulement si un slot est libre, ne pas déduire
la fin de fenêtre du seul nombre d’hôtes connus déjà connectés.
Gaming USB arrête les annonces et les nouveaux appairages, déconnecte le Mac BLE,
préserve bonds/OS et le BLE split. Le sort de Windows BLE en Gaming reste ouvert.
Le retour Windows garde la destination USB, sans attendre la reconnexion du Mac.
Un appairage engagé dans la fenêtre peut finir dans les trente secondes suivant
son début. Aucun nouveau bond n’est admis si tous les slots sont occupés.
La catégorie Gaming inclut ses deux couches, le passage interne Gaming → Gaming 2
ne doit pas réouvrir les annonces. Ne pas appliquer une coupure de BLE hôte si
l’USB n’offre pas de destination utilisable. Le bénéfice de latence reste à mesurer.
Le retour au précédent est implémenté dans le nouveau lot, à valider sur matériel.
La grâce de cinq secondes est remplacée par la fenêtre de trente secondes ouverte
à la première connexion du démarrage.
Le premier hôte reste utilisable sans attendre et ne doit pas écraser la préférence
durable pendant cette attente. Une recherche explicite annule ce retour.
Les détails de cette politique sont dans ../connections/AGENTS.md.

Le raccourci devra prendre un instantané du nombre de destinations utilisables et
distinctes au geste, pas du nombre de connexions BLE. Une destination : garder la
source jusqu’au premier autre appareil utilisable, puis consommer l’intention de
bascule. Plusieurs : sélectionner la suivante avec les gardes existantes, rouvrir
trente secondes mais ne pas armer de bascule sur les nouveaux arrivants. Proposition
pour zéro : recherche et sélection du premier appareil prêt. Une seconde liaison
USB/BLE du même appareil ne doit ni compter deux fois ni consommer cette recherche.
Une nouvelle demande remplace l’ancienne, la recherche annule le rappel automatique
de l’ancien hôte. Les frappes sur la source continuent pendant la fenêtre.
Ne pas sélectionner un nouvel appareil avant sécurité et OS utilisable. La clôture
du délai sans cible conserve la sélection précédente. Aucune arrivée ne doit prendre
la destination plusieurs fois pour un seul geste.

L’utilisateur valide que le geste depuis Gaming quitte d’abord ses couches pour
revenir à Windows puis ouvre la recherche. Le retour par ce geste porte une intention
de recherche distincte du simple retour de couche, et ne doit pas ouvrir deux fenêtres
ni exécuter deux demandes. Gaming → Gaming 2 reste dans Gaming, sans ouverture.

Le premier lot de développement des fenêtres est maintenant isolé dans
keyboard/src/connections/, avec son AGENTS et host_window_policy.c. C’est une
politique pure sur des destinations déjà identifiées/prêtes, sans activation dans
le firmware. Son test natif rejoint les neuf suites existantes, les dix passent
en C11 strict avec ASan/UBSan. Ne pas lier les deux adaptateurs auto simultanément
pour contourner l’absence actuelle de firmware mixte. L’intégration radio et
l’attribution d’un bond neuf sans changement de destination sont le lot suivant.

## Transition après raccourci et fenêtre de trente secondes

Le sélecteur utilise maintenant host_window_policy pour le geste : au plus un hôte
prêt arme une recherche, plusieurs choisissent le suivant déjà prêt. Les identités
sont des jetons locaux qui changent si l’adresse native du slot ou son état vide
change, sans journaliser cette adresse. L’acquisition réveille le worker lorsqu’un
hôte devient sécurisé et que son OS mémorisé est utilisable. Aucun sondage périodique.
La fenêtre d’annonces reste pilotée par le seul worker radio, avec la même durée.
Une recherche ne sélectionne qu’une arrivée. Un appairage commencé dans sa fenêtre
peut devenir sa cible après fermeture, une fois sauvegardé. Une nouvelle commande,
une source remplacée ou une sélection externe interdit cette adoption tardive.

Le problème rapporté venait du réarmement du drainage par toute nouvelle frappe.
Pendant une cible en attente, ble_device_buffer retient les nouveaux événements de
position en RAM avant les comportements ZMK, avec source, état et timestamp.
Les positions initialement maintenues gardent leurs releases sur l’ancien hôte.
L1 et le bouton du geste restent utilisables pour remplacer la demande.
Les keycodes des anciens comportements continuent de repousser leur propre drainage,
pas les positions retenues. Les rotations sont ignorées pendant la transition.
Après sélection native et application vérifiée de la nouvelle keymap, les événements
sont réémis dans l’ordre, sans passer par l’ancien groupe. Aucun de ces événements
n’est ajouté aux traces, à NVS ou aux logs. La capacité est 64 événements.
Un dépassement abandonne le lot entier, signale une erreur sans son contenu et absorbe
les releases futures. Une cible perdue/remplacée ou une commande externe abandonne
aussi le lot. Aucun rejeu vers une destination de remplacement arbitraire.
La présence de cette file bloque les commandes USB mutantes pendant la transition.

Douze suites natives ASan/UBSan couvrent les règles, la borne de deux minutes,
l’ordre/source/timestamps du tampon, doublons, releases, abandon et dépassement.
Le harnais du vrai ble_auto.c vérifie saisie immédiate, releases vers l’ancien hôte,
recherche/reconnexion, absence de vol par une arrivée après un geste multi-hôte,
expiration et fin d’appairage après fermeture. Preuves privées dans
.local/validation/ble-switch. Les essais physiques du nouveau lot restent à faire.
La sélection native reste exclusivement sur le worker système : le filtre des
positions ne déclenche ni changement de profil ni rejeu, il retient les entrées
et notifie ce worker. L’armement du tampon au geste précède les frappes suivantes.
Le harnais vérifie aussi une touche retenue encore maintenue après sélection,
les keycodes tardifs de l’ancienne macro et un échec de sélection native, sans
transfert de release orpheline vers l’ancien appareil.
Les trois paires normal/debug/ble-trace compilent et passent les gardes UF2,
SHA-256 et comparaison des couches, macros et comportements. La droite ble-trace
reste identique à la précédente. Les conteneurs éphémères sont libérés et les
services préexistants restent actifs. Pas de flash, lecture série ou changement de bond.

## Préférence de destination et activité sans hôte

La reprise du démarrage utilise une préférence propre, jamais ble/active_profile.
Le worker résout l’empreinte vérifiée du bond par le backend de connections, puis
host_session_target propose le premier disponible ou le préféré avant l’échéance.
Une sélection explicite annule le rappel. Les callbacks natifs de connexion du
profil déjà sélectionné ne sont pas des choix manuels : observed_profile permet
cette distinction. internal_selection protège les mutations décidées par le worker.
Une cible automatique acceptée avant l’échéance garde les mêmes gardes de release,
HID vide et drainage que le raccourci, ainsi que son jeton d’identité. Les nouvelles
frappes restent en RAM. La sauvegarde à la fermeture attend la fin de la transition.

Les touches/sensors physiques comptent comme activité même avant une keymap valide,
par appel du callback exporté zmk_listener_activity du socle figé. Aucun keycode,
paquet radio ou lecture série ne passe par ce chemin. L’activité native ne fait
qu’actualiser son horodatage et annoncer ACTIVE, sans exécuter de comportement.
Recontrôler ce symbole et son contrat à chaque migration, sans patch de dépendance.
Les durées 120/600 secondes et la garde de 750 ms ne changent pas.

La mémorisation distingue désormais le provisoire de reprise, le geste manuel et le
repli sur perte de l’hôte réellement utilisé. host_session_update suit recovery et
le délai radio, host_session_manual annule rappel/checkpoint. Le profil utilisé est
noté seulement avec connexion et keymap prêtes. Une demande réussie prépare un jeton
save_candidate, vérifié après application effective des couches et avant rejeu.
Les erreurs natives, demandes remplacées et identités modifiées ne sauvegardent
pas de cible périmée. Une expiration de fenêtre explicite ne réarme pas checkpoint.
La perte totale annule les anciennes demandes et réarme le rappel au précédent,
sans refuser un connu à cause d’une ancienne perte. L’admission suit la fenêtre
radio, le sélecteur n’ajoute aucun masque de refus individuel.
Une sélection USB explicite change réellement le profil puis utilise le même flux.
La nouvelle politique reste à valider sur matériel, sans modifier les keymaps.

## Couverture reproductible des adaptateurs

scripts/test.sh inclut désormais le vrai usb_observer.c avec usb_auto.c et le
vrai ble_observer.c avec son décodeur ATT. Les cas contrôlent callbacks natifs,
générations, refus d’OS sans HID, erreurs et reset concurrent, distinction du split,
paquet original intact et absence de payload de frappe dans les observations.
Le sélecteur BLE inclut les gardes de rapports clavier/média et modificateurs,
la saisie retenue, les erreurs natives et les différentes origines de mémorisation.
Acquisition et backend réels sont testés avec bonds, horloge et NVS simulés :
initial/reconnexion/reboot, remplacement d’identité, erreurs, saturation et effacement.
Ces harnais sont versionnés dans keyboard/tests/integration, sans lecture série.
Les règles encore futures (Linux, identité commune USB/BLE et Gaming persistant)
ne deviennent pas implémentées parce que certaines politiques pures sont testées.
Voir docs/tests-automatises.md pour la frontière entre logiciel et matériel.
