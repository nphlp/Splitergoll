# Choisir l’ordinateur Bluetooth

**Fonctionnement prévu, pas encore implémenté.**

Un emplacement ZMK conserve un appareil appairé. Trois états sont distincts :

- **Appairé** : le clavier connaît l’appareil et ses clés de connexion.
- **Connecté** : une liaison Bluetooth existe avec lui.
- **Sélectionné** : il reçoit les touches du clavier.

La moitié droite du clavier ne compte pas comme un ordinateur.

## Pendant l’utilisation

| Situation | Comportement souhaité |
| --- | --- |
| Aucun ordinateur connecté | Attendre une connexion |
| Un seul ordinateur connecté | Le sélectionner automatiquement |
| Un second ordinateur se connecte | Garder la destination actuelle |
| Bascule demandée au clavier | Sélectionner l’autre ordinateur |
| La destination se déconnecte, un seul ordinateur reste | Sélectionner celui qui reste |

Le raccourci exact reste à choisir. Il change la destination des touches,
et charge l’[OS mémorisé pour cet appareil](memoire-os.md).

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

ZMK arrête normalement ses annonces quand l’ordinateur sélectionné est connecté.
Accueillir le second ordinateur demandera aussi d’adapter ces annonces. Ce scénario
à deux ordinateurs n’a pas encore été testé sur notre firmware.
