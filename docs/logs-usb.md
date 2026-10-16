# Logs USB

Préciser le matériel et la keymap, puis ajouter `debug` :

```sh
make build wire multi debug
make build bluetooth windows debug
make build wire macos debug
```

L’option utilise le snippet natif `zmk-usb-logging` de ZMK. Elle ajoute une console
série USB sur chaque moitié, avec les mêmes rôles, keymap et délais que le profil
normal. En Bluetooth, l’USB sert à la console et à la charge, la saisie reste en BLE.

Le matériel et la keymap restent obligatoires, même en diagnostic.
Les caches sont séparés. Les dossiers et UF2 portent par exemple `wire-multi-debug`
ou `bluetooth-macos-debug`, et `build-info.txt` indique `build_mode=debug`.
Les compilations sans `debug` gardent les logs USB désactivés.
Les logs servent au diagnostic, ils ne mesurent pas à eux seuls la latence complète
du clavier. Voir [les mesures possibles et leurs limites](latence.md).

## Lire la console

1. Flasher les UF2 de diagnostic avec la [procédure habituelle](flash-firmware.md), sans reset des appairages.
2. Brancher la moitié à observer en USB. Le port série apparaît après son redémarrage.
3. Sur macOS, repérer le port puis ouvrir la console :

```sh
ls /dev/cu.usbmodem*
screen /dev/cu.usbmodemXXXX 115200
```

Remplacer `XXXX` par le port trouvé. Pour quitter `screen` : `Ctrl+A`, puis `\`,
puis confirmer avec `y`. Sous Linux, utiliser le port `/dev/ttyACM…` dans un terminal
série. Sous Windows, choisir le port COM dans un terminal série.

Avec deux USB branchés, chaque moitié a sa propre console. Un reset peut fermer le
port, il faut alors rouvrir le terminal. Les logs peuvent exposer les touches et des données
d’appairage : conserver les captures privées hors Git, par exemple dans `.local/logs/`.

La lecture des deux consoles de V1 en `wire macos debug` est validée. La lecture
du profil Bluetooth sur V2 reste à tester.

Après le diagnostic, reflasher le profil sans `debug`, car les logs augmentent
la consommation. La veille profonde reste
bloquée tant que l’USB alimente le clavier, voir [veille et réveil](veille.md).

[Snippet officiel ZMK 0.3.0](https://github.com/zmkfirmware/zmk/blob/v0.3.0/app/snippets/zmk-usb-logging/snippet.yml).
