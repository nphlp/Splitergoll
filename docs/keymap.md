# Keymap

La référence est la keymap complète de `dongle` au commit `2584ca4`.
Les affectations, accents, macros et encodeurs sont conservés. Les branches
`-minimal` sont uniquement des archives.
Cette keymap constitue le profil `multi` actuel : les trois OS et le jeu dans
un même firmware.

La migration conserve ces affectations. Dans les comportements partagés, le nouveau
verrouillage implicite de `&to` et `&tog` est désactivé pour garder les transitions de la 0.3.

## Choisir une keymap

| Argument | Contenu | Démarrage |
| --- | --- | --- |
| `auto` | macOS ou Windows avec gaming, détection USB expérimentale | Attente puis OS reconnu |
| `multi` | Keymap actuelle, trois OS et gaming, changement d’OS au clavier | Windows |
| `windows` | Windows et les deux couches gaming | Windows |
| `macos` | macOS, sans gaming ni bascule OS | macOS |
| `linux` | Linux, sans gaming ni bascule OS | Linux |

Par exemple : `make build wire macos` ou `make build bluetooth windows debug`.
Le matériel et la keymap sont deux choix obligatoires. `auto` est réservé à `wire`,
voir [son fonctionnement et les essais](keymap-auto.md).
Même `multi` doit être précisé, aucune keymap n’est choisie par défaut.

Les profils fixes gardent leur OS après redémarrage et veille profonde. Le clic
droit affiche toujours son nom sur la base, mais ne change plus d’OS avec `L1`.
Le pavé numérique revient à la base du même OS. Celui de macOS reprend aussi
ses modificateurs et ses symboles. Le fonctionnement de `wire macos` sur les deux
moitiés est confirmé, le parcours complet des touches et les autres profils restent à tester.

## Organisation

Les fichiers de sélection sont dans `keyboard/config/keymaps/` :
le build les choisit explicitement avec `KEYMAP_FILE`.

| Fichier | Rôle |
| --- | --- |
| `multi.keymap` | Composition active des quinze couches, avec changement d’OS |
| `windows.keymap`, `macos.keymap`, `linux.keymap` | Sélection de chaque OS fixe |
| `layers/windows.dtsi`, `layers/macos.dtsi` | Couches et schémas communs aux profils fixes et à `auto` |
| `auto.keymap`, `common/auto-layers.h` | Assemblage des deux groupes et numéros de couches automatiques |
| `common/base-layer.dtsi` | 44 positions de la base, partagées par les keymaps |
| `common/behaviors.dtsi` | Macros, tap dances et morphs partagés |
| `common/fixed-layers.h` | Couches et combinaison `L1` + `L2` des profils fixes |
| `common/multi-layers.h` | Numéros et combinaisons des couches de `multi` |
| `common/function.h`, `common/french_layout/` | Helpers et codes de touches de la configuration active |

L’archive autonome est dans [`archives/keymap-multi/`](../archives/keymap-multi/).
Elle conserve `splitergoll.keymap`, ses cinq headers d’origine et leurs empreintes,
sans être utilisée par les builds. Faire évoluer les profils actifs, pas cette archive.

Le matériel reste dans `keyboard/boards/shields/splitergoll/` :

| Fichier | Rôle |
| --- | --- |
| `splitergoll.dtsi` et les overlays | Correspondance entre positions physiques et keymap |
| Fichiers `.conf`, `Kconfig.*` et `Splitergoll.zmk.yml` | Rôles des moitiés et déclaration du shield |
| `splitergoll_dongle.conf` et `.overlay` | Configuration historique à conserver pour réintégrer le dongle |

Le préprocesseur assemble ces fichiers. ZMK traite ensuite les appuis et l’état
des couches sur la centrale.
La configuration active ne dépend plus de la keymap de sauvegarde ni de ses helpers.

