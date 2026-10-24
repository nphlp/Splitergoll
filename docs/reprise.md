# Reprendre le développement

## Point de départ

**Prochaine étape : l’identité commune USB/Bluetooth aidée par les applications.**
Les compagnons Mac et Windows existent. Cadrer un diagnostic qui leur fait annoncer
la même identité locale sur les deux liaisons, sans changer la saisie pour ce premier
lot. Le [guide d’identité](identite-ordinateur.md) garde les contraintes et limites.
Le clavier reste utilisable sans app, aucune fusion par simple égalité d’OS.

La base de l’[app Windows](application-windows.md) est jugée satisfaisante après
les corrections. La largeur passe de 340 à 255 unités logiques à la demande de
l’utilisateur, sans nouvelle validation visuelle de cette largeur réduite. Les
cartes avec les silhouettes des deux moitiés sont au TODO. La lecture réelle des
batteries de V2 et le menu contextuel sont confirmés. Aucun nouveau flash requis.

Le lot Bluetooth est publié dans `bd22c0e`, sur `unified-firmware`.
Les comportements principaux sont validés sur V1 avec le diagnostic à dix secondes :
choix manuel mémorisé immédiatement, persistance après coupure, retour du préféré,
repli, recherche sans résultat et reconnexion après perte totale sans boucle.
L’exclusion individuelle est retirée. Ne pas refaire toute cette série.

L’utilisateur confirme ensuite le flash des deux moitiés de V1 en **wire windows
normal**, gaming inclus, pour jouer. Après le diagnostic `power-journal`,
l’utilisateur annonce le retour de V2 au firmware de base.
À la reprise, vérifier quel matériel est disponible avant toute manipulation.

## 1. Automatisation terminée, complément V2

Les essais courts de V1 sont positifs : sauvegarde manuelle, checkpoint de reprise
au voisinage d’IDLE et persistance après coupure. **IDLE fonctionne sous USB, seul
SLEEP est bloqué.** Ces essais sont utiles, sans prouver le comportement sur batterie.
La répétition dédiée à deux minutes est abandonnée à la demande de l’utilisateur.

La [suite automatisée](tests-automatises.md) est renforcée : 24 suites C natives,
112 cas de protocole, six tests Zephyr/QEMU et les 44 tests de l’app passent.
Les différents ordres de checkpoint/IDLE ne demandent plus une répétition manuelle.
Cela ne simule ni le câblage, ni la radio du vrai V2, ni son réveil GPIO.

**Le complément sur batterie de V2 est positif.** Après le nouvel appairage
réalisé par l’utilisateur, le Mac est seul au slot 0, son OS et sa préférence
sont sauvegardés sans erreur. Le journal lu après l’essai conserve le même
identifiant de démarrage et ne présente aucune perte.

Sur batterie, la fenêtre se ferme après dix secondes, puis la gauche passe en
IDLE environ 0,6 seconde plus tard. Le rebranchement USB ne provoque pas de retour
à ACTIVE. Le Mac reste connecté et mémorisé. La préférence étant déjà identique,
aucune nouvelle écriture NVS n’était attendue. Ce test ne mesure pas la consommation,
le sommeil profond ou l’état IDLE de la droite, qui n’a pas son propre journal.

Le diagnostic reste à dix secondes pour recherche/IDLE, dix minutes pour SLEEP.
La paire `bluetooth auto` normale est compilée et contrôlée. L’utilisateur annonce
avoir reflashé le firmware de base. Après redémarrage de l’app et retrait de V1,
il constate une actualisation des batteries de V2 à cinq secondes. La cause de
l’ancien affichage n’est pas établie. Aucune nouvelle série d’attentes n’est demandée.

**Tests/journal publiés dans `9325f07`, puis wire et app dans `24babd6`.** Le wire strict bloque
maintenant les annonces hôtes de la centrale et conserve le BLE split, les keymaps
et les bonds. Les contrôles logiciels passent et l’utilisateur confirme l’essai
court sur V1. Aucun relevé radio supplémentaire n’est déduit de ce retour.

