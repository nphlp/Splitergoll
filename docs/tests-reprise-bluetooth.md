# Tester le dernier appareil mémorisé

Utiliser le diagnostic à **10 secondes** et les
[préparatifs](tests-connexions-bluetooth.md). Garder les appairages et utiliser
le clavier intégré du Mac pour répondre. Le clic simple droit indique l’OS.
`make ble profiles <port>` consulte la sélection et `POLICY`, sans les modifier.

## Résultats sur V1

Validés avec le diagnostic v5 à dix secondes, Mac et Windows déjà appairés :

- Sauvegarde manuelle dans les deux sens avant expiration, avec environ sept secondes restantes.
- Windows conservé après coupure électrique juste après sa sélection.
- Mac utilisable en premier, puis retour prioritaire de Windows dans la fenêtre.
- Perte de Windows : repli sur Mac, préférence Mac sauvegardée et Windows exclu.
- Un raccourci lève l’exclusion, reconnecte Windows, le sélectionne et le mémorise.
- Si Windows mémorisé ne revient pas au démarrage, Mac seul devient durable à l’échéance.

Le sens inverse du rappel n’est pas répété dans ce lot, à la demande de l’utilisateur.
**Régression corrigée en v6 :** Mac sélectionné, couper Windows puis Mac et réactiver
Windows seul provoque une boucle connexion/déconnexion. Réactiver Mac seul fonctionne,
un redémarrage débloque Windows. L’exclusion conservée après perte totale est la
piste principale, sans capture interne de cet incident. Ne pas attribuer ce résultat
à Windows ou à l’OS mémorisé sans vérification.

La recherche sans résultat est également positive : Mac reste sélectionné et
mémorisé après fermeture, sans erreur. La recherche jusqu’à IDLE, le nouveau bond
et le délai normal de 120 secondes ne sont pas validés par ces essais.
Le correctif v6 retire l’exclusion individuelle. L’utilisateur confirme que Windows
revient seul sans boucle après la perte des deux connexions, sans reset.
L’utilisateur confirme également la reproduction exacte sans boucle, puis la
fermeture avec Windows restant et la reconnexion/sélection Mac après un seul
raccourci. Les comportements principaux sont validés au délai de dix secondes.

## 1. Choix manuel enregistré immédiatement

1. Avec Mac et Windows connectés, sélectionner Windows par L1 + clic droit.
2. Dès la bascule terminée, lire le statut, sans attendre les dix secondes :
   `remembered=<slot Windows> saved=1 error=0` attendu.
3. Débrancher/rebrancher l’USB de la gauche avant la fin de la fenêtre : Windows
   doit reprendre la sélection quand les deux reviennent.
4. Rejouer vers le Mac. Vérifier aussi que l’expiration ne remplace pas ce choix.

`remembered` désigne un slot, pas un OS. Une commande sans cible n’écrit rien.

## 2. Le préféré revient en second

1. Avec les deux connectés, sélectionner le Mac et vérifier sa mémorisation.
2. Débrancher **d’abord l’USB de gauche**, puis couper le Bluetooth du Mac.
3. Rebrancher la gauche. Windows doit fonctionner en premier.
4. Réactiver le Mac avant dix secondes après le retour de Windows : le Mac doit
   reprendre la sélection, même après des frappes sur Windows.
5. Rejouer en inversant Mac et Windows.

Couper le Bluetooth avant l’alimentation testerait un repli immédiatement mémorisé,
pas le retour du préféré après démarrage. Un reset simple peut aussi servir si
l’ordre d’arrivée est maîtrisé. Noter les résultats sans confondre les deux situations.

## 3. Le préféré ne revient pas

Rejouer l’étape 2, mais attendre douze secondes avant de réactiver le préféré.
L’hôte revenu seul doit maintenant être mémorisé. Le préféré ne se reconnecte pas
tout seul après la fermeture. Un raccourci doit le retrouver, le sélectionner et
le mémoriser dès la bascule réussie, sans attendre une nouvelle échéance.

## 4. Le geste manuel annule le rappel

Après le démarrage avec un seul hôte provisoire, faire L1 + clic droit avant le
retour du préféré. Il s’agit désormais d’une recherche explicite : la première
arrivée utilisable devient le choix durable dès la bascule. Avec deux hôtes déjà
prêts, la prochaine destination est directement choisie et mémorisée.

## 5. Déconnexion avec un autre hôte restant

Avec les deux hôtes connectés, sélectionner Mac, attendre la fermeture de la fenêtre,
puis couper le Bluetooth Windows et le réactiver. Il ne doit pas revenir tant que
la fenêtre reste fermée. Un seul raccourci doit le reconnecter et le sélectionner.
Si la fenêtre est encore ouverte, son retour est au contraire autorisé : il n’y a
plus d’exclusion individuelle. Une perte partielle ne réarme jamais le délai.

Si le destinataire se déconnecte, l’autre doit devenir utilisable et mémorisé.
Son retour ultérieur ne doit pas voler le choix effectué par ce repli.

## 6. Régression : perte totale sans reset

1. Avec les deux connectés, sélectionner Mac et vérifier sa mémorisation.
2. Couper Windows puis Mac, sans reset et en gardant les USB branchés.
3. Appuyer puis relâcher une touche gauche pour garder le clavier actif.
4. Réactiver Windows : il doit fonctionner sans boucle connexion/déconnexion.
5. Réactiver Mac avant dix secondes après Windows : Mac doit reprendre la sélection.
6. Lire le statut : les deux connectés, `active=0 remembered=0`, sans erreur.

Ce cas reproduit le bug de l’ancienne exclusion. Le test logiciel du vrai adaptateur
couvre les deux ordres. L’utilisateur confirme le cas Windows perdu puis revenu
en premier sur V1 v6. Un redémarrage entre les coupures et le retour ne validerait
pas cette régression.

## 7. Recherche sans résultat

Avec un seul hôte connecté, fermer le Bluetooth de l’autre et faire le raccourci.
Attendre douze secondes : la sélection et la préférence doivent rester inchangées.
Rouvrir ensuite par le raccourci pour continuer les essais.

## 8. Nouvel appairage, séparément

Avec accord pour libérer un slot ou un troisième appareil disponible, ouvrir la
recherche depuis un seul hôte prêt et appairer le nouvel appareil. Vérifier son OS,
sa sélection et sa sauvegarde de préférence dès qu’il devient utilisable.
Aucun effacement n’est nécessaire aux étapes précédentes.

Pour chaque cas : OK/KO, destination réelle, OS affiché et statut après bascule.
Reprendre ensuite [les essais sans hôte](tests-recherche-bluetooth.md), puis revenir
au firmware normal de **120 secondes**. Les essais à dix secondes ne valident pas
à eux seuls ce délai normal ni le réveil profond sur batterie de V2.
