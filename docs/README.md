# Documentation Splitergoll

Splitergoll est un clavier ergonomique en deux moitiés, avec 42 touches, deux
encodeurs cliquables et un firmware ZMK. V1 est alimenté par USB. V2 a une
batterie par moitié et peut fonctionner en Bluetooth sur batterie.

- [Installer le firmware](demarrer/02-compiler.md)
- Retrouver une touche sur [macOS](layouts-clavier/02-macos.md), [Windows](layouts-clavier/03-windows.md) ou [Linux](layouts-clavier/04-linux.md)
- [Connecter un ordinateur](utiliser-le-clavier/01-bluetooth.md) ou [vérifier les batteries](utiliser-le-clavier/02-batteries.md)

## Sommaire complet

### Démarrer

- [01. Identifier son matériel et ses connexions](demarrer/01-materiel.md)
- [02. Choisir et compiler son firmware](demarrer/02-compiler.md)
- [03. Flasher le clavier](demarrer/03-flasher.md)

### Layouts clavier

- [01. Tronc commun](layouts-clavier/01-tronc-commun.md)
- [02. macOS](layouts-clavier/02-macos.md)
- [03. Windows](layouts-clavier/03-windows.md)
- [04. Linux](layouts-clavier/04-linux.md)
- [05. Profil multi](layouts-clavier/05-multi.md)
- [06. Profil auto](layouts-clavier/06-auto.md)

### Utiliser le clavier

- [01. Appairer et changer d’ordinateur](utiliser-le-clavier/01-bluetooth.md)
- [02. Batteries et recharge](utiliser-le-clavier/02-batteries.md)
- [03. Veille et réveil](utiliser-le-clavier/03-veille-et-reveil.md)
- [04. Application macOS](utiliser-le-clavier/04-application-macos.md)
- [05. Application Windows](utiliser-le-clavier/05-application-windows.md)

### Référence technique

#### Firmware

- [01. Build local](technique/firmware/01-build-local.md)
- [02. Matériel et profils de connexion](technique/firmware/02-materiel-et-profils.md)
- [03. Architecture des keymaps](technique/firmware/03-architecture-keymaps.md)
- [04. Règles d’appairage](technique/firmware/04-gestion-appairage.md)
- [05. Reconnexion Bluetooth](technique/firmware/05-reconnexion-bluetooth.md)
- [06. Sélection et mémoire de l’ordinateur](technique/firmware/06-selection-appareil.md)
- [07. Mémoire et correction de l’OS](technique/firmware/07-memoire-os.md)
- [08. Destinations de saisie USB et Bluetooth](technique/firmware/08-destinations-saisie.md)
- [09. Identité de l’ordinateur](technique/firmware/09-identite-ordinateur.md)
- [10. Mesure des batteries](technique/firmware/10-mesure-batteries.md)
- [11. États d’alimentation](technique/firmware/11-etats-alimentation.md)

#### Diagnostics

- [01. Lire les logs USB](technique/diagnostics/01-logs-usb.md)
- [02. Diagnostiquer la détection d’OS en USB](technique/diagnostics/02-detection-os-usb.md)
- [03. Capturer l’USB sur Windows](technique/diagnostics/03-capture-usb-windows.md)
- [04. Diagnostiquer la détection d’OS en Bluetooth](technique/diagnostics/04-detection-os-bluetooth.md)
- [05. Capturer le Bluetooth depuis le Mac](technique/diagnostics/05-capture-bluetooth-mac.md)
- [06. Capturer le Bluetooth sur Windows](technique/diagnostics/06-capture-bluetooth-windows.md)
- [07. Vérifier l’identité USB et Bluetooth](technique/diagnostics/07-identite-usb-bluetooth.md)
- [08. Lire le journal d’activité](technique/diagnostics/08-journal-activite.md)
- [09. Examiner la latence](technique/diagnostics/09-latence.md)
- [10. Dépanner ou réinitialiser l’appairage](technique/diagnostics/10-depannage-bluetooth.md)

#### Validation

- [01. Tests automatisés](technique/validation/01-tests-automatises.md)
- [02. Essais wire sur Mac et Windows](technique/validation/02-tests-wire.md)
- [03. Essais du profil auto en USB](technique/validation/03-tests-auto-usb.md)
- [04. Essais du profil auto en Bluetooth](technique/validation/04-tests-auto-bluetooth.md)
- [05. Essais de connexion Bluetooth](technique/validation/05-tests-connexions-bluetooth.md)
- [06. Essais de reprise Bluetooth](technique/validation/06-tests-reprise-bluetooth.md)
- [07. Essais de recherche sans hôte](technique/validation/07-tests-recherche-bluetooth.md)
- [08. Essais de mémoire d’OS](technique/validation/08-tests-memoire-os.md)
- [09. Essais batteries et veille](technique/validation/09-tests-batteries-veille.md)
- [10. Essai de sélection V1 et V2 sur Mac](technique/validation/10-tests-selection-clavier-macos.md)

#### Expérimentations

- [01. Priorité USB sur le même ordinateur](technique/experimentations/01-priorite-usb.md)
- [02. Gaming USB et mémorisation du mode](technique/experimentations/02-gaming-usb.md)
- [03. Hypothèses sur les signatures Bluetooth](technique/experimentations/03-signatures-bluetooth.md)

#### Historique

- [01. Récupérer les firmwares ZMK 0.3.0](technique/historique/01-firmwares-stables.md)
- [02. Migration ZMK et Zephyr](technique/historique/02-migration-zmk.md)
- [03. Bilan des essais V1 et V2](technique/historique/03-bilan-des-essais.md)

#### Suivi

- [01. Reprendre le développement](technique/suivi/01-reprendre.md)
- [02. Planifier les essais selon le matériel](technique/suivi/02-planification.md)
- [03. TODO et idées](technique/suivi/03-todo.md)
