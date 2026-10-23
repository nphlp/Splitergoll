# Application macOS

## Intention

Premier lot limité aux deux batteries. Une fenêtre et un indicateur dans la barre
des menus, en utilisant les services GATT existants. L’app reste facultative pour la saisie.
Le guide humain est [application-macos.md](../../docs/application-macos.md).
Les évolutions sont suivies dans le [TODO](../../docs/TODO.md), sans liste dupliquée ici.

## Organisation

- `BatteryCore` : décodage GATT, identité des côtés, dernières lectures et disponibilité.
  BatterySymbolLayout conserve les proportions du symbole dans le cadre de chaque ligne.
- `BatteryMonitor` : adaptateur CoreBluetooth, entièrement sur l’acteur principal.
- `BatteriesView` : vue observée et tableau de données indépendant, utilisable pour contrôler le rendu sans Bluetooth.
- `Application` : cycle AppKit, fenêtre SwiftUI fixe 320 × 174 points et NSStatusItem. Dock visible avec la fenêtre, caché lorsqu’elle est fermée. Démarrage discret sans fenêtre.
- `LaunchAtLogin` : inscription via SMAppService au premier lancement réel. `LoginRegistration` teste sa politique sans toucher aux préférences système.
- `MenuBarBadge` : image template de deux lignes, contour gauche puis valeur en haut,
  valeur puis contour droit en bas. Largeur stable, symboles monochromes adaptés au
  thème macOS et texte équivalent pour l’accessibilité.
- `Assets/AppIcon.svg` : contour noir épais de 32 px du PCB gauche sur carré blanc
  arrondi de style macOS. Ombre extérieure centrée, sigma 24 px, noir à 32 %.
  Les angles sont adoucis, sans touches ni encodeur.
- `Sources/SplitergollBatteries/Resources/keyboard-left.svg` et `keyboard-right.svg` :
  contours transparents validés, chargés via Bundle.module et embarqués avec SwiftPM.
- `Assets/pcb-left.svg` : dessin séparé de la moitié gauche, sur fond transparent. Non utilisé dans l’icône ni dans le build, à la demande de l’utilisateur.
- `Assets/pcb-left-white-on-black.svg` et `pcb-left-black-on-white.svg` : anciennes
  silhouettes conservées côte à côte. Canvas 1024, coins extérieurs transparents.
- `Assets/proposals/` : propositions SVG conservées pour comparer les variantes.
  Trois épaisseurs (16/24/32 px sur canvas 1024) et ombres intérieure/extérieure/relief.
  Les ombres utilisent feGaussianBlur, sans image intégrée. La planche est un SVG autonome.
  Elles sont centrées, offsets X/Y nuls : intérieure sigma 20 à 50 %, extérieure sigma
  24 à 32 %, relief sigma 16/18 à 40/28 %. Aucun éclairage directionnel.
  Les premiers essais de tracés superposés avaient des paliers et sont remplacés.
  L’utilisateur retient app-outline-bold-outer.svg pour l’app et les deux contours
  de menus. Leurs copies actives sont AppIcon.svg et Resources/keyboard-*.svg.
- `Assets/RenderIcon.swift` : lit le filtre extérieur du SVG, sépare contour et fond,
  puis reproduit son flou avec Core Image car CoreSVG/AppKit l’ignore. Ces API natives
  ne sont utilisées que pendant le build, sans dépendance externe ni image embarquée
  dans le SVG. Dix PNG sont assemblés en ICNS par iconutil. Cache sous .local, invalidé
  sur changement du SVG ou du convertisseur. Ressources embarquées avant signature.
- `app.sh` : SwiftPM sans dépendance externe, bundle local et signature ad hoc. Aucun ajout à `/Applications`.
- `install.sh` : préparation du bundle déjà compilé, fermeture normale des copies
  source/destination, remplacement puis lancement de la copie installée.
  Le dossier Applications est lu par Foundation via JXA, sans chemin système inventé.
  APP_INSTALL_DIRECTORY peut le remplacer explicitement pour une validation isolée.

