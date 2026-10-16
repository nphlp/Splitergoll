# Latence

Le premier diagnostic utilise V1 sans batterie, en `wire` sur le MacBook Pro M4 Max.
La gauche envoie les touches par USB, la droite les transmet à la gauche en BLE.
Une différence de réactivité a initialement été ressentie à droite en debug,
puis était devenue imperceptible en normal. La comparaison reprise ci-dessous
ne reproduit pas d’écart gauche/droite significatif. L’effet des lecteurs de logs
sur la réactivité globale reste incertain. Aucun délai total n’est mesuré.
Le diagnostic court est clos, sans réglage radio ni cause identifiée. Le retour
des deux moitiés au firmware normal est annoncé, mais reste à confirmer.

Les deux consoles fonctionnent et montrent les événements de droite reçus par la
gauche, sans erreur ni déconnexion observée pendant les captures courtes.
L’hypothèse d’une seule moitié en debug lors du premier essai n’est pas vérifiée.

## Flash et lecture des logs

Le flash installe le firmware jusqu’au prochain flash. `wire macos debug` reste
donc en debug même quand aucun programme ne lit ses logs.
Ouvrir une console signifie seulement lire le port USB depuis le Mac.
Les 90 secondes étaient la durée de cette lecture, sans flash ni changement de profil.

## Ce que les logs permettent

Les consoles USB montrent les appuis détectés, la réception des événements de droite,
leur traitement et les erreurs éventuelles. Sur une même moitié, les timestamps
permettent d’observer des délais entre les étapes effectivement journalisées.

Les deux moitiés ont des horloges indépendantes. Le timestamp de droite n’est pas
transmis avec l’événement : la gauche le recrée lors de son traitement. Soustraire leurs
logs ne mesure donc pas le délai BLE. L’arrivée des lignes sur le Mac inclut aussi
la mise en file et le transfert de la console. Le debug peut lui-même modifier les timings.

Pour mesurer le trajet complet, du contact électrique à la réception par le Mac,
il faut un repère de départ et une capture synchronisée, par exemple un banc matériel.
Une vidéo au ralenti du geste et de l’écran donne seulement une estimation,
incluant le mouvement de la touche, l’application et l’affichage.

## Diagnostic rapide

Garder le même document de texte et les deux USB branchés pendant tout le test.
À chaque phase : dix appuis brefs et espacés sur la touche `A` à gauche, dix sur
`P` à droite, puis cinq alternances `ap`. Aucun Shift ni changement de couche.
Noter l’écart gauche/droite et le ressenti général, même si le résultat est incertain.

| Phase | Firmware | Lecture des consoles | Ressenti rapporté |
| --- | --- | --- | --- |
| 1 | `wire macos debug`, déjà installé | Fermée | Aucun écart net gauche/droite |
| 2 | Le même, sans flash | Ouverte sur les deux moitiés | Les deux moitiés semblent un peu plus lentes |
| 3 | Le même, sans flash | Fermée de nouveau | Différence incertaine, sans écart gauche/droite significatif |
| 4 | `wire macos` normal, après flash des deux moitiés | Désactivée dans le firmware | Retour annoncé, pas encore confirmé |

Avant la phase 2, attendre que l’utilisateur ait son document prêt et donne le signal.
Annoncer l’ouverture effective, puis la fermeture de la capture. Les phases 1 à 3
ne demandent aucun flash ni reset. La phase 4 utilise les
[commandes de flash normales](flash-firmware.md), sans reset des réglages.

Pendant la capture, relever les erreurs, déconnexions, files d’attente et paramètres
BLE observables. Distinguer les paramètres demandés de ceux négociés.
L’absence d’erreur ne prouve pas que le délai est faible. Garder les captures privées
dans `.local/`, sans annoncer une latence totale à partir des seuls logs.

La migration ZMK/Zephyr est en cours de validation. Si une mesure chiffrée devient
nécessaire, choisir une instrumentation adaptée séparément.
