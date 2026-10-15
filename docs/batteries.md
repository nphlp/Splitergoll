# Batteries des deux moitiés

Configuration étudiée : ZMK 0.3.0. La saisie sur les deux batteries est validée.
Lors des essais aux délais courts, un lecteur CoreBluetooth sur le Mac a lu **63 % à gauche
et 62 % à droite**, plusieurs fois. Les deux mesures remontent correctement.
Le menu Bluetooth macOS affiche seulement 63 %, la mesure principale de gauche.

## État actuel

Chaque moitié possède son capteur et `CONFIG_ZMK_BATTERY_REPORTING=y`.
La gauche est la seule connectée au Mac. Son service Battery Service annonce
sa mesure locale. L’intervalle est de 60 secondes. Il avait été raccourci à
5 secondes pendant les essais, cette configuration temporaire est retirée.

`config/profiles/bluetooth-left.conf` active maintenant la récupération de la
mesure droite et son proxy vers l’hôte. La droite garde son rôle de périphérique.
Le service principal annonce toujours la gauche, le service auxiliaire la droite.
Il ne calcule ni moyenne ni somme des deux batteries.

## Configuration

Sur la centrale du profil Bluetooth uniquement :

```conf
CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y
CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_PROXY=y
```

La première option récupère la batterie droite sur la liaison interne.
La seconde expose cette mesure aux hôtes dans un service Battery Service auxiliaire.
Les deux mesures restent distinctes. Il ne s’agit pas de connecter la droite au Mac.

La batterie principale utilise le service GATT `0x180F` et la caractéristique `0x2A19`.
Le proxy ajoute un autre service de batterie, avec le périphérique identifié comme
`Peripheral 0` et une description de présentation auxiliaire.

La documentation ZMK précise que les systèmes hôtes n’affichent pas tous plusieurs
batteries. Activer ces options ne garantit donc pas deux indicateurs dans les réglages
macOS. Il faut distinguer les données exposées et leur présentation par le Mac.

## Affichage sur le Mac

Les deux services ont été lus et identifiés comme principale et auxiliaire.
Le lecteur du dépôt affiche les deux niveaux. Les options ZMK activées n’ajoutent
pas de deuxième indicateur dans le menu natif lors de cet essai.

Depuis la racine du dépôt, avec les deux moitiés éveillées et le clavier connecté au Mac :

```sh
make bat
make bat avg
```

`make bat` affiche les deux derniers niveaux annoncés, sans attendre 30 secondes.
`make bat avg` lit toutes les 5 secondes pendant 30 secondes et affiche la moyenne
de chaque moitié, séparément. Le profil normal mesure les batteries toutes les
60 secondes : plusieurs lectures peuvent donc porter sur la même mesure.

Le lecteur nécessite macOS et les outils de ligne de commande Xcode pour Swift.
Il se compile automatiquement au premier lancement, puis utilise son cache dans
`.local/tools/batteries/`. Après `make clear`, il se recompile. Aucun `make setup`
ni Docker n’est nécessaire. Autoriser l’accès Bluetooth si macOS le demande.
Le nom du clavier est lu dans la configuration de la centrale.

Le diagnostic utilise des lectures répétées, sans abonnement aux notifications.
Les notifications et l’affichage dans un lecteur permanent restent à tester.
Les deux moitiés ont retrouvé le profil `bluetooth` aux délais normaux après les essais.

Un indicateur unique représentant la batterie la plus faible pourrait aussi être
étudié, mais ce calcul ne fait pas partie des deux options natives ci-dessus.
Il demanderait une adaptation du firmware et une décision sur les mesures absentes.

La commande de lecture est versionnée. Un affichage permanent dans la barre des
menus reste une amélioration possible. La validation devra également vérifier
l’autonomie et le sommeil du Mac, la documentation
signalant que les notifications batterie peuvent réveiller certains hôtes macOS.

## Sources

- [Mesure et affichage par défaut](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/features/battery.md).
- [Configuration batterie et limites hôtes](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/config/battery.md).
- [Options du split](https://github.com/zmkfirmware/zmk/blob/v0.3.0/app/src/split/bluetooth/Kconfig).
- [Service auxiliaire exposé par la centrale](https://github.com/zmkfirmware/zmk/blob/v0.3.0/app/src/split/bluetooth/central_bas_proxy.c).
