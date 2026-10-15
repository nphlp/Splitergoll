# TODO et idées

## Première version : câble et build local

- [x] Créer `unified-firmware` et les archives `wire` / `wire-minimal`.
- [x] Récupérer le README et les fichiers de conception de `main`.
- [x] Garder la keymap complète de `dongle`, les helpers et les GPIO.
- [x] Configurer la gauche centrale USB et la droite périphérique BLE.
- [x] Figer ZMK 0.3.0, les dépendances et l’image Docker.
- [x] Préparer un Makefile avec `make setup` et `make build`.
- [x] Compiler les deux moitiés et vérifier les UF2, leurs empreintes et les rôles.
- [x] Confirmer le flash et la saisie USB des deux moitiés sur le Mac avec le clavier PC sans batterie, variante `wire debug`.
- [ ] Terminer les essais `wire` sur Mac puis Windows : toutes les touches, encodeurs, accents, Shift, couches et redémarrages. Voir [les essais wire](tests-wire.md).
- [ ] Mesurer la latence ressentie à droite en comparant les variantes normale et debug avant tout réglage radio.

## Étapes suivantes

- [x] Ajouter dans `keymap.md` les sept schémas du README avec les légendes de `dongle`.
- [x] Préparer le futur README dans `docs/README.md`, index des autres documents.
- [x] Alléger le Makefile et répartir le build dans des scripts séparés.
- [x] Remplacer le besoin de `.env` par des constantes en tête du script commun.
- [x] Revalider `make setup` et les compilations des profils après ce découpage.
- [x] Ajouter `make stop` et `make clear`, limités aux conteneurs et fichiers de ce checkout.
- [x] Vérifier le nettoyage effectué avec `clear` puis refaire une préparation complète.
- [ ] Valider réellement l’arrêt d’un build avec `stop`.
- [x] Ajouter des logs en direct repliables, les journaux complets et les durées du setup/build.
- [x] Nommer les sorties par profil et date lisible : `wire-AAAA-MM-JJ-HH-MM-SS`.
- [x] Choisir explicitement le matériel et la keymap, par exemple `make build wire multi` ou `make build bluetooth macos`.
- [x] Faire afficher les profils disponibles par `make build`, sans compilation implicite.
- [x] Compiler et contrôler les quatre UF2 avec des caches séparés par profil et moitié.
- [ ] Préparer la montée de version ZMK/Zephyr, adapter les noms de cartes et revalider les deux firmwares.
- [ ] Migrer le bootloader vers l’API de rétention, conserver les codes UF2 et vérifier `&bootloader` / `&sys_reset` sur le matériel.
- [x] Déclarer la matrice des deux moitiés comme source de réveil avec `wakeup-source`.
- [x] Compiler les deux moitiés avec et sans veille, vérifier la gestion d’alimentation et la source de réveil.
- [x] Activer la veille après 15 minutes et le reporting batterie dans le profil `bluetooth`.
- [x] Réaliser les essais avec inactivité 10 secondes, veille 60 secondes et mesure batterie 5 secondes, puis retirer le profil temporaire.
- [x] Confirmer la reprise immédiate après inactivité et la reconnexion après veille en environ 3 secondes, sans reset.
- [x] Tester séparément le réveil gauche et droite : premier appui sans saisie, second appui fonctionnel après reconnexion. La droite seule ne réveille pas la gauche.
- [x] Reflasher les deux moitiés avec les délais Bluetooth normaux après les essais, sans reset des appairages.
- [x] Valider les builds `wire debug` et `bluetooth debug`, avec consoles USB et artefacts séparés.
- [ ] Tester les consoles USB sur le matériel avec les deux profils, puis revenir aux firmwares normaux.
- [ ] Tester le réveil de toutes les touches et des encodeurs.
- [ ] Résoudre le drapeau réseau `DOCKER_INSECURE_NO_IPTABLES_RAW` au niveau du moteur Docker, sans masquer l’avertissement.
- [x] Conserver la sauvegarde intacte de `multi` et ses dépendances dans `archives/keymap-multi/`, avec empreintes, pour le commit du découpage.
- [x] Séparer les keymaps `windows` avec jeu, `macos`, `linux` et `multi`, choisies après le matériel dans la commande de build.
- [x] Rendre le matériel et la keymap obligatoires, sans sélection implicite, et expliquer leur ordre dans l’aide.
- [x] Choisir un OS fixe pour les profils spécialisés, conserver les bascules actuelles uniquement dans `multi`.
- [x] Partager la couche de base entre les quatre keymaps et aligner les bindings des deux moitiés sous leurs schémas.
- [x] Garder `splitergoll.keymap` et ses helpers historiques intacts, utiliser une composition `multi` indépendante avec les composants communs actifs.
- [x] Regrouper cette archive hors du shield et des profils actifs, retirer le doublon de sauvegarde de la configuration.
- [x] Valider les huit paires normales et leurs affectations compilées, plus `wire macos debug` et `bluetooth windows debug` : vingt UF2 contrôlés.
- [ ] Tester au clavier les profils fixes : touches, encodeurs, accents, retour du pavé numérique, gaming Windows et démarrage après veille.
- [ ] Conserver le choix d’OS de `multi` après veille profonde. Le retour macOS vers Windows est confirmé.
- [x] Ajouter le Bluetooth direct sans dongle, conservé pendant la charge USB.
- [ ] Ajouter les profils priorité USB et dongle.
- [x] Documenter le [flash](flash-firmware.md) et l’[appairage](appairage-bluetooth.md) des profils actuels.
- [x] Résoudre l’appairage de droite avec un reset vérifié des deux moitiés, documenter la procédure et le diagnostic.
- [x] Revenir aux deux firmwares Bluetooth normaux sans logs et confirmer la saisie.
- [x] Tester la saisie Bluetooth directe des deux moitiés sur le MacBook, USB branchés pour l’alimentation.
- [x] Tester la saisie sur batterie seule, sans USB ni dongle.
- [x] Activer la récupération et le proxy de batterie droite sur la centrale Bluetooth.
- [x] Lire les deux mesures sur le Mac : gauche 63 %, droite 62 %. Le menu natif n’affiche que la gauche, voir [batteries](batteries.md).
- [x] Ajouter `make bat` pour une lecture rapide et `make bat avg` pour une moyenne par moitié sur 30 secondes.
- [x] Garder un Makefile direct : chaque commande appelle son script, qui valide ses arguments. L’aide colorée est dans un script Bash.
- [ ] Étudier un affichage permanent des deux batteries sur le Mac, valider les notifications, l’autonomie et le sommeil de l’hôte.
- [ ] Après les essais, choisir la veille profonde de la centrale ou son maintien en inactivité pour un réveil depuis la droite.
- [ ] Tester branchement USB, débranchement, charge secteur et préférences sauvegardées.
- [ ] Vérifier l’outillage de build sur Windows/WSL.
- [ ] Promouvoir `docs/README.md` à la racine à la fin, adapter les liens et conserver les photos, conceptions et inspirations historiques.

## Idées

- [ ] Étudier la détection automatique de l’OS de l’ordinateur connecté (Windows, macOS, Linux) pour proposer la keymap adaptée. Vérifier sa fiabilité en USB, Bluetooth direct et via dongle, avec un choix manuel de secours.
- Garder le choix d’OS après les modes pavé numérique et jeu.
- Réexaminer les macros de retour de mode qui écrivent puis effacent du texte.
- Ajouter le scroll aux encodeurs et des informations de batterie/connexion.
- Mesurer l’autonomie et améliorer la veille et la puissance BLE.
- Étudier l’arrêt du Bluetooth vers le Mac pendant l’usage USB.
- Étudier séparément un fonctionnement sans aucune radio et ses besoins matériels.
