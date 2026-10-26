# Application macOS

L’app affiche les deux batteries du Splitergoll connecté en Bluetooth au Mac,
son OS actif et le rapprochement USB/Bluetooth avec le firmware de diagnostic.
Elle reste facultative pour taper et pour sélectionner la keymap.

## Lancer l’app

`make app` affiche les actions disponibles, sans lancer l’app.

Sur macOS 13 ou plus récent, avec Swift 6 fourni par les outils Xcode :

```sh
make app start
```

L’app démarre dans la barre des menus, sans icône dans le Dock ni fenêtre ouverte.
Le menu contient **Connecté / Déconnecté**, **Clavier suivi**, **Ouvrir l’app** et **Quitter l’app**.
La fenêtre compacte réunit un tableau **Clavier / Liaison / État**, la connexion, les deux
batteries avec leurs contours, l’**OS actif**, la liaison de **Saisie** et le
rapprochement **USB/Bluetooth**. Sa largeur reste fixe à 380 points et sa hauteur
s’adapte au contenu. Les détails et **Copier le diagnostic** sont repliés.
Fermer la fenêtre laisse les batteries fonctionner dans la barre des menus.
Chaque clavier Bluetooth connu apparaît. Pour le clavier suivi, une ligne USB
s’ajoute lorsque le diagnostic confirme le même contrôleur. Cliquer sur une ligne
choisit les informations à suivre, sans changer la destination des frappes.
La coche et **Sélectionné** indiquent uniquement la liaison utilisée vers ce Mac,
**Autre ordinateur**
une saisie Bluetooth vers un autre hôte. **Diagnostic** signifie que le firmware
`identity` réserve USB au rapprochement. Le [profil usb-priority](priorite-usb.md)
permet la vraie bascule : USB **Sélectionné**, Bluetooth **Désactivé** pour la saisie.
La connexion Bluetooth reste présente. Le nom au-dessus des batteries précise le
clavier suivi indépendamment de la coche de saisie.
Les deux lignes de la barre affichent le contour gauche suivi de son niveau en haut,
puis le niveau et le contour droit en bas. Les contours suivent le thème clair ou sombre.
Le Dock montre l’icône tant que la fenêtre est ouverte, y compris minimisée.
Fermer la fenêtre la retire du Dock, sans quitter l’app ni retirer son menu.

Autoriser le Bluetooth si macOS le demande. Le clavier doit déjà être connecté :
l’app ne lance pas d’appairage et ne change pas l’ordinateur destinataire.
Le nom `Splitergoll` est reconnu, ainsi qu’un nom personnalisé commençant par
`Splitergoll `, comme `Splitergoll V1` ou `Splitergoll V2`. Si plusieurs correspondent,
l’app ne choisit pas à ta place au démarrage et laisse les niveaux indisponibles.
Si elle suit déjà un clavier, elle continue à le lire quand un second apparaît.
Le [sélecteur V1/V2](selection-clavier-app.md) mémorise ton choix explicite. Si ce clavier
est absent, son nom reste affiché avec des tirets, sans suivre l’autre à sa place.
Un niveau indisponible ou égal à **0 %** est remplacé par `-`, sans afficher d’ancienne valeur.
Le zéro reste conservé dans la mesure brute : cette convention ne prouve pas une déconnexion.
À 100 %, un **éclair** remplace le pourcentage, horizontal et agrandi dans la barre,
vertical dans la fenêtre. Cette convention
ne confirme pas la charge : une batterie pleine sans USB donne aussi 100 %.
Le détail reste accessible en survolant l’icône.
À la perte de la liaison droite, le ZMK utilisé envoie zéro : le tiret apparaît
après détection radio et réception de cette valeur. Si la gauche se déconnecte du
Mac, les deux niveaux sont masqués. Les lectures périmées sont également masquées.
Le zéro peut aussi être une batterie vide : ce n’est pas un état de connexion distinct.
Sur V2, les tirets à la déconnexion et les éclairs au branchement USB des deux
moitiés ont été confirmés par l’utilisateur.

## Installer dans Applications

