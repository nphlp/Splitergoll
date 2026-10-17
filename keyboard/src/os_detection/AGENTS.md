# Sélection automatique de l’OS USB

## Intention et périmètre

Implémentation propre à Splitergoll, sans module tiers ni modification de ZMK/Zephyr.
`wire auto` est expérimental : Windows avec gaming et macOS, sans profil Linux ou BLE.
Les signatures de nos deux hôtes et la sélection de couches sont confirmées avec
CDC sur V1. L’utilisateur voit macOS sur le Mac et Windows sur le PC via l’encodeur
droit, confirme gaming puis retour Windows et le fonctionnement habituel sur Mac.
L’utilisateur a ensuite flashé `wire auto` normal sur V1. La reconnaissance sans
CDC reste à confirmer physiquement. Aucun flash implicite.
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

La sélection exige aussi HID prêt, zéro touche physique maintenue et 750 ms sans
position, rotation ou événement de keycode. Cette pause dépasse les tap-dances 250 ms,
le hold-tap 200 ms et l’attente 500 ms des macros actuelles. Réexaminer ce garde si les
comportements différés changent. La décision est figée jusqu’au cycle suivant, même
si un motif tardif change ensuite l’hypothèse. Aucune écriture NVS.

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

Essais attendus : Mac → Windows → Mac sans nouveau flash, raccourcis, accents,
encodeurs, pavé, gaming, démarrage avec touche tenue, reset du bus et réveil. Refaire
l’aller-retour sans CDC après la version instrumentée. Linux, autres versions d’OS,
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
