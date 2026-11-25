# 02. macOS

[Documentation](../README.md) › [Layouts clavier](../README.md#layouts-clavier) › 02. macOS

[← 01. Tronc commun](01-tronc-commun.md) · [03. Windows →](03-windows.md)

Le profil fixe `macos` démarre sur cette base. Son jeu de couches est aussi utilisé quand `auto` sélectionne macOS.

[Légende et gestes communs](01-tronc-commun.md). `·` indique une touche héritée.

[Base](#0-base) · [L1](#1-l1--navigation-et-symboles) · [L2](#2-l2--accents-et-nombres) · [L3](#3-l3--système-et-aerospace) · [Pavé numérique](#4-pavé-numérique)

## 0. Base

```text
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃    ESC    ┃     A     ┃     Z     ┃     E     ┃     R     ┃     T     ┃   ┃     Y     ┃     U     ┃     I     ┃     O     ┃     P     ┃   BSPACE  ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    TAB    ┃     Q     ┃     S     ┃     D     ┃     F     ┃     G     ┃   ┃     H     ┃     J     ┃     K     ┃     L     ┃     M     ┃    DEL    ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     W     ┃     X     ┃     C     ┃     V     ┃     B     ┃   ┃     N     ┃   , ou ?  ┃   . ou !  ┃   ; ou /  ┃   : ou \  ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  VOL- │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │  LEFT │
│  VOL+ │ ┃ Play→Mute ┃ ┃  COMMAND  ┃   OPTION  ┃     L2    ┃   SPACE   ┃   ┃ SHIFT+LOCK┃     L1    ┃   ENTER   ┃    CTRL   ┃ ┃   macOS   ┃ │ RIGHT │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

La [base commune](01-tronc-commun.md#couche-de-base) fournit la grille. Le profil
macOS renseigne les six cases paramétrables ci-dessous.

| Case du tronc commun | Sur macOS |
| --- | --- |
| `BASE_CTRL` | Command (⌘) au pouce gauche |
| `BASE_ALT` | Option (⌥) au pouce gauche |
| `BASE_GUI` | Control au pouce droit |
| `BASE_DOT` | `.` ou `!` avec Shift, via le binding macOS `m_dot_excl` |
| `BASE_BSLS` | `:` ou `\` avec Shift, via le binding macOS `m_colon_bsls` |
| `BASE_OS` | Le clic de l’encodeur droit écrit brièvement « MacOS » |

## 1. L1 · Navigation et symboles

```text
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃           ┃           ┃Début ligne┃           ┃ Fin ligne ┃           ┃   ┃     "     ┃     (     ┃     )     ┃     [     ┃     ]     ┃     ·     ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃           ┃           ┃     ←     ┃     ↑     ┃     →     ┃     ->    ┃   ┃     '     ┃     {     ┃     }     ┃     <     ┃     >     ┃     ·     ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃           ┃   Mot ←   ┃     ↓     ┃   Mot →   ┃     =>    ┃   ┃     `     ┃     #     ┃     &     ┃     |     ┃     @     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  LEFT │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │   UP  │
│ RIGHT │ ┃           ┃ ┃  COMMAND  ┃   OPTION  ┃     ·     ┃   SHIFT   ┃   ┃   SHIFT   ┃     ·     ┃     ·     ┃    CTRL   ┃ ┃           ┃ │  DOWN │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

En profil fixe, le clic de l’encodeur droit en L1 est sans action.
Avec [bluetooth auto](06-auto.md#en-bluetooth), il sélectionne l’ordinateur suivant.

## 2. L2 · Accents et nombres

```text
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃   BSPACE  ┃    œ/Œ    ┃     °     ┃     _     ┃     -     ┃     +     ┃   ┃     =     ┃     7     ┃     8     ┃     9     ┃    €/£    ┃     ·     ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    DEL    ┃    ô/Ô    ┃    é/É    ┃    ê/Ê    ┃    è/ë    ┃     *     ┃   ┃     0     ┃     4     ┃     5     ┃     6     ┃     $     ┃     ·     ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃    î/ï    ┃   ù/û/µ   ┃    ç/Ç    ┃   à/â/À   ┃     /     ┃   ┃     .     ┃     1     ┃     2     ┃     3     ┃     %     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  LEFT │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │   UP  │
│ RIGHT │ ┃           ┃ ┃     ·     ┃   OPTION  ┃     ·     ┃   ENTER   ┃   ┃   SPACE   ┃     ·     ┃     ·     ┃    CTRL   ┃ ┃           ┃ │  DOWN │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

## 3. L3 · Système et AeroSpace

```text
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃     F1    ┃     F2    ┃ AS tuiles ┃AS flottant┃ AS accord.┃     F6    ┃   ┃     F7    ┃ Foc/Dépl 1┃ Foc/Dépl 2┃ Foc/Dépl 3┃    F11    ┃    F12    ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃   Reset   ┃           ┃ Foc/Dépl ←┃ Foc/Dépl ↑┃ Foc/Dépl →┃           ┃   ┃   ´ mort  ┃   ` mort  ┃   ^ mort  ┃   ¨ mort  ┃   ~ mort  ┃           ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃           ┃           ┃ Foc/Dépl ↓┃           ┃    ⇧⌥Q    ┃   ┃           ┃           ┃           ┃           ┃           ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│   UP  │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │  LEFT │
│  DOWN │ ┃ Bootloader┃ ┃           ┃           ┃     ·     ┃           ┃   ┃   SHIFT   ┃     ·     ┃           ┃           ┃ ┃ Efface BLE┃ │ RIGHT │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Raccourcis AeroSpace émis par le clavier

Les positions reprennent la lettre de la couche de base. Dans le schéma,
« AS » désigne AeroSpace et « Foc/Dépl » indique focus ou déplacement. Pour
déplacer une fenêtre, utiliser **Shift gauche**, disponible au pouce droit sur L3.
Les chiffres ci-dessous sont ceux de la rangée numérique.

| Position sur la base | Action | Sans Shift | Avec Shift |
| --- | --- | --- | --- |
| S | Focus / déplacer à gauche | Ctrl+⌥+⌘+3 | Ctrl+⌥+⇧+3 |
| D | Focus / déplacer vers le haut | Ctrl+⌥+⌘+1 | Ctrl+⌥+⇧+1 |
| F | Focus / déplacer à droite | Ctrl+⌥+⌘+4 | Ctrl+⌥+⇧+4 |
| C | Focus / déplacer vers le bas | Ctrl+⌥+⌘+2 | Ctrl+⌥+⇧+2 |
| U | Aller à l’espace 1 / y déplacer la fenêtre | Ctrl+⌥+⌘+5 | Ctrl+⌥+⇧+5 |
| I | Aller à l’espace 2 / y déplacer la fenêtre | Ctrl+⌥+⌘+6 | Ctrl+⌥+⇧+6 |
| O | Aller à l’espace 3 / y déplacer la fenêtre | Ctrl+⌥+⌘+7 | Ctrl+⌥+⇧+7 |
| Z | Mode tuiles | Ctrl+⌥+⌘+0 | Binding fixe |
| E | Mode flottant | Ctrl+⌥+⌘+8 | Binding fixe |
| R | Mode accordéon | Ctrl+⌥+⌘+9 | Binding fixe |

Sur les trois dernières positions, Shift ne choisit pas un autre raccourci
dans le firmware. La colonne « Action » donne l’intention des raccourcis côté
ordinateur. Leur effet dépend des réglages AeroSpace et des autres raccourcis macOS.

Le reste de L3 comprend les touches de fonction visibles sur le schéma, les
accents morts, `⇧⌥Q` en position **B**, la commande de redémarrage en position
**Tab** et les clics bootloader à gauche et effacement des appairages à droite.
Les raccourcis sont définis dans [macos.h](../../keyboard/config/keymaps/common/french_layout/macos.h)
et les doubles actions dans [behaviors.dtsi](../../keyboard/config/keymaps/common/behaviors.dtsi).

## 4. Pavé numérique

```text
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃     ·     ┃     A     ┃     Z     ┃     ↑     ┃     -     ┃     +     ┃   ┃     =     ┃     7     ┃     8     ┃     9     ┃    €/£    ┃     ·     ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃     ·     ┃     Y     ┃     ←     ┃     ↓     ┃     →     ┃     *     ┃   ┃     0     ┃     4     ┃     5     ┃     6     ┃     $     ┃     ·     ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃           ┃     X     ┃     C     ┃     V     ┃     /     ┃   ┃     .     ┃     1     ┃     2     ┃     3     ┃     %     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  LEFT │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │   UP  │
│ RIGHT │ ┃           ┃ ┃  COMMAND  ┃           ┃   BSPACE  ┃    DEL    ┃   ┃   SPACE   ┃    Base   ┃   ENTER   ┃    CTRL   ┃ ┃   Numpad  ┃ │  DOWN │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

Triple appui sur L1 depuis la base pour entrer. Appuyer sur la position L1 pour revenir à la base.

Sources du layout : [layers/macos.dtsi](../../keyboard/config/keymaps/layers/macos.dtsi), [base partagée](../../keyboard/config/keymaps/common/base-layer.dtsi) et [comportements](../../keyboard/config/keymaps/common/behaviors.dtsi).

---

[Documentation](../README.md) › [Layouts clavier](../README.md#layouts-clavier) › 02. macOS

[← 01. Tronc commun](01-tronc-commun.md) · [03. Windows →](03-windows.md)
