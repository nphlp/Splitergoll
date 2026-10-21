# Application macOS

## Intention

Premier lot limité aux deux batteries. Une fenêtre et un indicateur dans la barre
des menus, en utilisant les services GATT existants. L’app reste facultative pour la saisie.
Le guide humain est [application-macos.md](../../docs/application-macos.md).
Les évolutions sont suivies dans le [TODO](../../docs/TODO.md), sans liste dupliquée ici.

## Organisation

- `BatteryCore` : décodage GATT, identité des côtés, dernières lectures et disponibilité.
- `BatteryMonitor` : adaptateur CoreBluetooth, entièrement sur l’acteur principal.
- `BatteriesView` : vue observée et tableau de données indépendant, utilisable pour contrôler le rendu sans Bluetooth.
- `Application` : cycle AppKit, fenêtre SwiftUI fixe 320 × 174 points et NSStatusItem. Dock visible avec la fenêtre, caché lorsqu’elle est fermée. Démarrage discret sans fenêtre.
- `LaunchAtLogin` : inscription via SMAppService au premier lancement réel. `LoginRegistration` teste sa politique sans toucher aux préférences système.
- `MenuBarBadge` : image template de deux lignes, gauche en haut et droite en bas, avec texte équivalent pour l’accessibilité.
- `Assets/AppIcon.svg` : icône sélectionnée, silhouette noire du PCB gauche sur carré blanc arrondi de style macOS. Les angles du contour sont adoucis, sans touches ni encodeur.
- `Assets/pcb-left.svg` : dessin séparé de la moitié gauche, sur fond transparent. Non utilisé dans l’icône ni dans le build, à la demande de l’utilisateur.
- `Assets/pcb-left-white-on-black.svg` et `pcb-left-black-on-white.svg` : variantes conservées côte à côte. Canvas 1024, coins extérieurs transparents. AppIcon.svg reprend la version sur fond blanc, garder cette copie cohérente en cas d’édition.
- `Assets/RenderIcon.swift` : rasterise le SVG en dix PNG, assemblés en ICNS par iconutil. Cache sous .local, invalidé sur changement du SVG ou du convertisseur. Ressource embarquée avant signature.
- `app.sh` : SwiftPM sans dépendance externe, bundle local et signature ad hoc. Aucun ajout à `/Applications`.

Interface : make app affiche l’aide, make app start compile et ouvre, make app build
compile seulement, make app test teste. Toute action erronée affiche l’aide avant
contrôle des outils, création de cache ou accès Bluetooth. Pas d’alias à tirets.

Le nom Bluetooth vient de `CONFIG_ZMK_KEYBOARD_NAME` dans la configuration de la
centrale et est injecté dans le bundle. Aucun nom implicite en cas de configuration
manquante. Tous les chemins sont relatifs au checkout. Les caches et le bundle sont
sous `.local/`, avec `CLANG_MODULE_CACHE_PATH` limité aux sous-processus du script.

## Invariants

Pas de scan, de nouvel appairage, de sélection de destination ou d’accès HID.
Retrouver uniquement un périphérique déjà connecté au système. Refuser de choisir
arbitrairement entre plusieurs correspondances. La connexion cliente CoreBluetooth
est distincte de la connexion du clavier gérée par macOS.

Lire `180F/2A19` et `2904`, sans écrire le CCC ni activer les notifications.
Identifier gauche/droite par `0x0106/0x0108`, jamais par l’ordre des services.
Refuser formats invalides, niveaux hors de 0 à 100 et côtés en doublon. Une seule
batterie identifiée peut être présentée, l’autre reste inconnue.

Ne conserver qu’une lecture par côté. La date représente sa réception par le Mac,
jamais l’échantillonnage ADC. Ne pas déduire une charge certaine ou le niveau réel de 100 %.
Une lecture actuelle de 100 % affiche le symbole battery.100percent.bolt, dans
la fenêtre et la barre. Garder « charge possible » pour l’accessibilité et la limite en infobulle.
La donnée brute reste 100. Une batterie pleine sans USB produit aussi ce résultat.
Une lecture ancienne n’affiche jamais cette indication de charge possible.
Ne pas ajouter de panneau d’explication dans la fenêtre minimale.
Les lectures ont lieu toutes les 5 secondes, sans chevauchement. Après 90 secondes
sans lecture ou en perte de connexion, la barre affiche `-`. Les anciennes valeurs
restent seulement en mémoire et ne sont plus affichées. Elles ne survivent pas au processus.
Une nouvelle session vide identités et lectures en attente. Un autre périphérique
réinitialise aussi les valeurs historiques.

