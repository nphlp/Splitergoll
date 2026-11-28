# 2. Choisir et compiler son firmware

[Documentation](../README.md) › [Développement](../README.md#développement) › 2. Choisir et compiler son firmware

[← 1. Identifier son matériel et ses connexions](1-materiel.md) · [3. Flasher le clavier →](3-flasher.md)

## Choisir le profil matériel puis la keymap

| Usage | Matériel à passer à `make build` |
| --- | --- |
| Saisie par le câble USB branché à gauche | `wire` |
| Saisie Bluetooth directe | `bluetooth` |

| Keymap | Utilisation |
| --- | --- |
| [macos](../layouts/2-macos.md) | Layout fixe macOS |
| [windows](../layouts/3-windows.md) | Layout fixe Windows, avec jeu |
| [linux](../layouts/4-linux.md) | Layout fixe Linux |
| [multi](../technique/firmware/3-architecture-keymaps.md#comportement-de-multi) | Trois OS à sélectionner au clavier, avec jeu |
| [auto](../layouts/5-auto.md) | macOS ou Windows choisi selon la connexion ou l’ordinateur |

## Compiler les deux moitiés

Démarrer Docker et ouvrir un terminal à la racine du dépôt. Au premier build,
lancer `make setup`. Choisir ensuite une commande adaptée, par exemple :

```sh
make build wire windows
make build bluetooth macos
```

Les deux arguments sont obligatoires. Le build dépose les deux fichiers UF2 et
leurs métadonnées sous `dist/firmware/`. Le
[guide de flash](3-flasher.md) explique comment les installer sur le clavier.
Pour les prérequis complets, l’organisation du build et les options de
diagnostic, voir la [référence du build](../technique/firmware/1-build-local.md).

---

[Documentation](../README.md) › [Développement](../README.md#développement) › 2. Choisir et compiler son firmware

[← 1. Identifier son matériel et ses connexions](1-materiel.md) · [3. Flasher le clavier →](3-flasher.md)
