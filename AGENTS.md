# Splitergoll

## Intention

Faire évoluer les deux claviers physiques existants vers une configuration ZMK
commune, paramétrable et compilable localement. Le projet est classé `sync` avec
les règles globales de l'utilisateur.

Le cadrage, les idées et les TODO détaillés sont dans
[la feuille de route](docs/feuille-de-route.md). Cette étape réunit la documentation
et les fichiers de conception de `main` avec le firmware de `dongle` au commit
`2584ca4`, sans modifier ce firmware.

`README.md` reprend à l'identique le README historique de `main`. `Images/`,
`Design PCB/` et `Design 3D/` sont également repris sans modification depuis
`cbad9a0`. Le README sera actualisé après la relecture, pour refléter les affectations
de `dongle` et les nouveaux profils lorsque leurs contrats seront validés.

## Contexte et invariants

- Le clavier du PC Windows utilise deux câbles USB, sans batterie.
- Le clavier du MacBook utilise deux moitiés sur batterie et un dongle XIAO BLE.
- Les contrôleurs des moitiés sont des répliques de nice!nano v2. Le dongle Nordic
  visible dans les anciennes photos n'est pas utilisé.
- La keymap complète de `dongle` est la référence. Préserver ses 44 positions,
  macros, couches conditionnelles, accents et comportements Shift lors de la refonte.
- L'OS, la topologie centrale/périphériques et la sortie USB/Bluetooth sont des
  paramètres distincts. Les rôles sont configurés dans le firmware.
- La sélection USB/Bluetooth porte sur la sortie vers l'ordinateur. La liaison
  interne entre les moitiés reste nécessaire avec le matériel actuel.
- Le Bluetooth direct sans dongle est un profil à ajouter et à valider, pas un
  fonctionnement déjà validé sur les deux claviers.
- Les branches historiques restent intactes. Les archives publiées `wire` et
  `wire-minimal` pointent respectivement sur `main` à `cbad9a0` et `main-minimal`
  à `27ed2b7`, sans adaptation de leur contenu.

## TODO

- [x] Créer et publier `unified-firmware` depuis `dongle`.
- [x] Documenter le contexte, les décisions et les questions ouvertes en français.
- [x] Créer et publier les archives `wire` et `wire-minimal`.
- [x] Reprendre le README et les fichiers de conception de `main`.
- [ ] Actualiser le README historique après la relecture et le cadrage des profils.
- [ ] Valider l'organisation commune et les contrats des commandes de build.
- [ ] Choisir une version ZMK et un environnement local reproductibles.
- [ ] Implémenter et vérifier les profils OS et les deux topologies.
- [ ] Valider les deux usages actuels puis le Bluetooth direct sur le matériel.

## Ideas / backlog

- Build local sans dépendance aux GitHub Actions, avec des artefacts distincts.
- Profils Windows, macOS et Linux, variantes complète et minimale.
- USB prioritaire et retour au Bluetooth pour le futur profil direct du Mac.
- Charge secteur avec recours possible à une sélection manuelle de sortie.
- Arrêt du Bluetooth vers le Mac en USB et étude séparée d'un fonctionnement sans
  aucune radio, selon la faisabilité et les besoins matériels.
- Simplification des macros, scroll sur encodeurs, retours de batterie et de connexion.
- Conservation du choix d'OS en passant par les modes pavé numérique et jeu.

## Validation attendue

Pour la documentation, vérifier les liens locaux et distinguer décisions retenues,
propositions et comportements testés. Aucun build ne valide cette étape documentaire.

Pour la future implémentation, contrôler les affectations et les transitions de
couches par OS, compiler les parties de chaque profil dans des répertoires isolés,
puis vérifier les usages sur les appareils. Inclure le branchement USB de données,
le débranchement, la charge secteur et les préférences de sortie sauvegardées.
