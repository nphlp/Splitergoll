# Application Windows

**Lecture des deux batteries confirmée sur le PC avec V2.** L’application lit les
deux batteries de V2. Elle reste facultative pour taper et ne change ni le
firmware, ni les appairages, ni l’ordinateur sélectionné par le clavier.

## Premier essai

Copier `Splitergoll.exe` dans un dossier stable du PC. Aucun SDK ou runtime .NET
à installer, aucun installateur ou droit administrateur nécessaire pour l’usage normal.

Avec V2 déjà connecté en Bluetooth à Windows, double-cliquer sur l’exécutable.
L’app démarre près de l’horloge, éventuellement sous la flèche des icônes masquées.
Les deux niveaux réels sont lus toutes les cinq secondes. Aucun nouveau flash.

Facultatif : le menu propose une démonstration sans Bluetooth, avec les valeurs
fictives 90/10. Elle ne consulte pas les claviers ni les préférences. On peut aussi
la lancer directement depuis PowerShell :

```powershell
.\Splitergoll.exe --demo
```

Quitter la démonstration depuis le menu pour retrouver les mesures réelles.
V1 n’a pas de batterie.

## Utilisation

- Une icône, gauche en haut et droite en bas. `%` est conservé s’il reste lisible.
- Un clic ouvre le panneau des deux niveaux exacts. « Ouvrir l’app » affiche la
  fenêtre compacte et redimensionnable. Les deux vues affichent le logo Splitergoll.
  La largeur initiale suit le DPI de l’écran, la hauteur suit le texte. La fenêtre
  garde une taille minimale lisible. Fermer conserve le lecteur en arrière-plan.
- Un clic droit permet de choisir le clavier suivi, lancer avec Windows, basculer
  la démonstration ou quitter. Une seule instance peut fonctionner par session.
- La préférence de clavier est conservée dans les données utilisateur. Un préféré
  absent reste sélectionné avec des tirets, sans choisir un autre clavier à sa place.
- Lecture toutes les cinq secondes. Zéro, valeur absente ou périmée donne un tiret.
  Cent affiche un éclair de **charge possible**, sans preuve de charge effective.
  Le câblage actuel ne mesure toujours pas la capacité réelle sous USB.

Les images sont dessinées à la définition du moniteur, sans agrandir un bitmap.
La police et le symbole `%` sont choisis sur la plage des valeurs, avec une
présentation stable. Les premiers essais confirment les niveaux réels et le menu
contextuel. La base est jugée satisfaisante après les corrections de dimensions
et d’icône. À la demande de l’utilisateur, la largeur passe de 340 à 255 unités
logiques, soit un quart de moins. Cette largeur réduite reste à confirmer sur le PC.
Les cartes avec silhouettes gauche/droite sont prévues dans le TODO.
Le rendu des chiffres de la zone de notification est conservé.

## Mises à jour et avertissements

**Pas de réinstallation à chaque essai.** Quitter l’app depuis son menu, remplacer
l’exécutable et relancer. Le choix du clavier reste conservé. Garder le même dossier
si « Lancer avec Windows » est activé, pour ne pas changer la cible du raccourci.

Le portable de test n’est pas signé. [SmartScreen](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/smartscreen-reputation)
peut afficher un avertissement, notamment après téléchargement. Un nouveau build
peut de nouveau être inconnu, une signature ne garantit pas non plus l’absence
d’alerte pour une application nouvelle. Un certificat auto-signé n’apporte pas
la confiance publique d’un éditeur reconnu.

Si « Informations complémentaires » puis « Exécuter quand même » sont proposés,
ne les utiliser qu’après avoir vérifié la source de notre build et son empreinte.
Un blocage par Smart App Control ou une politique peut ne pas proposer ce choix :
conserver alors le message pour le diagnostic. Aucune désactivation de protection
ni installation de certificat n’est effectuée par nos scripts ou notre application.

Le dossier du build contient `SHA256SUMS` et `LIRE-MOI.txt`. Vérifier l’exécutable :

```powershell
Get-FileHash .\Splitergoll.exe -Algorithm SHA256
```

## Build et vérifications

Depuis le Mac, avec le SDK .NET 10 sur PATH ou indiqué par `DOTNET_COMMAND` :

```sh
make app test windows
make app build windows
```

Le SDK déjà préparé dans `.local/windows-sdk/sdk` est également reconnu. Les caches
restent dans `.local/`, les portables dans `dist/apps/windows-…/`. Le nom Bluetooth
vient de la configuration centrale. Le portable cible win-x64.

Les tests du noyau vérifient décodage, sessions, lectures périmées, préférences,
sélection et règles de formatage, sans API Windows ni Bluetooth. Les tests UI sont
compilés sur Mac, leur exécution est signalée comme non réalisée. Ils contrôlent
les vraies fenêtres sous Windows avec différentes tailles de police, sans Bluetooth.
Le rendu corrigé, la reprise après sommeil et le lancement de session restent
à valider sur le PC. Le [rapprochement USB/Bluetooth](identite-ordinateur.md)
est un lot futur, sans identifiant hôte ni canal d’écriture dans ce portable.
