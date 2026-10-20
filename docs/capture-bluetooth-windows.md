# Capturer le Bluetooth sur Windows

Ce guide est l’alternative avec USB branchés au PC Windows. Pour garder les USB
sur le Mac et y lire directement les échanges avec Windows, utiliser
[la capture depuis le Mac](capture-bluetooth-mac.md), sans transfert de script.

Le Mac et Windows doivent utiliser deux emplacements d’appairage distincts.
Ce sont des connexions mémorisées, pas des keymaps. La keymap du diagnostic reste
macOS sur les deux ordinateurs afin de comparer le même firmware.

## Préparer sur le Mac

Le diagnostic initial n’avait pas de sélecteur d’emplacement. Reflasher uniquement
la gauche avec le nouveau `ble-trace`, sans `settings_reset` :

```sh
make flash bluetooth macos left ble-trace
```

Après redémarrage, repérer son port série et lire les emplacements :

```sh
make ble-profile /dev/cu.usbmodem1101 status
```

Adapter le port si nécessaire. Noter la case `connected=1` du Mac, pas seulement
`active` : un appareil peut être connecté sans être sélectionné pour la saisie.
`paired=1` signifie appairé. Aucune adresse ni clé n’est affichée.
Copier le fichier [capture.ps1](../diagnostics/os-signature/capture.ps1) mis à jour
dans un dossier de travail du PC, par exemple Téléchargements. Il fonctionne seul.

## Connecter Windows

1. Couper temporairement le Bluetooth du Mac, sans oublier le clavier.
2. Brancher les deux USB de V1 au PC Windows pour son alimentation. La saisie
   reste en Bluetooth. Ne pas passer en bootloader.
3. Dans le Gestionnaire de périphériques, trouver le port série de gauche sous
   « Ports (COM et LPT) ». Utiliser son numéro réel dans les commandes.
4. Ouvrir PowerShell dans le dossier contenant `capture.ps1`, sans administrateur,
   puis choisir un emplacement libre :

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\capture.ps1 -Port COM4 -Profile free
```

Vérifier `status=ok` et noter `selected` : c’est l’index destiné à Windows.
`busy` signifie qu’un hôte reste connecté, `full` qu’aucune case n’est libre.
Dans ces cas, ne rien effacer. La commande ne remplace aucun appairage.
Ne pas relancer `free` pour revenir à une case déjà appairée : utiliser son index.

Dans les paramètres Bluetooth Windows, ajouter un appareil Bluetooth et choisir
`Splitergoll`. Attendre dix secondes après sa connexion, sans taper, puis lire :

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\capture.ps1 -Port COM4 -Ble
```

Transmettre le fichier `windows-ble-….txt` créé dans le dossier courant, ainsi que
la réponse `windows-ble-profile-….txt` si la sélection ou la connexion échoue.
Ne pas supprimer une entrée Windows préexistante en échec sans l’examiner d’abord.

Le premier essai sur V1 a conservé le Mac en case 0 et appairé Windows en case 1.
Pour une nouvelle connexion Windows, utiliser `-Profile 1` à la place de
`-Profile free`, puis réactiver le Bluetooth du PC et reconnecter le clavier.
Attendre dix secondes et refaire la capture `-Ble`, sans nouveau flash ni oubli
d’appareil. Cette capture montrera les lectures lorsque Windows connaît déjà le clavier.

## Revenir au Mac

Couper le Bluetooth du PC ou déconnecter le clavier de Windows. Rebrancher ses USB
au Mac, Bluetooth du Mac encore coupé, puis sélectionner l’index Mac noté auparavant :

```sh
make ble-profile /dev/cu.usbmodem1101 0
```

Remplacer `0` par l’index réellement noté, puis réactiver le Bluetooth du Mac.
La connexion existante doit reprendre sans nouvel appairage.

Si le Mac affiche « Connecté » sans recevoir les touches, lire `make ble-profile
<port> status` : son emplacement peut être connecté alors qu’un autre reste actif
pour la saisie. Couper temporairement le Bluetooth du Mac, sélectionner son index,
puis réactiver le Bluetooth. Une connexion affichée ne suffit pas à sélectionner
la destination des touches. Aucun reset des appairages n’est nécessaire.

Le sélecteur utilise l’API native de ZMK, qui sauvegarde l’index actif après
60 secondes sous tension. Une coupure plus tôt peut restaurer l’ancien index,
sans effacer les appairages. `status` permet de le vérifier et un index explicite
de le corriger. La commande ne lance aucun flash, reset ou effacement de clés.
