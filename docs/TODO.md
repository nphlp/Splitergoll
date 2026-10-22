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

Le [tableau de planification](planification.md) distingue les sujets terminables avec
Mac + V2 et ceux à valider lors d’une séance commune Mac/Windows. Au bureau,
priorité aux essais de l’app et des batteries. Les transitions Mac/Windows restent différées.

Détection USB et BLE Mac/Windows, mémoire d’OS et bascule entre appareils sont
validées sur V1 dans les conditions indiquées ci-dessous. Linux automatique reste à faire.
La fenêtre commune de dix secondes, avec reconnexion et nouvel appairage, est
intégrée au firmware, à valider sur V1. Les destinations mixtes restent à faire.

Ordre des prochains lots :

1. Valider la fenêtre commune sur V1 : premier appairage Mac puis Windows,
   mémoire d’OS, reconnexion des connus, fermeture et réouverture. Le nouveau firmware
   réserve une place sans détourner la saisie et borne à 30 secondes un appairage
   commencé dans la fenêtre. Aucun effacement supplémentaire n’est nécessaire.
2. Réunir USB et BLE autour des [destinations](destinations-saisie.md), en résolvant
   l’association entre l’hôte USB et son éventuel bond BLE. Puis intégrer Gaming USB.
3. Compléter le retour au précédent appareil au réveil, la correction manuelle d’OS,
   l’optimisation du délai de bascule, puis Linux et le dongle.

Le matériel courant est Mac + V2 au bureau. Les essais Mac/Windows sont en attente.
Lors de la séance précédente à domicile, l’utilisateur
confirme avoir flashé `bluetooth auto ble-trace`, puis effacé les deux slots hôtes
et oublié l’entrée du Mac. Le lot précédemment flashé refuse les nouveaux
appairages. La nouvelle gauche lève cette limitation et son flash est confirmé.
La lecture des slots et le nouvel appairage restent à valider. V2 garde son usage Bluetooth macOS au bureau.
Les essais courts sur
batterie de V2 confirment la reprise après inactivité et le réveil par la gauche.
La première frappe, toutes les touches et l’autonomie restent à vérifier. Aucun nouveau reset d’appairage,
flash, bascule matérielle ou intervention sur le PC occupé n’est implicite dans ce travail.
Un essai des fenêtres avec le Mac seul resterait partiel. Il demanderait d’abord
de préparer la mémoire d’OS propre à V2, absente du profil macOS normal.

- [x] Préparer le diagnostic USB sans bascule de keymap et valider une première
  lecture sur V1 et le Mac : 24 requêtes sans perte, voir [le protocole](detection-os.md).
- [x] Comparer trois démarrages sur le Mac : les 24 requêtes ont les mêmes
  paramètres et le même ordre, sans perte ni rejet.
- [x] Lire et comparer trois captures Windows avec le même firmware : 57, 58 et
  57 requêtes sans perte. Plusieurs différences Mac/Windows sont stables,
  une demande initiale est répétée dans un essai Windows.
- [x] Regrouper le code du clavier dans `keyboard/` et séparer toute l’étude des
  signatures dans `diagnostics/os-signature/`, sans modifier les commandes Make.
- [x] Préparer un classificateur diagnostique qui tolère les répétitions et les
  indices contradictoires, avec résultat inconnu, sans changer la keymap avant validation.
- [x] Flasher le classifieur diagnostique sur V1 et confirmer son hypothèse macOS.
- [x] Reflasher le correctif de sonde initiale facultative et confirmer Windows sur V1.
- [x] Confirmer le retour au Mac avec le même firmware corrigé, sans nouveau flash.
  La keymap reste inchangée pendant cet essai.
- [x] Isoler les échanges du lecteur CDC et valider la sélection automatique sur
  V1 Mac/Windows avec les descripteurs normaux.
