# Destinations de saisie

**Évolution prévue, pas encore implémentée.** Les firmwares `wire` et `bluetooth`
actuels restent distincts. Leur futur rapprochement doit permettre de choisir un
ordinateur avec la meilleure liaison disponible et sa keymap.

## Le cas de référence

La gauche est reliée en USB au PC Windows. Le clavier est aussi connecté en
Bluetooth au PC Windows et au Mac. L1 + clic encodeur droit doit parcourir :

| Ordinateur | Liaison retenue | Keymap |
| --- | --- | --- |
| PC Windows | USB | Windows avec gaming |
| MacBook Pro | Bluetooth | macOS |

Windows Bluetooth reste une liaison connue, mais ne constitue pas une troisième
destination lorsque Windows USB est utilisable. La connexion BLE peut rester présente.
Une exception est envisagée pour le Gaming USB : déconnecter le Mac et arrêter les
annonces pendant le jeu, puis rouvrir dix secondes au retour à Windows.
Voir [la gestion de l’appairage](gestion-appairage.md).

## Priorité par ordinateur

L’USB est prioritaire sur le Bluetooth **pour le même ordinateur**. Sa présence
n’empêche pas de sélectionner un autre ordinateur en Bluetooth. Une seule destination
reçoit les frappes à la fois, sa liaison et sa keymap doivent changer ensemble.
Le raccourci ouvrira aussi dix secondes de recherche : avec une seule destination,
le premier autre appareil prêt prend la sélection. Avec plusieurs, le geste passe
à la suivante et les nouveaux arrivants ne la changent pas.

Seul l’USB de la gauche peut transporter la saisie. Un chargeur ou un USB qui ne
permet pas la saisie ne doit pas créer de destination. Le comportement au branchement
et au débranchement reste à préciser, notamment le repli sur le BLE du même appareil.

## Reconnaître le même ordinateur

Notre détection USB reconnaît un OS, pas l’identité de l’ordinateur. Deux appareils
Windows ne doivent donc pas être fusionnés sur ce seul indice. Il reste à définir
comment associer l’hôte USB à un appareil Bluetooth mémorisé. Sans association fiable,
ne pas retirer arbitrairement une destination de la liste.

Les slots Bluetooth conservent leurs appairages. Une destination est le choix proposé
à l’utilisateur, elle pourra regrouper plusieurs liaisons vers un même appareil.
