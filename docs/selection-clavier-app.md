# Choisir le clavier suivi par l’app

**Fonctionnalité retenue, pas encore implémentée.** Permettre de choisir les batteries de
V1 ou V2 lorsque plusieurs Splitergoll sont connectés au Mac.
Ce choix concerne l’affichage de l’app, sans changer la destination des frappes.

## Menu prévu

Ajouter « Clavier suivi » dans la barre des menus. Le sous-menu présente les
noms disponibles, par exemple « Splitergoll V1 » et « Splitergoll V2 », avec
une coche sur le clavier choisi. Le nom apparaît aussi dans la fenêtre et
l’infobulle, sans élargir l’indicateur compact des deux batteries.

Le nom sert de libellé. L’identifiant CoreBluetooth du périphérique sur ce Mac
sert à reconnaître le choix, pour éviter de confondre deux noms identiques ou
un clavier renommé. Vérifier les libellés réellement renvoyés par macOS lors du test.

## Règles retenues

- Sans choix mémorisé, suivre automatiquement l’unique clavier compatible connecté.
- Avec plusieurs candidats, demander une sélection dans le menu.
- Conserver le choix explicite après fermeture ou redémarrage de l’app.
- Si ce clavier est absent, afficher des tirets et conserver son nom. Ne pas
  suivre silencieusement l’autre clavier.
- Au changement de clavier, vider les valeurs précédentes et ignorer les
  réponses tardives de l’ancien lecteur avant d’afficher les nouvelles mesures.

Le premier lot reste limité aux appareils déjà connectés en Bluetooth et exposant
les services batterie attendus. Un V1 sans batterie en wire strict n’aura pas de
niveau utile à afficher. Lire un clavier uniquement par USB serait un autre sujet.

## Validation prévue

Tester les choix avec zéro, un et deux claviers, les noms identiques ou renommés,
la déconnexion du clavier sélectionné, le retour après redémarrage de l’app et
une réponse tardive pendant une bascule. Les tests automatisés vérifieront la
sélection et l’isolation des mesures. Un essai V1/V2 contrôlera les noms et les
services réellement exposés, sans modifier les appairages.

L’observation actuelle est limitée : après fermeture de V1 et redémarrage de
l’app, V2 s’actualise à cinq secondes selon l’utilisateur. Cela ne prouve pas
que l’ancienne anomalie venait de deux claviers ou du firmware de diagnostic.
