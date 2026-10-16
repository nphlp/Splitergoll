# Migration ZMK et Zephyr

La migration est préparée, pas appliquée. Le [snapshot stable](firmwares-stables.md)
conserve les 32 firmwares ZMK 0.3.0, leurs empreintes et les références de build dans Git.
ZMK gère le clavier, Zephyr fournit le système et les pilotes utilisés par ZMK.

## Les versions

État vérifié dans les sources officielles le 16 octobre 2026 :

| Élément | Version actuelle | Évolution disponible |
| --- | --- | --- |
| ZMK | 0.3.0, publiée le 1er août 2025 | Toujours la dernière stable. La 0.4.0 est en préparation |
| Zephyr utilisé par ZMK | 3.5.0 avec correctifs ZMK | 4.1.0 avec correctifs ZMK, sur la branche de développement ZMK |

Zephyr 3.5.0 est sorti le 20 octobre 2023, Zephyr 4.1.0 le 7 mars 2025.
ZMK a annoncé son passage à Zephyr 4.1 le 9 décembre 2025.
Notre ZMK a donc environ quatorze mois, mais aucun retard sur les releases stables.
Utiliser Zephyr 4.1 maintenant demande de figer un commit de développement ZMK.
Ne pas remplacer isolément Zephyr par sa dernière version indépendante de ZMK.

Sources : [releases ZMK](https://github.com/zmkfirmware/zmk/releases),
[future 0.4](https://github.com/zmkfirmware/zmk/pull/3024),
[Zephyr 3.5](https://github.com/zephyrproject-rtos/zephyr/releases/tag/v3.5.0),
[Zephyr 4.1](https://github.com/zephyrproject-rtos/zephyr/releases/tag/v4.1.0).

## Ce qui peut nous servir

- Remplacement du mécanisme déprécié de `&bootloader` par la rétention du mode de démarrage.
- Correctifs du traitement des rotations et d’un plantage des logs avec un encodeur sans binding.
- Correctifs de déconnexions USB et des buffers de découverte BLE entre moitiés.
- Verrouillage des couches par `&to` et `&tog`, intéressant mais à vérifier avec nos tap-dances.
- Indicateurs LED et sortie `OUT_NONE`, pour de futurs usages, sans les activer dans cette migration.

Ces changements figurent dans les [notes provisoires de la 0.4](https://github.com/zmkfirmware/zmk/pull/3024).
Ils ne prouvent ni une baisse de latence ni une correction de notre réveil depuis la droite.
Le support UART entre moitiés et ZMK Studio existaient déjà avant cette migration.

## Les ruptures à contrôler

| Sujet | Impact sur Splitergoll |
| --- | --- |
| Identification des cartes | Remplacer `nice_nano_v2` par la cible explicite `nice_nano@2.0.0//zmk` dans le build et la matrice manuelle |
| Bootloader | Contrôler la rétention et les codes UF2 de la carte, puis tester `&bootloader`, reset et double appui |
| Couches | `&to` et `&tog` verrouillent désormais la couche. Vérifier `&mo`, tap-dances, pavé, gaming et bascules OS |
| Réglages persistants | Le choix de sortie migre de `endpoints/preferred` à `endpoints/preferred2`. Un retour arrière peut perdre cette préférence |
| Compilation et matériel | Adapter image Docker et manifeste, puis vérifier GPIO, matrice, encodeurs, veille, batteries et logs USB |

Le partitionnement nice!nano du snapshot examiné reste identique : application à
`0x26000`, réglages à `0xEC000` sur 32 Kio. Recontrôler le Devicetree compilé et les
bornes UF2 avant tout flash. Splitergoll est un shield, la migration HWMv2 des
cartes personnalisées ne lui impose pas à elle seule une réécriture complète.

Sources : [guide Zephyr 4.1 de ZMK](https://zmk.dev/blog/2025/12/09/zephyr-4-1),
[comportement des couches](https://zmk.dev/docs/keymaps/behaviors/layers),
[migration des réglages de sortie](https://github.com/zmkfirmware/zmk/commit/6e7e0de2b621073392816b284ab4931bee709ee8).

## Détection d’OS

Aucune détection native USB/Bluetooth de Windows, macOS et Linux n’a été identifiée
dans les changements examinés. La [demande officielle](https://github.com/zmkfirmware/zmk/issues/2553)
reste ouverte. ZMK utilise encore l’ancienne pile USB, son remplacement est un chantier distinct.
La migration ne fournit donc pas automatiquement la reconnaissance d’OS souhaitée.

## Validation avant utilisation

1. Conserver le snapshot versionné et l’environnement de build actuel. Ne pas lancer `make clear`.
2. Après le commit du snapshot stable, préparer la migration vers un commit de développement figé.
3. Compiler les huit paires normales et des variantes debug, sans flash automatique.
4. Comparer bindings, couches et matériel compilés. Tester aussi les transitions de couches.
5. Tester V1 en `wire macos`, puis Windows, en gardant les anciens UF2 pour revenir en arrière.
6. Valider séparément V2 : appairages, deux batteries, veille et réveil. Aucun reset des réglages systématique.

La compilation seule ne valide pas la saisie, la radio ou la conservation des réglages.
