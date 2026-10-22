# Politique de connexion des hôtes

## Intention

Séparer la politique de connexion des API radio et de la détection d’OS.
Le moteur C pur et ses tests couvrent la politique cible. L’adaptateur relie les
fenêtres de reconnexion et l’accueil d’un nouveau bond au firmware bluetooth auto.
La recherche avec sélection sur une arrivée et Gaming USB restent séparés.
Les règles humaines sont dans docs/gestion-appairage.md et docs/destinations-saisie.md.

## Contrat

Les identifiants 0 à 15 désignent des destinations logiques, pas des transports.
L’adaptateur doit regrouper USB/BLE d’un même appareil seulement si son identité
commune est établie. Le moteur ne déduit aucune identité d’un OS.
Chaque destination reçoit un jeton non nul qui change si son identité est remplacée.
Le masque ready ne contient que des appareils sécurisés avec une keymap utilisable.
Un slot BLE inconnu, non chiffré ou dont l’OS n’est pas encore vérifié n’est pas prêt.

Le démarrage et la sortie ordinaire de Gaming USB ouvrent dix secondes sans recherche
de nouvelle destination. Le raccourci ouvre ou réarme dix secondes : zéro ou une
destination arme une recherche, plusieurs demandent la suivante déjà prête.
Le nombre est figé au geste. Une seule bascule de recherche est admise par demande.
Les arrivées suivantes ne reprennent pas la sélection. La saisie n’est pas pilotée
par ce moteur, l’adaptateur garde la source utilisable pendant l’attente.

Une échéance ferme les nouvelles arrivées, mais une cible prête choisie avant cette
échéance peut attendre le relâchement des touches après celle-ci. Sa disparition ou
son remplacement annule la demande. Une autre sélection explicite annule la recherche,
même sans changement du masque. Les tickets de génération rejettent les travaux périmés.
Un nouveau geste remplace la recherche précédente. Les gestes rapides sur plusieurs
destinations avancent depuis la demande en cours, comme le raccourci actuel.

Gaming ne peut être armé que si l’adaptateur confirme une sortie USB utilisable.
Il ferme la fenêtre et annule les demandes. Un second état Gaming reste Gaming.
Le raccourci depuis Gaming revient au mode normal avant d’ouvrir une seule fenêtre
et d’appliquer sa recherche. La sortie Gaming ordinaire ne vole pas la sélection.
Le moteur n’efface aucun bond, ne choisit aucun slot libre et ne coupe aucune radio.

## Intégration suivante

La politique doit être sérialisée par l’adaptateur, elle n’a pas son propre verrou.
La prise d’une cible attend les gardes d’entrée existantes : positions relâchées,
rapport HID vide et comportements différés terminés. Elle exprime une décision,
pas la réussite d’un appel natif. Toute erreur de sélection native doit rester visible.
Le compteur de slots libres et les capacités radio restent fournis par ZMK.
Les fenêtres n’impliquent pas une garantie de reconnexion instantanée par l’hôte.

ZMK lie nativement le nouvel appairage au profil actif. L’adaptateur réserve au
contraire un slot vide sans déplacer ce profil. Aucun patch des dépendances.

## Fenêtre commune et attribution

`ble_reconnect.c` intercepte bt_le_adv_start/stop et les convertit en notifications
d’un seul worker. Lui seul appelle les vraies API legacy. Les wrappers accusent
réception, les erreurs HCI restent dans le statut. Les données d’annonces sont
copiées dans un stockage borné. La fenêtre commence après chargement des bonds.

Avec des places libres et sans transaction en cours, les annonces acceptent les
connexions connues et nouvelles. Sans place ou pendant un appairage, le filtre
n’autorise que les connus absents. Arrêt, filtre puis démarrage sont ordonnés et
revérifiés avant/après HCI. Une erreur de filtre ne permet aucun démarrage ouvert.
Le scanner split reste passif sans filtre. La liste du contrôleur lui est globale,
ne pas ajouter un autre utilisateur sans revoir cette intégration.

