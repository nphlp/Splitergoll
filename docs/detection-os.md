# Diagnostic de détection d’OS

Objectif : reconnaître l’ordinateur à partir de sa connexion, sans logiciel
compagnon permanent ni changement manuel d’OS. Le diagnostic observe les échanges
et annonce une hypothèse. Il ne change pas encore la keymap.

Tous les fichiers de cette étude sont dans `diagnostics/os-signature/` :
`firmware/` pour la capture embarquée, `snippets/` pour son activation, `tests/`
pour le stockage et `capture.sh`, `capture.ps1`, `check.awk` pour les lecteurs.
Le code du clavier reste groupé dans `keyboard/`.

## Préparer V1 en USB

```sh
make build wire macos usb-trace
make flash wire macos left usb-trace
```

La droite garde son firmware `wire` normal de la même version. Le build produit
aussi son UF2, mais seul celui de gauche ajoute le diagnostic. Aucun reset des
réglages n’est nécessaire. Sur Windows, conserver cette même keymap pour comparer
exactement le même firmware, sans attendre des raccourcis macOS qu’ils soient adaptés.

La centrale conserve les 256 premiers en-têtes SETUP de huit octets de chaque
cycle USB, avec leur ordre et un horodatage local. Aucun contenu de frappe ni
clé Bluetooth n’est enregistré. Une lecture ne vide pas cette mémoire.

## Lire une capture

Sur le Mac, identifier le port série de gauche avec `ls /dev/cu.usbmodem*`,
puis utiliser son nom exact, par exemple :

```sh
make usb-capture /dev/cu.usbmodem123
```

Sous Windows, trouver le port de gauche dans le Gestionnaire de périphériques,
puis depuis la racine du dépôt, dans PowerShell :

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\diagnostics\os-signature\capture.ps1 -Port COM3
```

L’option `Bypass` permet ce lancement si la politique d’exécution bloque les scripts.
Elle reste limitée au processus PowerShell lancé, sans changer la politique enregistrée.
Ce lecteur ne demande pas de terminal administrateur. Voir [la documentation Microsoft](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_powershell_exe?view=powershell-5.1#-executionpolicy-executionpolicy).

Remplacer les exemples de ports par ceux réellement présents. Fermer les autres
lecteurs série. Brancher la droite sur un chargeur USB pendant cette identification
évite de confondre deux ports. Aucun logiciel supplémentaire n’est nécessaire pour
le lecteur PowerShell, le pilote série natif doit être disponible.

La lecture prend au plus 15 secondes après ouverture du port et reste dans
`.local/usb-traces/`. Elle refuse une capture tronquée, désordonnée ou signalant
des pertes. Seules les requêtes mémorisées sont restituées, pas les frappes.

## Comparer Mac et Windows

1. Garder les mêmes UF2, câbles et branchement direct, sans hub pour le premier essai.
2. Débrancher les deux USB de V1 pour repartir d’un démarrage neuf.
3. Brancher les USB à l’ordinateur testé, attendre cinq secondes, puis lire la gauche.
4. Répéter trois fois sur le Mac, puis trois fois sur Windows. Ne pas taper pendant la capture.
5. Comparer les requêtes, leurs paramètres et leur ordre. Les temps seuls ne sont pas une signature fiable.

Une fois des différences reproductibles identifiées, tester aussi Linux, les hubs,
la reconnexion et le réveil avant d’activer une reconnaissance automatique.

## Limites du diagnostic

Le port série ajoute une interface CDC ACM aux descripteurs USB. Les premières
signatures décrivent donc un clavier instrumenté, pas encore le firmware normal.
Le lecteur peut lui-même ajouter des requêtes série. Il faudra isoler ces échanges
et revalider les signatures sans cette interface avant d’en faire une fonctionnalité.
Un échec d’énumération qui empêche l’apparition du port n’est pas lisible par ce canal.

## Résultats sur V1

Les cinq captures Mac contiennent chacune 24 requêtes, les cinq Windows 57,
58, 57, 53 et 53. Aucun rejet ni perte n’est signalé. Le lecteur PowerShell fonctionne sur
le PC avec la commande ci-dessus. Les captures restent dans `.local/usb-traces/`.

Les motifs de noms sont cohérents entre les essais. Le début de l’énumération et
les répétitions peuvent varier : la capture Windows sur le clavier commence sur 18
octets, sans sonde de 64 visible.
Les requêtes CDC sont isolées, car elles dépendent aussi du pilote et du lecteur série.

| Demande observée | Mac | PC Windows |
| --- | --- | --- |
| Début de la capture du descripteur du périphérique | 8 octets | 64 ou 18 octets selon le cycle |
| Lecture des noms du clavier | 2 octets puis lecture complète | Lectures sur 255 octets, sondage sur 4 puis lecture complète du nom du produit |

Le code est propre à Splitergoll, inspiré des motifs étudiés par
[QMK](https://docs.qmk.fm/features/os_detection) et
[Cormoran](https://github.com/cormoran/zmk-feature-os-detection/blob/main/docs/fingerprints.md),
puis confronté à nos captures. Aucun module externe ni changement de descripteur USB n’est ajouté.

## Comment la décision est prise

Le classifieur associe chaque sondage à sa relecture, pour le même nom et la même
langue. Deux motifs « 2 puis complet » suggèrent macOS. Un motif « 4 puis complet »
et des lectures sur 255 pour deux noms distincts suggèrent Windows. Les sondes
8/64 sont complémentaires : leur absence ne bloque pas la décision, leur
contradiction la rend ambiguë. Répéter un même nom ne renforce pas l’indice.
Les tailles des rapports HID et les requêtes du lecteur série sont ignorées.

Il attend 250 ms sans nouvelle demande utile avant d’annoncer son hypothèse.
Ce délai concerne la décision d’OS, pas la saisie des touches. Des indices insuffisants
donnent « inconnu », des motifs contradictoires donnent « ambigu ».
Un reset du bus USB ou une déconnexion efface les indices du cycle précédent.

La lecture affiche `Signature USB : hypothèse macOS` ou `Windows`. La décision est
recalculée sur l’instantané au moment de la demande de capture, pas appliquée aux couches.
L’aller-retour Mac → Windows → Mac est confirmé sur V1, sans nouveau flash entre
Windows et le retour au Mac. La détection sans CDC, Linux et les hubs restent à tester.
Aucune bascule de keymap n’est activée à ce stade.

Le script `.ps1` est enregistré en UTF-8 avec BOM pour les accents sous Windows
PowerShell. Les fichiers de capture restent en UTF-8 sans BOM pour leur analyse.
Le lecteur mis à jour et l’affichage des accents sont confirmés sur le PC.

## Suite Bluetooth

V1 peut aussi tester le Bluetooth direct avec ses deux USB pour l’alimentation,
sans batterie. Ce sera une capture BLE distincte, pas une détection de l’OS
Bluetooth à partir du chargeur ou de l’ordinateur qui fournit son USB.

Pour revenir à l’usage normal : `make flash wire macos left`.
