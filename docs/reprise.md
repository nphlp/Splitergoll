# Reprendre le développement

## Point de départ

Le lot Bluetooth est publié dans `bd22c0e`, sur `unified-firmware`.
Les comportements principaux sont validés sur V1 avec le diagnostic à dix secondes :
choix manuel mémorisé immédiatement, persistance après coupure, retour du préféré,
repli, recherche sans résultat et reconnexion après perte totale sans boucle.
L’exclusion individuelle est retirée. Ne pas refaire toute cette série.

L’utilisateur confirme ensuite le flash des deux moitiés de V1 en **wire windows
normal**, gaming inclus, pour jouer. V2 conserve son dernier firmware connu.
À la reprise, vérifier quel matériel est disponible avant toute manipulation.

## 1. Vérifier les deux minutes et l’inactivité

**Premier chantier conseillé, sans changer les durées.** La fenêtre normale et
l’inactivité valent toutes deux 120 secondes, mais ne démarrent pas forcément ensemble.
IDLE laisse fonctionner les tâches différées, contrairement au sommeil profond.
Le code prévoit la fin de la fenêtre et sa sauvegarde même en IDLE, sans générer d’activité.
La vérification réelle de cette rencontre reste à faire.

Avec `bluetooth auto ble-trace` normal à 120 secondes :

- Vérifier la sauvegarde de reprise quand le préféré ne revient pas, puis sa
  persistance après redémarrage. Les choix manuels restent sauvegardés immédiatement.
- Faire terminer la fenêtre avant, pendant et après le passage en IDLE : pas de
  sauvegarde perdue, de réouverture, de bascule parasite ou de fausse activité.
- Distinguer la recherche continue sans hôte, qui s’arrête en IDLE, d’une fenêtre
  bornée déjà ouverte, qui peut y finir. Les lectures USB ne réveillent pas le clavier.
- Sur V2 uniquement, compléter la reprise après sommeil profond sur batterie,
  sans USB. Garder l’échéance de dix minutes, sans prolongation par les annonces.

Voir [la reprise Bluetooth](tests-reprise-bluetooth.md),
[la recherche](tests-recherche-bluetooth.md) et [la veille](veille.md).

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
séance commence au point 1, puis passe au cadrage des destinations. Aucun flash,
reset d’appairage ou service n’est lancé automatiquement à la reprise.
