# Keymap automatique Bluetooth

`bluetooth auto` réunit macOS et Windows avec gaming. Il utilise l’OS mémorisé
de l’appareil Bluetooth sélectionné. L’ordinateur qui alimente l’USB ne détermine
pas la keymap. Le premier flash sur V1 est effectué et sa mémoire Mac est retrouvée.
La saisie et la bascule des couches Mac/Windows restent à valider.

```sh
make build bluetooth auto
```

## Fonctionnement

Au premier appairage, le clavier observe les services demandés et
[mémorise l’OS reconnu](memoire-os.md). Aux reconnexions et après redémarrage,
il reprend exclusivement cet OS. Les appairages et enregistrements déjà créés
par notre diagnostic sont compatibles, aucun reset n’est nécessaire.

Une fois la connexion sécurisée et sa mémoire vérifiée, les couches du bon OS
deviennent disponibles. Les affectations sont partagées avec les profils fixes,
y compris le pavé numérique et le gaming Windows.

Un appareil sans OS mémorisé, une erreur ou un résultat inconnu laisse les touches
en attente, sans choisir Windows par défaut. La correction manuelle viendra plus tard.
La fin des touches maintenues et des comportements différés précède toute bascule.
Aucun délai fixe de détection n’est ajouté au démarrage.

Depuis la base, **L1 + clic encodeur droit**, puis relâchement, sélectionne le prochain ordinateur
actuellement connecté. Un appareil seulement appairé est ignoré. Ce raccourci est
préparé, son essai matériel reste à faire. La [sélection automatique d’appareil](selection-appareil.md)
sera l’étape suivante.
Le profil garde la saisie Bluetooth pendant la charge USB et ses délais de veille normaux.

## Premier essai

Commencer avec le port de contrôle, sans changer les appairages :

```sh
make build bluetooth auto ble-trace
make flash bluetooth auto right ble-trace
make flash bluetooth auto left ble-trace
```

Flasher chaque moitié séparément. Garder l’USB gauche au Mac, même pour Windows.
Utiliser le clavier intégré du Mac pour piloter les essais :

```sh
make ble-profile /dev/cu.usbmodem1101 status
make ble-memory /dev/cu.usbmodem1101
make ble-profile /dev/cu.usbmodem1101 1
```

Adapter le port et l’index au statut réel, ici Mac 0 et Windows 1. Une fois connecté,
le clic droit doit afficher Windows. Tester Control, le gaming et le retour à sa base.
Revenir au Mac avec le même sélecteur sur 0, puis vérifier l’affichage macOS et Command.
Ne pas taper sur Splitergoll pendant les changements de destination.

Le diagnostic de session Windows peut rester inconnu en reconnexion : la décision
vient de `stored=windows`, pas d’une nouvelle reconnaissance. Les essais avec
touches maintenues, redémarrage et firmware normal complètent cette validation.
V2 sur batterie, Linux et d’autres ordinateurs restent à tester.

Lorsque le PC est disponible, vérifier le raccourci avec les deux hôtes connectés :
maintenir L1, cliquer à droite, tout relâcher, puis contrôler l’OS au clic simple.
Refaire le geste pour revenir au Mac. Avec le Mac seul connecté, le geste ne doit
rien changer. Si Windows reste déconnecté, sa reconnexion doit être établie avant
ce test, le raccourci ne la déclenche pas.
