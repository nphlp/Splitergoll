# Diagnostic d’activité et de sauvegarde

Observer les transitions réelles sur batterie, puis lire l’historique par USB
avant SLEEP, sans activité artificielle ni écriture de journal en flash.
Option explicite SPLITERGOLL_POWER_TRACE, uniquement dans un module de diagnostic
ajouté au build bluetooth auto ble-trace. Aucun effet dans les builds usuels.

Le stockage C pur est borné et append-only, sans écrasement. Les lectures ne le
vident pas. Les pertes et rejets sont visibles, un journal incomplet est refusé
par le lecteur. Un identifiant aléatoire propre au démarrage permet de détecter
un reset entre lectures, sans représenter une identité Bluetooth.

Événements autorisés : démarrage, état ACTIVE/IDLE/SLEEP, présence USB, état de
fenêtre/annonces, résultat d’une écriture de préférence. Aucun rapport HID,
touche, nom d’hôte, adresse, bond ou empreinte n’est stocké. Les callbacks
ajoutent seulement une petite structure sous spinlock, sans timer, HCI ni NVS.
Les appels depuis le moteur sont conditionnels à l’option de diagnostic.
La restitution P utilise le transport série existant, avec son contrôle DTR.

Les tests natifs couvrent validation, capacité, saturation, absence d’écrasement,
horloge 32 bits, lecture répétée et déduplication des états de fenêtre.
Les harnais versionnés contrôlent les politiques et les adaptateurs, sans simuler
le circuit d’alimentation, la radio réelle ou les caches des systèmes hôtes.

Le journal RAM n’est pas conservé après sommeil profond ou coupure. Un reset
pendant une expérience invalide sa continuité. L’instrumentation modifie un peu
le coût CPU/RAM, ce n’est pas un outil de mesure de consommation ou de latence.

## Ideas / backlog

- Six Ztests sous QEMU couvrent activity.c et l’ordonnancement réel. Garder la
  radio, NVS, alimentation et réveil GPIO distincts de cette preuve logicielle.
- Gaming : mesurer un effet d’IDLE avant de décider de le désactiver.
