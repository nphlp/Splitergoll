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
- [x] Confirmer le flash et le fonctionnement de V1 avec `wire windows`, gaming inclus. Le parcours complet des couches reste distinct.
- [ ] Terminer les essais `wire` sur Mac puis Windows : toutes les touches, encodeurs, accents, Shift, couches et redémarrages. Voir [les essais wire](tests-wire.md).
- [x] Réaliser le diagnostic court de latence sur V1 et le Mac. Aucun écart gauche/droite significatif reproduit, effet des lecteurs incertain, aucune mesure totale ni cause établie. Voir [le résultat et le protocole](latence.md).

## Étapes suivantes

Diagnostic rapide de latence clos. Suite retenue : migration ZMK/Zephyr,
puis détection automatique d’OS et autres améliorations du backlog. Les essais
complets des keymaps restent à terminer. Les essais de veille et d’autonomie
nécessitent V2, le clavier sur batteries.

La priorité USB est différée après l’auto-détection d’OS, pour viser un clavier
« full auto » qui adapte sa keymap et sa sortie USB/Bluetooth à l’ordinateur connecté.

- [x] Préparer le diagnostic USB sans bascule de keymap et valider une première
  lecture sur V1 et le Mac : 24 requêtes sans perte, voir [le protocole](detection-os.md).
- [x] Comparer trois démarrages sur le Mac : les 24 requêtes ont les mêmes
  paramètres et le même ordre, sans perte ni rejet.
- [x] Lire et comparer trois captures Windows avec le même firmware : 57, 58 et
  57 requêtes sans perte. Plusieurs différences Mac/Windows sont stables,
  une demande initiale est répétée dans un essai Windows.
- [x] Regrouper le code du clavier dans `keyboard/` et séparer toute l’étude des
  signatures dans `diagnostics/os-signature/`, sans modifier les commandes Make.
- [ ] Préparer un classificateur diagnostique qui tolère les répétitions et les
  indices contradictoires, avec résultat inconnu, sans changer la keymap avant validation.
- [ ] Isoler les échanges du lecteur CDC, comparer les signatures avec Linux et
  revalider avec les descripteurs normaux avant toute décision automatique.
