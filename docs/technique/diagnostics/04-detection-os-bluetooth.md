# 04. Diagnostiquer la détection d’OS en Bluetooth

[Documentation](../../README.md) › [Référence technique](../../README.md#référence-technique) › [Diagnostics](../../README.md#diagnostics) › 04. Diagnostiquer la détection d’OS en Bluetooth

[← 03. Capturer l’USB sur Windows](03-capture-usb-windows.md) · [05. Capturer le Bluetooth depuis le Mac →](05-capture-bluetooth-mac.md)

Objectif : reconnaître l’ordinateur connecté en Bluetooth, puis choisir sa keymap
automatiquement. On commence par comparer les échanges sur le Mac puis Windows.
**Une keymap fixe reste fixe.** Le diagnostic partage le moteur de mémoire avec
[la keymap automatique Bluetooth](../validation/04-tests-auto-bluetooth.md). Les observations et
la [validation de mémoire](../validation/08-tests-memoire-os.md) restent distinctes de la sélection des couches.

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
4. Lancer `make ble capture <port-série-gauche>` et conserver le chemin affiché.

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

Garder l’USB gauche au Mac et suivre [le guide de bascule](05-capture-bluetooth-mac.md).
L’[alternative Windows](06-capture-bluetooth-windows.md) reste disponible. Le diagnostic ajoute
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
Le fonctionnement prévu est décrit séparément : [sélection de l’ordinateur](../firmware/06-selection-appareil.md)
et [mémoire et correction de l’OS](../firmware/07-memoire-os.md). Le présent guide reste consacré
aux observations et à la validation des signatures.

## Refaire les essais

Les captures actuelles donnent les bonnes hypothèses sur nos deux ordinateurs.
Leur rejeu teste le code sur ces mêmes données, pas la répétabilité de l’appairage
ni la reconnaissance d’autres machines. Le premier essai Windows était bien un
nouvel appairage, l’absence d’ancien appairage au premier essai Mac n’est pas prouvée.

On peut garder les deux USB sur le Mac pendant que le clavier se connecte en
Bluetooth à Windows : [capture de Windows depuis le Mac](05-capture-bluetooth-mac.md).
Répéter des connexions existantes vérifie la reconnexion. Pour tester de nouveaux
appairages, il faudra préparer et autoriser l’effacement ciblé du seul lien testé
sur le clavier et l’ordinateur. Aucun reset global des réglages n’est nécessaire.

## Ce que l’on compare

L’ordre des requêtes de découverte, les UUID consultés, le MTU, les paramètres
proposés puis négociés, les fonctionnalités BLE et les changements de sécurité.
Aucune adresse, clé ou frappe n’est enregistrée. Aucune requête de version du
contrôleur n’est ajoutée. Le diagnostic récent sauvegarde uniquement la décision
de l’acquisition initiale, voir [la mémoire d’OS](../firmware/07-memoire-os.md).

Les [captures publiques de Cormoran](https://github.com/cormoran/zmk-feature-os-detection/blob/main/docs/fingerprints.md#ble)
montrent des pistes, mais aussi une ambiguïté Windows/Linux et un résultat macOS
par défaut trop fragile pour notre usage. Le contrôleur radio et les paramètres
ne prouvent pas un OS. `bluetooth auto` utilise désormais la mémoire pour choisir
les couches. Les [essais du profil auto](../validation/04-tests-auto-bluetooth.md) précisent ce qui a été observé.

[Hypothèses, captures de référence et résultats](../experimentations/03-signatures-bluetooth.md).

---

[Documentation](../../README.md) › [Référence technique](../../README.md#référence-technique) › [Diagnostics](../../README.md#diagnostics) › 04. Diagnostiquer la détection d’OS en Bluetooth

[← 03. Capturer l’USB sur Windows](03-capture-usb-windows.md) · [05. Capturer le Bluetooth depuis le Mac →](05-capture-bluetooth-mac.md)
