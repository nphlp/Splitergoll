# 02. Choisir et compiler son firmware

[Documentation](../README.md) › [Démarrer](../README.md#démarrer) › 02. Choisir et compiler son firmware

[← 01. Identifier son matériel et ses connexions](01-materiel.md) · [03. Flasher le clavier →](03-flasher.md)

## Choisir le profil matériel puis la keymap

| Usage | Matériel à passer à `make build` |
| --- | --- |
| Saisie par le câble USB branché à gauche | `wire` |
| Saisie Bluetooth directe | `bluetooth` |

| Keymap | Utilisation |
| --- | --- |
| [macos](../layouts-clavier/02-macos.md) | Layout fixe macOS |
| [windows](../layouts-clavier/03-windows.md) | Layout fixe Windows, avec jeu |
| [linux](../layouts-clavier/04-linux.md) | Layout fixe Linux |
| [multi](../layouts-clavier/05-multi.md) | Trois OS à sélectionner au clavier, avec jeu |
| [auto](../layouts-clavier/06-auto.md) | macOS ou Windows choisi selon la connexion ou l’ordinateur |

## Compiler les deux moitiés

Démarrer Docker et ouvrir un terminal à la racine du dépôt. Au premier build,
lancer `make setup`. Choisir ensuite une commande adaptée, par exemple :

```sh
make build wire windows
make build bluetooth macos
```

Les deux arguments sont obligatoires. Le build dépose les deux fichiers UF2 et
leurs métadonnées sous `dist/firmware/`. Le
[guide de flash](03-flasher.md) explique comment les installer sur le clavier.
Pour les prérequis complets, l’organisation du build et les options de
diagnostic, voir la [référence du build](../technique/firmware/01-build-local.md).

---

[Documentation](../README.md) › [Démarrer](../README.md#démarrer) › 02. Choisir et compiler son firmware

[← 01. Identifier son matériel et ses connexions](01-materiel.md) · [03. Flasher le clavier →](03-flasher.md)
