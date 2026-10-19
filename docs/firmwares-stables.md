# Firmwares stables

Le snapshot ZMK 0.3.0 avant migration reste dans l’historique Git au commit
`7e7be6d`. Il a été retiré du checkout à la demande de l’utilisateur après les
essais positifs de V1 en `wire macos`. Les autres profils et V2 restent à valider.

## Contenu conservé dans Git

Les 32 UF2 couvrent les combinaisons manuelles disponibles au moment du snapshot :

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

## Récupérer et reflasher l’ancienne version

Depuis la racine du dépôt, récupérer uniquement les fichiers du snapshot,
sans changer de branche ni remplacer la configuration actuelle :

```sh
git archive 7e7be6d archives/firmware/zmk-0.3.0 | tar -x
```

Cette commande recrée l’archive comme fichiers non suivis. Choisir ensuite le
matériel et la keymap correspondant au clavier. Pour V1 sur le Mac, mettre
uniquement la droite en bootloader, puis utiliser le chemin explicite :

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

`make clear` supprime les caches et builds locaux, sans toucher à l’historique Git
ni à cette archive si elle a été récupérée. Pour reconstruire exactement cette
version, le commit source est indiqué dans `snapshot.json`, avec son manifeste
et son image Docker figés.