Le [sélecteur V1/V2 de l’app](selection-clavier-app.md) est développé : choix
explicite mémorisé, nom visible et mesures isolées. Tests et build signé passent. L’utilisateur confirme l’installation et la sélection
de V2 seul dans le menu. Les essais de persistance, bascule V1/V2 et absence du
préféré sont reportés à sa demande. V1 reste en wire sur Windows.
La priorité revient au cadrage de l’identité commune USB/BLE, puis Gaming.
Aucun nouveau flash n’est nécessaire avant d’avoir défini le diagnostic utile.

USB + OFF et USB + ON alimentent tous deux le contrôleur, seul ON permet la charge
avec le câblage actuel. Ce réglage reste déclaré par l’utilisateur, aucun GPIO ne
le détecte. Le sommeil profond court reste facultatif, pour une question de réveil
précise. Voir [le journal](diagnostic-activite.md) et [les essais batteries/veille](tests-batteries-veille.md).

Le ressenti de veille trop rapide sur V2 motive une nouvelle compilation normale,
sans supposer quel ancien UF2 tourne encore. La paire
`bluetooth-auto-2026-10-24-13-57-26` est recompilée et contrôlée : IDLE 120 000 ms,
SLEEP 600 000 ms et batteries 5 s sur les deux moitiés, sans diagnostic ou reset des
réglages. UF2 et SHA-256 valides. L’utilisateur confirme ensuite le flash des
deux moitiés de V2. Les délais compilés sont contrôlés, sans nouveau test
chronométré sur batterie après ce flash. Tous les changements alors en attente
sont committés et poussés dans `24babd6`. La reprise USB/BLE est une nouvelle étude,
sans nouveau firmware mixte ou flash à effectuer à ce stade.

## 2. Réunir USB et Bluetooth par ordinateur

**À concevoir puis développer.** Cas de référence : Windows relié à la gauche en
USB et aussi connecté en BLE, Mac connecté en BLE. Le raccourci parcourt seulement :

| Destination | Liaison utilisée |
| --- | --- |
| PC Windows | USB prioritaire, sans destination Windows BLE en double |
| Mac | Bluetooth |

Il faut établir l’identité commune USB/BLE. Reconnaître Windows ne suffit pas à
identifier un ordinateur : deux PC Windows doivent rester distincts. L’objectif
est automatique, sans programme à installer sur chaque hôte, faisabilité à vérifier.
La recherche Bluetooth continue de suivre sa fenêtre, même pendant l’usage USB.
Définir ensuite les effets d’un branchement/retrait : priorité de transport,
changement éventuel de sélection, repli et alimentation par simple chargeur.
Voir [les destinations de saisie](destinations-saisie.md).

## 3. Gaming USB et mémorisation du mode

**À développer après les destinations.** Entrer en Gaming sélectionne et mémorise
Windows USB, ferme immédiatement la recherche et réduit les liaisons BLE hôtes.
La liaison entre les deux moitiés reste nécessaire.

Nouvelle demande : après coupure, redémarrage ou rebranchement au même Windows USB,
retrouver Gaming si c’était le mode choisi, plutôt que revenir à Windows normal.
Les cas sans USB et la sortie volontaire restent à préciser dans
[le cadrage Gaming](gaming-usb.md). Aucun effacement d’appairage n’est prévu.

## Ensuite, par thème

| Sujet | Reste à faire |
| --- | --- |
| Bluetooth et OS | Nouveaux appairages et cas limites, correction manuelle persistante de l’OS, mesure du délai de bascule |
| Linux et dongle | Captures Linux avant classification, réintégration du XIAO BLE au build |
| Keymaps | Accès à `ü`, essais restants des profils et gestes, devenir de `multi` comme archive |
| Batterie et veille | Première frappe, réveil par touches/encodeurs, autonomie, compromis de veille de la centrale |
| App macOS | Reprise après sommeil/session, futur état USB réel, OS/couche et réglages |
| Outillage et socle | Migration KSCAN vers input, warning tap-dance, drapeau Docker, build/flash Windows et commande bootloader USB |
| Dépôt | Finaliser le README, préparer la purge historique des anciennes photos avec métadonnées |

Le [TODO](TODO.md) conserve le détail complet et les validations. La prochaine
étape est le cadrage des destinations USB/BLE. Les essais complémentaires de l’app sont reportés. Aucun flash,
reset d’appairage ou service n’est lancé automatiquement à la reprise.
