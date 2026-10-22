# Tester la mémoire d’OS Bluetooth

But : vérifier détection initiale, sauvegarde puis reconnexion sur Mac et Windows,
avec un seul nouveau flash gauche. La droite et la keymap macOS restent inchangées.
Garder l’USB gauche au Mac pour toutes les lectures, même pendant l’appairage Windows.

Le nouvel `bluetooth auto` apprend aussi directement l’OS pendant sa fenêtre
d’accueil. Pour ce parcours, suivre les [essais de connexion](tests-connexions-bluetooth.md).
Le diagnostic fixe ci-dessous reste utile pour étudier la mémoire séparément.

## Préparation

```sh
make flash bluetooth macos left ble-trace
make ble profiles /dev/cu.usbmodem1101
make ble memory /dev/cu.usbmodem1101
```

Pas de `settings_reset`. Les appairages créés avant l’ajout de la mémoire restent
utilisables mais n’ont pas encore d’OS enregistré. Vérifier la saisie des deux moitiés.

Pour reprendre après le correctif Windows, conserver l’appairage Mac déjà validé.
Après le flash gauche, vérifier que `stored=macos` est retrouvé. Refaire ensuite
uniquement l’appairage Windows : son ancien résultat inconnu reste inchangé tant
que l’on ne reprend pas cette acquisition initiale.

## Refaire un seul appairage

**À réaliser ensemble après accord sur l’emplacement à effacer.**
Le PC doit être disponible pour l’essai Windows. Utiliser le clavier intégré du
Mac pendant la réassociation, car V1 peut être momentanément déconnecté.
Les index actuels sont Mac 0 et Windows 1, à vérifier dans le statut.

1. Sélectionner l’emplacement testé avec `make ble select <port> <index>`.
2. Lancer `make ble forget <port> <index>`, vérifier l’index et confirmer avec `y`.
   La commande refuse un autre emplacement que celui sélectionné et toute saisie en cours.
   Elle efface un seul bond, jamais tous les appairages.
3. Oublier uniquement Splitergoll dans les réglages Bluetooth de l’ordinateur testé,
   puis l’appairer de nouveau au même emplacement.
4. Après connexion, lire `ble capture`, puis `ble memory`.
   Résultat attendu : le bon OS, `origin=initial` et `state=saved`.
5. Déconnecter/reconnecter ce seul hôte sans l’oublier. Relire la mémoire : le même
   `stored` doit être retrouvé, sans nouvel apprentissage.

Faire ce parcours sur le Mac puis Windows. Aucun script ne doit être transféré au PC.
La keymap reste macOS pendant ces essais : ne pas utiliser la saisie Windows comme
validation de son layout. Revenir au Mac par sélection explicite à la fin.

Aucun délai fixe n’est imposé avant lecture. Le résultat initial peut encore évoluer
pendant la découverte des services. Les dix secondes utilisées pour les premières
captures étaient une marge de comparaison, pas un délai de détection.
Taper n’invalide pas le test de mémoire, mais envoie les touches à l’hôte sélectionné.

## Redémarrage

Après lecture des captures, faire un seul reset bref de gauche, sans passer en
bootloader. Relire le statut et la mémoire après reconnexion : les deux OS doivent
être conservés. Un emplacement non reconnecté peut rester `unverified`.
S’il reste déconnecté, sélectionner explicitement cet emplacement avec `ble select`,
sans taper pendant le test, puis relire la mémoire après reconnexion. Revenir ensuite
à l’ordinateur voulu. Cette sélection ne nécessite aucune manipulation sur le PC.
Si `error`, `ambiguous` ou `dropped=1` apparaît, garder les captures et diagnostiquer
avant de répéter un effacement. Ces essais ne valident pas d’autres ordinateurs.
