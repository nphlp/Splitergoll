# Appairage Bluetooth

La gauche est la centrale : elle se connecte à la droite et au Mac. Ces deux
liaisons ont leurs propres clés d’appairage. Flasher une nouvelle keymap ou
appuyer sur reset ne supprime pas ces clés.

## Effacer seulement les ordinateurs appairés

Avec la keymap fixe `macos`, maintenir **L1 + L2**, attendre une demi-seconde,
puis cliquer sur **l’encodeur droit**. Relâcher les touches. Ce raccourci appelle
`BT_CLR_ALL` : tous les slots d’ordinateurs sont vidés et le slot 0 est sélectionné.
La liaison entre les deux moitiés et la keymap sont conservées.

Oublier ensuite l’ancienne entrée Splitergoll dans les réglages Bluetooth du Mac,
puis l’appairer à nouveau. Les deux moitiés doivent pouvoir communiquer pour
effectuer le raccourci. Le clic gauche de cette même couche ouvre le bootloader.

Ce geste ne fait pas un `settings_reset`. Le bouton reset redémarre seulement
le contrôleur. Le nouveau `bluetooth auto` de reconnexion ne permet pas encore
de nouveaux appairages, utiliser ici le profil fixe prévu pour V2.

## Réinitialiser les réglages

Lors d’un changement de centrale ou d’un appairage incohérent :

1. Flasher le firmware `settings_reset` nice!nano v2 sur les deux moitiés. Laisser la première sous ce firmware pendant le reset de la seconde.
2. Vérifier que l’effacement a eu lieu, puis remettre les deux UF2 du profil souhaité, droite puis gauche.
3. Oublier l’ancienne entrée Bluetooth sur le Mac et refaire l’appairage.

Le reset garde le Bluetooth désactivé : les moitiés ne se réappairent pas pendant
l’effacement. Il ne change pas les affectations de touches. Le reset cible la
carte, il convient aux profils `wire` et `bluetooth`.

Les UF2 sont préparés à part dans `.local/flash/` :

- `settings-reset-nice-nano-v2.uf2` : reset officiel.
- `settings-reset-nice-nano-v2-logging.uf2` : même effacement avec logs USB pour le contrôler.

Cette opération efface aussi les profils hôtes. Elle ne doit pas devenir
systématique à chaque changement de keymap.
[Procédure ZMK](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/troubleshooting/connection-issues.mdx).

## Si la droite ne transmet pas ses touches

Vérifier successivement le firmware présent, le scan des touches et la connexion
BLE. Les [messages du Finder](flash-firmware.md#messages-du-finder) ne suffisent
pas à diagnostiquer le flash. Des UF2 temporaires avec le snippet `zmk-usb-logging`
permettent d’observer les événements et l’appairage.
[Logs USB ZMK](https://github.com/zmkfirmware/zmk/blob/v0.3.0/docs/docs/development/usb-logging.mdx).

Le blocage rencontré ici venait d’une clé conservée sur la droite et d’une nouvelle
demande d’appairage de la gauche. La droite refusait son remplacement, laissant
la liaison non chiffrée. Les effacements vérifiés sur les deux moitiés, puis la
restauration des mêmes firmwares normaux et le nouvel appairage Mac, ont résolu le
problème. La saisie fonctionne aussi sur batterie seule, sans USB ni dongle.

Aucune touche ni protection Bluetooth n’a été changée pour cette correction.
Les preuves techniques et les limites du diagnostic restent dans `AGENTS.md`.
