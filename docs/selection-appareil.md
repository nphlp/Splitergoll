# Choisir l’ordinateur Bluetooth

Un emplacement ZMK conserve un appareil appairé. Trois états sont distincts :

- **Appairé** : le clavier connaît l’appareil et ses clés de connexion.
- **Connecté** : une liaison Bluetooth existe avec lui.
- **Sélectionné** : il reçoit les touches du clavier.

La moitié droite du clavier ne compte pas comme un ordinateur.

## Raccourci dans `bluetooth auto`

Depuis la couche de base, maintenir **L1**, cliquer sur **l’encodeur droit**, puis tout relâcher sélectionne
l’ordinateur connecté suivant. Le clavier attend la fin des appuis et des
comportements différés avant de changer de destination, puis charge son OS mémorisé.
Le clic simple sur l’encodeur continue d’afficher l’OS.

Les emplacements vides ou déconnectés sont ignorés. Si le seul ordinateur connecté
est déjà sélectionné, rien ne change. Le geste reste disponible quand l’OS sélectionné
est inconnu. Il ne modifie ni les appairages ni l’OS enregistré. Essai matériel à faire.

**Limite actuelle :** ZMK arrête ses annonces quand l’ordinateur sélectionné est
connecté. Le raccourci ne provoque donc pas la reconnexion d’un appareil seulement
appairé. L’accueil du second ordinateur sera traité avec l’automatisation ci-dessous.

## Sélection automatique prévue

Ces règles ne sont pas encore implémentées :

| Situation | Comportement souhaité |
| --- | --- |
| Aucun ordinateur connecté | Attendre une connexion |
| Un seul ordinateur connecté | Le sélectionner automatiquement |
| Un second ordinateur se connecte | Garder la destination actuelle |
| Bascule demandée au clavier | Sélectionner l’ordinateur connecté suivant |
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

Deux connexions simultanées ont déjà été observées sur V1. L’arrivée et la sélection
automatiques du second ordinateur restent à tester après adaptation des annonces.
