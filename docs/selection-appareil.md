# Choisir l’ordinateur Bluetooth

Un emplacement ZMK conserve un appareil appairé. Trois états sont distincts :

- **Appairé** : le clavier connaît l’appareil et ses clés de connexion.
- **Connecté** : une liaison Bluetooth existe avec lui.
- **Sélectionné** : il reçoit les touches du clavier.

La moitié droite du clavier ne compte pas comme un ordinateur.
Ce guide décrit le Bluetooth actuel. Le futur choix USB + Bluetooth est cadré dans
[destinations de saisie](destinations-saisie.md), avec priorité USB par ordinateur.

## Raccourci dans `bluetooth auto`

Depuis la base, maintenir **L1**, cliquer sur **l’encodeur droit**, puis tout relâcher.
Le clic simple continue d’afficher l’OS. Chaque destination utilise son OS mémorisé.

| Appareils prêts au geste | Résultat |
| --- | --- |
| Plusieurs | Passer au suivant déjà prêt, sans laisser une arrivée ultérieure reprendre la main |
| Un seul | Ouvrir la recherche et passer au premier autre appareil prêt |
| Aucun | Ouvrir la recherche et sélectionner le premier prêt |

La [fenêtre de trente secondes](reconnexion-bluetooth.md) accepte les connus et
les nouveaux appareils sur un slot libre. Le geste ne supprime aucun appairage.
Une nouvelle association attend une connexion sécurisée et un OS reconnu avant la bascule.

Les anciens appuis et macros finissent sur leur ordinateur. Les nouvelles frappes
pendant la transition attendent en RAM puis sont transmises avec la nouvelle keymap.
Elles ne réarment plus la garde de 750 ms. Une cible perdue, une nouvelle commande
ou une file pleine peut annuler ce lot, sans le transmettre à une autre destination.

Les essais sur V1 avec la fenêtre temporaire de dix secondes confirment les
allers-retours Mac/Windows, leurs bonnes keymaps et la saisie immédiate retenue
pendant la transition. Shift maintenu retarde la bascule, qui reprend au relâchement.
Le raccourci reconnecte et sélectionne Windows après fermeture de la fenêtre.
Avec zéro connexion, Mac arrivé en premier reste sélectionné quand Windows rejoint.
Les [destinations USB/BLE et Gaming USB](destinations-saisie.md) restent à développer.

## Sélection automatique prévue

Ces règles ne sont pas encore implémentées :

| Situation | Comportement souhaité |
| --- | --- |
| Aucun ordinateur connecté, clavier actif | Rechercher les connus et accepter un nouvel appairage jusqu’à l’inactivité |
| Un seul ordinateur connecté | Le sélectionner automatiquement |
| Un second ordinateur se connecte | Garder la destination actuelle |
| La destination se déconnecte, un seul ordinateur reste | Sélectionner celui qui reste |

Chaque destination utilise l’[OS mémorisé pour cet appareil](memoire-os.md).
La recherche sans hôte et son arrêt en veille légère restent à développer, voir
[la gestion de l’appairage](gestion-appairage.md).

## Après redémarrage ou veille profonde

**À développer.** Le clavier devra conserver le dernier ordinateur sélectionné,
par exemple **A**, au-delà du reset ou de la veille profonde.
Si **B** se reconnecte avant lui pendant la fenêtre de démarrage :

1. B est sélectionné immédiatement, puisqu’il est seul.
2. La préférence pour A reste valable jusqu’à la fin des **30 secondes de recherche
   du démarrage**, sans ajouter un autre délai après l’arrivée de B.
3. Si A revient avant cette échéance, le clavier revient automatiquement sur A,
   même si des frappes ont déjà été envoyées à B.
4. Sinon, B reste sélectionné et devient le nouvel ordinateur mémorisé.
5. Si A revient plus tard, il ne reprend pas la sélection.

Cette attente ne bloque pas la saisie sur B. Taper sur B n’annule ni ne
prolonge la fenêtre : les frappes déjà envoyées restent sur B, les suivantes
partent vers A après la bascule. Une bascule manuelle d’appareil annule le retour
automatique. Le choix explicite sera mémorisé à la fermeture de sa fenêtre.
La recherche explicite déclenchée par L1 + clic droit doit aussi annuler ce retour
automatique, pour ne pas être contredite par la reconnexion de l’ancien appareil.

La future préférence utilise notre propre mémoire, distincte du slot mémorisé par
ZMK et de la mémoire des OS. À la fermeture de la fenêtre, sauvegarder l’appareil
**sélectionné et connecté**, pas le dernier à avoir établi sa liaison. Sans appareil
sélectionné utilisable, garder la préférence précédente. Un reset avant cette
sauvegarde réutilise le dernier choix déjà enregistré.