- [ ] Comparer les signatures avec Linux avant d’ajouter sa reconnaissance automatique.
- [x] Séparer les cycles USB dans le diagnostic et exiger des motifs de noms suffisants et concordants.
- [x] Implémenter `wire auto` : sélection USB expérimentale macOS / Windows avec gaming, couches partagées avec les profils fixes, attente sans OS implicite.
- [x] Confirmer la sélection de `wire auto usb-trace` sur V1 : noms macOS/Windows via l’encodeur droit, gaming puis retour Windows, fonctionnement habituel sur Mac.
- [x] Flasher V1 avec `wire auto` normal, sans `usb-trace`.
- [x] Confirmer `wire auto` normal sur macOS. Sur Windows, le premier essai ne produit aucune saisie.
- [x] Capturer l’énumération Windows du firmware normal avec `capture.ps1 -UsbEvents`, contrôler les pertes et comparer les requêtes avant de modifier le classifieur.
- [x] Reproduire le classement inconnu sans CDC et reconnaître le motif de noms 255 puis 262, avec tests de régression et rejeu des captures précédentes.
- [x] Confirmer le correctif sur V1 : `wire auto` normal, saisie et détection Windows puis retour macOS, sans nouveau flash entre les hôtes.
- [x] Optimiser l’initialisation `wire auto` : retirer les 750 ms au démarrage à froid et empêcher les appuis ignorés de prolonger l’attente, avec maintien des gardes de relâchement et de comportements différés.
- [x] Constater un démarrage plus rapide après la première optimisation, selon le ressenti utilisateur, sans mesure chronométrée.
- [x] Déclencher la sélection sur les requêtes et événements USB, sans stabilisation fixe de 250 ms ni contrôle périodique de 20 ms.
- [x] Valider le démarrage sur événements : Mac → Windows → Mac, bon OS affiché par l’encodeur et rapidité ressentie à chaque branchement, sans mesure chronométrée.
- [ ] Tester les cas particuliers au branchement : frappe précoce et touche maintenue.
- [x] Corriger les apostrophes des chaînes PowerShell et reproduire les ParserError avec le parseur officiel avant/après, sans modifier les regex USBTRACE.
- [x] Valider le nouveau mode ETW sur Windows : démarrage/arrêt de sa seule session et export des fichiers dans le dossier courant.
- [ ] Confirmer le décodage Tracerpt avec `-lr` sur le PC lors d’une prochaine capture utile.
- [x] Confirmer l’usage des layouts `auto` normal sur Mac et Windows et du gaming : l’utilisateur rapporte un fonctionnement correct, sans problème de bascule ou de disposition.
- [x] Rendre `capture.ps1` autonome sous Windows : un seul fichier dans le dossier de travail, capture `.txt` directement dans le dossier courant.
- [x] Confirmer sur Windows la destination du lecteur autonome mis à jour.
- [ ] Confirmer la séparation des cycles lors d’un reset du bus sans coupure d’alimentation.
  L’aller-retour Mac/Windows avec débranchement USB est validé.
- [x] Préparer `ble-trace` et lire la première connexion V1/Mac : 49 événements,
  sans perte ni rejet, avec découverte HID et connexion chiffrée.
- [x] Comparer une reconnexion Mac sans reset ni coupure USB : 27 événements,
  dont 20 requêtes ATT, sans relecture de la description HID ni perte/rejet.
- [x] Valider sur matériel la sélection de la case libre 1 pour Windows,
  puis le retour à la case Mac 0, avec les deux appairages conservés.
- [x] Capturer la première connexion Windows en BLE : 76 événements, dont
  67 requêtes ATT, sans perte/rejet et avec chiffrement établi sans erreur.
- [x] Capturer une reconnexion Windows déjà appairée, en réutilisant la case 1 :
  six événements, dont deux requêtes ATT (MTU et batterie), sans perte/rejet.
- [x] Comparer les premières connexions et reconnexions Mac/Windows sur V1 alimenté
  par USB. Les lectures distinctives Windows disparaissent après appairage,
  voir [le résultat](detection-os-bluetooth.md).
- [x] Définir la direction du firmware BLE : un seul ordinateur connecté est
  automatiquement sélectionné, plusieurs nécessitent une bascule explicite.
  L’OS reconnu initialement est mémorisé pour l’appairage, sous réserve de valider
  la fiabilité de sa reconnaissance. Voir [la sélection d’appareil](selection-appareil.md)
  et [la mémoire d’OS](memoire-os.md).
