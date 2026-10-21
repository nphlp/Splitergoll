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

## Consommation

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
la durée normale de dix minutes. Le délai normal d’inactivité passe à deux minutes
dans les prochains builds, le diagnostic déjà installé garde ses délais courts.

L’ancienne keymap `multi`, testée sous ZMK 0.3.0, revenait à Windows après veille
profonde : la couche choisie en mémoire n’était pas sauvegardée. Les profils fixes
ont directement leur OS en couche zéro. La vérification du nom macOS après réveil
reste distincte du constat que la saisie a repris.

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
dix minutes et la mesure batterie à 60 secondes. Les appairages sont conservés.
La remise du nouveau build normal sur V2 reste à faire après les essais.
Les USB branchés pour charger bloquent la veille profonde pendant la charge.
Dans `multi`, la première frappe de réveil et le retour à Windows restent les mêmes
après une veille profonde, seul son délai change.

Pour flasher ou redémarrer le contrôleur : [flash du firmware](flash-firmware.md).
