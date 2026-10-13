# Unification et évolution de Splitergoll

## État du chantier

Phase de cadrage. La branche de travail part de `dongle` au commit `2584ca4`.
Cette première étape documente le projet et récupère les fichiers de conception
de `main`, sans modifier le firmware, lancer un build ou flasher un clavier.

`README.md` est la copie exacte du README de `main`, avec un nom standard.
Les photos et les dossiers `Design PCB/` et `Design 3D/` proviennent du même commit
`cbad9a0`. Ce README historique reste à actualiser après la relecture : certaines
illustrations d'affectations et indications macOS ne décrivent pas encore la
configuration de `dongle` retenue pour l'unification.

## Matériel et usages actuels

Deux claviers physiques existent et doivent continuer à fonctionner.

| Clavier | Ordinateur | Connexion actuelle | Alimentation |
| --- | --- | --- | --- |
| Clavier du bureau | PC Windows | Mode appelé `wire`, un câble USB par moitié | USB, sans batterie |
| Clavier mobile | MacBook | Deux moitiés BLE et un dongle USB central | Batterie dans chaque moitié |

Les moitiés utilisent des répliques de nice!nano v2. Le dongle utilisé est un
XIAO BLE. Le dongle Nordic visible dans les anciennes photos n'est pas utilisé.

Le Bluetooth direct vers le MacBook, sans dongle, est un nouveau profil à mettre
en place et à valider sur le clavier mobile. La moitié gauche deviendra centrale,
ce qui peut augmenter sa consommation par rapport à son rôle périphérique actuel.

## Décisions retenues

- Garder les branches historiques intactes et ne pas modifier `main` maintenant.
- Les archives `wire` et `wire-minimal` sont créées et publiées comme copies
  exactes de `main` à `cbad9a0` et de `main-minimal` à `27ed2b7`.
- Développer l'unification sur une nouvelle branche issue de `dongle`.
- Prendre la keymap complète de `dongle` comme référence pour tous les profils OS
  et toutes les connexions. Comparer aussi les macros, helpers et couches
  conditionnelles lors des futurs changements.
- Conserver la disposition actuelle des accents de `dongle`.
- Conserver son comportement Shift : absent de la couche tertiaire commune et
  présent dans la couche tertiaire macOS. Le retrait commun date de `c3d02d3`,
  le 4 février 2025. L'ajout macOS date de `2584ca4`, le 26 juillet 2025.
- Avoir une seule branche et une source de configuration commune, avec des profils
  explicites. Éviter quatre copies complètes du firmware à entretenir.
- Définir les contrats et les limites avant toute refonte du firmware.

## Distinguer keymap, topologie et sortie

La keymap définit les touches, couches, raccourcis, accents et macros. Son profil
OS est indépendant de la connexion au PC ou au Mac.

La topologie définit quelle carte est centrale. Cette carte traite les événements
et l'état des couches. Les autres cartes sont des périphériques.

| Topologie envisagée | Centrale | Périphériques | Sortie vers l'ordinateur |
| --- | --- | --- | --- |
| Directe | Moitié gauche | Moitié droite | USB ou Bluetooth direct |
| Dongle | XIAO BLE | Moitiés gauche et droite | USB du dongle pour l'usage actuel |

Le rôle central/périphérique est configuré dans le firmware. Le passage entre ces
deux topologies demande une reconstruction et un flash des parties concernées,
avec une procédure d'appairage adaptée. Ce n'est pas un toggle natif de sortie.

La sortie USB/Bluetooth peut en revanche être choisie à chaud sur la centrale.
Dans la configuration `main` historique, la gauche est centrale et la droite reste
un périphérique BLE. Les deux câbles vers l'ordinateur ne créent pas une liaison de
données entre les moitiés. Le câble droit fournit l'alimentation.