- [x] Préparer et tester sans matériel la mémoire persistante dans le diagnostic gauche,
  avec état de sauvegarde, identité du bond et gestion des erreurs.
- [x] Flasher la gauche et lire le diagnostic mémoire sur V1 : deux hôtes connectés,
  Mac sélectionné, mémoire encore vide pour les anciens appairages, sans erreur ni perte.
- [x] Refaire l’appairage Mac et confirmer son acquisition initiale :
  `stored=macos`, `origin=initial`, `state=saved`, sans erreur ni perte.
- [x] Reconnecter le Mac sans nouvel appairage et retrouver `stored=macos`,
  `state=saved`, `learning=0`, sans erreur ni perte.
- [x] Refaire l’appairage Windows et capturer sa découverte partielle : trois des
  quatre indices présents, résultat inconnu sauvegardé sans erreur. La détection n’est pas validée.
- [x] Prendre en charge cette variante Windows avec des tests positifs et négatifs,
  sans déduire l’OS d’une absence de requêtes, du lecteur USB ou du numéro de slot.
- [x] Flasher le correctif de découverte partielle, conserver le Mac appairé et
  refaire uniquement l’appairage Windows pour confirmer `stored=windows` sur matériel.
  Le résultat est sauvegardé avec `origin=initial`, `evidence=005c`, sans erreur ni perte.
  Le Mac retrouve aussi `stored=macos` après ce flash, sans nouvel apprentissage.
- [x] Valider sur V1 la restauration des deux OS après reset et reconnexion :
  `stored=macos` et `stored=windows`, `state=saved`, `learning=0`, sans erreur ni perte.
  Windows retrouve son OS malgré une reconnexion sans signature suffisante.
- [x] Préparer `bluetooth auto` avec les mêmes couches que les profils fixes,
  depuis l’OS mémorisé de l’appareil sélectionné, avec garde des touches et sans OS par défaut.
- [x] Flasher la gauche `bluetooth auto ble-trace` et lire son état sur V1 : Mac sélectionné,
  `stored=macos`, `state=saved`, sans nouvel apprentissage ni erreur. Windows reste déconnecté.
- [x] Confirmer sur V1 en `bluetooth auto ble-trace` la bascule Mac/Windows par
  raccourci et l’association correcte de leurs keymaps, selon le retour utilisateur.
- [ ] Compléter les essais BLE : pavé, gaming, touches maintenues, redémarrage et
  firmware normal. Voir [le guide](keymap-auto-bluetooth.md).
- [x] Préparer L1 + clic encodeur droit dans `bluetooth auto` pour sélectionner uniquement
  l’appareil connecté suivant, après relâchement des touches. Aucun appel à un appareil déconnecté.
- [ ] Compléter les cas limites du raccourci sur V1 : Mac seul sans effet,
  maintien d’une touche et retour depuis un OS indisponible.
- [ ] Mesurer puis réduire le délai de bascule d’appareil : environ deux secondes
  ressenties sur V1. Distinguer le geste, la garde de 750 ms et la disponibilité de
  la nouvelle destination, sans supprimer les protections des comportements différés.
- [ ] Compléter les essais matériels de mémoire : remplacement d’appairage,
  correction manuelle et veille profonde sur V2. Les tests natifs ne remplacent
  pas ces essais. Ne pas déduire Windows d’une absence de lectures ni de l’index du slot.
- [ ] Ajouter une correction manuelle persistante de l’OS de l’appareil sélectionné,
  prioritaire et conservée aux reconnexions. Le raccourci doit rester utilisable
  avec un OS initial inconnu. Aucun choix de geste ni changement de keymap maintenant.
