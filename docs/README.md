# Splitergoll

Clavier ergonomique split basé sur ZMK : 42 touches et deux encodeurs cliquables,
avec une keymap française complète et des adaptations Windows, macOS et Linux.

Ce fichier prépare le futur `README.md` et sert d’index à la documentation.
Il reste dans `docs/` pendant le chantier, le README actuel est conservé.

## Version en cours

Sur `unified-firmware` : build local et profil câble, avec la keymap de `dongle`.
La gauche envoie les touches par USB, la droite est alimentée par son câble et
communique avec la gauche en BLE. Les deux firmwares sont compilés localement,
les essais sur le clavier Windows restent à faire.

Les profils OS paramétrables et Bluetooth/dongle viendront ensuite. Le clavier
MacBook conserve son firmware dongle actuel. Les branches historiques restent
intactes et les variantes minimales sont des archives.

## Démarrer

Avec Make et Docker démarré, depuis la racine du dépôt :

```sh
make setup
make build
```

Le [guide de build](build-local.md) détaille les prérequis et les fichiers UF2 obtenus.
Ces commandes ne flashent pas le clavier.

## Documentation

- [Build local](build-local.md) : commandes, environnement et résultats.
- [Matériel et connexions](configuration-materielle.md) : cartes, rôles et limites.
- [Keymap](keymap.md) : organisation, couches et sept schémas de touches.
- [TODO et idées](TODO.md) : suivi des tâches restantes.
