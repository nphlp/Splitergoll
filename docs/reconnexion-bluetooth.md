# Connecter les ordinateurs en dix secondes

Dans `bluetooth auto`, la gauche ouvre **10 secondes** au démarrage, après reset
et avec **L1 + clic encodeur droit**. Un réveil de veille profonde recommence le
démarrage. Ce nouveau lot est compilé et testé en logiciel, à valider sur V1.

## Reconnexion et nouvel appareil

- Un ordinateur connu revient avec son appairage et son OS mémorisé.
- Un nouvel ordinateur peut s’appairer si un slot est libre. Le clavier réserve
  ce slot sans changer la destination des frappes, puis apprend et mémorise son OS.
- Un seul nouvel appairage est traité à la fois. Plusieurs peuvent se succéder
  pendant la fenêtre. Les connus peuvent revenir pendant ce temps.
- Si tous les connus sont revenus, l’accueil reste ouvert jusqu’à l’échéance
  tant qu’une place est disponible. Si les slots sont pleins, aucun n’est effacé.
- À dix secondes, les nouvelles demandes se ferment. Un appairage déjà commencé
  peut finir, au plus tard **30 secondes après son début**. Les connexions établies restent actives.

Le premier nouvel appareil devient utilisable s’il est seul connecté et si la
destination sélectionnée est toujours celle observée à son accueil. L’ajout d’un autre appareil ne prend pas
la main sur celui utilisé. Les touches attendent une connexion sécurisée et un OS reconnu.

## Changer d’ordinateur

**L1 + clic encodeur droit**, puis relâchement, ouvre de nouveau dix secondes et
sélectionne le prochain ordinateur déjà connecté. Si l’autre arrive ensuite,
refaire le geste pour le sélectionner. La recherche avec bascule sur une arrivée
et le [retour au précédent appareil](selection-appareil.md) restent à développer.
Le raccourci reste accessible même si aucune keymap d’OS n’est encore utilisable.

Pas besoin d’effacer les slots pour reconnecter les connus ou ajouter un appareil.
Les anciens firmwares réservés aux connus refusaient les nouveaux appairages,
même avec tous les slots vides. Reflasher la nouvelle gauche lève cette limitation,
sans `settings_reset` ni modification de la liaison entre les moitiés.

## Lire l’état

Avec la gauche sous `bluetooth auto ble-trace` :

```sh
make ble profiles /dev/cu.usbmodem123
make ble memory /dev/cu.usbmodem123
```

Adapter le port à la gauche. `WINDOW` indique le temps d’accueil restant, les annonces
et les erreurs, `SLOT` les hôtes appairés/connectés. Le temps de `WINDOW` ne représente
pas la marge de fin d’appairage. La mémoire confirme l’OS et sa sauvegarde.
Ces lectures n’ouvrent aucune fenêtre et ne changent aucun appairage ni sélection.
La [gestion d’appairage](gestion-appairage.md) distingue ce lot des prochaines étapes.
