# TODO et idées

## Première version : câble et build local

- [x] Créer `unified-firmware` et les archives `wire` / `wire-minimal`.
- [x] Récupérer le README et les fichiers de conception de `main`.
- [x] Garder la keymap complète de `dongle`, les helpers et les GPIO.
- [x] Configurer la gauche centrale USB et la droite périphérique BLE.
- [x] Figer ZMK 0.3.0, les dépendances et l’image Docker.
- [x] Préparer un Makefile avec `make setup` et `make build`.
- [x] Compiler les deux moitiés et vérifier les UF2, leurs empreintes et les rôles.
- [ ] Tester le profil `wire` sur le clavier Mac, puis sur le PC Windows : touches, encodeurs, accents, Shift, couches et redémarrages.

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
- [x] Nommer les sorties par profil et date lisible : `wire-AAAA-MM-JJ-HH-MM-SS`.
- [x] Choisir le profil avec `make build wire` ou `make build bluetooth`, conserver une keymap unique.
- [x] Faire compiler tous les profils par `make build`, sans profil implicite.
- [x] Compiler et contrôler les quatre UF2 avec des caches séparés par profil et moitié.
- [ ] Préparer la montée de version ZMK/Zephyr, adapter les noms de cartes et revalider les deux firmwares.
- [ ] Migrer le bootloader vers l’API de rétention, conserver les codes UF2 et vérifier `&bootloader` / `&sys_reset` sur le matériel.
- [x] Déclarer la matrice des deux moitiés comme source de réveil avec `wakeup-source`.
- [x] Compiler les deux moitiés avec et sans veille, vérifier la gestion d’alimentation et la source de réveil.
- [x] Activer la veille après 15 minutes et le reporting batterie dans le profil `bluetooth`.
- [ ] Tester le réveil de toutes les touches, la première frappe et la reconnexion BLE.
- [ ] Résoudre le drapeau réseau `DOCKER_INSECURE_NO_IPTABLES_RAW` au niveau du moteur Docker, sans masquer l’avertissement.
- [ ] Sauvegarder dans Git la keymap `multi` actuelle et ses dépendances avant tout découpage.
- [ ] Définir les profils de keymap `windows` avec jeu, `macos`, `linux`, `multi` et leurs paramètres de build.
- [ ] Définir si un OS de build est fixe ou seulement initial.
- [x] Ajouter le Bluetooth direct sans dongle, conservé pendant la charge USB.
- [ ] Ajouter les profils priorité USB et dongle.
- [x] Documenter le [flash](flash-firmware.md) et l’[appairage](appairage-bluetooth.md) des profils actuels.
- [x] Résoudre l’appairage de droite avec un reset vérifié des deux moitiés, documenter la procédure et le diagnostic.
- [x] Revenir aux deux firmwares Bluetooth normaux sans logs et confirmer la saisie.
- [x] Tester la saisie Bluetooth directe des deux moitiés sur le MacBook, USB branchés pour l’alimentation.
- [x] Tester la saisie sur batterie seule, sans USB ni dongle.
- [ ] Décider puis tester la remontée des deux batteries et leur affichage sur le Mac, voir [l’étude](batteries.md).
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
