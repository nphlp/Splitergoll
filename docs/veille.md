# Veille et réveil

V2 fonctionne sur batterie avec réveil par les touches. Les essais courts après
migration confirment la reprise, avec une limite : la droite ne peut pas réveiller
une gauche en veille profonde.

## Délais

| État | `bluetooth` | Comportement prévu |
| --- | --- | --- |
| Inactivité | 2 minutes | Connexion BLE conservée |
| Veille profonde | 10 minutes | Réveil local par une touche, puis reconnexion |

Les deux délais partent de la dernière activité : la veille profonde arrive
après dix minutes au total, pas dix minutes après le passage en inactivité.

Le profil `wire` désactive la veille profonde. Le profil `bluetooth` l’active et
la matrice est déclarée comme source de réveil.
[Fonctionnement ZMK](https://zmk.dev/docs/features/low-power-states).

**L’inactivité IDLE existe aussi sous USB.** Dans le code ZMK compilé, seul SLEEP
est conditionné par l’absence d’alimentation USB. V1 a donc réellement atteint
IDLE pendant les essais Bluetooth alimentés par USB. Cela ne valide pas SLEEP.
Le mode matériel `wire` et le diagnostic `bluetooth` alimenté par USB sont deux
configurations distinctes, même sur le même V1.

## Consommation

`Idle` signifie « inactif ». Ce n’est pas un réglage de fréquence CPU : ZMK
signale l’état et les composants concernés réduisent leurs tâches. Sur Splitergoll,
les mesures de batterie s’arrêtent et la recherche continue sans hôte se ferme.
La matrice et les connexions restent utilisables. Aucune mesure ne démontre une
consommation divisée par deux, notamment sans écran ni éclairage à éteindre.

En sommeil profond, le cœur et la radio s’arrêtent. Le circuit de détection GPIO
du nRF52840 reste capable de détecter un niveau électrique produit par une touche.
Il provoque un redémarrage, pas une reprise à l’instruction précédente. La RAM
ordinaire est réinitialisée par le démarrage, les réglages en flash sont conservés.
Voir [les états ZMK](https://zmk.dev/docs/features/low-power-states) et
[le mode System OFF du nRF52840](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/power.html).

**System OFF est le mode logiciel le plus économe du nRF52840.** Notre veille
profonde l’utilise déjà via `sys_poweroff()`. La batterie reste reliée et une petite
consommation subsiste pour les circuits encore alimentés et le réveil. Couper
l’interrupteur retire l’alimentation batterie, mais ne coupe pas un USB branché.

Actif, le clavier traite les frappes et entretient les connexions BLE. En inactivité,
ces connexions restent ouvertes, mais les mesures de batterie s’arrêtent. Sur V2 sans
écran ni LED externe, le gain par rapport à une pause avant l’inactivité peut donc
être faible. La veille profonde coupe la radio et réduit beaucoup plus la consommation.
Le courant réel de V2 reste à mesurer sur batterie avec un firmware sans logs USB.

La centrale consomme davantage que la périphérique pour entretenir la liaison split.
Le [modèle d’estimation ZMK](https://zmk.dev/power-profiler) donne des repères pour
la nice!nano officielle, pas une mesure de nos répliques ni une garantie d’autonomie.
Avec une nice!nano v2, un hôte et aucun écran/éclairage, ce modèle donne environ
0,53 mA à gauche et 0,038 mA à droite connectés sans frappe, contre un socle de
carte de 0,018 mA en sommeil, hors autodécharge. Une touche maintenue ajoute de
la consommation : cela ne distingue pas à lui seul l’état ACTIVE de l’état IDLE.
Ces calculs viennent des [données du modèle](https://github.com/zmkfirmware/zmk/blob/4dd95d7840eb3d5a3f59bb29fbf1ab054715ef85/docs/src/data/power.js).

Pour les essais ponctuels de V2 aux délais courts, suivre
[le protocole batteries et veille](tests-batteries-veille.md). Les logs se lisent
avec USB, le réveil de veille profonde se teste sur batterie sans USB.

## Résultats sur V2

Avec le diagnostic réglé à 10 secondes d’inactivité et 60 secondes de veille
profonde, les deux USB débranchés et les batteries sur ON :

- La reprise après inactivité paraît immédiate à droite, sans latence perçue.
- Après environ une minute, des appuis répétés à droite ne produisent aucune saisie.
- Des appuis à gauche rétablissent la saisie des deux côtés en 2 à 5 secondes,
  selon l’estimation de l’utilisateur. Aucun reset n’est rapporté.

La première frappe après veille, toutes les touches et les encodeurs restent
à vérifier séparément. Ce retour ne mesure pas la latence et ne valide pas
la durée normale de dix minutes. L’utilisateur a ensuite reflashé les deux moitiés
de V2 avec `bluetooth auto` normal : deux minutes d’inactivité, dix de veille profonde.
Ces réglages sont contrôlés au build, sans nouvelle mesure chronométrée sur batterie.

L’ancienne keymap `multi`, testée sous ZMK 0.3.0, revenait à Windows après veille
profonde : la couche choisie en mémoire n’était pas sauvegardée. Les profils fixes
ont directement leur OS en couche zéro. La vérification du nom macOS après réveil
reste distincte du constat que la saisie a repris.

## Fenêtre Bluetooth et inactivité

La recherche normale et l’inactivité durent chacune deux minutes, avec des points
de départ distincts. IDLE n’arrête pas les tâches différées : la fin de fenêtre et
la sauvegarde peuvent encore s’exécuter. Une fenêtre bornée peut finir en IDLE,
la recherche continue sans hôte s’y arrête. La radio ne compte pas comme une frappe.

Les essais courts de V1 confirment fermeture, IDLE, sauvegarde et persistance,
sans reprise d’activité. L’ordre précis n’est pas mesuré entre deux lectures USB.
Le [point de reprise](reprise.md) distingue ce résultat du complément sur batterie.
L’USB de V1 empêche SLEEP, dont l’essai demande V2.

Sur V2 sous USB, ON relie la batterie au contrôleur et OFF l’isole. IDLE reste
possible et SLEEP reste bloqué dans les deux cas. L’interrupteur n’est pas relié
à un GPIO de lecture : son état doit être indiqué pendant le diagnostic. Détecter
l’USB ne prouve ni la présence d’une batterie ni une charge effective.

## Essai au chrono

Utiliser le profil `bluetooth` normal. Ne pas reflasher le reset pour cet essai.

1. Mettre les deux interrupteurs batterie sur marche, débrancher les deux USB.
2. Taper sur les deux moitiés pour démarrer leurs délais, puis lancer le chrono après la dernière frappe.
3. Après 130 secondes, tester la reprise après inactivité. Cette frappe relance le délai.
4. Repartir de la dernière frappe et attendre plus de dix minutes sans utiliser les moitiés.
5. Faire un appui bref sur une lettre de gauche, attendre la reconnexion, puis tester la droite.
6. Noter si le réveil fonctionne sans reset et si la première lettre arrive ou sert seulement au réveil.

Pour valider V2 sur batterie, réaliser les essais d’inactivité et de veille avec
les deux USB débranchés, interrupteurs ON. Les logs USB servent au diagnostic
d’alimentation et ne sont pas lus pendant ces essais. Une reprise rapide sous USB
a été rapportée, elle ne remplace pas la validation sur batterie.

ZMK bloque la veille profonde dès que l’USB fournit du courant, y compris depuis
un chargeur. Le délai est compté depuis la dernière activité, pas depuis le débranchement.
Son contrôle périodique et la reconnexion ajoutent quelques secondes au délai affiché.

La veille profonde coupe le BLE sur chaque moitié. Une touche de droite ne peut
donc pas réveiller à distance une gauche éteinte. Si l’on souhaite reprendre la
saisie depuis n’importe quelle moitié, garder la centrale en inactivité plutôt
qu’en veille profonde est une option à décider après les essais.

## Revenir aux délais normaux

Reflasher les deux UF2 du profil `bluetooth`, disponibles pour V2 après `make build bluetooth macos`,
sans firmware de reset. Cela rétablit l’inactivité à deux minutes, la veille à
dix minutes et la mesure batterie à 5 secondes pendant l’activité. Les appairages sont conservés.
L’utilisateur confirme la remise du profil `bluetooth auto` normal sur les deux moitiés de V2.
Les USB branchés pour charger bloquent la veille profonde pendant la charge.
Dans `multi`, la première frappe de réveil et le retour à Windows restent les mêmes
après une veille profonde, seul son délai change.

Pour flasher ou redémarrer le contrôleur : [flash du firmware](flash-firmware.md).
