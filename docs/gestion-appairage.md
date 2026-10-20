# Ouvrir et fermer l’appairage

**Proposition en discussion, pas encore implémentée.** L’appairage crée une nouvelle
association. La reconnexion réutilise une association existante, sans la refaire.
Le moteur de décision est développé et testé sans matériel. Les annonces, l’admission
et les raccourcis du firmware ne l’utilisent pas encore, aucun nouveau flash n’est prêt.

## Fenêtres de dix secondes

Trois événements ouvrent les annonces et l’appairage pendant **10 secondes** :

1. Le démarrage du clavier, y compris après reset.
2. Le retour du Gaming USB à la base Windows.
3. L1 + clic encodeur droit. Depuis Gaming, ce geste revient d’abord à Windows.

Pendant la fenêtre, les ordinateurs connus peuvent se reconnecter et de nouveaux
appareils peuvent s’appairer sur un slot libre. Tous les connus connectés n’impliquent
pas une fermeture anticipée si un nouvel appareil peut encore être ajouté.

Après la fenêtre, les annonces et les nouveaux appairages se ferment, les connexions
établies restent présentes. Le même raccourci peut rouvrir la fenêtre plus tard.

## Effet du raccourci

Le nombre de destinations disponibles **au moment du geste** détermine son effet :

| Destinations | Action immédiate | Si un autre appareil devient prêt pendant les 10 secondes |
| --- | --- | --- |
| Une | Continuer à l’utiliser et ouvrir la recherche | Basculer sur le premier autre appareil prêt |
| Plusieurs | Sélectionner la suivante et ouvrir la fenêtre | Garder le choix qui vient d’être fait |
| Aucune | Proposition : ouvrir la recherche | Sélectionner le premier appareil prêt |

USB et BLE du même ordinateur ne comptent qu’une fois, selon les
[règles de destination](destinations-saisie.md). Un appareil connu peut se reconnecter
sans nouvel appairage, un nouveau doit finir son association et avoir une keymap utilisable.

Pour éviter les bascules successives, la recherche doit sélectionner au plus un nouvel
appareil. Sans arrivée utilisable, le choix reste inchangé. Chaque nouveau geste
relance dix secondes et remplace l’intention précédente, sans suspendre la saisie.
Une cible prête retenue avant l’échéance peut attendre le relâchement des touches
après celle-ci. Les nouvelles arrivées sont alors refusées, les connexions conservées.

## Gaming USB

L’entrée en Gaming USB ferme la fenêtre en cours, arrête les annonces vers les
ordinateurs et déconnecte le Mac en BLE. Les appairages et OS mémorisés sont conservés.
La liaison BLE entre les deux moitiés reste nécessaire.

Au retour à Windows USB, la nouvelle fenêtre de 10 secondes laisse revenir le Mac.
Windows reste utilisable en USB et sélectionné, la reconnexion du Mac ne prend pas
la destination. Le raccourci permet ensuite de le sélectionner. Si le retour vient
de L1 + clic encodeur droit, il porte au contraire la demande explicite de recherche
et de bascule vers le premier autre appareil prêt.

Ce choix vise à réduire l’activité radio vers les ordinateurs pendant le jeu.
Son bénéfice sur la latence n’est pas mesuré. Ces règles concernent le futur mode
mixte avec Gaming USB, pas une coupure de la seule sortie d’un clavier en Bluetooth.

## Décisions restantes

- Définir la fin d’un appairage commencé avant la fermeture de la fenêtre.
- Prévoir le cas où tous les slots sont occupés, sans effacement automatique.
- Préciser si Gaming USB déconnecte aussi Windows BLE, en plus du Mac.

L1 + clic encodeur droit réunit le choix de destination et l’ouverture de la fenêtre.
Le diagnostic USB permet déjà de choisir un slot libre, mais ne constitue pas encore
cette gestion d’ouverture/fermeture. La [procédure de réparation](appairage-bluetooth.md)
reste distincte, aucun effacement des réglages n’est nécessaire pour ajouter un appareil.
