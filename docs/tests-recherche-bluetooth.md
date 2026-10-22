# Tester la recherche sans ordinateur

Utiliser le diagnostic à **10 secondes** et les
[préparatifs](tests-connexions-bluetooth.md). La veille légère reste à deux minutes,
la profonde à dix minutes. Garder les USB pour ces essais sur V1.
Une lecture USB ne compte pas comme une frappe.

## 1. Chercher au-delà de dix secondes

1. Avec les deux hôtes connectés, sélectionner le Mac et vérifier `remembered=<slot Mac>`
   après la bascule, puis couper les Bluetooth des deux ordinateurs.
2. Faire un reset simple de gauche et démarrer un chrono, sans toucher V1.
3. Après quinze secondes, lire `make ble profiles <port>` : attendre
   `POLICY active=1 without_host=1` et `WINDOW advertising=1 error=0`.
4. Réactiver le Mac : il doit se reconnecter et devenir utilisable sans raccourci.
5. Réactiver Windows dans les dix secondes suivantes : il rejoint sans prendre
   la sélection. Le Mac était aussi la préférence enregistrée.

Sans hôte, `remaining_ms=10000` indique une admission continue, pas dix secondes
restantes avant fermeture. La première connexion démarre le compte à rebours.

## 2. La veille légère ferme la recherche

1. Couper de nouveau les deux Bluetooth, puis reset de gauche et chrono.
2. Ne toucher aucune touche ni encodeur pendant **125 secondes**.
3. Lire le statut : attendre `active=0 without_host=0`, `remaining_ms=0 advertising=0`.
4. Appuyer puis relâcher une touche gauche, attendre une seconde et relire :
   `active=1 without_host=1 advertising=1` attendus.
5. Réactiver un ordinateur : il doit revenir sans reset ni raccourci.

## 3. Une perte partielle ne rouvre rien

Connecter les deux hôtes, attendre la fermeture, puis couper seulement le Bluetooth
de celui qui n’est pas sélectionné. Le réactiver : il ne doit pas revenir tout seul.
Un seul raccourci doit rouvrir la fenêtre, le reconnecter et le sélectionner.
Pendant une fenêtre encore ouverte, son retour est autorisé. Il n’y a pas de liste
d’exclusion par appareil. Rejouer en inversant les OS si nécessaire.

Le sommeil profond reste empêché par les USB de V1. Sa reprise se vérifiera
séparément sur V2, sur batterie. Ne pas assimiler l’essai de veille légère à ce test.