Un battement toutes les 5 secondes contrôle connexions et échéances, avec une
tolérance de 250 ms. Il demande directement une lecture, sans second seuil de
cadence qui ferait sauter un battement en cas de léger décalage.
Garde de connexion/lecture 15 secondes contrôlé par ce battement, sans délai temps réel
garanti. Après erreur, attendre 30 secondes avant une nouvelle tentative.
Attendre la clôture cliente avant de reconnecter. Aucun bouton d’actualisation.
Au sommeil du Mac, libérer le lecteur et invalider le timer. Au réveil, retrouver
la connexion système. Aucun verrou ou assertion empêchant la veille, aucun scan de reprise.
Fermer la fenêtre conserve le menu. Quitter ferme uniquement les ressources de l’app.

La fenêtre ne contient que le titre, Connecté/Déconnecté et les batteries gauche/droite.
Le menu contient connexion, Ouvrir l’app et Quitter l’app. Les erreurs du lecteur
ne sont pas détaillées dans cette interface. Ne pas y remettre les dates et cadences.
LSUIElement et la politique accessory masquent le Dock au démarrage. showWindow
passe en regular avant de montrer la fenêtre. Son delegate windowWillClose revient
en accessory, sans quitter le lecteur. Une minimisation conserve le Dock, car la
fenêtre reste ouverte. L’app ne montre la fenêtre que sur une action explicite,
jamais au démarrage de session.
L’inscription de session a lieu au lancement réel, pas lors du build ou des tests.
Conserver un marqueur après inscription réussie ou en attente d’approbation et ne
pas réinscrire un service désactivé ultérieurement. Signaler un échec sans l’annoncer
comme actif. Le bundle est autonome et copiable dans Applications par l’utilisateur.
Quitter avant copie ou remplacement et relancer depuis le chemin installé. Vérifier
l’entrée d’ouverture de session si l’ancienne copie locale avait été enregistrée.
make clear supprime seulement la copie locale et les caches, jamais l’app installée.
Le profil Bluetooth mesure désormais toutes les 5 secondes pendant ACTIVE, après
reflash des deux moitiés. IDLE/SLEEP arrêtent ce timer, sans forçage ADC par l’app.

La perte isolée de droite n’est pas déductible d’un niveau inchangé. Le getter
split/central.c restitue le cache sans état de lien et central_bas_proxy.c le sert
toujours. Ne pas afficher arbitrairement hors ligne ou zéro parce que ce niveau
est stable. Le tiret indique une donnée connue indisponible par le lecteur.
Le vrai état individuel demandera une télémétrie firmware séparée, notée au TODO.

## Validation

`make app test` exécute les tests applicatifs sans Bluetooth. Couvrir décodage,
réponses hors ordre, identités en doublon, valeurs invalides, lecture partielle,
péremption, horloge reculant et reconnexion sans reprise de canaux anciens.
`make app build` compile avec les avertissements traités comme erreurs et vérifie
la signature du bundle. `swift format lint --strict --recursive companion/macos/Sources companion/macos/Tests companion/macos/Package.swift`
contrôle les sources avec `.swift-format`. Inclure aussi Assets dans ce contrôle.
Pas de nouvelle suite de tests du script shell ou du dessin de l’icône.

Le contrôle du rendu hors écran utilise uniquement des données fictives et ne
démarre pas BatteryMonitor. Les essais Bluetooth réels et sommeil du Mac restent
distincts des tests du modèle et nécessitent une séance avec l’utilisateur.
Ne pas lancer l’app réelle en arrière-plan implicitement pendant un build ou un test.

Vingt et un tests passent, dont l’indication incertaine à cent pour cent, sa disparition
sur lecture périmée, l’ordre vertical gauche/droite et les erreurs de l’inscription
de session, sans ServiceManagement réel ni écriture de préférences. Les états connecté, charge
possible, en attente et déconnecté
ont été rendus hors écran avec des données fictives, en clair et sombre.
Les artefacts privés sont sous .local/validation/macos-app. Aucune lecture réelle
de V2 ou ouverture de fenêtre de l’app n’a été faite par l’agent pour ces contrôles.
Le symbole avec éclair, le tiret et l’icône originale sont rendus hors écran.
Le bundle final intègre le fichier AppIcon.icns avant signature. Une copie dans
un autre dossier privé du projet conserve exécutable, configuration et signature.
Les dépendances dynamiques sont uniquement celles de macOS, aucun chemin du dépôt
n’est nécessaire à l’exécution. La copie réelle vers Applications et sa relance
restent à l’utilisateur, ainsi que la vérification de l’ouverture de session.
La présence conditionnelle dans le Dock se valide manuellement : une simulation
de booléens ne vérifierait pas le comportement AppKit réel. Les tests de modèle
et la compilation ne lancent pas de fenêtre ni de lecture Bluetooth implicitement.
L’icône SVG est validée XML, rendue par AppKit puis embarquée en ICNS. Un seul
chemin fermé et un fond carré arrondi, sans touches ni encodeur. Le contour du PCB
gauche est adouci pour cette silhouette graphique, pas une exportation de fabrication.
Les deux variantes restent disponibles. Build signé et 21 tests de modèle passent.
Le Dock contextuel reste à essayer en ouvrant, minimisant et fermant la fenêtre réelle.
