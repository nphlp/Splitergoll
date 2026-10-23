# Gaming USB

**Comportement cible, pas encore implémenté dans le futur firmware USB + BLE.**
La keymap Windows actuelle possède déjà ses couches de jeu. Le chantier décrit
ici concerne les connexions et la mémorisation de ce mode.

## Entrer en Gaming

- Exiger une vraie sortie USB utilisable vers Windows.
- Sélectionner Windows USB et mémoriser ce choix avec le mode Gaming.
- Fermer immédiatement la fenêtre Bluetooth et arrêter les annonces hôtes.
- Déconnecter le Mac en BLE, conserver les appairages et les OS enregistrés.
- Conserver le BLE entre les deux moitiés, nécessaire avec le câblage actuel.

Le gain de latence reste à mesurer. La présence d’un câble d’alimentation seul
ne suffit pas à autoriser Gaming USB.

## Retrouver le mode choisi

Demande retenue pour le développement : si le dernier choix était Gaming,
une coupure, un reset ou un rebranchement au même Windows USB doit retrouver
Gaming, plutôt que revenir systématiquement à Windows normal.
La mémorisation doit distinguer l’ordinateur, le transport utilisable et le mode.
Elle ne doit pas appliquer les couches Gaming sur Mac ni supposer l’identité
commune à partir du seul OS. Les écritures identiques devront être dédupliquées.

## Sortir de Gaming

Le retour volontaire à Windows normal rouvre deux minutes de recherche,
sans abandonner la saisie USB ni sélectionner automatiquement le Mac à son retour.
Depuis Gaming, L1 + clic encodeur droit quitte d’abord Gaming, rouvre la recherche
et porte la demande de bascule vers une autre destination, selon les règles communes.

## Points à décider avant le code

- Déconnecter aussi Windows BLE pendant Gaming, ou seulement le Mac ?
- Si l’USB disparaît : attendre son retour, repasser en Windows BLE normal ou
  utiliser une autre destination ? Quand restaurer Gaming après ce repli ?
- Une sortie manuelle de Gaming mémorise-t-elle immédiatement Windows normal ?
  Quel mode retrouver après un détour volontaire par Mac puis retour sur Windows ?
- Quelle portée et quelle identité donner à cette mémoire, après redémarrage,
  nouvel appairage ou connexion à un autre PC Windows ?

## Validation prévue

Tester entrée pendant une recherche, fermeture immédiate, maintien de la liaison
split et de la saisie USB, conservation des bonds, sortie par raccourci et reprise
après coupure/rebranchement. Vérifier que Mac ne reçoit aucun événement Gaming.
La fermeture de recherche et l’écriture de mémoire ne doivent pas générer d’activité.

Voir [les destinations USB/BLE](destinations-saisie.md),
[la gestion d’appairage](gestion-appairage.md) et [le TODO](TODO.md).
