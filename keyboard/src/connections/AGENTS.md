# Politique de connexion des hôtes

## Intention

Séparer la politique de connexion des API radio et de la détection d’OS.
Ce premier lot fournit un moteur C pur et ses tests. Il n’est pas encore relié
au firmware, aux annonces ou aux callbacks d’authentification ZMK.
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
du bond avant de brancher ce moteur. Aucun patch implicite des dépendances.

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
