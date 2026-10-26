# Destinations de saisie

**Premier prototype Mac développé, essai physique à faire.** Le mode
[bluetooth auto usb-priority](priorite-usb.md) choisit USB pour le même ordinateur
déclaré, avec repli Bluetooth. Les profils normaux `wire` et `bluetooth` restent
distincts. Windows, les installations multi-utilisateurs et Gaming restent à compléter.

## Le cas de référence

La gauche est reliée en USB au PC Windows. Le clavier est aussi connecté en
Bluetooth au PC Windows et au Mac. L1 + clic encodeur droit doit parcourir :

| Ordinateur | Liaison retenue | Keymap |
| --- | --- | --- |
| PC Windows | USB | Windows avec gaming |
| MacBook Pro | Bluetooth | macOS |

Windows Bluetooth reste une liaison connue, mais ne constitue pas une troisième
destination lorsque Windows USB est utilisable. La connexion BLE peut rester présente.
Le [Gaming USB](gaming-usb.md) devra déconnecter le Mac et arrêter les annonces
pendant le jeu, puis rouvrir deux minutes au retour volontaire à Windows normal.
Voir [la gestion de l’appairage](gestion-appairage.md).

## Priorité par ordinateur

L’USB est prioritaire sur le Bluetooth **pour le même ordinateur**. Sa présence
n’empêche pas de sélectionner un autre ordinateur en Bluetooth. Une seule destination
reçoit les frappes à la fois, sa liaison et sa keymap doivent changer ensemble.
Le raccourci ouvrira aussi deux minutes de recherche : avec une seule destination,
le premier autre appareil prêt prend la sélection. Avec plusieurs, le geste passe
à la suivante et les nouveaux arrivants ne la changent pas.

Seul l’USB de la gauche peut transporter la saisie. Un chargeur ou un USB qui ne
permet pas la saisie ne doit pas créer de destination. Le comportement au branchement
et au débranchement est décrit dans le [prototype Mac](priorite-usb.md) : priorité
après preuve, transition normale après relâchements et repli Bluetooth au retrait.

## Reconnaître le même ordinateur

Notre détection USB reconnaît un OS, pas l’identité de l’ordinateur. Deux appareils
Windows ne doivent donc pas être fusionnés sur ce seul indice. Il reste à définir
comment associer l’hôte USB à un appareil Bluetooth mémorisé. La voie retenue pour
le prochain lot utilise les compagnons Mac et Windows pour annoncer la même
identité locale sur les deux liaisons. Le [diagnostic Mac](diagnostic-identite.md)
est validé sur Mac + V2 pour la correspondance initiale. Son profil `identity`
ne change aucune destination. Le nouveau profil `usb-priority` utilise cette preuve
pour la saisie réelle. Son essai physique et le client Windows restent à faire.
Sans association fiable, ne pas retirer arbitrairement une destination de la liste.

Les captures existantes sont réutilisables : dix USB et dix-sept Bluetooth valides,
plus un fichier Bluetooth sans trace. Aucun identifiant commun n’apparaît dans
les champs conservés. Elles restent privées sous `.local/`, sans versionnement.

La [recherche sur l’identité de l’ordinateur](identite-ordinateur.md) distingue
les champs ordinaires et deux protocoles d’appairage qui transmettent une adresse
d’hôte par USB : Apple Magic Keyboard et Microsoft Seamless Pairing pour stylets.
Leur compatibilité avec notre clavier n’est pas établie. Microsoft exige notamment
une activation par extension INF côté PC. Vérifier ces prérequis avant de préparer
un firmware : une capture plus large du profil actuel n’a pas de champ exploitable
identifié à ce stade. Aucun nouveau flash ou changement de keymap demandé.

Le croisement des OS et l’égalité Caps Lock/Num Lock sont rejetés comme preuves.
Le compagnon restera facultatif : sans association fiable fournie par l’app,
le clavier fonctionne. Le prototype revient en Bluetooth en l’absence d’association,
sans choisir arbitrairement l’ordinateur relié au câble.
Un essai avec deux machines du même OS restera nécessaire pour vérifier que
le rapprochement ne dépend pas de l’OS détecté.

Les slots Bluetooth conservent leurs appairages. Une destination est le choix proposé
à l’utilisateur, elle pourra regrouper plusieurs liaisons vers un même appareil.

## Transport et arbitrages

ZMK fournit déjà la [sélection de sortie USB/BLE](https://zmk.dev/docs/keymaps/behaviors/outputs).
Cela permet de garder le Bluetooth pendant une sortie USB. Notre futur contrôleur
doit coordonner ce transport, l’ordinateur sélectionné et sa keymap. Le simple
basculement de sortie ne résout pas l’identité commune.

Le branchement USB doit permettre d’utiliser le câble pour l’ordinateur associé,
mais ne suffit pas à décider de déconnecter tous les hôtes Bluetooth : le Mac doit
rester accessible pendant l’usage de Windows USB. Séparer la sélection de
l’ordinateur, le choix de sa liaison et la fenêtre de recherche Bluetooth.

Définir l’effet d’un branchement pendant la saisie sur un autre ordinateur,
le retrait du câble et l’absence d’association fiable. La fermeture des connexions
Bluetooth en Gaming sera un lot ultérieur. Aucune de ces règles ne découle du seul
nom d’OS détecté. La restriction USB du profil wire actuel reste un correctif distinct.