Interface : make app affiche l’aide, make app start compile et ouvre, make app build
compile seulement, make app install met à jour puis lance le bundle existant, make app test teste.
Toute action erronée affiche l’aide avant
contrôle des outils, création de cache ou accès Bluetooth. Pas d’alias à tirets.

Le nom Bluetooth vient de `CONFIG_ZMK_KEYBOARD_NAME` dans la configuration de la
centrale et est injecté dans le bundle. Aucun nom implicite en cas de configuration
manquante. Tous les chemins sont relatifs au checkout. Les caches et le bundle sont
sous `.local/`, avec `CLANG_MODULE_CACHE_PATH` limité aux sous-processus du script.

## Invariants

Pas de scan, de nouvel appairage, de sélection de destination ou d’accès HID.
Retrouver uniquement un périphérique déjà connecté au système. Le nom configuré
est accepté seul ou suivi d’un espace et d’un suffixe non blanc, pour les libellés
personnalisés comme Splitergoll V1/V2. Garder la casse et la frontière du nom :
ni SplitergollV2, ni Splitergoll_Left, ni un nom cible vide ne correspondent.
Refuser de choisir arbitrairement entre plusieurs correspondances, même si l’une
garde le nom exact. Le lecteur CLI suit la même règle. La connexion cliente CoreBluetooth
est distincte de la connexion du clavier gérée par macOS.

Lire `180F/2A19` et `2904`, sans écrire le CCC ni activer les notifications.
Identifier gauche/droite par `0x0106/0x0108`, jamais par l’ordre des services.
Refuser formats invalides, niveaux hors de 0 à 100 et côtés en doublon. Une seule
batterie identifiée peut être présentée, l’autre reste inconnue.

Ne conserver qu’une lecture par côté. La date représente sa réception par le Mac,
jamais l’échantillonnage ADC. Ne pas déduire une charge certaine ou le niveau réel de 100 %.
Une lecture actuelle de 100 % affiche le symbole bolt.fill seul, vertical dans
la fenêtre et tourné de 90 degrés dans la barre. Garder « charge possible » pour
l’accessibilité et la limite en infobulle.
La donnée brute reste 100. Une batterie pleine sans USB produit aussi ce résultat.
Une lecture ancienne n’affiche jamais cette indication de charge possible.
Une lecture actuelle à zéro affiche un tiret, par convention utilisateur. Le modèle
conserve zéro et sa date, ainsi que l’état connecté. Un niveau de un redevient lisible.
La même indication alimente la fenêtre, le menu et l’accessibilité, sans toucher au lecteur CLI.
Le symbole de charge du menu conserve ses proportions après rotation, avec un
facteur commun limité par la colonne « 99 % » et neuf points de hauteur. Aucun
étirement des axes ni image intermédiaire qui dégraderait le rendu du symbole.
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
make app install refuse les liens symboliques, un bundle de mauvaise identité ou
signature. La source doit être déjà compilée. Après préparation de la copie, demander
la fermeture normale des seules copies source et destination via NSRunningApplication,
avec attente maximale de dix secondes. Une autre copie du même bundle ouverte
ailleurs ou un refus de fermeture bloque la mise à jour, sans forceTerminate ni kill.
Revérifier la fermeture juste avant le remplacement, puis lancer la nouvelle copie
avec open. L’app exécute alors son initialisation normale, dont l’ouverture de session.
Le build et les tests n’effectuent toujours aucun lancement réel.
Préparer et vérifier une copie temporaire sur le volume de destination avant tout
remplacement. Garder l’ancienne app jusqu’à réussite du déplacement, la restaurer
sur échec et nettoyer uniquement le dossier temporaire créé par cette installation.
Ne modifier aucune autre app, permission système ou préférence d’ouverture de session.
Le profil Bluetooth mesure désormais toutes les 5 secondes pendant ACTIVE, après
reflash des deux moitiés. IDLE/SLEEP arrêtent ce timer, sans forçage ADC par l’app.