- [ ] Étudier ensuite les signatures Bluetooth sur V1 alimenté en USB, sans batterie.

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
- [x] Comparer les versions et identifier les ruptures : voir [la préparation de migration](migration-zmk.md). ZMK 0.3.0 reste la dernière stable, la 0.4 est en préparation.
- [x] Compiler et archiver les 32 UF2 de ZMK 0.3.0 : deux matériels, quatre keymaps, deux modes et deux moitiés, avec métadonnées et empreintes. Le snapshot reste dans l’historique Git après son retrait du checkout. Voir [les firmwares stables](firmwares-stables.md).
- [x] Retenir la migration vers un snapshot de développement figé avec Zephyr 4.1, après publication du snapshot stable pour le retour arrière.
- [x] Migrer le build vers ZMK `4dd95d7840eb` et Zephyr 4.1 : manifeste figé, image officielle, nom de carte et contrôles UF2. Compiler les 32 UF2 et comparer leurs keymaps et paramètres matériels au snapshot stable.
- [x] Conserver les transitions de couches sans le nouveau verrouillage implicite, avec comparaison du moteur sur 245 952 séquences d’actions.
- [x] Activer le bootloader par rétention via la variante nice!nano, contrôler le code UF2 `0x57` et le partitionnement.
- [x] Valider les essais rapportés sur V1 en `wire macos` : saisie, Shift, encodeurs, copier/coller, reconnexion, couches, accents, triple appui pavé, accès par `&bootloader` et retour par reset.
- [ ] Compléter la validation de migration : gestes non encore essayés, `&sys_reset`, double appui physique, autres profils, V2, BLE vers l’hôte, réglages persistants, veille et batteries.
- [ ] Migrer la matrice vers l’API input dans un chantier distinct pour remplacer KSCAN déprécié.
- [ ] Résoudre en amont l’avertissement `const` du driver tap-dance, sans masquer le diagnostic du compilateur.
- [x] Déclarer la matrice des deux moitiés comme source de réveil avec `wakeup-source`.
- [x] Compiler les deux moitiés avec et sans veille, vérifier la gestion d’alimentation et la source de réveil.
- [x] Activer la veille après 15 minutes et le reporting batterie dans le profil `bluetooth`.
- [x] Réaliser les essais avec inactivité 10 secondes, veille 60 secondes et mesure batterie 5 secondes, puis retirer le profil temporaire.
- [x] Confirmer la reprise immédiate après inactivité et la reconnexion après veille en environ 3 secondes, sans reset.
- [x] Tester séparément le réveil gauche et droite : premier appui sans saisie, second appui fonctionnel après reconnexion. La droite seule ne réveille pas la gauche.
- [x] Reflasher les deux moitiés avec les délais Bluetooth normaux après les essais, sans reset des appairages.
- [x] Valider les builds `wire debug` et `bluetooth debug`, avec consoles USB et artefacts séparés.
- [x] Lire les deux consoles USB de V1 en `wire macos debug` et observer les événements de droite reçus par la gauche.
- [x] Confirmer le retour de V1 à `wire macos` normal, désormais sur le firmware migré.
- [ ] Tester les consoles USB du profil Bluetooth sur V2, puis revenir aux firmwares normaux.
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
- [ ] Définir et ajouter le profil Bluetooth avec priorité USB : bascule à la connexion à l’ordinateur, retour Bluetooth au débranchement et comportement sur chargeur secteur.
- [ ] Réintégrer le dongle XIAO BLE au build paramétrable, comme centrale des deux moitiés.
- [x] Documenter le [flash](flash-firmware.md) et l’[appairage](appairage-bluetooth.md) des profils actuels.
- [x] Ajouter `make flash` sur macOS : dernier build du profil et de la moitié, date complète et relative, confirmation unique avant copie.
- [x] Valider le flash automatique sur les deux moitiés et leur fonctionnement avec `wire macos` sur le Mac.
- [ ] Étendre la détection des bootloaders à Linux/Windows, tester le chemin UF2 explicite, debug et l’annulation sur matériel.
- [x] Résoudre l’appairage de droite avec un reset vérifié des deux moitiés, documenter la procédure et le diagnostic.
- [x] Revenir aux deux firmwares Bluetooth normaux sans logs et confirmer la saisie.
- [x] Tester la saisie Bluetooth directe des deux moitiés sur le MacBook, USB branchés pour l’alimentation.
- [x] Tester la saisie sur batterie seule, sans USB ni dongle.
- [x] Activer la récupération et le proxy de batterie droite sur la centrale Bluetooth.
- [x] Lire les deux mesures sur le Mac : gauche 63 %, droite 62 %. Le menu natif n’affiche que la gauche, voir [batteries](batteries.md).
- [x] Ajouter `make bat` pour une lecture rapide et `make bat avg` pour une moyenne par moitié sur 30 secondes.
- [x] Garder un Makefile direct : chaque commande appelle son script, qui valide ses arguments. L’aide colorée est dans un script Bash.
- [ ] Étudier un affichage permanent des deux batteries sur le Mac, valider les notifications, l’autonomie et le sommeil de l’hôte.
- [ ] Comparer la veille profonde de la centrale et son maintien en inactivité : reprise depuis la droite, première frappe, délai de reconnexion et impact sur l’autonomie, puis choisir le comportement.
- [ ] Tester branchement USB, débranchement, charge secteur et préférences sauvegardées.
- [ ] Vérifier l’outillage de build sur Windows/WSL.
- [ ] Promouvoir `docs/README.md` à la racine à la fin, adapter les liens et conserver les photos, conceptions et inspirations historiques.

## Idées

- [ ] Détecter l’OS de l’ordinateur connecté (Windows, macOS, Linux) et sélectionner automatiquement la keymap adaptée.
  L’objectif est une reconnaissance passive à la connexion USB ou Bluetooth, sans logiciel à installer,
  sans association manuelle entre ordinateur et OS, ni profil ou keymap à changer à l’usage.
  Évaluer les signatures réelles, la fiabilité et les limites en USB, Bluetooth direct et via dongle,
  y compris après reconnexion et réveil. Une sélection manuelle ne remplit pas cet objectif.
  Un programme compagnon reste une alternative à rediscuter si la reconnaissance passive ne suffit pas.
- [ ] Garder le choix d’OS après les modes pavé numérique et jeu.
- [ ] Réexaminer les macros de retour de mode qui écrivent puis effacent du texte.
- [ ] Ajouter le scroll aux encodeurs et des informations de batterie/connexion.
- [ ] Mesurer l’autonomie et améliorer la veille et la puissance BLE.
- [ ] Étudier l’arrêt du Bluetooth vers le Mac pendant l’usage USB.
- [ ] Étudier séparément un fonctionnement sans aucune radio et ses besoins matériels.
