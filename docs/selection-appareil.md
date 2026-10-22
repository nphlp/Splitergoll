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

## Sélection automatique

Implémentée dans le nouveau lot, testée en logiciel et à valider sur le clavier :

| Situation | Comportement souhaité |
| --- | --- |
| Aucun ordinateur connecté, clavier actif | Rechercher les connus et accepter un nouvel appairage jusqu’à l’inactivité |
| Un seul ordinateur connecté | Le sélectionner automatiquement |
| Un second ordinateur se connecte | Garder la destination actuelle |
| La destination se déconnecte, un seul ordinateur reste | Sélectionner celui qui reste |

Chaque destination utilise l’[OS mémorisé pour cet appareil](memoire-os.md).
La recherche sans hôte suit maintenant l’activité du clavier, voir
[la gestion de l’appairage](gestion-appairage.md).

## Après redémarrage ou veille profonde

**Implémenté, essais matériels à faire.** Le clavier conserve le dernier ordinateur sélectionné,
par exemple **A**, au-delà du reset ou de la veille profonde.
Si **B** se reconnecte avant lui pendant la fenêtre de démarrage :

1. B est sélectionné immédiatement, puisqu’il est seul.
2. La préférence pour A reste valable jusqu’à la fin des **30 secondes de recherche
   ouverte à la première connexion du démarrage**. Sans hôte, la recherche restait
   ouverte jusqu’à l’inactivité.
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

La préférence utilise notre propre mémoire, distincte du slot mémorisé par
ZMK et de la mémoire des OS. À la fermeture de la fenêtre, sauvegarder l’appareil
**sélectionné et connecté**, pas le dernier à avoir établi sa liaison. Sans appareil
sélectionné utilisable, garder la préférence précédente. Un reset avant cette
sauvegarde réutilise le dernier choix déjà enregistré.

La préférence est liée au bond vérifié, pas au numéro de slot ou au nom de l’OS.
Le même choix ne réécrit pas la flash. Une erreur de sauvegarde conserve l’ancien
choix et apparaît dans `POLICY error`. Si une transition est encore en cours à
l’échéance, la sauvegarde attend sa fin. Voir [les essais de reprise](tests-reprise-bluetooth.md).

## Prochaine règle de mémorisation

Le firmware essayé sauvegarde encore tous les choix à la fermeture. Les évolutions
suivantes sont retenues pour le prochain lot, pas encore codées :

| Origine du choix | Quand le rendre durable |
| --- | --- |
| Premier appareil au démarrage ou après perte de tous les hôtes | À la fin de la fenêtre, pour laisser revenir le précédent |
| Raccourci vers un hôte déjà prêt | Dès la réussite de la bascule |
| Raccourci qui retrouve ou appaire un hôte | Dès la réussite de la bascule vers cet hôte utilisable |
| Perte de la destination utilisée, avec un autre hôte restant | Dès la réussite du repli, en annulant le rappel vers l’hôte perdu |

La première arrivée provisoire au démarrage n’est pas une perte de destination :
elle ne doit pas écraser immédiatement la préférence. Une demande sans résultat
n’écrit rien. Une demande remplacée ne vaut pas une bascule réussie. Après un choix
immédiatement sauvegardé, une ancienne échéance ne doit pas rétablir un ancien choix.

La fenêtre réelle de deux minutes et l’exclusion d’un hôte perdu jusqu’à une nouvelle
recherche explicite sont acceptées pour le prochain lot. Le diagnostic restera à dix
secondes. Le retour prioritaire au démarrage disposera aussi de cette fenêtre.

**Compromis à surveiller :** une coupure radio involontaire peut déclencher le repli
sur l’autre ordinateur, sa mémorisation et l’exclusion de l’ancien. Le clavier peut
alors sembler changer d’ordinateur tout seul et ne pas revenir. Ce phénomène n’a
pas été observé. S’il apparaît, examiner en priorité cette politique avec les traces
de déconnexion et le masque d’exclusion, avant de conclure à une cause. Une nouvelle
recherche explicite doit permettre de réadmettre les appareils exclus.
