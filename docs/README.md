# Splitergoll

Clavier ergonomique split basé sur ZMK : 42 touches et deux encodeurs cliquables,
avec une keymap française complète et des adaptations Windows, macOS et Linux.

Ce fichier prépare le futur `README.md` et sert d’index à la documentation.
Il reste dans `docs/` pendant le chantier, le README actuel est conservé.

## Version en cours

Sur `unified-firmware` : build local et profils `wire` / `bluetooth`, avec la
keymap de `dongle`. La gauche envoie les touches par USB ou Bluetooth direct,
la droite communique avec elle en BLE. Le profil Bluetooth garde la saisie en
Bluetooth pendant la charge USB et active la veille sur batterie.
Les deux moitiés fonctionnent sur le Mac après réinitialisation et nouvel appairage,
avec leurs USB branchés pour l’alimentation et sur batterie seule, sans dongle.
La veille et le réveil restent à valider.

La keymap actuelle correspond à `multi`. Les profils `windows` avec jeu, `macos`,
`linux`, ainsi que priorité USB et `dongle`, viendront ensuite. Les branches
historiques restent intactes et les variantes minimales sont des archives.

## Démarrer

Avec Make et Docker démarré, depuis la racine du dépôt :

```sh
make setup
make build
```

`make build` compile les deux profils. Pour n’en compiler qu’un :
`make build wire` ou `make build bluetooth`.

Le [guide de build](build-local.md) détaille les prérequis et les fichiers UF2 obtenus.
Ces commandes ne flashent pas le clavier.

## Documentation

- [Build local](build-local.md) : commandes, environnement et résultats.
- [Matériel et connexions](configuration-materielle.md) : cartes, rôles et limites.
- [Veille et réveil](veille.md) : états d’inactivité et reprise à la frappe.
- [Flash du firmware](flash-firmware.md) : copie des UF2, bootloader et messages du Finder.
- [Appairage Bluetooth](appairage-bluetooth.md) : clés de connexion, reset et diagnostic.
- [Batteries](batteries.md) : mesure actuelle et affichage des deux niveaux.
- [Keymap](keymap.md) : organisation, couches et sept schémas de touches.
- [TODO et idées](TODO.md) : suivi des tâches restantes.
