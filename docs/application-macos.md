# Application compagnon macOS

**À développer.** Une petite application native en Swift, avec une interface SwiftUI.
Elle observe le clavier et reste facultative : la saisie et la sélection automatique
de keymap fonctionnent sans elle. Son code sera séparé du firmware et des diagnostics.

## Premier lot : les deux batteries uniquement

Le premier développement se limite à :

- Une fenêtre simple avec les batteries gauche et droite, l’état de connexion et
  l’ancienneté des informations reçues.
- Un indicateur dans la barre des menus, par exemple `G 76 % · D 79 %`.
- Une actualisation automatique et la reprise après déconnexion ou veille du Mac.
- Des actions pour rouvrir la fenêtre et quitter l’app. Fermer la fenêtre conserve
  l’indicateur, relancer l’application déjà ouverte réaffiche la fenêtre.

La couche, l’OS et les réglages avancés attendent un lot suivant. Ce premier lot
réutilise les services batterie existants, sans nouveau protocole firmware.

Une donnée absente ou trop ancienne doit être indiquée comme telle. Les valeurs
sont les niveaux annoncés par le clavier. Le firmware actuel ne transmet pas
l’état USB par moitié : l’app doit rappeler que les mesures sous USB ne sont pas
fiables et ne prouvent pas une batterie pleine. Elle ne peut pas deviner la charge
à partir d’un 100 %. Cette limite matérielle reste distincte du développement de l’app.

## Évolutions conservées pour plus tard

La fenêtre pourra ensuite proposer deux onglets, **Informations** et **Réglages**.
Les informations supplémentaires seront la couche active, avec un nom lisible
comme Base, L1, L2, Pavé ou Gaming, et l’OS utilisé par la keymap du clavier.
Cet OS peut différer du Mac hébergeant l’app. Si plusieurs couches sont actives,
montrer celle de priorité la plus haute et permettre de comprendre la base OS utilisée.

Les cinq options demandées restent au backlog :

| Option | Fonctionnement attendu |
| --- | --- |
| Afficher dans la barre des menus | Garder un accès à l’app et aux informations lorsque la fenêtre est fermée |
| Masquer dans le Dock | Permettre un usage discret en arrière-plan, sans perdre l’accès à la fenêtre ni à Quitter |
| Lancer au démarrage | Ouvrir l’app à l’ouverture de session, sans afficher la fenêtre. Activation explicite par l’utilisateur |
| Afficher la batterie | Montrer les deux niveaux dans la barre des menus, par exemple `G 76 % · D 79 %` |
| Afficher la couche active | Ajouter son nom dans la barre des menus, indépendamment des batteries |

Les deux dernières options dépendent de l’affichage dans la barre des menus.
Les réglages sont conservés entre les lancements. Fermer la fenêtre en mode
arrière-plan ne quitte pas l’application. Une action Quitter reste accessible.
Les valeurs initiales des options restent à choisir lors de la conception de l’interface.

## Ce qui existe et ce qu’il manque

Le lecteur Swift de `make bat` sait déjà identifier et lire les deux batteries
par CoreBluetooth. C’est la base du premier lot, pas encore un lecteur permanent.

La couche active et l’OS utilisé ne sont pas exposés à ce lecteur. Il faudra un
petit protocole d’état côté firmware, avec une lecture initiale puis des mises à
jour, sans analyser les logs USB ni les frappes. L’état USB droit devra aussi
remonter par la centrale. Le transport et le format restent à définir.
L’app devra gérer les firmwares anciens en indiquant les fonctions indisponibles.
Elle ne change ni les appairages ni la destination de saisie pour lire les informations.

Les premiers points à valider sont la reprise après veille, les autorisations
Bluetooth, les mesures anciennes et la consommation. Les états supplémentaires
et le lancement à l’ouverture de session seront validés dans leurs lots respectifs.
Une app persistante ne doit pas réveiller le Mac ou le clavier pour maintenir son affichage.

## Ordre de réalisation

1. Construire la fenêtre et la barre des menus avec les deux batteries, sur le
   firmware existant, en signalant les limites de mesure sous USB.
2. Tester déconnexion, veille, données anciennes et fonctionnement du clavier
   sans l’app. Tester la logique applicative automatiquement.
3. Ensuite seulement, définir le protocole des états supplémentaires et ajouter
   couche, OS et réglages avancés, avec leurs validations.

## Références Apple

- [MenuBarExtra](https://developer.apple.com/documentation/swiftui/menubarextra) pour la barre des menus.
- [Politique d’activation](https://developer.apple.com/documentation/appkit/nsapplication/setactivationpolicy(_:)) pour la présence dans le Dock.
- [SMAppService](https://developer.apple.com/documentation/servicemanagement/smappservice) pour l’ouverture de session.
