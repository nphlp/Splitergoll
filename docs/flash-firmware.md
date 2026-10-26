# Flash du firmware

Les commandes de build produisent les UF2, elles ne flashent pas le clavier.
Choisir le profil voulu et utiliser les deux fichiers du même dossier de build.
Le nom indique désormais le matériel et la keymap, avec `-debug` si demandé,
par exemple `splitergoll-wire-macos-right.uf2`.

Le [diagnostic d’identité](diagnostic-identite.md) se choisit explicitement avec
`make flash bluetooth auto left identity`. Seule la gauche ajoute cette fonction,
la droite est identique au firmware normal. Aucun reset d’appairage nécessaire.

## Depuis le terminal (macOS)

Mettre uniquement la moitié à flasher en bootloader avec un double appui sur reset,
puis lancer par exemple :

```sh
make flash wire macos right
make flash wire macos left
```

Le profil expérimental `make flash wire auto left usb-trace` suit les mêmes gardes,
voir [la keymap automatique](keymap-auto.md).

Chaque commande choisit le dernier build du matériel et de la keymap demandés.
Elle présente le matériel, la keymap, le côté puis les logs USB, comme dans la commande.
La date utilise le mois en français et une heure comme `11 h 50 min 08 s`.
L’ancienneté suit sur une deuxième ligne. Une ligne vide précède le fichier
relatif au dépôt et le bootloader. Le SHA-256 est contrôlé sans être affiché.
La confirmation `Continue? [y/N]` réagit à une seule touche, sans Entrée :
`y` ou `Y` confirme, `n`, Entrée ou toute autre touche annule. Sans build correspondant,
elle s’arrête avec un message explicite. `make flash` affiche l’aide.

Ajouter `debug` en dernier pour choisir un build avec logs USB, par exemple
`make flash bluetooth multi right debug`. Un build normal ne prend jamais un build
debug à sa place. Pour choisir un fichier précis :

```sh
make flash dist/firmware/wire-macos-AAAA-MM-JJ-HH-MM-SS/splitergoll-wire-macos-right.uf2
```

Le fichier doit rester dans son dossier de build, avec `build-info.txt` et
`SHA256SUMS`. La commande contrôle ces métadonnées, l’empreinte et le format UF2.
La date vient du nom de ce dossier, dans le fuseau local utilisé pour le build.
Elle refuse plusieurs bootloaders compatibles. Le disque ne permet pas de
distinguer physiquement gauche et droite, il faut donc préparer la moitié indiquée.

La copie évite les métadonnées du Finder et ne réinitialise pas les appairages.
« Copie terminée » confirme la copie, vérifier ensuite la saisie du clavier.
Le flash automatique et le fonctionnement des deux moitiés sont confirmés sur le
Mac avec `wire macos`. Windows/WSL et Linux utilisent pour le moment la copie
manuelle ci-dessous.

## Copier les UF2

1. Brancher la moitié droite en USB et faire un double appui sur son bouton reset.
2. Copier `splitergoll-<profil>-right.uf2` sur le disque `NICENANO`.
3. Après son redémarrage automatique, faire la même chose à gauche avec `splitergoll-<profil>-left.uf2`.

Un appui simple sur reset redémarre le contrôleur. Un double appui ouvre le
bootloader pour le flash. Un flash normal conserve les réglages d’appairage.
Lors d’un changement de centrale ou d’un blocage Bluetooth, suivre d’abord
[la procédure d’appairage](appairage-bluetooth.md).

## Messages du Finder

Le disque peut disparaître automatiquement à la fin de la copie pour redémarrer
le clavier. Cela peut provoquer « Erreur d’éjection » ou le code `-36` du Finder,
même si le flash a réussi. Ce message ne suffit pas à conclure à un échec.
Le contenu de `CURRENT.UF2` peut être comparé au build attendu dans le bootloader.
[Erreurs documentées par ZMK](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/troubleshooting/flashing-issues.md).

## Touche bootloader

Sur la keymap actuelle : maintenir **L1 + L2**, puis cliquer sur **l’encodeur gauche**
pour passer la gauche en bootloader. Garder son USB branché pour afficher le disque.
La console de diagnostic USB n’expose pas encore de commande bootloader. Le simple
branchement USB et les lectures `make ble` ne redémarrent pas le contrôleur.

L’option finale `usb-trace` sélectionne uniquement les builds de diagnostic USB,
par exemple `make flash wire macos left usb-trace`. Elle ne se rabat pas sur un
firmware normal ou `debug`. Voir [le diagnostic d’OS](detection-os.md).

Le build migré utilise la rétention de mode de démarrage de Zephyr et le code
UF2 `0x57` du nice!nano. Le build contrôle ces options et les limites de mémoire.
Le bootloader UF2 existant du contrôleur n’est pas remplacé par cette migration.
L’accès par `&bootloader` et le retour par reset simple sont confirmés sur V1
avec les nouveaux UF2. Le double appui reste à confirmer sur cette version.
L’ancien mécanisme reste dans le [snapshot stable 0.3](firmwares-stables.md),
récupérable dans l’historique Git.
