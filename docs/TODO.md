# TODO et idées

## Première version : câble et build local

- [x] Créer `unified-firmware` et les archives `wire` / `wire-minimal`.
- [x] Récupérer le README et les fichiers de conception de `main`.
- [x] Garder la keymap complète de `dongle`, les helpers et les GPIO.
- [x] Configurer la gauche centrale USB et la droite périphérique BLE.
- [x] Figer ZMK 0.3.0, les dépendances et l’image Docker.
- [x] Préparer un Makefile avec `make setup` et `make build`.
- [x] Compiler les deux moitiés et vérifier les UF2, leurs empreintes et les rôles.
- [ ] Tester le clavier Windows : touches, encodeurs, accents, Shift, couches et redémarrages.

## Étapes suivantes

- [x] Ajouter dans `keymap.md` les sept schémas du README avec les légendes de `dongle`.
- [x] Préparer le futur README dans `docs/README.md`, index des autres documents.
- [x] Alléger le Makefile et répartir le build dans des scripts séparés.
- [x] Remplacer le besoin de `.env` par des constantes en tête du script commun.
- [x] Revalider `make setup` et `make build` après ce découpage.
- [x] Ajouter `make stop` et `make clear`, limités aux conteneurs et fichiers de ce checkout.
- [x] Vérifier le nettoyage effectué avec `clear` puis refaire une préparation complète.
- [ ] Valider réellement l’arrêt d’un build avec `stop`.
- [x] Ajouter des logs en direct repliables, les journaux complets et les durées du setup/build.
- [ ] Préparer la montée de version ZMK/Zephyr, adapter les noms de cartes et revalider les deux firmwares.
- [ ] Définir les paramètres OS `windows`, `macos`, `linux`, `multi`.
- [ ] Définir si un OS de build est fixe ou seulement initial.
- [ ] Ajouter les profils Bluetooth direct, priorité USB et dongle.
- [ ] Documenter les transitions de flash et d’appairage.
- [ ] Tester le Bluetooth direct sur le MacBook.
- [ ] Tester branchement USB, débranchement, charge secteur et préférences sauvegardées.
- [ ] Vérifier l’outillage de build sur Windows/WSL.
- [ ] Promouvoir `docs/README.md` à la racine à la fin, adapter les liens et conserver les photos, conceptions et inspirations historiques.

## Idées

- Garder le choix d’OS après les modes pavé numérique et jeu.
- Réexaminer les macros de retour de mode qui écrivent puis effacent du texte.
- Ajouter le scroll aux encodeurs et des informations de batterie/connexion.
- Mesurer l’autonomie et améliorer la veille et la puissance BLE.
- Étudier l’arrêt du Bluetooth vers le Mac pendant l’usage USB.
- Étudier séparément un fonctionnement sans aucune radio et ses besoins matériels.