Le getter split/central.c restitue un cache, mais le callback BLE
split_central_disconnected de central.c envoie BATTERY_EVENT avec level=0. Le worker
met ce cache à zéro, puis le proxy BAS le restitue. L’ancienne conclusion disant
que le cache garde systématiquement le dernier niveau après déconnexion était
incomplète. Le tiret couvre déjà cette situation après détection radio et lecture.
Zéro reste ambigu avec une batterie vide ou une première mesure absente. Une vraie
télémétrie dédiée pourrait distinguer ces états, mais n’est pas nécessaire au rendu
actuellement demandé. Un niveau inchangé ne prouve jamais une déconnexion.

## Validation

Dernier contrôle : 30 tests passent, dont quatre tests AppKit du menu. Le cas
100/100 vérifie l’éclair horizontal seul sur chaque ligne, neuf points de hauteur,
plus large que haut, sans déformation ni pourcentage ajouté.
Le format Swift, la syntaxe shell, le build et la signature passent. L’installation
réelle est validée app fermée puis app ouverte, avec fermeture normale et relance
depuis Applications. Une seule copie installée reste active, avec un nouveau PID.
Pendant l’attente de fermeture, faire tourner NSRunLoop pour recevoir la notification
NSRunningApplication, plutôt que bloquer son actualisation avec un sleep.
Aucun firmware modifié ni flashé dans ce lot. L’utilisateur confirme sur V2 :
droite déconnectée donne un tiret droit, gauche déconnectée donne deux tirets,
branchement USB de chaque moitié donne son éclair. Cela valide ces indications,
pas une mesure de capacité réelle ou une détection physique du courant de charge.

`make app test` exécute les tests applicatifs sans Bluetooth. Couvrir décodage,
réponses hors ordre, identités en doublon, valeurs invalides, lecture partielle,
péremption, horloge reculant et reconnexion sans reprise de canaux anciens.
Les tests AppKit des menus vérifient les contours miroirs/transparents, leur placement
avec charge/zéro, leur disponibilité depuis le bundle et la largeur stable des états.
`make app build` compile avec les avertissements traités comme erreurs et vérifie
la signature du bundle. `swift format lint --strict --recursive companion/macos/Sources companion/macos/Tests companion/macos/Package.swift`
contrôle les sources avec `.swift-format`. Inclure aussi Assets dans ce contrôle.
Pas de nouvelle suite de tests du script shell ou du dessin de l’icône.

Le contrôle du rendu hors écran utilise uniquement des données fictives et ne
démarre pas BatteryMonitor. Les essais Bluetooth réels et sommeil du Mac restent
distincts des tests du modèle et nécessitent une séance avec l’utilisateur.
Ne pas lancer l’app réelle en arrière-plan implicitement pendant un build ou un test.

Vingt-quatre tests passent, dont zéro/tiret sans perte de mesure, les proportions
du symbole, l’indication incertaine à cent pour cent, sa disparition
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
n’est nécessaire à l’exécution. La copie vers Applications est maintenant effectuée
sur demande explicite, sans lancement. La relance et l’ouverture de session restent
à vérifier par l’utilisateur.
La présence conditionnelle dans le Dock se valide manuellement : une simulation
de booléens ne vérifierait pas le comportement AppKit réel. Les tests de modèle
et la compilation ne lancent pas de fenêtre ni de lecture Bluetooth implicitement.
L’icône SVG est validée XML, rendue par AppKit/Core Image puis embarquée en ICNS. Un seul
chemin fermé et un fond carré arrondi, sans touches ni encodeur. Le contour du PCB
gauche est adouci pour cette silhouette graphique, pas une exportation de fabrication.
Les anciennes variantes restent disponibles. Les 24 tests du modèle et les trois
tests de rendu des menus passent. Le bundle emporte les deux SVG, sans lecture du dépôt.
Le Dock contextuel reste à essayer en ouvrant, minimisant et fermant la fenêtre réelle.