Le bundle `.local/apps/Splitergoll.app` contient l’exécutable et son icône.
Il fonctionne sans le dépôt, Swift ou Docker une fois compilé.
La source [AppIcon.svg](../companion/macos/Assets/AppIcon.svg) représente le contour
noir épais du PCB gauche, avec une ombre extérieure centrée sur un carré blanc arrondi.
Le build convertit ce SVG en icône macOS. Les autres variantes restent dans
`companion/macos/Assets/`.

1. Compiler puis installer le bundle :

   ```sh
   make app build
   make app install
   ```

2. Autoriser le Bluetooth si macOS le demande au lancement de la copie installée.
3. Vérifier son entrée dans Réglages Système > Général > Ouverture de session.
   Si l’entrée désigne encore la copie locale, la retirer et ajouter celle d’Applications.

`install` vérifie et prépare le bundle déjà compilé, demande la fermeture normale
des copies locale et installée, puis remplace et ouvre l’app installée. Il attend
au plus dix secondes, sans fermeture forcée. Une autre copie ouverte ailleurs
ou une application de mauvaise identité arrête l’installation.
Une mise à jour remplace l’ancien bundle, sans conserver ses ressources périmées.
En cas d’échec du déplacement, l’ancienne copie est restaurée.
Le dossier Applications vient de macOS. Une destination explicite peut être fournie
par `APP_INSTALL_DIRECTORY`, notamment pour une validation isolée.

## Ouverture de session

Au premier lancement, l’app demande son inscription au démarrage de la session
avec [SMAppService, l’API native Apple](https://developer.apple.com/documentation/servicemanagement/smappservice/mainapp).
Si macOS demande une approbation, un message propose d’ouvrir les réglages système.
Une désactivation ultérieure dans ces réglages est respectée.
Le lancement à l’ouverture de session reste discret, sans fenêtre.

`make clear` supprime le bundle local et ses caches, sans toucher à la copie
installée dans Applications. `make app start` ouvre toujours la copie locale de développement.
Éviter d’utiliser les deux copies en parallèle.

## Actualisation

L’app relit les valeurs toutes les **5 secondes**. Les nouveaux firmwares Bluetooth
mesurent chaque batterie toutes les **5 secondes pendant l’activité**, après reflash
des deux moitiés. Les anciennes versions peuvent encore mesurer toutes les 60 secondes.
Les mesures s’arrêtent en inactivité et reprennent au réveil d’activité.
Une lecture par le Mac ne force pas une mesure physique.

L’app suspend ses lectures pendant le sommeil du Mac et tente de retrouver le clavier
au réveil. L’autonomie, la reprise réelle et le lancement à l’ouverture de session
restent à valider sur V2. Sous USB, les niveaux ne mesurent pas la charge restante,
voir [batteries](batteries.md).

## Vérifier et faire évoluer

`make app build` compile sans ouvrir l’app ni modifier les réglages système.
`make app test` teste le modèle, l’inscription de session avec des services simulés
et le rendu des contours de la barre, sans accès Bluetooth.
`make bat read` et `make bat avg` restent disponibles séparément.

Essayer la fenêtre, sa fermeture, la réouverture et Quitter depuis la barre.
Vérifier que le Dock apparaît à l’ouverture et disparaît à la fermeture.
Vérifier ensuite déconnexion/reconnexion, sommeil du Mac et ouverture de session.

## Informations du clavier

L’app entretient l’[identité commune](diagnostic-identite.md) du clavier suivi en
arrière-plan, même fenêtre fermée, pour permettre la priorité USB. Le fichier local
est créé à la première utilisation d’un clavier suivi disponible. Le sommeil du
Mac suspend les échanges, le réveil permet leur reprise. **Arrêter** dans les
détails est respecté jusqu’à **Relancer** ou au choix d’un autre clavier.

L’OS actif et la sortie de saisie sont lus dans le **clavier**, jamais déduits de
l’OS du Mac ou de la présence du câble. Un ancien firmware affiche **Non communiqué**.
Ces informations sont relues toutes les cinq secondes et masquées si elles expirent.
Le profil `identity` compare sans changer les frappes. Le profil `usb-priority`
utilise la correspondance pour sélectionner la sortie USB. Quitter l’app provoque
le repli Bluetooth après expiration. Les couches et les options avancées restent
dans le [TODO](TODO.md).