`ble_pairing_state.c` réserve une seule place libre, de préférence le slot actif
s’il est vide. Une référence de connexion garde son identité pendant SMP et la
résolution d’adresse privée. Une autre arrivée inconnue est refusée jusqu’à libération.
Le début SMP doit appartenir aux dix secondes. Il dispose ensuite de 30000 ms,
indépendants des réouvertures et sans réarmer la fenêtre. Une réussite reçue avant
cette borne peut attendre le worker, y compris après une déconnexion immédiate.
Sans début SMP, la connexion candidate expire avec la fenêtre.

Le wrapper auth_cb conserve les callbacks split natifs et contrôle l’admission hôte.
Le wrapper auth_info_cb remplace seulement le callback ZMK enregistré juste après
son auth_cb dans le socle figé. Les inscriptions antérieures restent natives, une
seconde inscription du même callback ZMK est refusée. Pas de nouvel appairage d’une
identité déjà enregistrée, d’écrasement automatique de clé ou de slot occupé.

Après réussite SMP avec bond, le worker reprend le verrou du sélecteur et revérifie
slot vide, identité non attribuée et bond présent. Il préécrit la structure publique
zmk_ble_profile en conservant son nom, avec retour d’erreur settings_save_one.
Il appelle ensuite le helper exporté set_profile_address du ble.c figé, qui n’a pas
encore de déclaration dans ble.h. Sa seconde écriture identique est dédupliquée par
NVS. Ce lien est spécifique au commit actuel : recontrôler prototype, disposition de
la structure et ordre des callbacks à chaque migration. Aucun en-tête privé Zephyr.
Une erreur de sauvegarde ne modifie pas la case en RAM et reste visible.

La mémoire reçoit explicitement pairing_complete après cette attribution, via
splitergoll_ble_acquisition_paired. En auto, son ancien callback direct est désactivé
pour éviter un apprentissage avant l’attribution ou en double. Les profils fixes
gardent leur callback statique paired et leur comportement natif. Le worker de
mémoire conserve ses événements ATT antérieurs, sa sécurité et ses gardes d’identité.

Le seul nouvel hôte peut être adopté si la sélection est encore celle observée à
l’accueil et qu’aucune demande explicite n’attend. Les mêmes gardes d’appuis, HID
vide et drainage s’appliquent. Un autre hôte connecté interdit cette adoption.
L’ajout pendant l’usage d’un connu ne détourne donc pas les frappes.
La recherche avec bascule sur une arrivée et le retour au précédent restent séparés.

Les callbacks ne prennent pas le mutex du sélecteur et n’écrivent pas NVS. Le worker
borne les transactions, annule SMP puis déconnecte un candidat expiré, et conserve
sa référence jusqu’à la fin de connexion. Seul le bond neuf non attribué de cette
transaction peut être nettoyé. Aucun bond d’un slot connu ni lien split n’est supprimé.
Les erreurs d’appairage/sauvegarde/nettoyage restent dans WINDOW.error, jusqu’à une
nouvelle réservation. Les erreurs radio conservent leurs réessais bornés par révision.

La lecture S de BLEPROFILE v3 reste passive, au format antérieur. WINDOW indique
le temps d’admission, pas le temps de fin SMP. L’instantané C expose aussi la place
réservée, son temps restant et la possibilité d’accueil, sans adresse ni clé.
La fonction est réservée à la gauche bluetooth auto. La droite et les profils fixes
conservent leur fonctionnement. USB/BLE mixtes et Gaming USB restent à développer.

## Validation

Compiler host_window_policy.c avec keyboard/tests/host_window_policy_test.c,
-Ikeyboard/include, C11, -Wall -Wextra -Werror et ASan/UBSan. Tester les échéances
exactes et l’horloge 32 bits, réarmement, recherches, plusieurs arrivées, changements
externes, identités remplacées, erreurs de topologie, Gaming et tickets périmés.
Les suites natives existantes restent pertinentes. Aucun essai matériel n’est
déduit d’un test pur. Les tâches restent centralisées dans docs/TODO.md.

