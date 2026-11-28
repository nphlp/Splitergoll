# 1. Identifier son matériel et ses connexions

[Documentation](../README.md) › [Développement](../README.md#développement) › 1. Identifier son matériel et ses connexions

[2. Choisir et compiler son firmware →](2-compiler.md)

| Clavier | Alimentation | Usage habituel |
| --- | --- | --- |
| V1 | Deux USB, une par moitié. Aucune batterie | Profil `wire`, PC Windows ou Mac |
| V2 | Une batterie et un interrupteur par moitié, ou USB | Profil `bluetooth`, Mac ou PC appairé |

La moitié gauche reçoit les touches de la droite en Bluetooth et envoie la
saisie à l’ordinateur. Le profil matériel détermine cette deuxième liaison.

- **`wire`** : USB vers l’ordinateur depuis la gauche. Le câble de droite sert
  à l’alimentation. Le Bluetooth entre les deux moitiés reste actif.
- **`bluetooth`** : Bluetooth direct entre la gauche et l’ordinateur. Un câble
  USB peut alimenter le clavier ou [recharger V2](../utilisation/2-batteries.md).

Le XIAO BLE du montage dongle fait partie du matériel historique.
Pour les cartes, les rôles et le trajet des touches, voir la
[référence matérielle](../technique/firmware/2-materiel-et-profils.md).

---

[Documentation](../README.md) › [Développement](../README.md#développement) › 1. Identifier son matériel et ses connexions

[2. Choisir et compiler son firmware →](2-compiler.md)
