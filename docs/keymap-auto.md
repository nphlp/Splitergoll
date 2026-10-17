# Keymap automatique USB

`auto` réunit macOS et Windows avec gaming dans un seul firmware. À la connexion,
la centrale gauche observe les demandes USB de l’ordinateur et active le groupe
de couches reconnu. Aucun logiciel compagnon ni changement manuel d’OS.
La sélection est confirmée sur V1 : l’encodeur droit affiche macOS sur le Mac et
Windows sur le PC, le gaming puis son retour à Windows fonctionnent. L’usage sur
Mac est confirmé. Le firmware normal sans port série est ensuite flashé sur V1,
son aller-retour Mac/Windows reste à confirmer.

```sh
make build wire auto
```

Les affectations viennent des mêmes fichiers que `macos` et `windows`, schémas inclus.
Le pavé numérique et le gaming conservent la base du bon OS à leur sortie.
Les quatre profils précédents restent disponibles.

## Comportement

- Les indices doivent se stabiliser pendant 250 ms. La sélection attend aussi
  que toutes les touches soient relâchées et que la saisie soit inactive depuis 750 ms.
- Avant la sélection, les touches et encodeurs sont ignorés, sans rejouer les appuis.
- Un OS inconnu ou ambigu laisse le clavier en attente, sans supposer Windows.
- Le choix reste fixe pendant le cycle USB. Un reset du bus ou une déconnexion
  impose une nouvelle reconnaissance, sans effacer les appairages.
- Suspendre puis reprendre l’USB sans reset conserve le choix. Le réveil réel reste à tester.

Cette version ne reconnaît ni Linux ni l’OS Bluetooth. Les signatures sont des
heuristiques, pas une preuve d’identité : d’autres machines ou un hub peuvent les
modifier ou produire une mauvaise reconnaissance. Le BIOS et les KVM ne sont pas validés.

## Premier essai sur V1

Commencer avec le port de contrôle :

```sh
make build wire auto usb-trace
make flash wire auto right usb-trace
make flash wire auto left usb-trace
```

Préparer chaque moitié séparément en bootloader, sans firmware de reset des réglages.
Après reconnexion, attendre deux secondes sans toucher le clavier. Lire le port
série gauche comme dans [le diagnostic USB](detection-os.md).
La capture distingue l’hypothèse de signature et la **keymap réellement appliquée**.

Tester sur le Mac : saisie des deux côtés, raccourcis Command, accents, encodeurs,
entrée et sortie du pavé. Brancher ensuite ce même firmware sur Windows, sans
reflasher : vérifier Control, pavé, gaming et retour à la base. Revenir au Mac.
Les raccourcis supposent toujours la disposition française adaptée sur l’ordinateur.

Après cet aller-retour, flasher `wire auto` sans `usb-trace` sur les deux moitiés et
refaire l’essai. Le retrait du port série change les descripteurs, cette validation
est nécessaire. Pour revenir au profil fixe : `make flash wire macos left`, puis
la droite du même profil si elle a aussi été changée. Aucun reset d’appairage.

À terme, lorsque `auto` couvrira les usages validés, `multi` sera conservé comme
archive autonome et retiré des profils actifs. Il reste actif pour le moment.