Validation réalisée : les dix suites natives passent avec ASan/UBSan et avertissements
traités comme erreurs. La nouvelle suite énumère toutes les topologies jusqu’à huit
destinations, vérifie la limite seize, les échéances à 1 ms près, le débordement de
l’horloge, les gestes multiples, la recherche sans source et les transitions Gaming.
Les identités remplacées, changements explicites externes, masques invalides et
travaux munis d’un ancien ticket ne sélectionnent aucune cible. Les tests distinguent
arrivée à l’échéance refusée et cible déjà prête gardée avant l’échéance.
Aucune compilation Docker, lecture série, action radio ou écriture NVS dans ce lot.

Validation de l’adaptation de reconnexion : onze suites natives avec ASan/UBSan,
nouvelle politique également en -Wconversion -Wshadow -pedantic. L’analyse statique
Clang ne trouve pas de diagnostic. Un harnais privé compile le vrai ble_reconnect.c
avec doubles ZMK/HCI : notifications natives, copie bornée des annonces, admission
connue/inconnue/tardive, callbacks d’authentification préservés côté split, fermeture,
réouverture, erreurs radio et réessais bornés. Le harnais du vrai ble_auto.c conserve
les scénarios du raccourci, des relâchements et des identités remplacées. Ces harnais
ne simulent ni le contrôleur radio réel ni les reconnexions des OS.

Les lecteurs AWK/PowerShell acceptent/refusent les mêmes 42 réponses, dont v1/v2,
v3, ligne manquante/doublée, dépassements, masques incohérents et erreur radio.
Le BOM UTF-8 PowerShell est conservé. Les trois paires bluetooth auto normal,
debug et ble-trace compilent, leurs six arbres gardent les mêmes couches, macros,
conditions, encodeurs et comportements que la référence du raccourci précédent.
Les trois UF2 droits sont identiques aux précédents. Deux paires témoins wire auto
normal et bluetooth macos ble-trace sont identiques sur les deux moitiés, même
le diagnostic fixe conserve son protocole v2 et son binaire. Les dix UF2 passent
SHA-256 et les gardes de blocs/partitionnement.

Le désassemblage ARM confirme que update_advertising passe par les deux wrappers,
l’enregistrement natif par le wrapper d’authentification, et les opérations du worker
par les vraies API radio. Les dépendances restent aux commits prévus, sans patch.
Les avertissements observés restent KSCAN, const du tap-dance et le drapeau Docker
déjà connus. Aucun diagnostic nouveau dans le C local. Les conteneurs éphémères
ont disparu, les trois services préexistants sont conservés. Aucun flash ni accès
USB série effectué. Les preuves restent dans .local/validation/ble-reconnect/.

Validation du lot d’appairage : douze suites natives avec ASan/UBSan, dont la nouvelle
réservation testée sur tous les masques jusqu’à huit slots, la limite seize, les
échéances exactes, débordement, concurrence et absence de réarmement par doublon.
Le test du sélecteur couvre adoption du seul hôte, destination occupée et changement
explicite. Le harnais du vrai sélecteur vérifie aussi l’adoption immédiate à froid
et le drainage de 750 ms lorsqu’un ancien comportement reste possible.
Un harnais privé exécute le vrai adaptateur : slots vides/pleins, retours
simultanés, appairages successifs, résolution d’identité, succès après fermeture,
expiration à la borne, panne NVS, slot modifié pendant SMP et nettoyage du seul bond
neuf. Les références sont libérées et aucun callback ne sauvegarde dans ce harnais.
Les tests ne simulent pas le contrôleur radio réel ni une coupure de courant pendant NVS.
Preuves dans .local/validation/ble-pairing, sans accès USB ou modification des bonds réels.
