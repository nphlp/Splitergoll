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

La [fenêtre de deux minutes](reconnexion-bluetooth.md) accepte les connus et
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

## Quand mémoriser le choix

Les comportements principaux sont validés sur V1 avec
[le diagnostic de dix secondes](tests-reprise-bluetooth.md). Le délai normal et
les nouveaux appairages restent des essais distincts.

| Origine du choix | Sauvegarde |
| --- | --- |
| Premier hôte après démarrage, réveil profond ou perte de tous les hôtes | À la fin de la fenêtre de 120 secondes |
| Raccourci vers un hôte connecté | Dès la réussite de la bascule et de sa keymap |
| Raccourci qui retrouve ou appaire un hôte | Dès qu’il est utilisable et sélectionné |
| Perte de l’hôte utilisé, avec un autre disponible | Dès la réussite du repli sur celui qui reste |

Une recherche sans résultat n’écrit rien. Un geste annule le retour automatique
au précédent, même s’il ne trouve personne. Une ancienne échéance ne remplace
jamais une bascule déjà mémorisée. Une demande remplacée ou échouée n’est pas un choix réussi.

La préférence est liée à l’identité vérifiée de l’appareil, indépendamment du
numéro de slot ZMK et de la mémoire de son OS. Un choix inchangé ne réécrit pas
la flash. Une erreur de sauvegarde garde l’ancien choix durable et apparaît dans
`POLICY error` : la saisie reste sur la destination courante.

## Retrouver le précédent

Au démarrage ou après perte de tous les hôtes, le premier disponible **B** devient
utilisable sans attendre le préféré **A** :

1. Le retour de B ouvre la fenêtre de **120 secondes**, dix dans le diagnostic.
2. Si A revient avant l’échéance, il reprend la sélection, même si l’on a tapé sur B.
3. Sinon, B reste sélectionné et devient le nouveau choix mémorisé à la fermeture.
4. Un raccourci manuel annule ce rappel et mémorise sa nouvelle destination dès
   la bascule réussie. Sans cible, il conserve l’ancienne préférence durable.

La sélection attend toujours les relâchements et la fin des anciens comportements.
Taper ne prolonge pas la fenêtre. Sans hôte, la recherche suit
[les règles d’activité et d’appairage](gestion-appairage.md).

Le rappel dans les deux ordres après coupure d’alimentation est confirmé sur V1
avec le lot précédent. La nouvelle sauvegarde immédiate et la reprise sans reset
sont confirmées au délai court, voir [les résultats](tests-reprise-bluetooth.md).

## Déconnexion et recherche

La fenêtre de recherche décide des retours, sans exclusion individuelle :

| Situation | Reconnexion |
| --- | --- |
| Fenêtre ouverte | Tous les appareils connus peuvent revenir |
| Fenêtre fermée, un autre ordinateur reste connecté | Pas de reconnexion spontanée |
| Plus aucun ordinateur connecté, clavier actif | Recherche continue jusqu’à l’inactivité |

Une déconnexion partielle ne rouvre ni ne prolonge la fenêtre. Le raccourci peut
la rouvrir. Une reconnexion ne reprend pas la sélection à elle seule, sauf rappel
du préféré pendant la reprise ou recherche explicite d’une autre destination.
Le compromis est volontaire : pendant la fenêtre ouverte, un appareil déconnecté
manuellement peut revenir. Après sa fermeture, il reste absent si un autre est connecté.

L’ancienne exclusion individuelle est retirée : elle pouvait survivre à la perte
de tous les hôtes et provoquer une boucle connexion/déconnexion du premier revenu.
Le correctif passe les tests logiciels et le scénario qui bouclait est confirmé
fonctionnel sur V1 par l’utilisateur.
Une coupure radio du destinataire peut toujours provoquer le repli sur l’autre
ordinateur et sa mémorisation. Si la saisie change involontairement d’ordinateur,
examiner les déconnexions et les sélections, sans conclure automatiquement à une cause.
