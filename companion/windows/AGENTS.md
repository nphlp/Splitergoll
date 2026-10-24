# Application Windows

## Intention

Compagnon facultatif pour les deux batteries, sans changement de firmware.
Même comportement que l’app Mac, interface adaptée à Windows. Une icône unique
affiche gauche au-dessus de droite, avec `%` seulement si la lisibilité le permet.
Un panneau près de l’icône montre les valeurs exactes et permet d’ouvrir la fenêtre.
Le guide humain reste dans docs/application-windows.md.

## Invariants

- C#/.NET 10 LTS, Windows Forms et WinRT GATT. Noyau de décodage et sélection sans
  dépendance Windows, testé sur le Mac. Aucun test ne lance le Bluetooth ou l’app.
- Inventorier uniquement les périphériques BLE appairés et déjà connectés au système.
  Aucun scan radio, appairage, effacement, sélection d’hôte ou écriture GATT.
- BAS 180F, niveau 2A19, présentation 2904 de sept octets strictement validés.
  Identifiants 0106/0108 pour gauche/droite, jamais ordre des services.
  Refuser les côtés dupliqués et les niveaux hors de 0 à 100. Une seule moitié
  valide peut être affichée, l’autre reste indisponible.
- Lecture toutes les cinq secondes sans chevauchement. Relâcher les sessions à
  la suspension et à la fermeture, ne pas empêcher le sommeil du PC. Annuler et
  invalider les anciennes sessions à la sélection pour rejeter les réponses tardives.
- Préférence utilisateur par identifiant système et nom, jamais par OS ou numéro
  de slot du firmware. Aucun repli silencieux si le clavier préféré est absent.
- Zéro et lecture indisponible donnent un tiret. Cent donne un éclair de charge
  possible, jamais une preuve de courant de charge ou de batterie pleine.
  Les lectures sont horodatées à réception et périmées après 90 secondes.
- L’app démarre dans la zone de notification sans fenêtre. Fermer la fenêtre
  conserve l’app, quitter libère uniquement ses ressources. Le lancement de session
  est explicite, sans administrateur, aucune inscription pendant les tests/builds.
- Icône calculée à la taille du système et redessinée aux changements DPI/thème.
  Mesurer les glyphes, décider de `%` sur une largeur maximale stable. Ne pas
  étirer les chiffres pour les faire entrer. La lisibilité réelle exige Windows.
- Les deux vues utilisent des lignes AutoSize et des labels à hauteur préférée,
  jamais des hauteurs absolues autour du texte. La Form garde AutoSize désactivé,
  une largeur de départ de 255 unités logiques et une largeur minimale au DPI courant.
  Calculer la hauteur du tableau à largeur connue via GetPreferredSize, puis la taille
  minimale de fenêtre. Le panneau ajuste sa hauteur, la fenêtre principale est
  redimensionnable au-dessus du minimum de contenu. Ne pas réintroduire la boucle
  Form AutoSize + colonnes Percent qui pouvait écraser les vues.
  Replacer le panneau après Show et le calcul de layout/DPI, avec marge mise à l’échelle.
- Le même ICO multi-résolution est ApplicationIcon et ressource managée embarquée.
  Affecter explicitement Form.Icon pour la barre de titre et la barre des tâches.
  Le logo de contenu est un PictureBox Zoom à partir de la version 256 pixels,
  dans les deux vues. Libérer les copies graphiques à la destruction de la vue.
- Exécutable portable autonome win-x64 pour le PC de l’utilisateur, pas de MSI/MSIX
  dans ce premier lot. Mode de démonstration explicite sans aucun accès Bluetooth.
  Une seule instance par session utilisateur. Aucun certificat installé ou acheté,
  aucune désactivation de SmartScreen, Smart App Control ou Defender.
- Paramètres sous LocalApplicationData via les API Windows, caches/builds sous
  .local, livrables sous dist. Les chemins ne dépendent pas du nom de l’utilisateur.
  Le nom cible provient de la configuration centrale lors du build.

## Validation

Tester décodage, sessions, unicité, valeurs manquantes, péremption, sélection,
préférence absente et format de l’icône. Compiler le binaire Windows et vérifier
les avertissements. Le rendu WinForms/GDI, le GATT Windows, la reprise, le DPI,
SmartScreen et l’ouverture de session demandent ensuite le PC et V2.

Premier portable win-x64 compilé sur Mac, sans avertissement. Les 53 cas du noyau
Windows et les 44 tests Swift existants passent. Formatage, syntaxe des scripts
et diff contrôlés. Aucun lancement Windows, accès GATT réel ou changement de
préférence de session n’est réalisé par ces contrôles. L’utilisateur confirme
ensuite sur Windows la lecture réelle de V2 (89/79 puis 88/80 sur ses photos),
le menu contextuel et le choix du clavier visible. Ces observations ne valident
pas toutes les préférences, le sommeil du PC ou son lancement de session.
Une erreur d’écriture
de préférence doit conserver la nouvelle sélection et invalider les anciennes
mesures avant d’afficher l’erreur, jamais présenter l’ancien clavier sous le nouveau nom.

Les premières photos révèlent des titres/statuts rognés dans les deux vues sur
l’écran agrandi. Après la première correction, l’utilisateur rapporte un panneau
réduit à une barre verticale et une fenêtre trop étroite mais lisible après
étirement. Les trois nouvelles photos sont absentes des chemins fournis, donc
ce deuxième résultat repose sur son récit, pas une inspection visuelle.
La correction actuelle stabilise la largeur, mesure ensuite la hauteur et impose
un minimum de contenu, avec icône explicitement affectée et logo visible.
Le rendu de TrayRenderer reste inchangé à la demande de l’utilisateur : ne pas
attribuer le halo photographié au bitmap sans preuve.
Les 16 tests UI exercent les vrais contrôles, polices x1/x1,5/x2/x3, noms longs,
chevauchements, ouverture via ShowNear, largeur minimale, icône/logo présents,
réouverture, agrandissement et fermeture masquante. Ils ne simulent pas le DPI
réel du moniteur. Sur Mac ils sont compilés, pas exécutés, avec SKIP explicite.
L’utilisateur juge ensuite la base satisfaisante mais trop large et demande les
trois quarts de la largeur précédente, soit 255 au lieu de 340 unités logiques.
Cette réduction conserve les gardes de contenu et le DPI, sans nouvelle validation
physique de la largeur réduite. L’amélioration visuelle avec les silhouettes gauche
et droite est reportée dans docs/TODO.md. Aucun flash ou réappairage requis.

## Ideas / backlog

Identité commune USB/BLE par les compagnons : prochain chantier retenu, sans
identifiant hôte ni protocole d’écriture dans le lot batteries. Commencer par
un diagnostic de correspondance, sans changement de saisie. Voir docs/identite-ordinateur.md.
Interface Windows : deux cartes batterie avec les SVG gauche et droite existants
et leurs pourcentages, dans le panneau et la fenêtre. Voir docs/TODO.md.
