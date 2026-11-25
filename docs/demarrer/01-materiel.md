# 01. Identifier son matériel et ses connexions

[Documentation](../README.md) › [Démarrer](../README.md#démarrer) › 01. Identifier son matériel et ses connexions

[02. Choisir et compiler son firmware →](02-compiler.md)

| Clavier | Alimentation | Usage habituel |
| --- | --- | --- |
| V1 | Deux USB, une par moitié. Aucune batterie | Profil `wire`, PC Windows ou Mac |
| V2 | Une batterie et un interrupteur par moitié, ou USB | Profil `bluetooth`, Mac ou PC appairé |

La moitié gauche reçoit les touches de la droite en Bluetooth et envoie la
saisie à l’ordinateur. Le profil matériel détermine cette deuxième liaison.

- **`wire`** : USB vers l’ordinateur depuis la gauche. Le câble de droite sert
  à l’alimentation. Le Bluetooth entre les deux moitiés reste actif.
- **`bluetooth`** : Bluetooth direct entre la gauche et l’ordinateur. Un câble
  USB peut alimenter le clavier ou [recharger V2](../utiliser-le-clavier/02-batteries.md).

Le XIAO BLE du montage dongle fait partie du matériel historique.
Pour les cartes, les rôles et le trajet des touches, voir la
[référence matérielle](../technique/firmware/02-materiel-et-profils.md).

---

[Documentation](../README.md) › [Démarrer](../README.md#démarrer) › 01. Identifier son matériel et ses connexions

[02. Choisir et compiler son firmware →](02-compiler.md)