- [x] Examiner les sources publiques BLE : règles et cache de Cormoran,
  métadonnées SMP/LL de Blue2thprinting, annonces Apple Continuity et sélection
  manuelle d’oskey. Aucun module ajouté. Voir [les pistes](detection-os-bluetooth.md#pistes-retenues).
- [x] Préparer et évaluer un classifieur BLE diagnostique sur les captures existantes avant
  toute sélection de keymap : recherches ciblées DIS côté Mac, séquence de découverte
  et de cache GATT côté Windows, résultats inconnu/ambigu si les preuves manquent
  ou se contredisent. Ne pas prendre les numéros d’emplacement pour des signatures d’OS.
- [x] Intégrer le classifieur au seul diagnostic `ble-trace`, sans bascule de keymap
  ni mémoire d’OS : calcul à la lecture, une hypothèse par session et lecteurs compatibles.
- [x] Flasher la gauche de diagnostic mise à jour et confirmer l’hypothèse macOS
  sur V1 : 27 événements, sans perte ni rejet, lecture USB sur le Mac.
- [x] Confirmer le résultat du diagnostic intégré lors d’une reconnexion Windows :
  inconnu, avec seulement une requête de MTU, sans découverte ni mémoire d’OS.
- [x] Préparer le diagnostic de plusieurs hôtes BLE connectés : traces et hypothèses
  isolées par connexion, statut par emplacement et bascule USB sans couper le
  Bluetooth des ordinateurs. Garder un stockage borné et séparer observation,
  destination des frappes et hypothèse d’OS. Autoriser la bascule pendant
  une connexion et refuser les changements pendant une frappe ou un comportement différé.
  Une bascule seule ne déclenche pas une nouvelle découverte de services.
- [x] Flasher la gauche et observer sur V1 les deux connexions simultanées :
  32 événements sans perte ni rejet, macOS reconnu et Windows inconnu.
- [x] Confirmer par les statuts la bascule 0 → 1 → 0 sur V1 sans couper les Bluetooth,
  avec les deux connexions présentes et sans générer de frappes.
- [x] Confirmer la réception de frappes sur Windows avec la keymap macOS fixe,
  selon le retour utilisateur pendant le test d’appairage.
- [ ] Vérifier le refus de bascule pendant un maintien et sa réussite après relâchement.
- [x] Lire les traces Mac et Windows avec l’USB gauche conservé sur le Mac.
  Les deux ordinateurs restent appairés, il s’agit de reconnexions.
- [x] Préparer l’effacement ciblé `ble forget` et le [protocole mémoire](tests-memoire-os.md),
  sans exécuter d’effacement sur les appairages actuels.
- [ ] Réaliser les essais de nouvel appairage avec effacement ciblé autorisé,
  sans reset de tous les bonds, puis répéter les captures sur chaque ordinateur.
  Le protocole est dans [capture-bluetooth-mac.md](capture-bluetooth-mac.md).
- [ ] Compléter les indices d’appairage si nécessaire : métadonnées SMP publiques
  strictement sélectionnées, sans clés, avant de conclure sur la détection initiale.
- [ ] Implémenter la sélection de destination BLE : aucun hôte = attente, un hôte
  connu connecté = sélection automatique, plusieurs = maintien du choix courant et
  bascule explicite. Séparer les connexions hôtes de celle de la moitié droite.
- [ ] Après redémarrage/veille profonde, sélectionner le premier hôte disponible
  immédiatement et laisser 5 secondes au précédent pour revenir. S’il revient dans
  la fenêtre, le restaurer, sinon mémoriser le nouvel hôte. Une bascule manuelle
  annule la fenêtre. Taper sur le premier hôte ne l’annule ni ne la prolonge, et
  n’empêche pas le retour au précédent. Tester ces frappes, les limites et les
  déconnexions, puis confirmer la durée sur matériel.
- [x] Développer et tester le moteur pur des fenêtres : ouverture dix secondes,
  recherche avec zéro/une destination, bascule avec plusieurs, transitions Gaming,
  tickets périmés, identités remplacées et attente des relâchements. Aucune API radio
  ni intégration firmware dans ce premier lot.
- [x] Raccorder les annonces, l’admission et l’affectation des nouveaux bonds dans
  ZMK : place réservée sans changer la destination, sauvegarde contrôlée puis mémoire
  d’OS, sans patch de dépendance. Les autres décisions du moteur restent à raccorder.
- [x] Séparer la reconnexion connue du nouvel appairage dans le périmètre du développement.
  Un appareil connu garde son slot et ses clés. ZMK exige actuellement un slot actif
  vide pour un nouveau bond et refuse un slot occupé, sans remplacement automatique.
- [x] Intégrer les annonces filtrées de reconnexion dans `bluetooth auto` : dix secondes
  au démarrage et avec L1 + clic droit, sans changement de slot à l’arrivée d’un hôte.
  Le premier lot n’admettait aucun nouvel appairage. Le suivant ajoute cet accueil
  sur une place libre, sans suppression de clé existante.
- [ ] Valider sur V1 les deux ordres de retour Mac/Windows, la fermeture à dix secondes,
  la réouverture par raccourci et la saisie sur l’appareil resté sélectionné.
- [x] Séparer slot d’accueil et destination de saisie. Sérialiser les nouveaux
  appairages, garder les connus avec slots pleins et laisser finir SMP sous 30 secondes.
  Les réservations et erreurs sont testées, sans écrasement d’un slot occupé.
- [ ] Valider les nouveaux appairages sur V1 avec Mac puis Windows : conservation
  de la saisie sur le premier hôte, OS initial sauvegardé, raccourci et retour après reset.
- [ ] Gérer les annonces pour accueillir un second ordinateur pendant que le premier
  est connecté. Les deux connexions simultanées sont observées sur V1, l’arrivée
  contrôlée d’un second hôte pendant que le premier est déjà connecté reste à tester.
  La proposition courante ouvre annonces et nouveaux appairages pendant dix secondes
  au démarrage/reset, au retour Gaming USB → Windows et via L1 + clic encodeur droit.
  Sans recherche explicite, l’arrivée d’un hôte ne change pas la sélection.
  Tester les deux ordres de reconnexion sans sélection USB.
- [x] Ajouter un raccourci de sélection d’appareil connecté, sans commande USB,
  distinct du changement manuel d’OS : L1 + clic encodeur droit dans `bluetooth auto`.
  Les appareils déconnectés sont ignorés, le clic simple indique l’OS après la bascule.
- [ ] Définir la [gestion de l’appairage](gestion-appairage.md) : états ouvert/fermé,
  fenêtres de dix secondes, appairage en cours à l’échéance,
  slot libre et refus sans effacement si plein. Garder les bonds existants.
- [ ] Étendre L1 + clic encodeur droit pour rouvrir dix secondes : une destination
  disponible = bascule vers le premier autre appareil prêt, plusieurs = prochaine
  destination déjà disponible sans bascule sur une arrivée tardive. Compter les
  destinations distinctes au geste, pas les liaisons USB/BLE. Tester plusieurs arrivées,
  absence d’arrivée, nouveau geste qui remplace l’attente et absence de destination.
- [ ] Définir puis implémenter Gaming USB : arrêt des annonces, nouveaux appairages
  refusés et Mac BLE déconnecté. Le retour à Windows réouvre dix secondes sans
  bloquer sa saisie USB ni sélectionner le Mac à son arrivée. Décider du BLE Windows
  pendant Gaming. Ne pas couper la liaison entre moitiés ni la seule sortie BLE.
  Depuis Gaming, L1 + clic droit revient à Windows et déclenche explicitement la recherche,
  avec bascule vers le premier autre appareil prêt si Windows était le seul disponible.
- [ ] Mesurer l’effet de la suppression des liaisons BLE hôtes sur la latence Gaming USB,
  sans confondre gain supposé et mesure réelle. La liaison de droite reste en BLE.
- [ ] Dernier recours si le full auto échoue : mode manuel avec mémoire persistante,
  changement d’OS par raccourci (par exemple L1 + clic encodeur), restauré au démarrage
  et après veille profonde. Cette variante entièrement manuelle est distincte de
  la correction manuelle intégrée au mode automatique. Envisager un profil de build `manual` distinct de `auto`, `multi`
  et des profils fixes, sans figer maintenant les noms ni modifier les profils existants.

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
- [x] Reprendre les essais courts sur V2 après migration, sur batterie : reprise
  après inactivité imperceptible à droite, droite seule sans saisie après environ
  une minute, puis reprise des deux côtés en 2 à 5 secondes après appuis à gauche.
  Appuis répétés, première frappe et délais exacts non contrôlés.
- [x] Retenir deux minutes d’inactivité pour le profil Bluetooth normal, avec contrôle
  au build. Retenir dix minutes avant veille profonde, sans reflash implicite de V2.
- [x] Reflasher les deux moitiés avec les délais Bluetooth normaux après les essais, sans reset des appairages.
- [x] Valider les builds `wire debug` et `bluetooth debug`, avec consoles USB et artefacts séparés.
- [x] Lire les deux consoles USB de V1 en `wire macos debug` et observer les événements de droite reçus par la gauche.
- [x] Confirmer le retour de V1 à `wire macos` normal, désormais sur le firmware migré.
- [ ] Tester les consoles USB du profil Bluetooth sur V2, puis revenir aux firmwares normaux.
- [ ] Tester le réveil de toutes les touches et des encodeurs.
- [ ] Résoudre le drapeau réseau `DOCKER_INSECURE_NO_IPTABLES_RAW` au niveau du moteur Docker, sans masquer l’avertissement.
- [x] Conserver la sauvegarde intacte de `multi` et ses dépendances dans `archives/keymap-multi/`, avec empreintes, pour le commit du découpage.
- [x] Retirer `archives/`, `.github/` et l’ancienne matrice `build.yaml` du checkout à la demande de l’utilisateur, conserver les anciennes sources dans l’historique Git et actualiser les références.
- [x] Séparer les keymaps `windows` avec jeu, `macos`, `linux` et `multi`, choisies après le matériel dans la commande de build.
- [x] Rendre le matériel et la keymap obligatoires, sans sélection implicite, et expliquer leur ordre dans l’aide.
- [x] Choisir un OS fixe pour les profils spécialisés, conserver les bascules actuelles uniquement dans `multi`.
- [x] Partager la couche de base entre les quatre keymaps et aligner les bindings des deux moitiés sous leurs schémas.
- [x] Garder `splitergoll.keymap` et ses helpers historiques intacts, utiliser une composition `multi` indépendante avec les composants communs actifs.
- [x] Regrouper cette archive hors du shield et des profils actifs, retirer le doublon de sauvegarde de la configuration.
- [x] Valider les huit paires normales et leurs affectations compilées, plus `wire macos debug` et `bluetooth windows debug` : vingt UF2 contrôlés.
- [ ] Tester au clavier les profils fixes : touches, encodeurs, accents, retour du pavé numérique, gaming Windows et démarrage après veille.
- [ ] Vérifier puis ajouter l’accès à `ü`, signalé manquant dans la keymap.
  Choisir son geste et valider la saisie sur les OS concernés.
- [ ] Conserver le choix d’OS de `multi` après veille profonde. Le retour macOS vers Windows est confirmé.
- [x] Ajouter le Bluetooth direct sans dongle, conservé pendant la charge USB.
- [ ] Définir puis ajouter un firmware réunissant USB et BLE : L1 + clic encodeur droit
  parcourt des destinations, avec priorité USB pour un même ordinateur. Cas attendu :
  USB Windows + BLE Mac + BLE Windows donne deux choix, USB Windows et BLE Mac.
  Garder la possibilité de choisir Mac BLE lorsque Windows USB est présent.
- [ ] Définir l’association entre un hôte USB et un appareil BLE mémorisé pour éviter
  les doublons. L’OS seul n’identifie pas la machine, tester notamment deux PC Windows.
- [ ] Définir le comportement à l’arrivée/retrait de l’USB : prise éventuelle de sélection,
  repli sur le BLE du même ordinateur et absence de destination pour une simple charge.
  Le nom du futur profil de build n’est pas encore fixé, les profils actuels restent disponibles.
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
- [x] Ajouter `make bat read` pour une lecture rapide et `make bat avg` pour une moyenne par moitié sur 30 secondes.
- [x] Garder un Makefile direct : chaque commande appelle son script, qui valide ses arguments. L’aide colorée est dans un script Bash.
- [x] Regrouper les commandes avec des séparateurs : aide, Firmware, Batteries
  (bat/app) et Debug (port/usb/ble). Sous-commandes par espace, aide sans action ou en cas d’argument erroné.
- [x] Implémenter le premier lot de l’[application macOS](application-macos.md) en Swift : batteries
  gauche/droite dans une fenêtre et la barre des menus, actualisation et reconnexion.
  Services GATT existants, limites USB indiquées, tests du modèle. Essais matériels encore à faire.
- [x] Compacter la fenêtre avec une taille fixe, lire toutes les 5 secondes et
  afficher gauche/droite sur deux lignes dans la barre des menus. À 100 %, indiquer
  une charge possible sans la présenter comme une charge détectée.
- [x] Simplifier la fenêtre : titre, connexion et deux batteries. Menu limité à connexion,
  ouvrir et quitter. Dock limité à la fenêtre ouverte, inscription native à l’ouverture de session.
- [x] Passer les mesures des deux moitiés à 5 secondes pendant l’activité et préparer
  la paire `bluetooth macos`. Reflash et impact sur l’autonomie restent à valider.
- [x] Ajouter une icône à l’app, une batterie avec éclair à 100 % et un tiret pour
  les valeurs indisponibles. Documenter la copie autonome dans Applications.
- [x] Remplacer l’icône stylisée par un SVG aux contours et positions réels du PCB.
  Afficher le Dock à l’ouverture de la fenêtre et le masquer à sa fermeture.
- [x] Retenir la silhouette noire de gauche sur carré blanc arrondi comme icône,
  en conservant la variante sur fond noir à côté.
- [x] Valider sur V2 le tiret droit à la déconnexion de droite, les deux tirets
  à la déconnexion de gauche et l’éclair lors du branchement USB de chaque moitié.
- [x] Afficher 0 % comme un tiret dans l’app,
  par convention, en conservant la mesure brute. Le lecteur CLI garde sa valeur.
- [x] Corriger le symbole de batterie avec éclair dans la barre des menus :
  proportions natives conservées, validation du rendu hors écran.
- [x] Ajouter `make app install` pour copier le bundle déjà compilé dans Applications,
  avec contrôles d’identité, signature, fermeture et restauration sur échec.
  Installation et mise à jour validées dans un dossier isolé du dépôt, puis installation
  réelle autorisée et vérifiée dans Applications.
- [x] Préparer des propositions SVG de contours pour l’app et les deux symboles
  de menus. Corriger les ombres avec un flou SVG continu, sans image intégrée.
- [x] Retenir le contour épais avec ombre extérieure centrée pour l’icône, puis intégrer
  les contours gauche/droite validés dans les menus. Conversion ICNS native avec son ombre.
- [x] Accepter le nom personnalisé `Splitergoll V2` dans l’app et le lecteur CLI,
  sans choisir entre plusieurs correspondances. Deux niveaux lus sur V2 sans USB : 89/79 %.
- [x] Confirmer l’affichage des batteries dans l’app installée après correction du nom sur V2.
- [x] Remplacer le symbole de batterie en charge par un petit éclair seul à 100 %,
  horizontal dans la barre et agrandi dans la colonne du pourcentage, sans déformation
  ni changement de largeur du menu. Garder la charge possible en infobulle.
- [ ] Amélioration : transmettre la présence USB par moitié pour distinguer USB
  branché et batterie pleine, au lieu du raccourci 100 % = éclair. Utile notamment
  avec une future mesure indépendante de la cellule. USB présent ne prouve pas
  une charge effective : batterie déconnectée sur OFF ou déjà pleine.
- [x] Faire évoluer `make app install` : préparer la copie, quitter normalement
  les copies concernées, remplacer puis lancer l’app installée, sans fermeture forcée.
- [ ] Valider le lecteur permanent : mises à jour, données indisponibles, déconnexions,
  autonomie, sommeil du Mac, ouverture de session et présentation honnête des mesures sous USB.
- [ ] Ensuite, ajouter couche active et OS utilisé par la keymap dans l’app.
  Définir leur transmission et celle de l’état USB par moitié, avec compatibilité
  des firmwares anciens. Ce protocole ne bloque pas le premier lot batteries.
- [ ] Ensuite, rendre configurables la présence dans la barre des menus, le Dock,
  le lancement à l’ouverture de session et les informations affichées. Le mode
  actuel garde la barre des menus, montre le Dock avec la fenêtre et demande le lancement automatique.
- [ ] Comparer la veille profonde de la centrale et son maintien en inactivité : reprise depuis la droite, première frappe, délai de reconnexion et impact sur l’autonomie, puis choisir le comportement.
- [ ] Tester branchement USB, débranchement, charge secteur et préférences sauvegardées.
- [ ] Vérifier l’outillage de build sur Windows/WSL.
- [ ] Promouvoir `docs/README.md` à la racine à la fin, adapter les liens et conserver les photos, conceptions et inspirations historiques.

## Photos publiques et historique Git

- [x] Renommer les six photos avec des noms descriptifs en kebab-case et corriger les liens.
- [x] Retirer les blocs annexes des six WebP sans réencodage. Les pixels et dimensions
  sont conservés. Aucun EXIF, XMP ou GPS trouvé dans les onze objets image audités
  dans les références locales, seuls les huit WebP avaient un profil couleur ICC.
- [ ] Préparer la purge des anciennes photos et de leurs métadonnées dans Git.
  Inventorier les anciens chemins, blobs, branches et tags concernés, puis décider
  ensemble du périmètre : les branches historiques doivent actuellement rester intactes.
  Un blob encore accessible dans un commit n’est pas orphelin. Après une réécriture
  validée, supprimer les objets devenus orphelins en tenant compte des reflogs.
  Traiter séparément GitHub, ses références/caches et les autres copies éventuelles,
  selon les [limites de purge GitHub](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/removing-sensitive-data-from-a-repository)
  et la [conservation des objets Git](https://git-scm.com/docs/git-gc#_notes).

## Idées

- [ ] Détecter l’OS de l’ordinateur connecté (Windows, macOS, Linux) et sélectionner automatiquement la keymap adaptée.
  L’objectif est une reconnaissance passive à la connexion USB ou Bluetooth, sans logiciel à installer,
  sans association manuelle entre ordinateur et OS, ni profil ou keymap à changer à l’usage.
  Évaluer les signatures réelles, la fiabilité et les limites en USB, Bluetooth direct et via dongle,
  y compris après reconnexion et réveil. Une sélection manuelle ne remplit pas cet objectif.
  Un programme compagnon reste une alternative à rediscuter si la reconnaissance passive ne suffit pas.
- [ ] Lorsque `auto` couvrira les usages et sera validé, retirer `multi` des profils actifs en le conservant comme archive autonome.
- [x] Conserver l’OS détecté dans `auto` après les modes pavé numérique et jeu. Couches compilées vérifiées, essai matériel encore à faire.
- [ ] Réexaminer les macros de retour de mode qui écrivent puis effacent du texte.
- [ ] Ajouter le scroll aux encodeurs et des informations de batterie/connexion.
- [ ] Mesurer l’autonomie et améliorer la veille et la puissance BLE.
- [x] Observer une progression après charge de V2 en USB + ON : relevés sans USB
  gauche 53 → 63 %, droite 57 → 67 %, selon l’utilisateur. Les logs montrent
  environ 5 V et 100 % sous USB, puis 3945 mV et 67 % à droite au débranchement.
  Cela ne mesure ni le courant ni la capacité. Le PCB coupe la recharge sur OFF.
- [ ] Définir un affichage batterie adapté à l’USB : la mesure VDDH de V2 reflète
  alors l’alimentation et donne un faux 100 %. Distinguer cet état d’une mesure
  de cellule, sans inventer de pourcentage ni supposer une autre entrée ADC câblée.
  Réveiller chaque moitié avant les relevés sur batterie, les mesures s’arrêtant
  en inactivité. Voir [le protocole batteries et veille](tests-batteries-veille.md).
- [ ] Si une mesure pendant la charge est souhaitée, identifier le circuit des
  répliques et une entrée de mesure indépendante. Le PCB actuel n’en prévoit pas.
  Évaluer pont diviseur ou jauge, sans modification matérielle implicite.
- [ ] Étudier l’arrêt du Bluetooth vers le Mac pendant l’usage USB.
- [ ] Étudier séparément un fonctionnement sans aucune radio et ses besoins matériels.
