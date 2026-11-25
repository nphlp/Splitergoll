# 04. Application macOS

[Documentation](../README.md) › [Utiliser le clavier](../README.md#utiliser-le-clavier) › 04. Application macOS

[← 03. Veille et réveil](03-veille-et-reveil.md) · [05. Application Windows →](05-application-windows.md)

L’app affiche les batteries gauche et droite d’un Splitergoll connecté en
Bluetooth. Sur V2, la lecture des deux niveaux a été confirmée sur Mac.

## Installer et ouvrir

Sur macOS 13 ou plus récent, avec Swift 6 fourni par les outils Xcode :

```sh
make app build
make app install
```

L’installation ouvre la copie dans Applications. Autoriser le Bluetooth si
macOS le demande. Pour un lancement depuis le dépôt, utiliser `make app start`
et éviter d’ouvrir les deux copies simultanément.

L’app apparaît dans la barre des menus. Choisir **Ouvrir l’app** pour voir la
fenêtre et **Quitter l’app** pour arrêter les lectures. Fermer la fenêtre laisse
l’app dans la barre des menus. Au premier lancement, elle demande l’ouverture
automatique à la connexion. Ce réglage se gère dans **Réglages Système › Général
› Ouverture de session**.

## Choisir le clavier suivi

Connecter d’abord le clavier dans les réglages Bluetooth. L’app reconnaît
`Splitergoll` et les noms qui commencent par `Splitergoll `.

- Si un seul clavier compatible est connecté, l’app le suit automatiquement.
- Avec plusieurs claviers, utiliser **Clavier suivi** dans le menu ou cliquer
  sur une ligne dans la fenêtre. Ce choix mémorisé règle les batteries et
  informations affichées. La destination des frappes se choisit au clavier.
- Si le clavier choisi est absent, son nom reste visible et les niveaux
  deviennent des tirets.

## Lire les informations

La ligne du haut correspond à la gauche, celle du bas à la droite. Le tiret
représente une valeur indisponible ou nulle. L’éclair représente 100 %, ce qui
peut être un effet de l’USB. Les lectures se font toutes les cinq secondes.
Le firmware suspend ses mesures en inactivité. Voir [Batteries et recharge](02-batteries.md).

La fenêtre peut aussi afficher **OS actif**, **Saisie** et les liaisons connues
du clavier. Un firmware ancien affiche **Non communiqué** si les données
manquent. Dans le tableau, la coche indique la liaison de saisie sélectionnée,
indépendamment du clavier suivi.

Avec le firmware expérimental [usb-priority](../technique/experimentations/01-priorite-usb.md),
laisser l’app ouverte en arrière-plan pour garder la priorité USB. Fermer
la fenêtre laisse l’échange actif. **Arrêter** dans les détails le suspend
jusqu’à **Relancer** ou jusqu’au choix d’un autre clavier.

[Essai de sélection V1 et V2](../technique/validation/10-tests-selection-clavier-macos.md)
· [Détails techniques de l’app](../../companion/macos/AGENTS.md)

---

[Documentation](../README.md) › [Utiliser le clavier](../README.md#utiliser-le-clavier) › 04. Application macOS

[← 03. Veille et réveil](03-veille-et-reveil.md) · [05. Application Windows →](05-application-windows.md)
