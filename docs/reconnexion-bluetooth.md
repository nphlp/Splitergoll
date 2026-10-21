# Reconnecter les ordinateurs connus

Dans le nouveau `bluetooth auto`, la gauche ouvre **10 secondes de reconnexion**
quand Bluetooth et les appairages sauvegardés sont prêts au démarrage. Reset ou
réveil après veille profonde recommencent ce démarrage. La validation matérielle
de ce nouveau comportement reste à faire.

Le Mac et Windows peuvent ainsi revenir tous les deux. La connexion du Mac ne
coupe plus la recherche de Windows. Chaque ordinateur décide quand il se reconnecte.

## Ce qui se passe pendant la fenêtre

- Seuls les ordinateurs déjà enregistrés dans un slot avec leurs clés sont admis.
- Revenir ne change ni la destination sélectionnée ni son OS mémorisé.
- Après 10 secondes, les annonces cessent. Les connexions établies restent actives.
- Si tous les connus sont connectés, les annonces peuvent s’arrêter plus tôt.
  La fenêtre reste ouverte jusqu’à son échéance si l’un se déconnecte entre-temps.

**L1 + clic encodeur droit** réouvre dix secondes et garde la bascule existante
vers le prochain ordinateur déjà connecté. Si Windows arrive ensuite, un nouveau
geste permet de le sélectionner. La bascule automatique sur cette arrivée sera
ajoutée avec la sélection automatique d’appareil.

## Lire l’état sans rien changer

Avec la gauche sous `bluetooth auto ble-trace` :

```sh
make ble-profile /dev/cu.usbmodem123 status
```

Remplacer le port par celui de la gauche. La ligne `WINDOW` indique le temps
restant en millisecondes, les annonces actives et une éventuelle erreur radio.
`SLOT` indique les appareils appairés et connectés. Lire cet état ne réouvre pas
la fenêtre, ne change pas la sélection et ne modifie aucun appairage.

## Ajouter un nouvel appareil

Cette étape est distincte et reste à développer. Dans ce premier lot, les nouveaux
appairages sont refusés, même si un slot est libre. Les profils fixes conservent
leur fonctionnement d’appairage ZMK habituel.

ZMK associe actuellement l’accueil d’un nouvel appareil au slot sélectionné.
Un slot occupé refuse cet appairage, il n’est pas remplacé automatiquement.
Nous devons pouvoir réserver un slot vide tout en continuant à taper sur l’appareil
sélectionné. Il reste à décider comment ouvrir cet accueil, gérer les slots pleins
et terminer un appairage commencé juste avant la fermeture de sa fenêtre.

Les [règles cibles d’appairage](gestion-appairage.md) et le
[retour au précédent appareil](selection-appareil.md) restent des étapes séparées.
