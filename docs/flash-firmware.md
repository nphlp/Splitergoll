# Flash du firmware

Les commandes de build produisent les UF2, elles ne flashent pas le clavier.
Choisir le profil voulu et utiliser les deux fichiers du même dossier de build.

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

Le comportement de la touche bootloader est conservé avec ZMK 0.3.0.
L’avertissement `NRF_STORE_REBOOT_TYPE_GPREGRET` concerne son mécanisme interne,
qui reste nécessaire jusqu’à la migration. Les détails sont dans `AGENTS.md`.
