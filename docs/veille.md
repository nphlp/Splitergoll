# Veille et réveil

## Veille et réveil

| État | Délai | Comportement prévu |
| --- | --- | --- |
| Inactivité | 30 secondes | Connexion BLE conservée, reprise à la frappe |
| Veille profonde | 15 minutes dans le profil Bluetooth | Réveil par une touche, puis reconnexion |

Le profil `wire` désactive la veille profonde. Le profil `bluetooth` l’active et
la matrice est déclarée comme source de réveil. Le bouton reset ne doit donc pas
être nécessaire pour réveiller le clavier, mais ce comportement reste à tester
sur les deux moitiés, ainsi que l’arrivée de la première frappe.
[Fonctionnement ZMK](https://zmk.dev/docs/features/low-power-states).

La saisie des deux côtés est confirmée sur le Mac, avec USB branchés puis sur
batterie seule. Cela ne valide pas encore le réveil après quinze minutes.

Pour flasher ou redémarrer le contrôleur : [flash du firmware](flash-firmware.md).
