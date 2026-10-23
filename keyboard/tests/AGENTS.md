# Tests du firmware

Les tests C doivent contrôler les décisions observables et les erreurs, sans
réimplémenter l’algorithme qu’ils vérifient. Les tests unitaires compilent les
moteurs purs. Les tests d’intégration incluent les vrais adaptateurs et remplacent
seulement les frontières ZMK, HCI, USB et NVS.

Les doubles sont isolés sous integration/<domaine>/stubs et ne participent jamais
au firmware. Horloge contrôlée, aucun périphérique ni donnée utilisateur réelle.
Les scénarios doivent faire avancer le temps, sauf tests explicites de débordement.
Ne pas changer une assertion pour masquer un contrat obsolète : revenir à la règle
documentée et construire une chronologie cohérente.

scripts/test.sh compile avec avertissements fatals et ASan/UBSan. Les journaux de
test vont dans le dossier temporaire fourni au programme, jamais dans les sources.
Les tests Zephyr utilisent le noyau figé du workspace dans un conteneur éphémère,
sans accès USB/radio, et gardent leur résultat distinct des essais matériels.
QEMU Cortex-M3 exécute activity.c ZMK réel, pas une recopie de ses conditions.
Les frontières sys_poweroff, présence USB et bus ZMK restent doublées. Les délais
10/60 ms sont propres à la simulation et ne changent pas les firmwares.
Le lanceur borne la durée et nettoie uniquement son propre groupe de processus.
Les formats de diagnostic sont testés séparément par les validateurs réels AWK/PS,
avec fixtures synthétiques, jamais avec des données privées versionnées.

La matrice humaine docs/tests-automatises.md doit distinguer règles implémentées,
contrats futurs, simulations et faits vérifiés sur le vrai clavier. Ne pas prétendre
prouver toute absence de bug ou reproduire la consommation réelle du nRF52840.

## Ideas / backlog

- Chaque nouvel algorithme reçoit sa suite et ses cas d’erreur dans le même chantier.
- Garder peu d’essais physiques, centrés sur alimentation, radio et réveil.