Dans VS Code, `.vscode/settings.json` affiche les commentaires en style normal
pour garder les schémas lisibles. Cette surcharge vaut uniquement pour le dépôt
et conserve les couleurs du thème.
Les fragments de couches et les fichiers communs `.dtsi` utilisent la coloration C++, comme des `.h`,
sans changer leur extension ni leur rôle dans le build.

## Comportement de multi

La keymap comporte 44 positions et 15 couches techniques : bases Windows/Linux/macOS,
fonctions primaires, secondaires et tertiaires avec adaptations OS, pavé numérique
et jeu. Windows est initial, le changement d’OS actuel reste disponible.

Le choix d’OS est en mémoire et ne survit pas au redémarrage après veille profonde :
macOS revient à Windows, confirmé sur le clavier Mac. Les appairages Bluetooth
restent enregistrés. La conservation de l’OS fera partie du découpage des profils.

Les touches de fonction activent des couches temporairement. Des couches
conditionnelles combinent le choix d’OS et ces fonctions. Les tap dances distinguent
les appuis simples et multiples. Les morphs changent une action selon un modificateur.

Le comportement Shift de `dongle` est conservé : absent de la couche tertiaire
commune, présent sur celle de macOS. La disposition de ses accents reste identique.

Les profils fixes résolvent les touches héritées des adaptations Linux/macOS.
La base n’est écrite qu’une fois. Chaque profil fournit ses trois modificateurs,
ses deux bindings de ponctuation et le nom d’OS affiché par le clic droit.
Les autres couches restent explicites, avec un schéma commenté et les bindings
gauche/droite alignés en dessous. `multi` conserve ses bascules et ses limites.

## Schémas

Même disposition et même présentation que dans le README, avec les légendes
alignées sur les bindings de `dongle`. Ces sept schémas montrent la base Windows
et les couches communes, les adaptations Linux/macOS sont résumées ensuite.

- `·` : action héritée de la couche inférieure. `∅` : aucune action.
- `→` entre deux actions : appui bref puis maintien. `ou` : action avec Shift.
- Les cases isolées du bas sont les clics des encodeurs. Les deux lignes extérieures
  indiquent leurs deux actions de rotation.
- Deux appuis sur Shift verrouillent Shift. Un appui supplémentaire le déverrouille.
- Deux appuis sur `L1` activent le secondaire, deux appuis sur `L2` le primaire.

### Base

Lettres et ponctuation. `L1` et `L2` donnent accès aux fonctions. Le clic droit
affiche brièvement le nom de l’OS avec la macro existante.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃    ESC    ┃     A     ┃     Z     ┃     E     ┃     R     ┃     T     ┃   ┃     Y     ┃     U     ┃     I     ┃     O     ┃     P     ┃   BSPACE  ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    TAB    ┃     Q     ┃     S     ┃     D     ┃     F     ┃     G     ┃   ┃     H     ┃     J     ┃     K     ┃     L     ┃     M     ┃    DEL    ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     W     ┃     X     ┃     C     ┃     V     ┃     B     ┃   ┃     N     ┃   , ou ?  ┃   . ou !  ┃   ; ou /  ┃   : ou \  ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  VOL+ │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │  LEFT │
│  VOL- │ ┃ Play→Mute ┃ ┃    CTRL   ┃    ALT    ┃     L2    ┃   SPACE   ┃   ┃ SHIFT+LOCK┃     L1    ┃   ENTER   ┃    WIN    ┃ ┃  Windows  ┃ │ RIGHT │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Primaire, L1

