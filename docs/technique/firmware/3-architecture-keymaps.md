# 3. Architecture des keymaps

[Documentation](../../README.md) › [Technique](../README.md) › [Firmware](../README.md#firmware) › 3. Architecture des keymaps

[← 2. Matériel et profils de connexion](2-materiel-et-profils.md) · [4. Règles d’appairage →](4-gestion-appairage.md)

La keymap complète de `dongle` au commit `2584ca4` a servi de référence à
la migration. Le profil `multi` réunit les trois OS et le jeu dans un même firmware.
Les branches `-minimal` sont des archives.

Dans les comportements partagés, le verrouillage implicite de `&to` et `&tog`
est désactivé pour garder les transitions de la 0.3.

[Layouts actuels et variantes par OS](../../README.md#layouts)

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

La sauvegarde autonome avant découpage reste dans l’historique Git, par exemple
au commit `df55c35` sous `archives/keymap-multi/`. Le dossier `archives/` a été retiré
du checkout à la demande de l’utilisateur. Aucun profil actif n’utilisait ces copies.

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
et jeu. Le clavier démarre sur Windows. Maintenir L1 et cliquer sur l’encodeur
droit fait défiler Windows, Linux puis macOS.

Le choix d’OS est en mémoire et ne survit pas au redémarrage après veille profonde :
macOS revient à Windows, confirmé sur le clavier Mac. Les appairages Bluetooth
restent enregistrés. Les profils fixes démarrent toujours sur leur propre OS.

Les touches de fonction activent des couches temporairement. Des couches
conditionnelles combinent le choix d’OS et ces fonctions. Les tap dances distinguent
les appuis simples et multiples. Les morphs changent une action selon un modificateur.

Le comportement Shift de `dongle` est conservé : absent de la couche tertiaire
commune, présent sur celle de macOS. Les accents de saisie courante restent sur L2.

Les profils fixes résolvent les touches héritées des adaptations Linux/macOS.
La base n’est écrite qu’une fois. Chaque profil fournit ses trois modificateurs,
ses deux bindings de ponctuation et le nom d’OS affiché par le clic droit.
Les autres couches restent explicites, avec un schéma commenté et les bindings
gauche/droite alignés en dessous. `multi` conserve ses bascules et ses limites.

---

[Documentation](../../README.md) › [Technique](../README.md) › [Firmware](../README.md#firmware) › 3. Architecture des keymaps

[← 2. Matériel et profils de connexion](2-materiel-et-profils.md) · [4. Règles d’appairage →](4-gestion-appairage.md)
