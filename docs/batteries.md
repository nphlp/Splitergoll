# Batteries des deux moitiés

Configuration étudiée : ZMK 0.3.0. La saisie sur les deux batteries est validée.
La remontée de leurs deux niveaux reste une amélioration à décider.

## État actuel

Chaque moitié possède son capteur et `CONFIG_ZMK_BATTERY_REPORTING=y`.
La gauche est la seule connectée au Mac. Son service Battery Service annonce
sa mesure locale, avec un intervalle configuré de 60 secondes.

Le build gauche n’active pas `ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING`.
Il ne récupère donc pas la mesure de droite et n’active pas son proxy vers l’hôte.
Le pourcentage affiché dans les réglages Bluetooth du Mac correspond au niveau
annoncé par la gauche, pas à la moyenne ou à la somme des deux batteries.

## Possibilité native de ZMK

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

## Essai proposé

1. Activer la récupération et le proxy uniquement pour la centrale Bluetooth, conserver `wire` et la keymap.
2. Compiler et contrôler le firmware, puis le flasher lors d’un essai explicite.
3. Vérifier qu’un lecteur BLE trouve les deux mesures et reçoit leurs mises à jour.
4. Observer ce que les réglages Bluetooth du Mac affichent réellement.
5. Si nécessaire, choisir un petit lecteur sur le Mac pour afficher gauche et droite séparément.

Un indicateur unique représentant la batterie la plus faible pourrait aussi être
étudié, mais ce calcul ne fait pas partie des deux options natives ci-dessus.
Il demanderait une adaptation du firmware et une décision sur les mesures absentes.

Aucun outil Mac ni changement de reporting n’a encore été choisi. Une prochaine
validation devra également vérifier l’autonomie et le sommeil du Mac, la documentation
signalant que les notifications batterie peuvent réveiller certains hôtes macOS.

## Sources

- [Mesure et affichage par défaut](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/features/battery.md).
- [Configuration batterie et limites hôtes](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/config/battery.md).
- [Options du split](https://github.com/zmkfirmware/zmk/blob/v0.3.0/app/src/split/bluetooth/Kconfig).
- [Service auxiliaire exposé par la centrale](https://github.com/zmkfirmware/zmk/blob/v0.3.0/app/src/split/bluetooth/central_bas_proxy.c).