Le rendu zéro/charge et la comparaison du symbole avant/après utilisent des données
fictives, sans création de BatteryMonitor. Les artefacts sont dans
.local/validation/macos-app-refresh. Le symbole natif de 16 × 11 points est ajusté
à environ 13,09 × 9 points, sans déformation. Aucune app réelle n’est lancée.
Sept validations d’installation isolées couvrent première copie, mise à jour sans
ressource périmée, mauvaise identité, lien symbolique, app ouverte, bundle absent
et restauration après échec du déplacement. Aucune copie vers Applications ni
modification de préférences système pendant ces contrôles. Pas de suite d’outillage ajoutée.

Le lot des contours passe les 27 tests, le format Swift et la syntaxe des scripts.
L’icône aux dix tailles est produite avec son ombre centrée et contrôlée visuellement.
Le rendu des menus utilise des valeurs fictives depuis un bundle autonome, sans
lecteur Bluetooth ni fenêtre : niveaux, charge et zéro sont vérifiés.
make app install a installé /Applications/Splitergoll.app sur demande explicite.
Signature et égalité de l’exécutable, de l’ICNS et des deux SVG installés sont vérifiées.
Aucune app réelle ni inscription de session n’a été lancée pendant cette installation.

Sur V2 au bureau, l’app ouverte affiche deux tirets tandis que la saisie fonctionne.
Le périphérique CoreBluetooth porte le libellé personnalisé Splitergoll V2 : le
filtre exact initial ne trouvait aucune cible. Avec ce nom, le lecteur existant
lit deux niveaux à 100 %. Après retrait des deux USB, le lecteur corrigé lancé
par make bat read retrouve V2 et lit gauche 89 %, droite 79 %, également rapportés
par l’utilisateur. Aucun reflash ou effacement d’appairage n’a été nécessaire.
Les 29 tests passent, dont les suffixes, faux préfixes, noms vides et ambiguïtés
entre V1/V2. Après fermeture confirmée par l’utilisateur, le build corrigé est
installé dans Applications. Signature et égalité de l’exécutable, de l’ICNS et des
deux SVG installés sont vérifiées. Aucun lancement réel par l’agent. Après réouverture,
l’utilisateur confirme le fonctionnement de l’affichage sur V2. Cela ne valide pas
encore les reprises après sommeil, les déconnexions ou l’ouverture de session.

## Sélection du clavier suivi retenue pour le prochain lot

L’utilisateur valide le développement du choix V1/V2 dans le menu. Spécification dans
docs/selection-clavier-app.md, tâches dans docs/TODO.md. Aucun code de cette
fonctionnalité n’est encore implémenté et les invariants de sélection actuels restent
applicables jusqu’à ce lot. Cible retenue : noms de périphériques comme libellés, identifiant
CoreBluetooth local comme clé du choix, persistance et absence de bascule silencieuse
si le clavier préféré disparaît. Vider les mesures au changement, rejeter les
callbacks tardifs, tester les noms identiques, renommages et reconnexions.
Ce choix concerne le clavier physique lu par l’app, jamais le slot hôte du firmware.
Le firmware wire n’expose pas BAS dans le build contrôlé, mais conserve ZMK_BLE=y.
Ne pas déduire de la présence Bluetooth de V1 que l’app lisait forcément ses batteries.
Retour utilisateur : avec V1 désactivé et l’app redémarrée, V2 s’actualise à cinq
secondes. Cause de l’anomalie antérieure non établie, sans capture des mesures ADC
ou des lectures GATT pendant l’incident. Ne pas la présenter comme un mélange prouvé.

Ordre de développement validé : commit tests/journal, correction wire, sélecteur
app puis destinations USB/BLE et Gaming. L’essai du sélecteur doit utiliser V1 et
V2 connectés simultanément au Mac, avec les services batterie requis. V1 wire
strict ne répond pas à cette condition : préparer le profil BLE avec l’utilisateur,
sans flash, sélection hôte ou effacement implicite. Le découpage futur de Gaming
reste ouvert, la mémoire d’un mode ne nécessite pas forcément une keymap séparée.