Navigation, sélection avec Shift et symboles de programmation. Le clic droit
passe à Linux, puis à macOS depuis Linux, puis à Windows depuis macOS.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃     ∅     ┃     ∅     ┃    HOME   ┃     ∅     ┃    END    ┃     ∅     ┃   ┃     "     ┃     (     ┃     )     ┃     [     ┃     ]     ┃     ·     ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃     ∅     ┃     ∅     ┃     ←     ┃     ↑     ┃     →     ┃     ->    ┃   ┃     '     ┃     {     ┃     }     ┃     <     ┃     >     ┃     ·     ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     ∅     ┃   CTRL ←  ┃     ↓     ┃   CTRL →  ┃     =>    ┃   ┃     `     ┃     #     ┃     &     ┃     |     ┃     @     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  LEFT │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │   UP  │
│ RIGHT │ ┃     ∅     ┃ ┃     ·     ┃     ·     ┃     ·     ┃   SHIFT   ┃   ┃   SHIFT   ┃     ·     ┃     ·     ┃     ·     ┃ ┃ OS: Linux ┃ │  DOWN │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Secondaire, L2

Accents à gauche, nombres et calculs à droite. `+` sépare les actions obtenues
par un, deux ou trois appuis, par exemple `à+â+À`.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃   BSPACE  ┃    œ+Œ    ┃   °+²+³   ┃     _     ┃     -     ┃     +     ┃   ┃     =     ┃     7     ┃     8     ┃     9     ┃    €+£    ┃     ·     ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    DEL    ┃    ô+Ô    ┃    é+É    ┃    ê+Ê    ┃    è+ë    ┃     *     ┃   ┃     0     ┃     4     ┃     5     ┃     6     ┃     $     ┃     ·     ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃    î+ï    ┃   ù+û+µ   ┃    ç+Ç    ┃   à+â+À   ┃     /     ┃   ┃     .     ┃     1     ┃     2     ┃     3     ┃     %     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  LEFT │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │   UP  │
│ RIGHT │ ┃     ∅     ┃ ┃     ·     ┃     ·     ┃     ·     ┃   ENTER   ┃   ┃   SPACE   ┃     ·     ┃     ·     ┃     ·     ┃ ┃     ∅     ┃ │  DOWN │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Tertiaire, L1 + L2

