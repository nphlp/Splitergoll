# Application macOS

L’app affiche les deux batteries du Splitergoll connecté en Bluetooth au Mac.
Elle reste facultative pour taper et pour sélectionner la keymap.

## Lancer l’app

`make app` affiche les actions disponibles, sans lancer l’app.

Sur macOS 13 ou plus récent, avec Swift 6 fourni par les outils Xcode :

```sh
make app start
```

L’app démarre dans la barre des menus, sans icône dans le Dock ni fenêtre ouverte.
Le menu contient seulement **Connecté / Déconnecté**, **Ouvrir l’app** et **Quitter l’app**.
La fenêtre fixe de 320 × 174 points montre le titre, la connexion et les batteries
**Gauche** et **Droite**. Fermer la fenêtre laisse l’app fonctionner.
Les deux lignes de la barre affichent gauche au-dessus de droite.
Le Dock montre l’icône tant que la fenêtre est ouverte, y compris minimisée.
Fermer la fenêtre la retire du Dock, sans quitter l’app ni retirer son menu.

Autoriser le Bluetooth si macOS le demande. Le clavier doit déjà être connecté :
l’app ne lance pas d’appairage et ne change pas l’ordinateur destinataire.
Un niveau indisponible est remplacé par `-`, sans afficher d’ancienne valeur.
À 100 %, une **batterie avec éclair** remplace le pourcentage. Cette convention
ne confirme pas la charge : une batterie pleine sans USB donne aussi 100 %.
Le détail reste accessible en survolant l’icône.
La déconnexion isolée de droite n’est pas encore signalée : le proxy peut garder
son ancien niveau. Un retour d’état du firmware sera nécessaire pour ce cas.

## Installer dans Applications

Le bundle `.local/apps/Splitergoll.app` contient l’exécutable et son icône.
Il fonctionne sans le dépôt, Swift ou Docker une fois compilé.
La source [AppIcon.svg](../companion/macos/Assets/AppIcon.svg) représente la silhouette
noire du PCB gauche, aux angles adoucis, sur un carré blanc arrondi.
Le build convertit ce SVG en icône macOS. La variante sur fond noir reste à côté
dans `companion/macos/Assets/`.

1. Quitter Splitergoll depuis la barre des menus.
2. Copier `.local/apps/Splitergoll.app` dans le dossier **Applications** du Mac.
3. Ouvrir cette copie depuis Applications et autoriser le Bluetooth si demandé.
4. Vérifier son entrée dans Réglages Système > Général > Ouverture de session.
   Si l’entrée désigne encore la copie locale, la retirer et ajouter celle d’Applications.

Les nouveaux builds restent dans `.local/apps/`. Pour mettre à jour la copie
installée, quitter l’app, remplacer son bundle dans Applications, puis le rouvrir.

## Ouverture de session

Au premier lancement, l’app demande son inscription au démarrage de la session
avec [SMAppService, l’API native Apple](https://developer.apple.com/documentation/servicemanagement/smappservice/mainapp).
Si macOS demande une approbation, un message propose d’ouvrir les réglages système.
Une désactivation ultérieure dans ces réglages est respectée.
Le lancement à l’ouverture de session reste discret, sans fenêtre.

`make clear` supprime le bundle local et ses caches, sans toucher à la copie
installée dans Applications. Éviter d’utiliser les deux copies en parallèle.

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
`make app test` teste le modèle et l’inscription de session avec des services simulés.
`make bat read` et `make bat avg` restent disponibles séparément.

Essayer la fenêtre, sa fermeture, la réouverture et Quitter depuis la barre.
Vérifier que le Dock apparaît à l’ouverture et disparaît à la fermeture.
Vérifier ensuite déconnexion/reconnexion, sommeil du Mac et ouverture de session.
Les couches, l’OS actif et les options avancées restent dans le [TODO](TODO.md).
