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

Le build est migré vers un snapshot ZMK préparant la 0.4, avec Zephyr 4.1.
Les 32 UF2 sont contrôlés. Les premiers essais sur V1 en `wire macos` confirment
saisie, Shift, encodeurs, copier/coller, reconnexion et accès aux couches.
La [validation détaillée](tests-wire.md) continue. Les essais Bluetooth et veille
ci-dessous concernent encore la version stable 0.3.0.

Les deux moitiés fonctionnent sur le Mac après réinitialisation et nouvel appairage,
avec leurs USB branchés pour l’alimentation et sur batterie seule, sans dongle.
Le profil d’essai confirme un réveil sans reset et la lecture des deux batteries.
Le premier appui réveille sans écrire. La gauche doit être éveillée pour recevoir
la droite, et le choix d’OS revient à Windows après veille profonde.
Le Clavier V1 sans batterie fonctionne en `wire` sur le MacBook Pro M4 Max avec
`macos`. Le flash et le fonctionnement du profil `windows` (gaming inclus) sont
aussi confirmés sur V1. Le Clavier V2 au travail utilise ses batteries et le
Bluetooth direct. Le parcours complet des keymaps reste à valider.

Le profil expérimental `wire auto` sélectionne macOS ou Windows avec gaming à la
connexion USB. La sélection et le retour du gaming sont confirmés sur V1 avec
le firmware instrumenté. Après correction de la signature Windows, la saisie et
la détection Windows puis macOS sont aussi confirmées sans port série, avec le
même firmware normal sur V1. Un démarrage irrégulier au branchement sur le Mac
est ensuite signalé, y compris de Mac vers Mac : la fiabilité reste à vérifier.
Les autres ordinateurs restent à tester.

La keymap historique reste disponible dans `multi`, avec changement d’OS au clavier.
Les profils fixes `windows` avec jeu, `macos` et `linux` se choisissent au build
et restent sur leur OS après redémarrage. Le flash automatique et le fonctionnement
de `wire macos` sont confirmés sur les deux moitiés. Les essais complets des profils restent à faire.
La priorité USB est différée après la détection automatique d’OS, pour viser une
adaptation automatique de la keymap et de la sortie. `dongle` reste au backlog. Les branches
historiques restent intactes et les variantes minimales sont des archives.

## Démarrer

Avec Make et Docker démarré, depuis la racine du dépôt :

```sh
make setup
make build bluetooth multi
make build wire macos
```

`make build` seul affiche l’aide. Pour compiler, préciser le matériel (`wire` ou
`bluetooth`), puis la keymap (`auto`, `multi`, `windows`, `macos` ou `linux`).
`auto` est réservé à `wire`.
Les deux choix sont obligatoires, aucun n’est sélectionné par défaut.
Windows inclut toujours le gaming. Ajouter `debug` à la fin pour les logs USB,
par exemple `make build bluetooth windows debug`.

Le [guide de build](build-local.md) détaille les prérequis et les fichiers UF2 obtenus.
Ces commandes ne flashent pas le clavier.
Sur macOS, `make flash wire macos right` sélectionne le dernier build correspondant
et demande confirmation avant sa copie. Voir [le guide de flash](flash-firmware.md).
`make help` affiche toutes les commandes. Sur macOS, `make bat` lit les deux batteries
et `make bat avg` calcule une moyenne par moitié sur 30 secondes.

## Documentation

- [Build local](build-local.md) : commandes, environnement et résultats.
- [Logs USB](logs-usb.md) : firmwares de diagnostic et console de chaque moitié.
- [Matériel et connexions](configuration-materielle.md) : cartes, rôles et limites.
- [Essais wire](tests-wire.md) : vérifier le clavier sans batterie sur Mac et Windows.
- [Latence](latence.md) : diagnostic rapide, mesures possibles et limites des logs USB.
- [Migration ZMK](migration-zmk.md) : versions, nouveautés, ruptures et validation avant flash.
- [Capture USB Windows](capture-usb-windows.md) : observer le firmware normal sans port série.
- [Détection d’OS](detection-os.md) : préparer et comparer les captures de connexion USB, puis BLE.
- [Firmwares stables](firmwares-stables.md) : récupération du snapshot 0.3 depuis l’historique Git.
- [Veille et réveil](veille.md) : états d’inactivité et reprise à la frappe.
- [Flash du firmware](flash-firmware.md) : copie des UF2, bootloader et messages du Finder.
- [Appairage Bluetooth](appairage-bluetooth.md) : clés de connexion, reset et diagnostic.
- [Batteries](batteries.md) : mesure actuelle et affichage des deux niveaux.
- [Keymap automatique USB](keymap-auto.md) : macOS ou Windows avec gaming, sélection à la connexion.
- [Keymap](keymap.md) : organisation, couches et sept schémas de touches.
- [TODO et idées](TODO.md) : suivi des tâches restantes.
