# Keymap automatique USB

`auto` réunit macOS et Windows avec gaming dans un seul firmware. À la connexion,
la centrale gauche observe les demandes USB de l’ordinateur et active le groupe
de couches reconnu. Aucun logiciel compagnon ni changement manuel d’OS.
Sur V1, l’utilisateur confirme le fonctionnement des layouts sur Mac et Windows,
le gaming et les bascules automatiques avec le firmware normal, sans port série.
La sélection sur événements est aussi validée après flash : Mac → Windows → Mac,
avec le bon nom d’OS affiché par l’encodeur et un démarrage ressenti très rapide
à chaque branchement. Aucun délai précis n’a été mesuré.

```sh
make build wire auto
```

Les affectations viennent des mêmes fichiers que `macos` et `windows`, schémas inclus.
Le pavé numérique et le gaming conservent la base du bon OS à leur sortie.
Les quatre profils précédents restent disponibles.

## Comportement

- La sélection se déclenche dès que le motif USB est suffisant, que l’USB clavier
  est prêt et que toutes les touches sont relâchées, sans pause fixe de détection.
- Avant la sélection, les touches et encodeurs sont ignorés, sans rejouer les appuis.
- Les appuis ignorés ne repoussent plus le démarrage. Aucune pause supplémentaire
  de 750 ms n’est imposée à l’allumage. Après un reset du bus d’un clavier déjà actif,
  cette marge reste nécessaire pour terminer les anciens comportements différés.
- Un OS inconnu ou ambigu laisse le clavier en attente, sans supposer Windows.
- Le choix reste fixe pendant le cycle USB. Un reset du bus ou une déconnexion
  impose une nouvelle reconnaissance, sans effacer les appairages.
- Suspendre puis reprendre l’USB sans reset conserve le choix. Le réveil réel reste à tester.

Les requêtes, changements d’état USB et relâchements réveillent directement le
sélecteur. Il n’attend plus 250 ms et ne vérifie plus périodiquement toutes les 20 ms.
L’initialisation par l’ordinateur reste nécessaire, aucun délai total n’est garanti.
Le choix est fait sur les indices déjà reçus : un indice contradictoire arrivé
après sélection ne change pas la keymap avant le prochain cycle USB.

Le profil USB ne reconnaît pas Linux. La [variante Bluetooth](keymap-auto-bluetooth.md)
reprend l’OS mémorisé de son propre hôte. Les signatures sont des
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
