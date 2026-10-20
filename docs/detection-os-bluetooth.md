# Identifier l’OS en Bluetooth

Objectif : reconnaître l’ordinateur connecté en Bluetooth, puis choisir sa keymap
automatiquement. On commence par comparer les échanges sur le Mac puis Windows.
**Le diagnostic actuel ne choisit aucun OS.** La keymap reste celle du build.

V1 fonctionne sans batterie : ses deux USB fournissent l’alimentation. La gauche
transmet les touches en Bluetooth et expose un port série USB pour la capture.
L’ordinateur qui alimente le clavier n’est pas nécessairement son hôte Bluetooth.

## Préparer le clavier

```sh
make build bluetooth macos ble-trace
make flash bluetooth macos right ble-trace
make flash bluetooth macos left ble-trace
```

Mettre seulement la moitié choisie en bootloader à chaque copie. Aucun
`settings_reset` : conserver les appairages. Les deux USB restent branchés après
les flashs. Seule la gauche ajoute le diagnostic, la droite reste normale.
Le même build servira sur Windows, sans tester la saisie de sa keymap macOS.

## Première capture sur le Mac

1. Connecter `Splitergoll` dans les réglages Bluetooth du Mac. Si une entrée existe
   mais échoue, garder cet état pour le diagnostic avant tout oubli ou reset.
2. Attendre dix secondes après la connexion, sans taper.
3. Identifier le port de gauche avec `ls /dev/cu.usbmodem*`.
4. Lancer `make ble-capture <port-série-gauche>` et conserver le chemin affiché.

La lecture récupère les événements antérieurs à son ouverture, puis ferme le port.
Les fichiers restent dans `.local/ble-traces/`. Un reset physique simple de gauche
vide cet historique RAM, pas les appairages. Ne pas le faire avant de lire une capture.
On comparera ensuite une reconnexion, où le cache de l’hôte peut réduire les lectures.
Les dix secondes sont une marge de protocole, pas un délai ajouté au clavier.

La première capture sur V1/Mac est complète : 49 événements sans perte ni rejet,
dont la découverte HID et une connexion chiffrée. Cela valide la collecte, pas
encore une signature exclusive de macOS.