Sources : [rôles et transports ZMK](https://zmk.dev/docs/features/split-keyboards),
[sélection de sortie](https://zmk.dev/docs/keymaps/behaviors/outputs).

## Bluetooth direct et charge USB

Comportement souhaité pour le MacBook, à concevoir et tester :

1. Sur batterie, les touches passent directement au Mac en Bluetooth, sans dongle.
2. Si la centrale est branchée au Mac avec un câble de données, l'USB devient
   prioritaire pour la saisie, tout en permettant la charge.
3. Après débranchement, le clavier revient au Bluetooth si le Mac est connecté.
4. Sur un chargeur secteur, la saisie doit pouvoir rester en Bluetooth.

ZMK sait préférer l'USB et se rabattre sur le Bluetooth lorsque l'USB HID n'est pas
prêt. La détection des alimentations USB n'est toutefois pas garantie dans tous
les cas. Prévoir une commande explicite pour préférer le Bluetooth lorsque la
bascule automatique ne convient pas. Une préférence sauvegardée peut persister
après un redémarrage ou un flash.

La sélection de sortie remplace le chemin des touches vers le Mac. Elle ne prouve
pas que la radio Bluetooth du contrôleur est arrêtée.

La suppression de tout le Bluetooth, y compris entre les moitiés, reste une
question ouverte. Avec le câblage actuel, cette communication interne est
nécessaire. Une liaison filaire entre moitiés demanderait une étude matérielle
séparée. Ne pas promettre un mode sans aucune radio avec deux simples câbles USB
vers l'ordinateur.

Sources : [sélection et persistance de sortie](https://zmk.dev/docs/keymaps/behaviors/outputs),
[choix du transport dans ZMK](https://github.com/zmkfirmware/zmk/blob/main/app/src/endpoints.c).

## Build local

Objectif : générer les firmwares localement, sans dépendre des GitHub Actions.
Une première préparation avec accès réseau sera nécessaire pour récupérer les
outils, ZMK et ses dépendances. Les builds suivants utiliseront les caches locaux.

Piste à valider : compilation `west` dans l'image officielle ZMK, via un conteneur
éphémère lancé par une commande visible. Docker est déjà installé sur le Mac ARM
et le manifeste de l'image officielle propose `linux/arm64` et `linux/amd64`.

Fixer la version de ZMK et celle de l'environnement de compilation. Ne pas suivre
silencieusement une branche ou un tag mobile. Prévoir des dossiers de build et
des artefacts distincts par OS, topologie, variante et partie du clavier.

Les commandes ci-dessous sont des propositions de contrat, pas des commandes
implémentées. Le choix du nom de la commande reste ouvert.

```sh
./scripts/build-firmware --os windows --topology direct --layout full
./scripts/build-firmware --os macos --topology dongle --layout full
./scripts/build-firmware --os macos --topology direct --layout full
```

Le profil minimal doit rester possible. L'OS Windows, macOS ou Linux doit pouvoir
être sélectionné sans éditer manuellement la keymap. Définir si la sélection à la
compilation fixe l'OS ou seulement l'OS initial d'un firmware qui reste polyvalent.

La commande de build produit les artefacts attendus. Elle ne flashe pas
automatiquement les appareils et n'efface pas leurs appairages.

Le changement Zephyr 4.1 explique l'échec récent de l'ancien nom `nice_nano_v2`.
La documentation actuelle indique `nice_nano@2.0.0//zmk` et `xiao_ble//zmk` pour les
cartes correspondantes. Ces noms devront être vérifiés avec la version ZMK retenue
et les répliques réellement utilisées.

Sources : [build local ZMK](https://zmk.dev/docs/development/local-toolchain/build-flash),
[environnement conteneur](https://zmk.dev/docs/development/local-toolchain/setup/container),
[migration des cartes Zephyr 4.1](https://zmk.dev/blog/2025/12/09/zephyr-4-1).

## TODO

### Préparation

- [x] Créer et publier les archives `wire` et `wire-minimal` sans modifier leurs
  branches sources.
- [x] Créer et publier `unified-firmware` depuis `dongle`, avec son propre upstream.
- [x] Consigner les décisions et les questions ouvertes en français.
- [x] Récupérer le README de `main` sous le nom `README.md`, avec les photos et
  les fichiers de conception associés.
- [ ] Actualiser le README historique après la relecture et le cadrage des profils.
- [ ] Choisir la version ZMK et l'environnement de compilation reproductible.

### Unification et profils

- [ ] Définir une organisation commune pour le matériel, la keymap et les profils.
- [ ] Préserver les 44 positions, les encodeurs et les comportements de référence.
- [ ] Définir les profils Windows, macOS et Linux et le comportement de l'OS initial.
- [ ] Définir les variantes complète et minimale.
- [ ] Construire le profil du clavier Windows alimenté par USB.
- [ ] Construire le profil du clavier Mac avec dongle.
- [ ] Ajouter le profil Mac en Bluetooth direct sans dongle.
- [ ] Définir les touches de sélection USB/Bluetooth et les profils d'appairage.
- [ ] Vérifier le maintien du choix d'OS après les modes pavé numérique et jeu.

### Build et validation

- [ ] Préparer une commande de setup local avec des prérequis explicites.
- [ ] Construire les firmwares des parties concernées et nommer leurs artefacts
  sans ambiguïté.
- [ ] Vérifier les affectations, les macros et les transitions de couches pour
  chaque profil OS.
- [ ] Vérifier que les builds de profils différents ne partagent pas une mauvaise
  configuration en cache.
- [ ] Valider les deux claviers existants sur leur ordinateur respectif.
- [ ] Valider le Bluetooth direct sur le MacBook.
- [ ] Tester branchement USB de données, débranchement, charge secteur et
  redémarrage, y compris avec une préférence de sortie sauvegardée.
- [ ] Documenter les étapes de flash et d'appairage, puis les exécuter uniquement
  lorsque l'utilisateur le demande.

## Ideas / backlog

- Simplifier les macros et rendre les affectations plus lisibles.
- Ajouter le scroll aux encodeurs.
- Améliorer les informations de batterie et de connexion, avec ou sans LED.
- Étudier l'autonomie, la veille et la puissance BLE à partir de mesures réelles.
- Étudier la possibilité d'arrêter la liaison Bluetooth vers le Mac pendant
  l'utilisation USB, sans casser la communication entre les moitiés.
- Étudier séparément un fonctionnement sans aucun Bluetooth et ses besoins matériels.
- Réexaminer les macros de retour de mode qui écrivent puis effacent du texte dans
  l'application active.
- Conserver la CI GitHub comme validation optionnelle si elle apporte quelque chose,
  sans en faire un prérequis du build local.
