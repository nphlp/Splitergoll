# Tester la fenêtre Bluetooth

Sur V1, garder les deux USB pour l’alimentation et la gauche au Mac pour le
diagnostic. La saisie reste en Bluetooth. Utiliser le clavier intégré du Mac
pendant le flash et les changements de destination. Les essais Windows attendent
que le PC soit disponible.

## Premier appairage sur le Mac

1. Ouvrir les réglages Bluetooth du Mac avant de flasher la nouvelle gauche :

   ```sh
   make flash bluetooth auto left ble-trace
   ```

2. Après son redémarrage, choisir Splitergoll dans les nouveaux appareils pendant
   les dix secondes. Si la fenêtre est manquée, un reset bref de gauche la rouvre.
   Un double reset sert au bootloader, pas à cette réouverture.
3. Tester les deux moitiés et l’affichage macOS au clic simple de l’encodeur droit.
4. Lire `make ble profiles <port>` et `make ble memory <port>` : connexion présente,
   `stored=macos` et `state=saved`. Aucun délai fixe à attendre avant la lecture.

Les slots ont déjà été effacés par l’utilisateur pour cette séance. Ne pas les
ré-effacer et ne pas copier de `settings_reset`. Le numéro attribué au Mac dépend
du slot vide sélectionné, ne pas supposer qu’il est toujours zéro.

## Ajouter Windows puis changer de destination

Quand le PC est disponible, ouvrir ses réglages Bluetooth. Depuis le Mac, rouvrir
la fenêtre avec L1 + clic encodeur droit et appairer Splitergoll sur Windows.
Le Mac doit rester la destination, le nouvel appareil reçoit un autre slot libre.
Lire le statut et la mémoire depuis le Mac : `stored=windows` et `state=saved`.
La présence des deux hôtes connectés et des deux OS mémorisés est attendue.

Refaire le raccourci, tout relâcher, puis vérifier Windows au clic simple et tester
la saisie. Refaire le geste pour revenir au Mac. Les lectures USB ne changent pas
la destination, une commande `ble select` le fait explicitement.

## Reconnexion et fermeture

- Après reset de gauche, laisser Mac et Windows disponibles et vérifier leur retour
  pendant la fenêtre, sans nouvel appairage ni effacement. Tester ensuite les deux destinations.
- Après fermeture, un appareil déconnecté doit attendre une réouverture pour revenir.
  Le raccourci rouvre dix secondes et conserve les connexions déjà établies.
- Un appairage commencé avant fermeture peut finir sous trente secondes depuis son
  début. Le statut `WINDOW` décrit seulement les dix secondes d’accueil.

Ces cas restent à valider sur le matériel. Les tests logiciels couvrent aussi places
pleines, demandes concurrentes, limites exactes et erreurs de sauvegarde. Une capture
USB ne mesure ni la latence complète ni l’autonomie.
