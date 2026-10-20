# Capturer le Bluetooth sur Windows

Ce guide est l’alternative avec USB branchés au PC Windows. Pour garder les USB
sur le Mac et y lire directement les échanges avec Windows, utiliser
[la capture depuis le Mac](capture-bluetooth-mac.md), sans transfert de script.

Le Mac et Windows doivent utiliser deux emplacements d’appairage distincts.
Ce sont des connexions mémorisées, pas des keymaps. La keymap du diagnostic reste
macOS sur les deux ordinateurs afin de comparer le même firmware.

## Préparer sur le Mac

Le diagnostic récent permet la bascule entre appareils connectés. Reflasher
uniquement la gauche avec le nouveau `ble-trace`, sans `settings_reset` :

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

1. Garder le Bluetooth des deux ordinateurs activé, sans oublier le clavier.
2. Brancher les deux USB de V1 au PC Windows pour son alimentation. La saisie
   reste en Bluetooth. Ne pas passer en bootloader.
3. Dans le Gestionnaire de périphériques, trouver le port série de gauche sous
   « Ports (COM et LPT) ». Utiliser son numéro réel dans les commandes.
4. Ouvrir PowerShell dans le dossier contenant `capture.ps1`, sans administrateur,
   puis sélectionner l’index déjà appairé à Windows, 1 dans nos essais :

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\capture.ps1 -Port COM4 -Profile 1
```

Vérifier `status=ok` et noter `selected` : c’est l’index destiné à Windows.
`busy` demande de relâcher les touches et d’attendre une seconde. Avec les anciens
firmwares, il signifiait qu’un hôte restait connecté. `full` indique une absence de case libre.
Dans ces cas, ne rien effacer. La commande ne remplace aucun appairage.
Ne pas relancer `free` pour revenir à une case déjà appairée : utiliser son index.

Si Windows est déjà appairé, attendre sa reconnexion ou reconnecter son entrée
`Splitergoll`, sans l’oublier. Pour un tout premier appairage uniquement, sélectionner
`-Profile free` et ajouter le clavier dans les réglages Bluetooth Windows.
Attendre dix secondes après sa connexion, sans taper, puis lire :

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\capture.ps1 -Port COM4 -Ble
```

Transmettre le fichier `windows-ble-….txt` créé dans le dossier courant, ainsi que
la réponse `windows-ble-profile-….txt` si la sélection ou la connexion échoue.
Ne pas supprimer une entrée Windows préexistante en échec sans l’examiner d’abord.

Le premier essai sur V1 a conservé le Mac en case 0 et appairé Windows en case 1.
Pour une nouvelle connexion Windows, utiliser `-Profile 1` à la place de
`-Profile free`, et reconnecter le clavier si nécessaire, sans couper le Bluetooth.
Attendre dix secondes et refaire la capture `-Ble`, sans nouveau flash ni oubli
d’appareil. Cette capture montrera les lectures lorsque Windows connaît déjà le clavier.

## Revenir au Mac

Rebrancher l’USB de gauche au Mac, garder les deux Bluetooth actifs, puis sélectionner
l’index Mac noté auparavant :

```sh
make ble-profile /dev/cu.usbmodem1101 0
```

Remplacer `0` par l’index réellement noté.
La connexion existante doit reprendre sans nouvel appairage.

Si le Mac affiche « Connecté » sans recevoir les touches, lire `make ble-profile
<port> status` : son emplacement peut être connecté alors qu’un autre reste actif
pour la saisie. Sélectionner son index, sans couper le Bluetooth. Une connexion
affichée ne suffit pas à sélectionner la destination des touches. Aucun reset des appairages n’est nécessaire.

Le sélecteur utilise l’API native de ZMK, qui sauvegarde l’index actif après
60 secondes sous tension. Une coupure plus tôt peut restaurer l’ancien index,
sans effacer les appairages. `status` permet de le vérifier et un index explicite
de le corriger. La commande ne lance aucun flash, reset ou effacement de clés.
