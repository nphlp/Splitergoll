# Sélection automatique de l’OS USB

## Intention et périmètre

Implémentation propre à Splitergoll, sans module tiers ni modification de ZMK/Zephyr.
`wire auto` est expérimental : Windows avec gaming et macOS, sans profil Linux ou BLE.
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
fonctionnement comme un chantier ouvert. Le démarrage optimisé, les resets et la
reprise restent des essais distincts. Aucun délai matériel précis n’est encore mesuré.
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
`compatible = "splitergoll,os-auto"` active le sélecteur seulement sur la centrale USB.
L’observateur est compilé uniquement pour auto ou la centrale usb-trace. Les autres
firmwares n’ajoutent pas de code embarqué. La nouvelle pile USB, LTO et les SoC autres
que nRF52840 sont refusés. Les dépendances, l’image et les partitions sont inchangées.

## Couches

`auto.keymap` assemble une couche neutre 0 sans action, Windows 1 à 7 et macOS 8 à 12.
Les numéros sont partagés avec C dans `config/keymaps/common/auto-layers.h`.
`layers/windows.dtsi` et `layers/macos.dtsi` gardent leurs schémas et bindings exacts,
inclus aussi par les profils fixes. Les noms de nœuds sont préfixés dans auto.
Les tap-dances L1/L2 macOS ciblent leur propre groupe, les autres comportements sont
communs. Chaque groupe conserve sa combinaison primaire + secondaire → tertiaire.
La couche neutre ne fournit aucun raccourci de secours : le bootloader physique reste accessible.

Contrairement à la base zéro d’un profil fixe, une base de groupe serait retirée
par `&to NUMPAD` ou `&to GAMING`. Le wrapper local `zmk_keymap_layer_to` réactive donc
la base de l’OS déjà choisi après chaque transition. Le retour du pavé et du jeu cible
cette même base. Pas de sélection manuelle d’OS et pas de couche Windows cachée sous
macOS. La couche zéro neutre reste présente selon les invariants du moteur ZMK.

## Cycle et sûreté des appuis

Un reset ou une déconnexion USB vide les indices et change la génération sous spinlock.
Suspend/resume ne les efface pas. Aucun callback USB n’active de couche. Un travail
sur la file système vérifie l’état toutes les 20 ms, avec mutex partagé par la saisie.
Le classifieur exige 250 ms sans nouveau descripteur utile, ignore CDC/HID et ne suppose
jamais Windows par défaut. Les motifs et limites sont détaillés dans la spec du diagnostic.

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
indices contradictoires, overflow et stabilisation. État : HID absent, inconnu/ambigu,
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
Le Bluetooth exigera ses propres observations, l’USB d’alimentation ne révèle pas son hôte.
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
pertes, capacité et stabilisation restent actifs. Une application sur un autre OS
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
La stabilisation des signatures reste 250 ms et le contrôle périodique 20 ms :
ils ne constituent pas une mesure depuis le branchement du câble.

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
