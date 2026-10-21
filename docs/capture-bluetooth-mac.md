# Capturer le Bluetooth depuis le Mac

La gauche reste branchée en USB au Mac. La droite peut être alimentée par le PC.
Le clavier peut communiquer en Bluetooth avec les deux ordinateurs : leurs traces
sont séparées, indépendamment de celui qui reçoit les frappes.
Aucun script Windows ni transfert de fichiers n’est nécessaire.

## Préparer

Compiler puis flasher uniquement la gauche avec le nouveau diagnostic :

```sh
make build bluetooth macos ble-trace
make flash bluetooth macos left ble-trace
```

La droite est inchangée. Aucun `settings_reset`, oubli d’appareil ou arrêt du
Bluetooth. La keymap reste macOS, même lorsque Windows reçoit les touches.

## Lire et basculer

Les essais actuels utilisent Mac en emplacement 0 et Windows en 1. Ces index
viennent de nos appairages, ils ne sont pas imposés par le firmware.
`make port` explique comment trouver le port de gauche. Adapter les exemples au nom trouvé.

```sh
make ble profiles /dev/cu.usbmodem1101
make ble capture /dev/cu.usbmodem1101
make ble memory /dev/cu.usbmodem1101
```

`paired=1` indique un appairage enregistré, `connected=1` une liaison active.
`active` désigne la destination des frappes. La capture indique une hypothèse d’OS,
un emplacement et l’état connecté/déconnecté pour chaque session.
**Lire ne change pas la destination.**

Pour envoyer les prochaines frappes à Windows, puis revenir au Mac :

```sh
make ble select /dev/cu.usbmodem1101 1
make ble select /dev/cu.usbmodem1101 0
```

Exécuter une seule sélection à la fois et vérifier son résultat avant de taper.
Une sélection conserve les liaisons existantes. Si la cible appairée est déconnectée,
ZMK relance ses annonces pour permettre sa reconnexion. Si nécessaire, reconnecter
son entrée dans les réglages de l’ordinateur, sans l’oublier.
`busy` demande de relâcher les touches et d’attendre une seconde avant de réessayer.
Cette garde concerne la saisie, pas la présence d’un autre ordinateur connecté.

Après une nouvelle connexion, attendre dix secondes puis relancer `ble capture`.
C’est une marge d’observation, pas un délai du classifieur. Avant une capture
réalisée par l’agent, annoncer que le clavier est prêt. Les fichiers restent dans
`.local/ble-traces/`. Leur préfixe `macos` désigne le collecteur USB, pas l’hôte BLE.

## Ce que la capture conserve

Le tampon RAM contient au maximum 256 événements, environ 8 Kio, toutes sessions
confondues depuis le démarrage. Une fois plein, il compte les pertes et le lecteur
refuse la capture incomplète. Un redémarrage vide cet historique, pas les appairages.
Les frappes et les clés Bluetooth ne sont pas enregistrées.
La capture calcule son hypothèse dans la gauche lors de la lecture USB. L’acquisition
initiale pour la mémoire fonctionne aussi sans lecteur ouvert, dans un worker distinct.
Ces fonctions sont propres à `ble-trace`, absentes du firmware normal. Les anciens
dumps restent lisibles. Voir [la mémoire](memoire-os.md) et [ses essais](tests-memoire-os.md).

Une bascule ne déclenche pas une nouvelle découverte de services. Windows peut
rester « inconnu » lors d’une reconnexion utilisant son cache. Tester un nouvel
appairage demande un effacement ciblé autorisé, jamais un reset global des réglages.
