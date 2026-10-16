# Firmwares stables

Le snapshot [ZMK 0.3.0](../archives/firmware/zmk-0.3.0/) conserve les firmwares
avant la migration. Il est versionné dans Git, indépendant des fichiers de `dist/`.

## Contenu

Les 32 UF2 couvrent toutes les combinaisons actuellement disponibles :

- Matériel : `wire` et `bluetooth`.
- Keymap : `multi`, `windows` avec gaming, `macos` et `linux`.
- Mode : normal ou `debug`, pour les deux moitiés.

Le dongle n’est pas encore un profil compilable. Les variantes minimales historiques
restent hors périmètre. Les dossiers horodatés et noms des UF2 sont conservés.

`snapshot.json` indique les versions, le commit source et les 32 fichiers.
`west.yml` fige les dépendances, `source-checksums.json` identifie les sources.
Chaque paire conserve `build-info.txt` et `SHA256SUMS`. Le manifeste SHA-256 à la
racine contrôle aussi les métadonnées. Aucun log privé ni appairage n’est inclus.

Tous les firmwares compilent et passent les contrôles de configuration et UF2.
Cela ne signifie pas que chaque combinaison a été essayée physiquement : voir
[les essais wire](tests-wire.md) et [les essais de veille](veille.md).

## Revenir au firmware précédent

Choisir le matériel et la keymap correspondant au clavier. Pour V1 sur le Mac,
mettre uniquement la droite en bootloader, puis utiliser le chemin explicite :

```sh
archive=archives/firmware/zmk-0.3.0/wire-macos-2026-10-16-15-54-47
make flash "$archive/splitergoll-wire-macos-right.uf2"
```

Après son redémarrage, mettre uniquement la gauche en bootloader et lancer :

```sh
make flash "$archive/splitergoll-wire-macos-left.uf2"
```

Toujours remettre les deux fichiers de la même paire. Sans chemin explicite,
`make flash wire macos right` cherche dans `dist/`, pas dans cette archive.
La [procédure de flash](flash-firmware.md) détaille la confirmation et les contrôles.

L’archive restaure le programme, pas une sauvegarde des réglages NVS. Un retour
après migration peut perdre la préférence de sortie USB/Bluetooth. Ne pas flasher
de reset des réglages systématiquement. Vérifier ensuite la saisie des deux côtés.

`make clear` conserve cette archive, mais supprime les caches et builds locaux.
Pour reconstruire exactement cette version, utiliser le commit source indiqué
dans `snapshot.json`, avec son manifeste et son image Docker figés.
