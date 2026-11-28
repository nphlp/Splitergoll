# 2. Batteries et recharge

[Documentation](../README.md) › [Utilisation](../README.md#utilisation) › 2. Batteries et recharge

[← 1. Appairer et changer d’ordinateur](1-bluetooth.md) · [3. Veille et réveil →](3-veille-et-reveil.md)

V2 possède une batterie par moitié. Pour recharger les deux, mettre les deux
interrupteurs sur **ON** et brancher les deux USB. Sur **OFF**, la batterie de
la moitié concernée est isolée tandis que l’USB peut alimenter son contrôleur.

## Lire les niveaux

1. Garder les interrupteurs sur **ON** et retirer les deux USB.
2. Réveiller le clavier en appuyant sur une touche de chaque moitié.
3. Attendre l’actualisation dans l’[app macOS](4-application-macos.md) ou
   l’[app Windows](5-application-windows.md).

Sous USB, **100 % ou un éclair peut simplement refléter l’alimentation USB**.
Sur Mac, un tiret indique une valeur indisponible ou égale à zéro. Le menu
Bluetooth de macOS n’affiche que la valeur de gauche.

Depuis le dépôt, `make bat read` lit les deux niveaux sur Mac.
`make bat avg` affiche une moyenne par moitié sur 30 secondes.

[Mesure et affichage en détail](../technique/firmware/10-mesure-batteries.md)
· [Essais batteries et veille](../technique/validation/9-tests-batteries-veille.md)

---

[Documentation](../README.md) › [Utilisation](../README.md#utilisation) › 2. Batteries et recharge

[← 1. Appairer et changer d’ordinateur](1-bluetooth.md) · [3. Veille et réveil →](3-veille-et-reveil.md)
