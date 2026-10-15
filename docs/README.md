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
Le profil d’essai confirme un réveil sans reset et la lecture des deux batteries.
Le premier appui réveille sans écrire. La gauche doit être éveillée pour recevoir
la droite, et le choix d’OS revient à Windows après veille profonde.
Le profil `wire` fonctionne aussi sur le Mac avec le clavier PC sans batterie.
Le parcours complet de la keymap et les essais Windows restent à valider.

La keymap historique reste disponible dans `multi`, avec changement d’OS au clavier.
Les profils fixes `windows` avec jeu, `macos` et `linux` se choisissent au build
et restent sur leur OS après redémarrage. Leurs essais matériels restent à faire.
La priorité USB et `dongle` viendront ensuite. Les branches
historiques restent intactes et les variantes minimales sont des archives.

## Démarrer

Avec Make et Docker démarré, depuis la racine du dépôt :

```sh
make setup
make build bluetooth
make build wire macos
```

`make build` seul affiche les profils disponibles. Pour compiler, choisir
`make build wire` ou `make build bluetooth`.
Ajouter `debug` pour obtenir les logs USB, par exemple `make build bluetooth debug`.
Ajouter une keymap avant `debug` pour choisir `multi`, `windows`, `macos` ou `linux`.
Sans ce choix, la keymap reste `multi`.

Le [guide de build](build-local.md) détaille les prérequis et les fichiers UF2 obtenus.
Ces commandes ne flashent pas le clavier.
`make help` affiche toutes les commandes. Sur macOS, `make bat` lit les deux batteries
et `make bat avg` calcule une moyenne par moitié sur 30 secondes.

## Documentation

- [Build local](build-local.md) : commandes, environnement et résultats.
- [Logs USB](logs-usb.md) : firmwares de diagnostic et console de chaque moitié.
- [Matériel et connexions](configuration-materielle.md) : cartes, rôles et limites.
- [Essais wire](tests-wire.md) : vérifier le clavier sans batterie sur Mac et Windows.
- [Veille et réveil](veille.md) : états d’inactivité et reprise à la frappe.
- [Flash du firmware](flash-firmware.md) : copie des UF2, bootloader et messages du Finder.
- [Appairage Bluetooth](appairage-bluetooth.md) : clés de connexion, reset et diagnostic.
- [Batteries](batteries.md) : mesure actuelle et affichage des deux niveaux.
- [Keymap](keymap.md) : organisation, couches et sept schémas de touches.
- [TODO et idées](TODO.md) : suivi des tâches restantes.
