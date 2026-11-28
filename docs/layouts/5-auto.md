# 5. Profil auto

[Documentation](../README.md) › [Layouts](../README.md#layouts) › 5. Profil auto

[← 4. Linux](4-linux.md)

`auto` choisit le groupe [macOS](2-macos.md) ou [Windows](3-windows.md)
pendant l’utilisation. Les deux groupes reprennent les mêmes positions et
actions que les profils fixes, avec les différences ci-dessous. Pour Linux,
compiler le profil [linux](4-linux.md).

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

[Essais du profil auto en USB](../technique/validation/3-tests-auto-usb.md)

## En Bluetooth

Avec `bluetooth auto`, la keymap suit l’OS mémorisé pour l’ordinateur choisi.
**L1 + clic sur l’encodeur droit**, puis relâchement, change d’ordinateur ou
en recherche un autre. Ce geste reste accessible pendant l’attente initiale.
Le clic simple depuis la base continue d’écrire brièvement le nom de l’OS.
Le pavé et les couches de jeu reviennent à la base du groupe sélectionné.

- [Appairer et changer d’ordinateur](../utilisation/1-bluetooth.md)
- [Essais du profil auto en Bluetooth](../technique/validation/4-tests-auto-bluetooth.md)
- [Priorité USB expérimentale](../technique/experimentations/1-priorite-usb.md)

Sources du firmware : [auto.keymap](../../keyboard/config/keymaps/auto.keymap)
et [numéros de couches](../../keyboard/config/keymaps/common/auto-layers.h).

---

[Documentation](../README.md) › [Layouts](../README.md#layouts) › 5. Profil auto

[← 4. Linux](4-linux.md)
