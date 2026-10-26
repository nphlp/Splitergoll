# Tests automatisés

La suite protège les règles déjà implémentées. Chaque nouvel algorithme doit
ajouter ses scénarios, erreurs et régressions dans le même changement.
Les résultats logiciels et les essais sur le clavier restent deux preuves distinctes.

## Lancer les tests

Sur le Mac, avec Clang et Python 3 :

```sh
bash scripts/test.sh
```

Les **29 suites C** utilisent ASan/UBSan et des horloges contrôlées, sans clavier
ni attente réelle. Les **112 cas de protocole** passent dans les vrais validateurs
AWK. Si `pwsh` est disponible, 81 cas passent aussi dans les validateurs PowerShell.
Sinon ce contrôle est explicitement signalé comme non exécuté. Un runtime local
peut être fourni avec `POWERSHELL=/chemin/vers/pwsh bash scripts/test.sh`.
Les sorties sont isolées dans `.local/tests/`.

Après `make setup`, les six tests du vrai noyau Zephyr et du code d’activité ZMK
s’exécutent dans QEMU ARM, avec l’image et les sources figées du build :

```sh
bash scripts/test-zephyr.sh
```

Le conteneur est éphémère, sans réseau ni périphérique USB. QEMU est arrêté à son
verdict ou à l’expiration du délai, sans toucher aux autres processus.
L’app dispose séparément de 77 tests Swift, sans connexion Bluetooth ni écriture
des préférences système : mesures, choix V1/V2, persistance simulée, générations
de lecture, menu AppKit et inscription de session simulée.
Le diagnostic d’identité ajoute le format USB/BLE, les annonces, leur expiration,
les contradictions et conflits, ainsi que le fichier d’identité isolé dans un
dossier temporaire. Il ne crée pas l’identité réelle du Mac pendant les tests.
Le statut du clavier vérifie OS, transport, format strict, contexte et expiration.
La fenêtre unifiée est mise en page hors écran, y compris ses détails dépliés.
Le tableau couvre les liaisons du même contrôleur, les noms identiques, le clavier
préféré absent, les contextes périmés et la saisie dirigée vers un autre ordinateur.
Ces contrôles ne valident pas encore son rendu visuel sur V2.
La priorité USB couvre preuve obligatoire, contexte remplacé, attente de touches,
expiration, garde avant envoi et rapports protégés sur le canal USB non autorisé.
Les pilotes natifs sont remplacés aux frontières dans ces harnais. La compilation
nRF et l’inspection du binaire vérifient les points d’intégration, pas la radio réelle.
Le cycle en arrière-plan de l’app teste sommeil, arrêt explicite et reprise après erreur.

```sh
make app test
```

Le compagnon Windows possède aussi un noyau C# testé sans API Windows, Bluetooth
ou préférences système. Un SDK .NET 10 est requis pour lancer ses tests depuis le Mac :

```sh
make app test windows
```

Cette commande compile aussi 16 tests des vrais contrôles WinForms : panneau et
fenêtre, polices de taille normale à triple, nom long, texte non rogné, absence de
chevauchement, ouverture/réouverture réelle via ShowNear, largeur initiale,
icône/logo présents, agrandissement et fermeture masquante. Sur Mac, leur exécution
est explicitement ignorée, car WinForms exige Windows. Les polices agrandies
ne remplacent pas un essai réel de DPI ou de changement de moniteur.
Avec le SDK .NET sur Windows, depuis la racine du dépôt :

```powershell
dotnet run --project companion/windows/UiTests --configuration Release --runtime win-x64 -p:SplitergollKeyboardName=Splitergoll
```

Ce test ouvre brièvement les fenêtres, sans lecture Bluetooth ni app active.
Il ne faut pas installer le SDK pour l’usage normal du portable.

La compilation `make app build windows` produit ensuite le portable win-x64.
Elle ne valide pas le rendu GDI/WinForms, les pilotes GATT ni les protections du PC.

## Couverture

| Sujet | Vérifications automatisées |
| --- | --- |
| Détection USB | Motifs Mac/Windows, ambiguïtés, callbacks, HID prêt, reset/déconnexion, suspend/reprise, touches précoces, erreurs natives |
| Détection BLE | Filtrage hôte/split, paquets intacts, exclusion du contenu privé, acquisition initiale, reconnexion, remplacement de bond et débordements |
| Mémoire | Restauration, identité vérifiée, erreurs NVS, déduplication et observation d’une écriture sans écriture supplémentaire |
| Choix d’appareil | Retour du préféré, perte partielle/totale, choix manuel, expiration, tampon de saisie, releases, rapports clavier/média et modificateurs en cours |
| Wire USB seul | Annonces hôtes bloquées après demandes répétées, paramètres intacts et aucun appel de coupure radio |
| Appairage | Slots libres/pleins, nouveaux bonds, fin tardive du SMP, borne de 30 secondes, échecs et conservation du split |
| Activité | Fenêtre avant/pendant/après IDLE, recherche continue, fermeture en SLEEP, aucune activité artificielle, USB bloquant SLEEP |
| Diagnostics | Formats anciens et courant, troncatures, champs inattendus, capacité, pertes, lectures répétées et journal RAM |
| App macOS | Décodage batterie, indisponibilité, symboles et politique d’ouverture de session |
| Identité USB/BLE | Moteur réel et adaptateurs : refus non appairé/non chiffré/split, trames fragmentées/anciennes, contextes renouvelés, expiration, conflits et callback USB natif préservé |
| App Windows | Présentation GATT, niveaux, sessions périmées, préférences JSON, choix explicite, ambiguïtés et largeur des textes |

Les tests d’intégration exécutent les vrais adaptateurs du dépôt. Seules leurs
frontières radio, ZMK, USB ou stockage sont remplacées. QEMU ajoute le vrai
ordonnanceur, les timers et `activity.c` de ZMK. Ses six cas ne font pas tourner
le firmware complet, le contrôleur radio ou la flash NVS du nRF52840.

## Ce qui reste matériel

Les GPIO de réveil, la recharge, la consommation, les perturbations radio et les
pilotes des vrais ordinateurs exigent encore des essais ciblés. Les signatures
restent des heuristiques, sans garantie pour tous les ordinateurs.

Sur V2, le court essai du [journal sur batterie](diagnostic-activite.md) est positif :
fermeture de fenêtre puis IDLE à gauche, sans reset ni réveil au retour USB.
La droite n’a pas son propre journal. On ne répète pas la série Mac/Windows de
V1 ni une attente de deux minutes. Le sommeil profond court est un essai séparé,
à lancer seulement pour une question de réveil. USB/BLE unifié, Gaming persistant,
correction d’OS et détection Linux restent à développer, pas validés par cette suite.
