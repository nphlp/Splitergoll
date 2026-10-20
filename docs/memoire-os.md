# Mémoriser et corriger l’OS

La mémoire est préparée dans le diagnostic `bluetooth … ble-trace` de gauche.
**Détection, sauvegarde et restauration après reset sont validées sur V1/Mac et Windows.**
La keymap du build reste fixe. Les reconnexions reprennent l’OS sauvegardé sans
nouvel apprentissage. La veille profonde sur V2 reste à tester.
La sélection automatique des touches viendra après ces essais.

## Premier appairage

Le clavier observe la découverte de services et propose macOS ou Windows si les
indices sont suffisants. Après confirmation du nouvel appairage par Bluetooth,
il enregistre ce résultat pour cet appareil. Inconnu ou ambigu reste inconnu.
Une contradiction ultérieure pendant cette acquisition retire le choix enregistré.

Seul un petit enregistrement d’OS est sauvegardé, pas l’historique des échanges.
Il est associé à l’identité du bond, pas seulement au numéro d’emplacement.
Un appairage remplacé ne doit donc pas reprendre l’OS de son prédécesseur.

## Reconnexions

Le clavier retrouve exclusivement l’OS enregistré. Il ne relance pas
l’apprentissage, même si Windows n’envoie presque plus de découverte GATT.
Les appairages créés avant l’ajout de cette mémoire n’ont pas encore d’OS enregistré.

Un correctif du classifieur ne transforme pas un ancien résultat inconnu.
Il faut alors refaire uniquement l’appairage concerné pour reprendre son acquisition.
Ce parcours est confirmé sur Windows avec le correctif de découverte partielle,
en conservant l’appairage et l’OS mémorisé du Mac.

Une sauvegarde réussie survit au redémarrage, à la veille profonde et aux flashs
qui conservent les réglages. Effacer ou remplacer le bond invalide son association.
Une erreur d’écriture reste visible et ne vaut pas confirmation de sauvegarde.

## Vérifier la mémoire

```sh
make ble-memory /dev/cu.usbmodem1101
```

Adapter le port à la gauche. Cette lecture ne change ni la destination des touches,
ni la keymap, ni les appairages. On peut lire dès la connexion, sans attente fixe
de dix secondes. Pendant la découverte initiale, le résultat peut encore évoluer.
Dans chaque ligne :

- `measured` : dernière hypothèse de l’acquisition initiale de ce démarrage.
- `stored` : OS sauvegardé dont l’identité a été vérifiée.
- `state=saved` : sauvegarde confirmée. `empty` signifie aucune association utilisable,
  `unverified` une identité pas encore vérifiée, `error` une erreur à examiner.

Après redémarrage, `measured=unknown` avec `stored=windows` peut être normal :
Windows a été retrouvé en mémoire, sans nouvelle détection.
Un appareil pas encore reconnecté peut afficher `stored=unknown` et `unverified` :
son enregistrement attend la vérification de son identité, il n’est pas forcément perdu.
Le diagnostic détaillé des échanges reste `make ble-capture <port>`.

Les [essais de mémoire](tests-memoire-os.md) regroupent les tests Mac et Windows.

## Correction manuelle, prochaine étape

Un raccourci permettra de changer durablement l’OS de l’appareil sélectionné,
y compris s’il est inconnu. Le geste et son raccordement au firmware restent
à faire. Le moteur de mémoire prépare déjà cette priorité manuelle.
Un profil entièrement manuel reste un dernier recours distinct dans le [TODO](TODO.md).
