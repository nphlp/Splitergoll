# Migration ZMK et Zephyr

Le build utilise maintenant ZMK `4dd95d7840eb`, avec Zephyr 4.1 et ses correctifs ZMK.
Les 32 UF2 compilent et passent les contrôles logiciels. Sur V1 en `wire macos`,
l’utilisateur confirme saisie, Shift, encodeurs, copier/coller, reconnexion et accès
aux couches, dont le pavé par triple appui droit. L’accès au bootloader par L1 + L2
puis clic sur l’encodeur gauche et le retour par reset sont confirmés.
Les gestes restants sont suivis dans le [protocole wire](tests-wire.md).
Les essais suivants confirment le verrouillage Shift, les accents essayés dont
`à/â/À` et l’ordre des frappes alternées. Le résultat `ëè` sur trois appuis de F
correspond aux deux actions historiques `è/ë`, sans régression identifiée.
Le [snapshot stable](firmwares-stables.md) conserve dans l’historique Git les
32 firmwares ZMK 0.3.0 et les références nécessaires au retour arrière.
Il a été retiré du checkout à la demande de l’utilisateur après les essais V1.
ZMK gère le clavier, Zephyr fournit le système et les pilotes utilisés par ZMK.

## Les versions

État vérifié dans les sources officielles le 16 octobre 2026 :

| Élément | Snapshot stable conservé | Build migré |
| --- | --- | --- |
| ZMK | 0.3.0, publiée le 1er août 2025 | Commit `4dd95d7840eb` du 9 octobre 2026, préparant la 0.4 |
| Zephyr utilisé par ZMK | 3.5.0 avec correctifs ZMK | 4.1.0 avec correctifs ZMK, commit `10ba6d0cb38b` |

Zephyr 3.5.0 est sorti le 20 octobre 2023, Zephyr 4.1.0 le 7 mars 2025.
ZMK a annoncé son passage à Zephyr 4.1 le 9 décembre 2025.
La version stable conservée a environ quatorze mois, mais reste la dernière publiée.
Le nouveau build utilise un snapshot de développement figé, pas une release 0.4 officielle.
Ne pas remplacer isolément Zephyr par sa dernière version indépendante de ZMK.

Sources : [releases ZMK](https://github.com/zmkfirmware/zmk/releases),
[future 0.4](https://github.com/zmkfirmware/zmk/pull/3024),
[Zephyr 3.5](https://github.com/zephyrproject-rtos/zephyr/releases/tag/v3.5.0),
[Zephyr 4.1](https://github.com/zephyrproject-rtos/zephyr/releases/tag/v4.1.0).

## Ce qui peut nous servir

- Remplacement du mécanisme déprécié de `&bootloader` par la rétention du mode de démarrage.
- Correctifs du traitement des rotations et d’un plantage des logs avec un encodeur sans binding.
- Correctifs de déconnexions USB et des buffers de découverte BLE entre moitiés.
- Verrouillage des couches par `&to` et `&tog`, désactivé ici pour garder le comportement stable.
- Indicateurs LED et sortie `OUT_NONE`, pour de futurs usages, sans les activer dans cette migration.

Ces changements figurent dans les [notes provisoires de la 0.4](https://github.com/zmkfirmware/zmk/pull/3024).
Ils ne prouvent ni une baisse de latence ni une correction de notre réveil depuis la droite.
Le support UART entre moitiés et ZMK Studio existaient déjà avant cette migration.

## Les ruptures à contrôler

| Sujet | Impact sur Splitergoll |
| --- | --- |
| Identification des cartes | Cible explicite `nice_nano@2.0.0//zmk` dans le build et la matrice manuelle |
| Bootloader | Rétention et code UF2 `0x57` contrôlés. Accès par `&bootloader` et retour par reset confirmés sur V1, double appui à confirmer |
| Couches | Propriété `locking` retirée de `&to` et `&tog`. Les bindings sont conservés |
| Réglages persistants | Le choix de sortie migre de `endpoints/preferred` à `endpoints/preferred2`. Un retour arrière peut perdre cette préférence |
| Compilation et matériel | Image officielle 4.1 et manifeste figés, 32 UF2 contrôlés. Validation matérielle restante |

Le partitionnement nice!nano reste identique : application à `0x26000`, réglages à
`0xEC000` sur 32 Kio. Les arbres compilés et les bornes UF2 sont vérifiés.
Splitergoll est un shield, la migration HWMv2 des cartes personnalisées ne lui
impose pas à elle seule une réécriture complète.

Sources : [guide Zephyr 4.1 de ZMK](https://zmk.dev/blog/2025/12/09/zephyr-4-1),
[comportement des couches](https://zmk.dev/docs/keymaps/behaviors/layers),
[migration des réglages de sortie](https://github.com/zmkfirmware/zmk/commit/6e7e0de2b621073392816b284ab4931bee709ee8).

## Détection d’OS

Aucune détection native USB/Bluetooth de Windows, macOS et Linux n’a été identifiée
dans les changements examinés. La [demande officielle](https://github.com/zmkfirmware/zmk/issues/2553)
reste ouverte. ZMK utilise encore l’ancienne pile USB, son remplacement est un chantier distinct.
La migration ne fournit donc pas automatiquement la reconnaissance d’OS souhaitée.

## Validation avant utilisation

Les 32 arbres compilés conservent keymaps, conditions, macros, matrice, encodeurs
et paramètres batterie. Le moteur de transitions sans verrouillage est comparé
à celui de la 0.3. Les UF2, leurs empreintes et les rôles sont vérifiés.

1. Flasher V1 en `wire macos`, droite puis gauche, avec les deux USB branchés.
2. Tester les deux côtés, encodeurs, tap-dances, couches et retour du pavé numérique.
3. Tester la touche bootloader, le reset simple et le double appui physique.
4. Valider Windows et le gaming sur le PC, puis V2 séparément : BLE, batteries et veille.

Les anciens UF2 sont récupérables dans l’historique Git. Aucun reset des réglages systématique.

La compilation seule ne valide pas la saisie, la radio ou la conservation des réglages.

## Avertissements conservés

`KSCAN` est déprécié dans Zephyr 4.1. Remplacer la matrice par l’API input sera
un chantier distinct. Le compilateur signale aussi un qualificatif `const` perdu
dans les tap-dances ZMK. Le code copie les bindings avant invocation, sans modifier
le tableau de configuration. Le correctif amont reste à suivre. Aucun patch local
ni masquage d’avertissement n’est appliqué. Le drapeau Docker insecure reste un sujet du moteur.
