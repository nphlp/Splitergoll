# Mémoriser et corriger l’OS

**Fonctionnement prévu, pas encore implémenté.**

Le choix de l’ordinateur et le choix de sa keymap sont séparés.
Chaque appareil appairé aura un OS enregistré dans la mémoire du clavier.

## Premier appairage

1. Le clavier observe les échanges de la connexion initiale.
2. Le classifieur propose un OS quand les indices sont suffisants et cohérents.
3. Le résultat est mémorisé pour cet appareil et permet de choisir sa keymap.

Un résultat inconnu ou ambigu ne devient pas un OS inventé. Il faudra pouvoir
le corriger manuellement. La reconnaissance initiale reste à revalider sur de
nouveaux essais, voir [le diagnostic](detection-os-bluetooth.md).

## Reconnexions

Le clavier relit exclusivement l’OS enregistré pour cet appairage.
**Il ne relance pas la détection automatique.** Une reconnexion Windows sans
découverte GATT pourra donc retrouver Windows grâce à sa mémoire.

Cette mémoire survit au redémarrage, à la veille profonde et à un flash ordinaire
qui conserve les réglages. Effacer ou remplacer l’appairage invalide son association
d’OS. Le numéro d’emplacement seul ne suffit pas à reconnaître l’appareil.

## Correction manuelle

Un raccourci permettra de changer l’OS de l’appareil sélectionné.
Le choix sera enregistré et réutilisé à ses prochaines connexions, sans être
remplacé par une détection automatique. Il corrigera une mauvaise reconnaissance,
un résultat inconnu ou un changement d’OS sur le même ordinateur.

Le geste exact reste à définir. Cette correction fait partie du fonctionnement
normal souhaité. Un profil entièrement manuel reste une solution distincte
de dernier recours dans le [TODO](TODO.md).
