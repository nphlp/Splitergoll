# Politique de connexion des hôtes

## Intention

Séparer la politique de connexion des API radio et de la détection d’OS.
Le moteur C pur et ses tests couvrent la politique cible. Un premier adaptateur
relie uniquement les fenêtres de reconnexion connue au firmware bluetooth auto.
La recherche avec sélection, l’accueil d’un nouveau bond et Gaming USB restent séparés.
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

ZMK lie actuellement le nouvel appairage au profil actif. Ne pas remplacer ce
comportement par une bascule temporaire qui détourne la saisie pendant l’ajout.
Définir explicitement l’intégration des annonces, de l’admission et de l’affectation
du bond avant de brancher l’accueil des nouveaux appareils. Aucun patch implicite des dépendances.

## Reconnexion connue

`ble_reconnect.c` intercepte les appels externes bt_le_adv_start/stop de ZMK et
les convertit en notifications d’un seul worker. Seul ce worker appelle les vraies
API legacy. Les valeurs de retour des wrappers accusent réception de la demande,
pas une réussite HCI, les erreurs effectives sont exposées dans le statut et les logs.
Ne pas lancer en parallèle les annonces natives et un second advertiser.
Les données et paramètres natifs sont copiés dans un stockage borné, sans pointeur
de pile conservé. La fenêtre commence au premier appel ZMK après chargement des bonds.

Le worker réutilise host_window_policy pour le délai. Le raccourci réarme la fenêtre
et garde sa bascule actuelle vers un hôte déjà connecté. Aucune arrivée ne sélectionne
de profil dans ce lot, même après une recherche avec un seul appareil. Les règles
complètes du moteur restent à raccorder au sélecteur ultérieurement.

`ble_reconnect_state.c` impose arrêt, filtre puis démarrage, avec revérification de
l’échéance/génération avant et après HCI. Une liste partielle ou un échec d’arrêt
n’autorise aucun démarrage non filtré. La liste autorise seulement les profils hôtes
avec un bond existant, en retirant les hôtes déjà connectés. Le contrôleur résout
les adresses privées avec sa liste native, dimensionnée pour tous les bonds.
Le scanner split ZMK reste passif sans filtre, la liste du contrôleur lui est globale.
Ne pas ajouter d’autre utilisateur de cette liste sans revoir cette intégration.

Le wrapper d’enregistrement des callbacks d’authentification refuse tout nouvel
appairage hôte. Les callbacks et l’admission de la moitié droite restent natifs.
La connexion hôte est aussi contrôlée à son arrivée : slot connu, bond présent,
fenêtre encore ouverte. Les connexions établies restent actives à l’échéance.
Un hôte qui termine sa reconnexion après l’échéance est refusé. Cela reste à vérifier
sur les hôtes réels, surtout les adresses privées et les retours tardifs.

Les callbacks radio ne prennent pas le mutex du sélecteur et ne font aucune
commande HCI synchrone. Le worker prend ce mutex seulement pour son instantané
des profils, le relâche avant les API radio. Les échecs ont au plus trois réessais
temporisés par révision, pas de boucle de reconnexion infinie. Aucun bond ni OS NVS
n’est modifié. La fonction est réservée à la gauche bluetooth auto, profils fixes
et droite inchangés. L’ajout d’un appareil reste possible avec leur firmware habituel.

BLEPROFILE v3 ajoute une ligne WINDOW avant RESULT pour la disponibilité du
contrôleur, le temps restant, les annonces, masques connus/connectés et erreur.
La lecture S reste passive. Les lecteurs gardent la compatibilité v1/v2.
Le contrôleur impose la pile legacy sans EXT_ADV/LTO, le filtre hôte et une liste
contrôleur assez grande. Les sources ZMK/Zephyr restent intactes.

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
