# Ouvrir et fermer l’appairage

**Règles cibles, partiellement implémentées.** L’appairage crée une nouvelle
association. La reconnexion réutilise une association existante, sans la refaire.
Le firmware traite les [reconnexions et nouveaux appairages](reconnexion-bluetooth.md)
pendant trente secondes, au démarrage et avec le raccourci. La recherche avec bascule
sur une arrivée est confirmée sur V1 pour la reconnexion Windows, avec le diagnostic
à dix secondes. Gaming USB reste à développer.

## Fenêtres de trente secondes

Trois événements ouvrent les annonces et l’appairage pendant **30 secondes** :

1. Le démarrage du clavier, y compris après reset.
2. Le retour du Gaming USB à la base Windows.
3. L1 + clic encodeur droit. Depuis Gaming, ce geste revient d’abord à Windows.

Pendant la fenêtre, les ordinateurs connus peuvent se reconnecter et de nouveaux
appareils peuvent s’appairer sur un slot libre. Tous les connus connectés n’impliquent
pas une fermeture anticipée si un nouvel appareil peut encore être ajouté.

Après la fenêtre, les annonces et les nouveaux appairages se ferment, les connexions
établies restent présentes. Le même raccourci peut rouvrir la fenêtre plus tard.
La saisie ne prolonge pas la fenêtre. Si un appareil se déconnecte alors qu’un autre
reste connecté, cela ne rouvre pas la recherche.

## Sans ordinateur connecté, évolution prévue

Tant que le clavier est actif et sans hôte connecté, rechercher les connus et
accueillir de nouveaux appairages sans limite de trente secondes. À l’inactivité
après deux minutes, fermer cette recherche. Elle ne doit pas retarder la veille
profonde à dix minutes, qui reste empêchée par l’alimentation USB.

Ce comportement n’est pas implémenté. La veille légère de ZMK ne ferme pas à elle
seule les annonces : il faudra raccorder explicitement la politique radio à cet état.
Cette règle inclut la perte du dernier hôte : tant qu’aucun ne reste connecté, la
recherche reprend si le clavier est actif. Au retour du premier, une fenêtre
ordinaire laisse rejoindre les suivants. Une perte partielle ne la rouvre pas.

## Terminer un appairage

Un appairage commencé pendant ces trente secondes pourra se terminer ensuite, dans
la limite de **30 secondes depuis son début**. Cette marge ne rouvre pas l’accueil
aux autres appareils. Plusieurs appairages peuvent se succéder si leur démarrage
reste dans la fenêtre. Cette extension est implémentée, sa validation matérielle reste à faire.

Si tous les slots sont occupés, seuls les connus pourront revenir. Aucun appareil
enregistré ne sera effacé automatiquement pour faire de la place.

## Effet du raccourci

Le nombre de destinations disponibles **au moment du geste** détermine son effet :

| Destinations | Action immédiate | Si un autre appareil devient prêt pendant les 30 secondes |
| --- | --- | --- |
| Une | Continuer à l’utiliser et ouvrir la recherche | Basculer sur le premier autre appareil prêt |
| Plusieurs | Sélectionner la suivante et ouvrir la fenêtre | Garder le choix qui vient d’être fait |
| Aucune | Ouvrir la recherche | Sélectionner le premier appareil prêt |

USB et BLE du même ordinateur ne comptent qu’une fois, selon les
[règles de destination](destinations-saisie.md). Un appareil connu peut se reconnecter
sans nouvel appairage, un nouveau doit finir son association et avoir une keymap utilisable.

Pour éviter les bascules successives, la recherche doit sélectionner au plus un nouvel
appareil. Sans arrivée utilisable, le choix reste inchangé. Chaque nouveau geste
relance trente secondes et remplace l’intention précédente, sans suspendre la saisie.
Une cible prête retenue avant l’échéance peut attendre le relâchement des touches
après celle-ci. Les nouvelles arrivées sont alors refusées, les connexions conservées.

## Gaming USB

L’entrée en Gaming USB ferme la fenêtre en cours, arrête les annonces vers les
ordinateurs et déconnecte le Mac en BLE. Les appairages et OS mémorisés sont conservés.
La liaison BLE entre les deux moitiés reste nécessaire.

Au retour à Windows USB, la nouvelle fenêtre de 30 secondes laisse revenir le Mac.
Windows reste utilisable en USB et sélectionné, la reconnexion du Mac ne prend pas
la destination. Le raccourci permet ensuite de le sélectionner. Si le retour vient
de L1 + clic encodeur droit, il porte au contraire la demande explicite de recherche
et de bascule vers le premier autre appareil prêt.

Ce choix vise à réduire l’activité radio vers les ordinateurs pendant le jeu.
Son bénéfice sur la latence n’est pas mesuré. Ces règles concernent le futur mode
mixte avec Gaming USB, pas une coupure de la seule sortie d’un clavier en Bluetooth.

## Décisions restantes

- Rendre visible le manque de place lorsque tous les slots sont occupés.
- Préciser si Gaming USB déconnecte aussi Windows BLE, en plus du Mac.

L1 + clic encodeur droit réunit déjà le choix d’un hôte connecté et l’ouverture
complète de la fenêtre. La bascule sur la première arrivée est raccordée si le geste
a été fait avec au plus un appareil prêt, confirmée pour un connu sur V1. L’essai
d’un nouveau bond reste distinct. La réservation d’un slot d’accueil est indépendante
de la destination des frappes. La [procédure de réparation](appairage-bluetooth.md)
reste distincte, aucun effacement des réglages n’est nécessaire pour ajouter un appareil.
