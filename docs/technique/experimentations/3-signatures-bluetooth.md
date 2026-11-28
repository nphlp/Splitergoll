# 3. Hypothèses sur les signatures Bluetooth

[Documentation](../../README.md) › [Technique](../README.md) › [Expérimentations](../README.md#expérimentations) › 3. Hypothèses sur les signatures Bluetooth

[← 2. Gaming USB et mémorisation du mode](2-gaming-usb.md)

Ce relevé conserve les observations qui ont orienté le classifieur. Pour reproduire une capture, suivre le [diagnostic Bluetooth](../diagnostics/4-detection-os-bluetooth.md).

## Hypothèses du diagnostic

La première version du classifieur C a été intégrée au diagnostic `ble-trace` après tests sur les captures.
Il combine des recherches et lectures DIS pour macOS, et plusieurs opérations de
découverte/cache GATT pour Windows. Il n’utilise ni le nom du fichier, ni l’OS du
lecteur, ni l’emplacement Bluetooth pour décider.

Après flash de la gauche mise à jour, `make ble capture <port>` affiche une hypothèse
pour chaque session conservée. Le diagnostic récent sépare les connexions simultanées
et indique leur emplacement, sans le prendre pour une signature d’OS. Une lecture avec l’ancien firmware reste possible,
avec le message « Classification absente ». Aucun reset des appairages nécessaire.

| Capture | Hypothèse obtenue |
| --- | --- |
| Mac, première connexion et reconnexions | macOS |
| Windows, premier essai avec découverte complète | Windows |
| Windows, nouvel appairage avec découverte partielle | Windows avec le correctif, validé en rejeu et sur V1 |
| Windows, reconnexion avec seulement MTU et batterie | Inconnu |

Le correctif conserve les trois indices communs et accepte deux motifs de découverte :
recherche de services inclus, ou sondage du hash GATT avant puis après sa lecture
directe. Ce second motif reconnaît notre découverte partielle sans conclure à
Windows par absence de requêtes. Après flash, un nouvel appairage Windows confirme
ce motif et la sauvegarde de son OS, sans erreur ni perte. Le Mac reste appairé.

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
Le calcul de signature de session s’effectue pendant la lecture USB. Cette lecture
ne modifie pas la keymap, la destination des touches ou les appairages.
La mémoire apprend uniquement pendant l’acquisition d’un nouvel appairage.
Le sélecteur `auto` reprend son résultat aux reconnexions, indépendamment du lecteur USB.

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
reste à développer, voir [la mémoire d’OS](../firmware/7-memoire-os.md).

---

[Documentation](../../README.md) › [Technique](../README.md) › [Expérimentations](../README.md#expérimentations) › 3. Hypothèses sur les signatures Bluetooth

[← 2. Gaming USB et mémorisation du mode](2-gaming-usb.md)
