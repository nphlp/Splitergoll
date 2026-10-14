# Veille et redémarrage

## Réveil par les touches

ZMK distingue deux états automatiques :

| État | Délai par défaut | Effet |
| --- | --- | --- |
| Inactivité | 30 secondes | Connexion BLE conservée, reprise immédiate à la frappe |
| Veille profonde | 15 minutes, si activée | Microcontrôleur éteint, puis redémarrage et reconnexion au réveil |

La matrice `kscan0` porte maintenant `wakeup-source`. Cela permet à chaque touche
de la matrice de déclencher le réveil, sans bouton reset. Les GPIO et la keymap
restent identiques. La reconnexion BLE peut prendre quelques secondes.
[Fonctionnement ZMK](https://zmk.dev/docs/features/low-power-states).

Le profil câble garde `CONFIG_ZMK_SLEEP=n`. Pour les futurs profils sur batterie,
la configuration envisagée est :

```conf
CONFIG_ZMK_SLEEP=y
CONFIG_ZMK_IDLE_SLEEP_TIMEOUT=900000
```

Les branches historiques ne sont pas modifiées. Le firmware déjà installé sur
le clavier Mac ne reçoit donc pas cette correction sans compilation puis flash.

À valider sur le matériel : chaque touche des deux moitiés, clics des encodeurs,
première frappe après réveil, reconnexion au dongle ou au Mac, et charge USB.
Le réveil électrique ne garantit pas que la première frappe arrive à l’ordinateur.

Validation : les deux moitiés compilent avec et sans veille. Les essais avec veille
restent dans `.local/`, sans export dans `dist/`. Aucun flash ni réveil réel testé.

## Entrée dans le bootloader

Le mode bootloader reste utilisé pour flasher le clavier. Seul l’ancien mécanisme
`NRF_STORE_REBOOT_TYPE_GPREGRET` de transmission du mode de redémarrage est déprécié.
ZMK 0.3.0 l’active pour que `&bootloader` transmette le code UF2 attendu.

La nouvelle API est `bootmode_set()` avec `RETENTION_BOOT_MODE`. Son adoption
demande aussi l’adaptation du comportement reset, de la mémoire conservée et du
code spécifique au bootloader Adafruit nRF52. Ce n’est pas une option à remplacer
dans notre `.conf`. On conserve donc le fonctionnement actuel jusqu’à la migration.
[Intégration moderne ZMK](https://zmk.dev/docs/config/bootloader).
