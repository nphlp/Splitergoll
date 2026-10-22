# Tester le dernier appareil mémorisé

Utiliser le diagnostic à **10 secondes**, deux appairages conservés et les
[préparatifs](tests-connexions-bluetooth.md). Garder la gauche en USB au Mac.
Utiliser le clavier intégré du Mac pendant les coupures de son Bluetooth.
Les dix secondes commencent à la première connexion après démarrage, ou au raccourci.

## 1. Enregistrer le Mac

1. Connecter les deux ordinateurs, sélectionner le Mac avec le raccourci.
2. Attendre douze secondes depuis ce geste, puis `make ble profiles <port>`.
3. Attendre `saved=1 remembered=<slot Mac> restoring=0 error=0` dans `POLICY`.
   Le clic simple droit doit afficher macOS. Noter aussi `active` dans `BEGIN`.

`remembered` désigne un slot, pas un OS. Le diagnostic de mémoire d’OS reste distinct.

## 2. Windows arrive avant le Mac

1. Couper le Bluetooth du Mac. Windows doit devenir utilisable, sans raccourci.
2. Faire un reset simple de gauche, en gardant le Mac déconnecté.
3. Dès que Windows revient, taper quelques lettres : il doit fonctionner immédiatement.
4. Réactiver le Bluetooth du Mac avant la fin des dix secondes suivant cette connexion.
5. Le Mac doit reprendre la sélection et sa keymap, même après les frappes sur Windows.

La bascule attend toujours les relâchements nécessaires. Préparer les réglages
Bluetooth avant le reset pour ne pas dépasser les dix secondes.

## 3. Le Mac arrive avant Windows

Rejouer les deux étapes précédentes en inversant les ordinateurs : enregistrer
Windows, couper son Bluetooth, reset avec Mac disponible, puis réactiver Windows
dans la fenêtre. Windows doit reprendre la sélection. Cela teste les deux ordres.

## 4. Le précédent ne revient pas à temps

1. Enregistrer le Mac comme à l’étape 1, puis couper son Bluetooth et reset.
2. Laisser Windows revenir seul et attendre douze secondes après sa connexion.
3. `POLICY` doit maintenant donner `saved=1 remembered=<slot Windows> error=0`.
4. Réactiver le Mac sans geste : la fenêtre fermée empêche sa reconnexion.
5. Le raccourci doit permettre de le retrouver et de le sélectionner explicitement.

## 5. Reset avant la sauvegarde

1. Enregistrer le Mac comme à l’étape 1, avec les deux ordinateurs connectés.
2. Sélectionner Windows, puis reset avant dix secondes, sans attendre la fermeture.
3. Le choix durable reste le Mac : il doit reprendre la sélection à sa reconnexion.

Ce résultat est volontaire : le nouveau choix n’est durable qu’à la fin de la fenêtre.

## 6. Le choix manuel est prioritaire

Après un reset, avec les deux hôtes revenus pendant la fenêtre, sélectionner
explicitement Windows par le raccourci. Le clavier doit rester sur Windows, même
si le Mac était mémorisé avant le reset. Après douze secondes, vérifier que Windows
est mémorisé. Ne plus taper sur le Mac avec V1 pendant cette étape.

Pour chaque cas : résultat, destination réelle, OS au clic droit et sortie de
`make ble profiles <port>`. Une sortie lue n’ouvre pas de fenêtre.
