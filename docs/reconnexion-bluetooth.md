# Connecter les ordinateurs en trente secondes

Dans `bluetooth auto`, la gauche ouvre **30 secondes** au démarrage, après reset
et avec **L1 + clic encodeur droit**. Sans hôte, la recherche reste ouverte tant
que le clavier est actif. La première connexion ouvre trente secondes pour les
suivants. Un réveil de veille profonde recommence le démarrage. Les essais V1 avec la fenêtre temporaire de dix secondes confirment les
bascules, la saisie retenue et la reconnexion Windows sur demande. Le retour au
build normal de trente secondes est prévu, pas encore confirmé.

## Reconnexion et nouvel appareil

- Un ordinateur connu revient avec son appairage et son OS mémorisé.
- Un nouvel ordinateur peut s’appairer si un slot est libre. Le clavier réserve
  ce slot sans changer la destination des frappes, puis apprend et mémorise son OS.
- Un seul nouvel appairage est traité à la fois. Plusieurs peuvent se succéder
  pendant la fenêtre. Les connus peuvent revenir pendant ce temps.
- Si tous les connus sont revenus, l’accueil reste ouvert jusqu’à l’échéance
  tant qu’une place est disponible. Si les slots sont pleins, aucun n’est effacé.
- À trente secondes, les nouvelles demandes se ferment. Un appairage déjà commencé
  peut finir, au plus tard **30 secondes après son début**. Les connexions établies restent actives.

Le premier nouvel appareil devient utilisable s’il est seul connecté et si la
destination sélectionnée est toujours celle observée à son accueil. L’ajout d’un autre appareil ne prend pas
la main sur celui utilisé, sauf recherche explicitement ouverte par le raccourci
avec au plus un appareil prêt. Les touches attendent une connexion sécurisée et un OS reconnu.

## Changer d’ordinateur

**L1 + clic encodeur droit**, puis relâchement, ouvre de nouveau trente secondes :

- Un seul ordinateur prêt : continuer à l’utiliser, puis basculer sur le premier
  autre appareil prêt, qu’il soit déjà appairé ou nouveau.
- Plusieurs ordinateurs prêts : sélectionner le suivant déjà prêt. Une arrivée
  ultérieure ne remplace pas ce choix.
- Aucun prêt : rechercher et sélectionner le premier utilisable.

Les frappes reçues pendant la transition attendent la nouvelle keymap en RAM,
au lieu de repousser la bascule. Les touches déjà maintenues doivent être relâchées.
La garde de 750 ms laisse finir les anciens comportements. La file est bornée à
64 événements : une cible perdue ou un dépassement abandonne le lot, sans release
orpheline. Les rotations pendant cette courte transition sont ignorées.
Les allers-retours et la saisie immédiate sont confirmés sur V1, ainsi que le blocage
par Shift maintenu puis la reprise après relâchement.
Le [retour au précédent appareil](selection-appareil.md) et sa préférence durable
sont implémentés dans le nouveau lot, à valider sur matériel.
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

Le nouveau diagnostic `BLEPROFILE` v4 ajoute `POLICY` : `active` indique l’activité,
`without_host=1` une recherche continue sans hôte, `remembered` le slot dont le bond
vérifié correspond à la préférence, et `saved` sa présence en mémoire.
`remembered=-1` peut simplement signifier que le bond n’est pas encore vérifié
pour ce démarrage. `restoring` indique le retour prioritaire encore autorisé.
`POLICY error` concerne la sauvegarde de préférence, `WINDOW error` la radio/appairage.
Sans hôte, `remaining_ms` reste à la durée nominale, ce n’est pas un compte à rebours.
Les lecteurs restent compatibles avec les anciens diagnostics.
