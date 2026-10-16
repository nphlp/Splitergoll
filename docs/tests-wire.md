# Essais wire sur Mac et Windows

Utiliser le Clavier V1 sans batterie, avec les deux UF2 du même build `wire`.
Les [schémas de touches](keymap.md) restent la référence. La bascule d’OS décrite
ci-dessous concerne `multi`. Les profils fixes utilisent directement leur OS.

## Résultats confirmés

Le flash et la saisie des deux moitiés par USB sur le Mac sont confirmés par
l’utilisateur, avec la variante de diagnostic. Le clic d’encodeur qui affiche
l’OS paraît plus rapide qu’avec l’ancien firmware.

Le flash automatique avec `make flash wire macos right` puis `left` et le
fonctionnement des deux moitiés avec ce profil fixe sont aussi confirmés.
Le flash de `wire windows`, gaming inclus, et son fonctionnement sur V1
sont également confirmés. Le parcours détaillé des couches de jeu reste à vérifier.

Une latence était ressentie à droite avec `wire macos debug`. Après un passage par
`wire windows` normal, puis `wire macos` normal, l’écart est devenu imperceptible.
Comparer normal/debug à keymap identique avant d’en attribuer la cause aux logs ou au BLE.
La lecture des deux consoles USB est validée sur V1 en `wire macos debug`.
Le parcours complet des touches et couches sur les deux OS reste à faire.
Voir [le diagnostic de latence](latence.md).

## Préparer le clavier

1. Flasher la droite puis la gauche avec les deux UF2 du même build `wire`.
2. Ne pas flasher `settings_reset` tant qu’aucun problème d’appairage n’est établi.
3. Brancher les deux USB au Mac. Désactiver temporairement le Bluetooth du Mac
   pour vérifier que la saisie passe bien par USB. Cela ne coupe pas le BLE entre moitiés.
4. Vérifier quelques lettres de chaque côté avant de tester les couches.

Le câble gauche transporte la saisie et l’alimentation, le droit fournit l’alimentation.
Sans batterie, retirer les deux USB éteint complètement le clavier.

## Parcourir la keymap

Sur chaque ordinateur, choisir le mode OS dans la keymap et garder une disposition
d’entrée française adaptée. L’OS n’est pas détecté automatiquement.

- Vérifier les 42 touches, les deux clics d’encodeur et leurs rotations dans les deux sens.
- Tester Shift droit avec une lettre gauche et les couches activées depuis l’autre moitié.
- Tester les accents et symboles : `é è ê ë à â ù û ç œ É À Ç`, puis `0123456789 () [] {} <> @ # | \ /`.
- Parcourir les couches primaire, secondaire, tertiaire et pavé numérique.
- Vérifier copier/coller et navigation avec Ctrl sous Windows, Command/Option sous macOS.
- Vérifier les deux couches de jeu sous Windows, selon les actions documentées.

Le changement d’OS utilise `L1` et le clic d’encodeur droit : Windows → Linux → macOS
→ Windows. Relâcher `L1` entre deux changements. Les retours du pavé numérique et du
jeu vont actuellement à Windows, c’est une limite connue de `multi`.

## Changer d’ordinateur et redémarrer

Déplacer les deux USB vers Windows, puis refaire les vérifications dans le mode
Windows. Désactiver temporairement son Bluetooth pour confirmer la sortie USB.
Un débranchement complet redémarre le firmware et remet la keymap en mode Windows,
mais doit conserver l’appairage interne. Vérifier que les deux côtés refonctionnent
après reconnexion, sans firmware de reset.

En cas d’écart, noter l’ordinateur, la couche, la touche, le résultat attendu et
le résultat obtenu. Les [logs USB](logs-usb.md) sont disponibles pour le diagnostic.