Pour comparer la reconnexion, déconnecter puis reconnecter uniquement `Splitergoll`
dans les réglages Bluetooth, sans oublier l’appareil, débrancher les USB ou faire
reset. Attendre dix secondes, puis relire la gauche. Les sessions précédentes
restent dans la capture, séparées par leur numéro.
Sur le Mac testé, la reconnexion est automatique en environ une seconde :
laisser le clavier revenir, sans chercher à le maintenir déconnecté.
ZMK remet ses annonces connectables après une déconnexion, y compris volontaire,
tant que cet ordinateur reste la cible active. Ce comportement est
[documenté par ZMK](https://zmk.dev/docs/keymaps/behaviors/bluetooth#summary).

Sur le Mac testé, la reconnexion comporte 20 requêtes ATT contre 42 dans la première
session. La description HID n’est plus relue, alors que certaines informations du
clavier le sont encore. Une règle devra donc fonctionner aussi sans découverte complète.

## Puis Windows

Garder l’USB gauche au Mac et suivre [le guide de bascule](capture-bluetooth-mac.md).
L’[alternative Windows](capture-bluetooth-windows.md) reste disponible. Le diagnostic ajoute
un contrôle explicite des emplacements d’appairage, sans changer les touches.
Ce contrôle demande le firmware `ble-trace` récent sur la gauche. Une case garde
le Mac, une case libre accueillera Windows. Aucun effacement ni reset nécessaire.

Sur V1, le premier essai Windows est valide : 76 événements sans perte ni rejet,
dont 67 requêtes ATT, contre 42 à la première connexion Mac. Windows lit aussi
l’apparence du périphérique et des attributs de cache GATT que le Mac n’a pas lus
dans nos captures. Le MTU demandé est identique, il ne suffit donc pas à distinguer
les deux. Ces différences restent des pistes sur deux machines.
La reconnexion Windows déjà appairée est beaucoup plus réduite : six événements,
dont seulement deux requêtes ATT, pour le MTU et la batterie. La capture est complète,
sans perte ni rejet. Les lectures distinctives du premier appairage ont disparu.

| Requêtes ATT observées | Première connexion | Reconnexion |
| --- | ---: | ---: |
| Mac | 42 | 20 |
| Windows | 67 | 2 |

Les échanges actuels ne suffisent pas à identifier l’OS à chaque reconnexion.
Le fonctionnement prévu est décrit séparément : [sélection de l’ordinateur](selection-appareil.md)
et [mémoire et correction de l’OS](memoire-os.md). Le présent guide reste consacré
aux observations et à la validation des signatures.

## Refaire les essais

Les captures actuelles donnent les bonnes hypothèses sur nos deux ordinateurs.
Leur rejeu teste le code sur ces mêmes données, pas la répétabilité de l’appairage
ni la reconnaissance d’autres machines. Le premier essai Windows était bien un
nouvel appairage, l’absence d’ancien appairage au premier essai Mac n’est pas prouvée.

On peut garder les deux USB sur le Mac pendant que le clavier se connecte en
Bluetooth à Windows : [capture de Windows depuis le Mac](capture-bluetooth-mac.md).
Répéter des connexions existantes vérifie la reconnexion. Pour tester de nouveaux
appairages, il faudra préparer et autoriser l’effacement ciblé du seul lien testé
sur le clavier et l’ordinateur. Aucun reset global des réglages n’est nécessaire.

## Ce que l’on compare

L’ordre des requêtes de découverte, les UUID consultés, le MTU, les paramètres
proposés puis négociés, les fonctionnalités BLE et les changements de sécurité.
Aucune adresse, clé ou frappe n’est enregistrée. Aucune requête de version du
contrôleur n’est ajoutée, aucune décision d’OS n’est écrite en mémoire persistante.

Les [captures publiques de Cormoran](https://github.com/cormoran/zmk-feature-os-detection/blob/main/docs/fingerprints.md#ble)
montrent des pistes, mais aussi une ambiguïté Windows/Linux et un résultat macOS
par défaut trop fragile pour notre usage. Le contrôleur radio et les paramètres
ne prouvent pas un OS. Aucun classifieur ni mémoire d’OS ne pilote encore le firmware BLE.

## Hypothèses du diagnostic

Le classifieur C est intégré au seul firmware `ble-trace`, après tests sur les captures.
Il combine des recherches et lectures DIS pour macOS, et plusieurs opérations de
découverte/cache GATT pour Windows. Il n’utilise ni le nom du fichier, ni l’OS du
lecteur, ni l’emplacement Bluetooth pour décider.

Après flash de la gauche mise à jour, `make ble-capture <port>` affiche une hypothèse
pour chaque session conservée. Le diagnostic récent sépare les connexions simultanées
et indique leur emplacement, sans le prendre pour une signature d’OS. Une lecture avec l’ancien firmware reste possible,
avec le message « Classification absente ». Aucun reset des appairages nécessaire.

| Capture | Hypothèse obtenue |
| --- | --- |
| Mac, première connexion et reconnexions | macOS |
| Windows, découverte initiale | Windows |
| Windows, reconnexion avec seulement MTU et batterie | Inconnu |

Les preuves manquantes donnent « inconnu », les deux motifs complets donnent
« ambigu ». Les pertes, incohérences et erreurs de sécurité empêchent une conclusion.
Le rejeu de nos captures donne ces résultats sur nos deux ordinateurs, sans
validation sur Linux ou d’autres machines. Après flash de gauche, une lecture
réelle sur V1/Mac confirme l’hypothèse macOS : 27 événements, sans perte ni rejet.
Le diagnostic récent observe aussi les deux hôtes simultanés : 32 événements sans
perte ni rejet, macOS reconnu et Windows inconnu avec seulement une demande de MTU.
La sélection Mac → Windows → Mac est confirmée par les statuts, sans couper les
Bluetooth. Aucune frappe d’essai n’a été envoyée pendant cet aller-retour.
Ces essais conservent les appairages existants, ils ne valident pas la répétabilité
d’un premier appairage.
Le calcul s’effectue pendant la lecture USB, sans modifier la keymap, la destination
des touches ou les appairages. Il n’enregistre aucun OS en mémoire persistante.

## Pistes retenues

| Référence | Idée utile | Limite pour notre clavier |
| --- | --- | --- |
| [Cormoran](https://github.com/cormoran/zmk-feature-os-detection/blob/3052679f645b8fc1997275c2eaa6b86bdd55ca01/src/os_detection_ble.c) | Observer les lectures GATT et mémoriser le résultat par emplacement | Ses règles classeraient notre première capture Mac comme Android. Windows et Linux restent confondus. |
| [Blue2thprinting](https://github.com/darkmentorllc/Blue2thprinting) | Combiner GATT, métadonnées d’appairage SMP et informations de liaison | Outil de recherche, pas un classifieur embarqué prêt à intégrer. Les informations radio ne prouvent pas l’OS. |
| [Continuity](https://github.com/furiousMAC/continuity) | Reconnaître des annonces spécifiques à Apple | Il faudrait prouver qu’elles viennent de l’hôte connecté, pas d’un autre appareil à proximité. |

On privilégie l’étude des lectures GATT et des options publiques d’appairage,
sans collecter de clés ni modifier la sécurité. Leur combinaison reste à valider.
Un OS reconnu initialement pourrait être mémorisé pour les reconnexions pauvres
en échanges. Un résultat insuffisant doit rester inconnu, sans supposer macOS.

Le module [oskey](https://github.com/mentaldesk/oskey) illustre une sélection manuelle,
mais son état actuel ne persiste pas après extinction. Notre correction mémorisée
reste à développer, voir [la mémoire d’OS](memoire-os.md).
