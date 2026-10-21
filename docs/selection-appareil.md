# Choisir l’ordinateur Bluetooth

Un emplacement ZMK conserve un appareil appairé. Trois états sont distincts :

- **Appairé** : le clavier connaît l’appareil et ses clés de connexion.
- **Connecté** : une liaison Bluetooth existe avec lui.
- **Sélectionné** : il reçoit les touches du clavier.

La moitié droite du clavier ne compte pas comme un ordinateur.
Ce guide décrit le Bluetooth actuel. Le futur choix USB + Bluetooth est cadré dans
[destinations de saisie](destinations-saisie.md), avec priorité USB par ordinateur.

## Raccourci dans `bluetooth auto`

Depuis la couche de base, maintenir **L1**, cliquer sur **l’encodeur droit**, puis tout relâcher sélectionne
l’ordinateur connecté suivant. Le clavier attend la fin des appuis et des
comportements différés avant de changer de destination, puis charge son OS mémorisé.
Le clic simple sur l’encodeur continue d’afficher l’OS.

Les emplacements vides ou déconnectés sont ignorés pour la sélection immédiate.
Le geste ouvre aussi [dix secondes de reconnexion](reconnexion-bluetooth.md).
Si le seul ordinateur connecté est déjà sélectionné, la destination reste inchangée.
Le geste reste disponible quand l’OS sélectionné
est inconnu. Il ne modifie ni les appairages ni l’OS enregistré. La bascule et
l’association des keymaps sont confirmées sur V1 entre Mac et Windows en `ble-trace`.

L’attente ressentie est d’environ deux secondes, sans mesure instrumentée. Le code
prévoit une garde de 750 ms après la dernière activité admise pour laisser finir
les comportements différés. Le délai complet reste à mesurer puis à optimiser.

**ZMK standard** arrête ses annonces quand l’ordinateur sélectionné est connecté.
Le nouveau contrôleur de `bluetooth auto` les permet pendant dix secondes pour les
autres ordinateurs connus, sans déplacer la sélection. Ce lot reste à tester sur V1.

La proposition suivante ouvre les annonces pendant dix secondes au démarrage et
au retour de Gaming USB vers Windows, ainsi qu’avec L1 + clic encodeur droit.
Avec une seule destination, ce geste recherche et sélectionne le premier autre
appareil prêt. Avec plusieurs destinations, il passe à la suivante et une arrivée
ultérieure ne prend pas sa place. Les ouvertures sans ce geste ne font pas prendre
la sélection au nouvel arrivant. Un nouvel appareil peut être ajouté sur un slot libre.
Après fermeture, les connexions établies restent présentes. Les annonces sont
arrêtées pendant Gaming USB, avec déconnexion du Mac et conservation de son appairage.
Voir [la gestion de l’appairage](gestion-appairage.md) pour les règles encore à implémenter.
L’ordinateur décide quand se connecter, le clavier se rend disponible.

## Sélection automatique prévue

Ces règles ne sont pas encore implémentées :

| Situation | Comportement souhaité |
| --- | --- |
| Aucun ordinateur connecté | Attendre une connexion |
| Un seul ordinateur connecté | Le sélectionner automatiquement |
| Un second ordinateur se connecte | Garder la destination actuelle |
| La destination se déconnecte, un seul ordinateur reste | Sélectionner celui qui reste |

Chaque destination utilise l’[OS mémorisé pour cet appareil](memoire-os.md).

## Après redémarrage ou veille profonde

Le clavier conserve le dernier ordinateur sélectionné, par exemple **A**.
Si **B** se reconnecte avant lui :

1. B est sélectionné immédiatement, puisqu’il est seul.
2. Une fenêtre de **5 secondes** commence à cette première reconnexion utilisable.
3. Si A revient dans cette fenêtre, le clavier revient automatiquement sur A,
   même si des frappes ont déjà été envoyées à B.
4. Sinon, B reste sélectionné et devient le nouvel ordinateur mémorisé.
5. Si A revient plus tard, il ne reprend pas la sélection.

Ces cinq secondes ne bloquent pas la saisie sur B. Taper sur B n’annule ni ne
prolonge la fenêtre : les frappes déjà envoyées restent sur B, les suivantes
partent vers A après la bascule. Une bascule manuelle d’appareil annule le retour
automatique et mémorise le choix explicite. La durée sera à confirmer sur le matériel.
La recherche explicite déclenchée par L1 + clic droit doit aussi annuler ce retour
automatique, pour ne pas être contredite par la reconnexion de l’ancien appareil.

Deux connexions simultanées ont déjà été observées sur V1. L’arrivée et la sélection
automatiques du second ordinateur restent à tester après adaptation des annonces.
