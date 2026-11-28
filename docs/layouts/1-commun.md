# 1. Commun

[Documentation](../README.md) › [Layouts](../README.md#layouts) › 1. Commun

[2. macOS →](2-macos.md)

## Couche de base

Ce schéma reprend le commentaire de [base-layer.dtsi](../../keyboard/config/keymaps/common/base-layer.dtsi).
Les cases isolées en bas sont les clics des encodeurs. Les encadrés extérieurs
donnent leurs rotations dans l’ordre horaire puis antihoraire.

```text
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃    ESC    ┃     A     ┃     Z     ┃     E     ┃     R     ┃     T     ┃   ┃     Y     ┃     U     ┃     I     ┃     O     ┃     P     ┃   BSPACE  ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    TAB    ┃     Q     ┃     S     ┃     D     ┃     F     ┃     G     ┃   ┃     H     ┃     J     ┃     K     ┃     L     ┃     M     ┃    DEL    ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     W     ┃     X     ┃     C     ┃     V     ┃     B     ┃   ┃     N     ┃   , ou ?  ┃  BASE_DOT ┃   ; ou /  ┃ BASE_BSLS ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  VOL- │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │  LEFT │
│  VOL+ │ ┃ Play→Mute ┃ ┃ BASE_CTRL ┃  BASE_ALT ┃     L2    ┃   SPACE   ┃   ┃ SHIFT+LOCK┃     L1    ┃   ENTER   ┃  BASE_GUI ┃ ┃  BASE_OS  ┃ │ RIGHT │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

Chaque profil fixe renseigne les six cases `BASE_*`.

| Case | macOS | Windows | Linux |
| --- | --- | --- | --- |
| `BASE_CTRL` | Command | Ctrl | Ctrl |
| `BASE_ALT` | Option | Alt | Alt |
| `BASE_GUI` | Control | Win | Super |
| `BASE_DOT` | `.` ou `!` avec Shift | `.` ou `!` avec Shift | `.` ou `!` avec Shift |
| `BASE_BSLS` | `:` ou `\` avec Shift | `:` ou `\` avec Shift | `:` ou `\` avec Shift |
| `BASE_OS` | Écrit brièvement « MacOS » | Écrit brièvement « Windows » | Écrit brièvement « Linux » |

## Lire les schémas par OS

Chaque schéma garde la forme du clavier. Les trois premières rangées montrent
la grille de touches et la dernière les quatre pouces par moitié, entre les
clics d’encodeur. Au total, **42 touches et deux clics**.

| Lire une case | Ce que fait la touche |
| --- | --- |
| Case vide | Aucune action |
| `·` | Action de la couche active inférieure, selon les couches en cours |
| `, ou ?` | Virgule seule, point d’interrogation avec Shift |
| `é/É` ou `à/â/À` | Un, deux ou trois appuis rapprochés, dans cet ordre (fenêtre de 250 ms) |
| `Play→Mute` | Appui bref pour lecture/pause, maintien pour couper ou rétablir le son |

Chaque schéma indique aussi les rotations de chaque encodeur dans l’ordre
**↻ horaire / ↺ antihoraire** des paramètres ZMK. Les accents, lettres et symboles supposent
la disposition de saisie française côté ordinateur. Les actions décrivent les
bindings du firmware et peuvent dépendre des raccourcis du système ou de l’app.

## Accéder aux couches

| Geste depuis la base | Couche ou action |
| --- | --- |
| Maintenir **L1**, au pouce droit | L1, navigation et symboles |
| Maintenir **L2**, au pouce gauche | L2, accents et nombres |
| Maintenir **L1 + L2** | L3, fonctions système et raccourcis propres à l’OS |
| Appuyer deux fois sur **L1**, en gardant le dernier appui | L2 tant que la touche reste enfoncée |
| Appuyer deux fois sur **L2**, en gardant le dernier appui | L1 tant que la touche reste enfoncée |
| Appuyer trois fois sur **L1** | Pavé numérique jusqu’à la sortie explicite par la position L1 |
| Appuyer deux fois sur **Shift** de la base | Verrouiller Shift. Un appui supplémentaire le déverrouille |
| Cliquer sur l’encodeur droit depuis la base | Écrire brièvement le nom de l’OS puis l’effacer |

Les appuis multiples sont définis dans les
[comportements ZMK](../../keyboard/config/keymaps/common/behaviors.dtsi).
Avec L1 seule, les positions G et B restent sans action dans tous les profils.
Le clic droit en L1 est sans action dans les profils fixes. Il change d’OS dans
[multi](../technique/firmware/3-architecture-keymaps.md#comportement-de-multi) ou sélectionne l’ordinateur dans
[bluetooth auto](5-auto.md#en-bluetooth).

---

[Documentation](../README.md) › [Layouts](../README.md#layouts) › 1. Commun

[2. macOS →](2-macos.md)