Touches F, caractères fléchés, accents morts et commandes système. Shift est
absent au pouce droit dans cette couche commune, il revient dans l’adaptation macOS.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃     F1    ┃     F2    ┃     F3    ┃     F4    ┃     F5    ┃     F6    ┃   ┃     F7    ┃     F8    ┃     F9    ┃    F10    ┃    F11    ┃    F12    ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃   Reset   ┃     ∅     ┃  ← texte  ┃  ↑ texte  ┃  → texte  ┃     ∅     ┃   ┃     ∅     ┃   ` mort  ┃   ^ mort  ┃   ¨ mort  ┃   ~ mort  ┃   ALT+F4  ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     ∅     ┃     ∅     ┃  ↓ texte  ┃     ∅     ┃ Imp. écran┃   ┃   Clic G  ┃   Clic D  ┃     ∅     ┃     ∅     ┃     ∅     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│   UP  │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │  LEFT │
│  DOWN │ ┃ Bootloader┃ ┃    CTRL   ┃    ALT    ┃     ·     ┃     ∅     ┃   ┃     ∅     ┃     ·     ┃     ∅     ┃    Jeu    ┃ ┃ Efface BLE┃ │ RIGHT │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Pavé numérique

Trois appuis sur `L1` verrouillent cette couche. La touche `Windows` du pouce
droit revient à la base Windows. Le clic d’encodeur gauche ne fait rien.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃     ·     ┃     A     ┃     Z     ┃     ↑     ┃     -     ┃     +     ┃   ┃     =     ┃     7     ┃     8     ┃     9     ┃    €+£    ┃     ·     ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃     ·     ┃     Y     ┃     ←     ┃     ↓     ┃     →     ┃     *     ┃   ┃     0     ┃     4     ┃     5     ┃     6     ┃     $     ┃     ·     ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     ∅     ┃     X     ┃     C     ┃     V     ┃     /     ┃   ┃     .     ┃     1     ┃     2     ┃     3     ┃     %     ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  LEFT │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │   UP  │
│ RIGHT │ ┃     ∅     ┃ ┃    CTRL   ┃     ∅     ┃   BSPACE  ┃    DEL    ┃   ┃   SPACE   ┃  Windows  ┃   ENTER   ┃    WIN    ┃ ┃   Numpad  ┃ │  DOWN │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Jeu

`Jeu` sur la couche tertiaire active ce mode. `TAB→ESC` produit Tab à l’appui
bref et Échap au maintien. Maintenir `Jeu 2` active la couche suivante.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃  TAB→ESC  ┃     A     ┃     Z     ┃     E     ┃     R     ┃     T     ┃   ┃     Y     ┃     U     ┃     I     ┃     O     ┃     P     ┃   BSPACE  ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃    TAB    ┃     Q     ┃     S     ┃     D     ┃     F     ┃     G     ┃   ┃     H     ┃     J     ┃     K     ┃     L     ┃     M     ┃    DEL    ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     W     ┃     X     ┃     C     ┃     V     ┃     B     ┃   ┃     N     ┃   , ou ?  ┃   . ou !  ┃   ; ou /  ┃   : ou \  ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  VOL+ │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │  VOL+ │
│  VOL- │ ┃     ∅     ┃ ┃     +     ┃     -     ┃     '     ┃   SPACE   ┃   ┃   SHIFT   ┃   Jeu 2   ┃   ENTER   ┃    WIN    ┃ ┃   Gaming  ┃ │  VOL- │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

### Jeu 2

Les raccourcis sont les `Ctrl+F1` à `Ctrl+F12` réellement envoyés par le firmware.
Leurs actions dans les logiciels dépendent de leur configuration. Le clic droit
revient à Windows.

```keymap
┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓
┃     ∅     ┃     ∅     ┃     ∅     ┃     ∅     ┃     ∅     ┃     ∅     ┃   ┃     "     ┃  CTRL+F1  ┃  CTRL+F2  ┃  CTRL+F3  ┃  CTRL+F4  ┃     ·     ┃
┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫
┃     ·     ┃     ∅     ┃     ∅     ┃     ∅     ┃     ∅     ┃     ∅     ┃   ┃     '     ┃  CTRL+F5  ┃  CTRL+F6  ┃  CTRL+F7  ┃  CTRL+F8  ┃     ·     ┃
┗━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫   ┣━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┛
            ┃     ∅     ┃     ∅     ┃     ∅     ┃     ∅     ┃     ∅     ┃   ┃     `     ┃  CTRL+F9  ┃  CTRL+F10 ┃  CTRL+F11 ┃  CTRL+F12 ┃
╭───────╮   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ╭───────╮
│  VOL+ │ ┏━━━━━━━━━━━┓ ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓   ┏━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓ ┏━━━━━━━━━━━┓ │  VOL+ │
│  VOL- │ ┃     ∅     ┃ ┃    CTRL   ┃    ALT    ┃     ∅     ┃     ·     ┃   ┃     ·     ┃     ·     ┃     ·     ┃     ·     ┃ ┃  Windows  ┃ │  VOL- │
╰───────╯ ┗━━━━━━━━━━━┛ ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛   ┗━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛ ┗━━━━━━━━━━━┛ ╰───────╯
```

## Adaptations Linux et macOS

| OS | Adaptations |
| --- | --- |
| Linux | Même base, codes adaptés pour certains accents et symboles via les couches conditionnelles. La disposition hôte attendue est « Français (variante) ». |
| macOS | Command/Option/Control, navigation par ligne et mot, symboles et accents adaptés. La couche tertiaire contient les raccourcis AeroSpace et Shift au pouce droit. |

Les encodeurs gardent les mêmes actions de rotation selon la fonction active.
Les schémas ne reproduisent pas les 15 couches techniques séparément : les couches
OS modifient les positions nécessaires et héritent du reste.

Dans `multi`, les retours du pavé numérique et du jeu utilisent toujours `to 0`,
donc Windows. La conservation de son choix d’OS reste dans le [TODO](TODO.md).
Les sept schémas ci-dessus documentent `multi`, les profils fixes reprennent
l’adaptation de leur OS et les différences décrites en début de guide.
