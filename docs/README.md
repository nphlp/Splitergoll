# Splitergoll

Clavier ergonomique split basé sur ZMK : 42 touches et deux encodeurs cliquables,
avec une keymap française complète et des adaptations Windows, macOS et Linux.

Ce fichier prépare le futur `README.md` et sert d’index à la documentation.
Il reste dans `docs/` pendant le chantier, le README actuel est conservé.

## Version en cours

La branche de travail est `unified-firmware`, avec un snapshot ZMK préparant la 0.4
et Zephyr 4.1. Les moitiés communiquent en BLE, la gauche envoie la saisie à l’ordinateur.

| Fonction | État |
| --- | --- |
| Build local, flash guidé et diagnostics USB | Disponibles, parcours de flash validé sur V1 |
| Keymaps `multi`, `windows` avec jeu, `macos`, `linux` | Disponibles, `multi` garde la bascule manuelle d’OS |
| Détection USB et keymap `wire auto` | Mac/Windows validés sur V1 en firmware normal |
| Détection BLE initiale, mémoire et restauration par appareil | Mac/Windows validés sur V1, sans nouvelle détection aux reconnexions |
| `bluetooth auto` et L1 + clic encodeur droit | Bascule entre appareils connectés et bonnes keymaps confirmées sur V1 en `ble-trace` |
| Reconnexion des hôtes connus pendant dix secondes | Intégrée au nouveau `bluetooth auto`, validation matérielle à faire |
| Recherche avec bascule automatique et transitions Gaming | Moteur C développé et testé, intégration au firmware à faire |
| Arrêt des annonces et déconnexion du Mac en Gaming USB | Spécifiés, non implémentés sur le clavier |
| Destinations mixtes USB/BLE avec priorité USB par ordinateur | Spécifiées, association des identités USB/BLE à résoudre |
| Retour à l’ancien appareil dans les cinq secondes au réveil | Spécifié, non implémenté |
| Correction manuelle persistante d’un OS mal reconnu | Prévue, non implémentée |

Les heuristiques d’OS sont expérimentales, elles ne sont pas validées sur tous
les ordinateurs. Linux automatique et le dongle restent au backlog.
L’attente de bascule BLE ressentie d’environ deux secondes reste à mesurer et réduire.

V1 est sans batterie, V2 possède une batterie par moitié. La lecture des deux niveaux
sur batterie est revalidée après migration. Sous USB, le 100 % affiché sur V2 reflète
l’alimentation du contrôleur et ne prouve pas une batterie pleine.
Les essais courts après migration retrouvent une reprise immédiate après inactivité
et une reprise en 2 à 5 secondes après sollicitation de la gauche en veille profonde.
La droite seule ne réveille pas la centrale. L’autonomie, la première frappe et les
réveils par chaque touche restent à vérifier dans le [TODO](TODO.md).

La priorité est de valider les reconnexions connues puis de traiter l’accueil des
nouveaux appareils. Vient ensuite le firmware USB + BLE commun,
avec Gaming USB et le choix de destination. Les noms des futurs profils ne sont pas
encore fixés. Le [cadrage de l’appairage](gestion-appairage.md) distingue règles
prévues et moteur déjà développé. Les branches historiques et les variantes minimales
restent des archives.

## Démarrer

Avec Make et Docker démarré, depuis la racine du dépôt :

```sh
make setup
make build bluetooth multi
make build wire macos
```

`make build` seul affiche l’aide. Pour compiler, préciser le matériel (`wire` ou
`bluetooth`), puis la keymap (`auto`, `multi`, `windows`, `macos` ou `linux`).
`auto` utilise la signature USB en `wire`, ou l’OS mémorisé par appareil en `bluetooth`.
Les deux choix sont obligatoires, aucun n’est sélectionné par défaut.
Windows inclut toujours le gaming. Ajouter `debug` à la fin pour les logs USB,
par exemple `make build bluetooth windows debug`.

Le [guide de build](build-local.md) détaille les prérequis et les fichiers UF2 obtenus.
Ces commandes ne flashent pas le clavier.
Sur macOS, `make flash wire macos right` sélectionne le dernier build correspondant
et demande confirmation avant sa copie. Voir [le guide de flash](flash-firmware.md).
`make help` affiche toutes les commandes. Sur macOS, `make bat read` lit les deux batteries
et `make bat avg` calcule une moyenne par moitié sur 30 secondes.

## Documentation

- [Planification selon le matériel](planification.md) : choisir les chantiers réalisables avec Mac, Windows, V1 ou V2.
- [Build local](build-local.md) : commandes, environnement et résultats.
- [Logs USB](logs-usb.md) : firmwares de diagnostic et console de chaque moitié.
- [Matériel et connexions](configuration-materielle.md) : cartes, rôles et limites.
- [Essais wire](tests-wire.md) : vérifier le clavier sans batterie sur Mac et Windows.
- [Latence](latence.md) : diagnostic rapide, mesures possibles et limites des logs USB.
- [Migration ZMK](migration-zmk.md) : versions, nouveautés, ruptures et validation avant flash.
- [Capture USB Windows](capture-usb-windows.md) : observer le firmware normal sans port série.
- [Détection d’OS](detection-os.md) : préparer et comparer les captures de connexion USB, puis BLE.
- [Diagnostic Bluetooth](detection-os-bluetooth.md) : comparer Mac et Windows sans sélection automatique.
- [Sélection de l’ordinateur](selection-appareil.md) : comportement Bluetooth prévu, bascule et reprise après veille.
- [Reconnexion Bluetooth](reconnexion-bluetooth.md) : fenêtre de dix secondes pour les appareils connus, sans nouvel appairage.
- [Destinations de saisie](destinations-saisie.md) : futur choix USB/Bluetooth et priorité par ordinateur.
- [Gestion de l’appairage](gestion-appairage.md) : définir l’ouverture aux nouveaux appareils et les reconnexions.
- [Mémoire de l’OS](memoire-os.md) : acquisition initiale et sauvegarde expérimentale, correction manuelle prévue.
- [Essais de mémoire](tests-memoire-os.md) : appairage, reconnexion et redémarrage sur Mac puis Windows.
- [Capture Bluetooth depuis le Mac](capture-bluetooth-mac.md) : lire les échanges avec Mac ou Windows en gardant les USB sur le Mac.
- [Capture Bluetooth Windows](capture-bluetooth-windows.md) : conserver l’appairage Mac et lire les échanges du PC.
- [Firmwares stables](firmwares-stables.md) : récupération du snapshot 0.3 depuis l’historique Git.
- [Veille et réveil](veille.md) : états d’inactivité et reprise à la frappe.
- [Essais batteries et veille](tests-batteries-veille.md) : diagnostic ponctuel de V2, délais courts et logs USB.
- [Flash du firmware](flash-firmware.md) : copie des UF2, bootloader et messages du Finder.
- [Appairage Bluetooth](appairage-bluetooth.md) : clés de connexion, reset et diagnostic.
- [Batteries](batteries.md) : mesure actuelle et affichage des deux niveaux.
- [Application macOS](application-macos.md) : lancer le lecteur des deux batteries dans une fenêtre et la barre des menus, puis le valider sur V2.
- [Keymap automatique USB](keymap-auto.md) : macOS ou Windows avec gaming, sélection à la connexion.
- [Keymap automatique Bluetooth](keymap-auto-bluetooth.md) : OS mémorisé et bascule Mac/Windows confirmés sur V1 en diagnostic.
- [Keymap](keymap.md) : organisation, couches et sept schémas de touches.
- [TODO et idées](TODO.md) : suivi des tâches restantes.
