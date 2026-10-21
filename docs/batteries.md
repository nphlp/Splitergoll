# Batteries des deux moitiés

Les deux niveaux sont lus séparément sur le Mac. Après migration, l’utilisateur
relève **63 % à gauche et 67 % à droite sur V2, USB débranchés**.
Le menu Bluetooth macOS n’affiche qu’un niveau, celui de gauche.
La première validation du proxy sous ZMK 0.3.0 donnait gauche 63 %, droite 62 %.

## Recharger V2

Sur le schéma et le PCB du dépôt, chaque interrupteur est placé entre le « + »
de la batterie et la broche `B+` du contrôleur. **Laisser les deux interrupteurs
sur ON pour charger par USB.** OFF déconnecte aussi la batterie du circuit de charge,
alors que l’USB continue d’alimenter le clavier.

Cela décrit le câblage prévu dans `Design PCB/`. Après une charge USB avec ON,
les niveaux relevés sur batterie passent de 53/57 % à 63/67 % : une progression
cohérente avec une recharge des deux moitiés, sans mesure du courant ni de la capacité.
Le chargeur est intégré au contrôleur, voir la [documentation nice!nano](https://nicekeyboards.com/docs/nice-nano/faq/).

Un pourcentage lu sur OFF ne prouve pas le niveau de la batterie déconnectée.
Sur V2, **USB branché, la mesure remonte vers 5 V et affiche 100 %**, même si les
batteries ne sont pas pleines. Le pilote mesure l’alimentation VDDH du contrôleur,
influencée par l’USB. Ce 100 % ne permet pas de juger la recharge des cellules.

Pour comparer la charge, garder ON, débrancher les deux USB, appuyer sur une touche
de chaque moitié et laisser les valeurs se mettre à jour avant `make bat`.
Comparer des relevés dans les mêmes conditions, après une période de charge.
La commande relit les valeurs annoncées, elle ne déclenche pas une nouvelle mesure
ni un contrôle du courant de charge. Le pourcentage est une estimation par tension.

Le contrôleur distingue LED de charge et LED d’état utilisée par le bootloader.
Une lumière fixe avec USB et ON est compatible avec une charge en cours, sans
prouver sa progression. Sur les répliques, ne pas déduire la fonction d’une LED
de sa seule couleur ni de son comportement en mode bootloader.

## Mesurer pendant la charge

Un multimètre peut lire la tension aux bornes de la batterie pendant la charge,
USB branché et interrupteur ON. Cette tension n’est pas un pourcentage exact.
Le PCB ne relie pas la batterie à une entrée analogique dédiée. Le firmware actuel
lit VDDH et ne peut donc pas isoler sa tension sous USB par un simple calcul.

Une mesure automatique demanderait de vérifier le circuit exact des répliques,
puis, si nécessaire, d’ajouter une entrée analogique avec un pont diviseur adapté
ou une jauge de batterie. ZMK décrit [ces possibilités de mesure](https://zmk.dev/docs/hardware-integration/battery).
Le schéma du clavier ne décrit pas les composants internes des contrôleurs.

## État actuel

Chaque moitié possède son capteur et `CONFIG_ZMK_BATTERY_REPORTING=y`.
La gauche est la seule connectée au Mac. Son service Battery Service annonce
sa mesure locale. L’intervalle normal est de 60 secondes pendant l’activité.
**ZMK arrête les mesures en inactivité et les relance au réveil d’activité.**
Une lecture GATT ne relance pas elle-même la mesure. Pour comparer la charge,
appuyer sur une touche de chaque moitié avant le relevé.
Le [diagnostic ponctuel de V2](tests-batteries-veille.md) utilise des délais courts
et des logs USB, séparément des builds normaux.

`keyboard/config/profiles/bluetooth-left.conf` active maintenant la récupération de la
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
60 secondes pendant l’activité : plusieurs lectures peuvent donc porter sur la
même mesure, notamment après le passage en inactivité.

Le lecteur nécessite macOS et les outils de ligne de commande Xcode pour Swift.
Il se compile automatiquement au premier lancement, puis utilise son cache dans
`.local/tools/batteries/`. Après `make clear`, il se recompile. Aucun `make setup`
ni Docker n’est nécessaire. Autoriser l’accès Bluetooth si macOS le demande.
Le nom du clavier est lu dans la configuration de la centrale.

Le diagnostic utilise des lectures répétées, sans abonnement aux notifications.
Les notifications et l’affichage dans un lecteur permanent restent à tester.
Après un diagnostic aux délais courts, revenir au profil `bluetooth` normal.

Un indicateur unique représentant la batterie la plus faible pourrait aussi être
étudié, mais ce calcul ne fait pas partie des deux options natives ci-dessus.
Il demanderait une adaptation du firmware et une décision sur les mesures absentes.

La commande de lecture est versionnée. L’affichage des deux niveaux dans la barre
des menus est prévu dans le premier lot de l’[application macOS](application-macos.md),
avec une fenêtre d’informations. Couche, OS et réglages avancés viendront ensuite.
La validation devra vérifier
l’autonomie et le sommeil du Mac, la documentation
signalant que les notifications batterie peuvent réveiller certains hôtes macOS.

## Sources

- [Mesure et affichage par défaut](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/features/battery.md).
- [Configuration batterie et limites hôtes](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/config/battery.md).
- [Options du split](https://github.com/zmkfirmware/zmk/blob/v0.3.0/app/src/split/bluetooth/Kconfig).
- [Service auxiliaire exposé par la centrale](https://github.com/zmkfirmware/zmk/blob/v0.3.0/app/src/split/bluetooth/central_bas_proxy.c).
