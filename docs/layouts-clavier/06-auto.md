# 06. Profil auto

[Documentation](../README.md) › [Layouts clavier](../README.md#layouts-clavier) › 06. Profil auto

[← 05. Profil multi](05-multi.md)

`auto` choisit le groupe [macOS](02-macos.md) ou [Windows](03-windows.md)
pendant l’utilisation. Les deux groupes reprennent les mêmes positions et
actions que les profils fixes, avec les différences ci-dessous. Pour Linux,
compiler le profil [linux](04-linux.md) ou choisir Linux dans [multi](05-multi.md).

| Groupe | Base | L1 | L2 | L3 | Pavé | Jeu | Jeu 2 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Windows | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| macOS | 8 | 9 | 10 | 11 | 12 | | |

Ces numéros internes changent selon le profil. Les gestes pour atteindre L1,
L2, L3, le pavé et le jeu restent ceux de l’OS actif.

## En USB

Avec `wire auto`, la gauche observe les requêtes USB de l’ordinateur pour
distinguer macOS et Windows. La couche 0 d’attente laisse les touches et les
encodeurs inactifs tant qu’aucune décision n’est disponible. Une déconnexion
USB retire la sélection avant l’énumération suivante.

[Essais du profil auto en USB](../technique/validation/03-tests-auto-usb.md)

## En Bluetooth

Avec `bluetooth auto`, la keymap suit l’OS mémorisé pour l’ordinateur choisi.
**L1 + clic sur l’encodeur droit**, puis relâchement, change d’ordinateur ou
en recherche un autre. Ce geste reste accessible pendant l’attente initiale.
Le clic simple depuis la base continue d’écrire brièvement le nom de l’OS.
Le pavé et les couches de jeu reviennent à la base du groupe sélectionné.

- [Appairer et changer d’ordinateur](../utiliser-le-clavier/01-bluetooth.md)
- [Essais du profil auto en Bluetooth](../technique/validation/04-tests-auto-bluetooth.md)
- [Priorité USB expérimentale](../technique/experimentations/01-priorite-usb.md)

Sources du firmware : [auto.keymap](../../keyboard/config/keymaps/auto.keymap)
et [numéros de couches](../../keyboard/config/keymaps/common/auto-layers.h).

---

[Documentation](../README.md) › [Layouts clavier](../README.md#layouts-clavier) › 06. Profil auto

[← 05. Profil multi](05-multi.md)
