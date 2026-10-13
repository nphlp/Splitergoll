# Splitergoll

## Intention

Une configuration ZMK complète, commune et compilable en local. Projet classé
`sync` avec les règles globales. Branche de travail : `unified-firmware`.

Le [futur README](docs/README.md) indexe les guides.
Il reste dans `docs/` jusqu’à la mise à jour finale. Les guides
courts couvrent [le build](docs/build-local.md), [le matériel](docs/configuration-materielle.md)
et [la keymap](docs/keymap.md). Les tâches et idées sont centralisées dans
[docs/TODO.md](docs/TODO.md), à la demande de l’utilisateur.

## Invariants

- Deux claviers : PC Windows alimenté par deux USB, MacBook avec batteries et dongle XIAO BLE.
- Les moitiés sont des répliques de nice!nano v2. Le Nordic des photos n’est pas utilisé.
- La keymap complète de `dongle` à `2584ca4` fait référence : conserver les 44 positions,
  accents, Shift, macros, couches et encodeurs. Les helpers, transforms et GPIO sont inchangés.
- Les branches `-minimal` sont des archives, exclues du chantier.
- Les branches historiques restent intactes. `wire` archive `main` à `cbad9a0`,
  `wire-minimal` archive `main-minimal` à `27ed2b7`.
- Le README reste au contenu validé du commit `a1f6c35` jusqu’à sa mise à jour finale.
- Distinguer l’OS, le rôle central/périphérique et la sortie USB/Bluetooth.
- Le BLE entre moitiés reste nécessaire avec le câblage actuel. Le Bluetooth direct
  et la bascule automatique USB sont des profils futurs à valider.

## Étape active et validation

Build local et profil câble : gauche centrale USB, droite périphérique BLE,
alimentation USB, sans veille profonde ni reporting batterie.

Le Makefile expose deux commandes et délègue aux scripts `setup.sh` et `build.sh`.
Les constantes partagées sont en tête de `scripts/build-common.sh`. `container.sh`
exécute `west` et vérifie les firmwares. Aucun `.env` n’est nécessaire.
ZMK 0.3.0 et les
59 projets résolus sont figés dans `config/west.yml`. Les workspaces sont séparés
par empreinte de l’environnement, les caches CMake par moitié.

`config/splitergoll.conf` contient les options communes. Les fichiers `.conf` des
moitiés contiennent leurs différences. La matrice `build.yaml` couvre uniquement
les deux moitiés du profil câble.

Valider la syntaxe Make/shell, le manifeste, la conservation de la keymap et le
build réel. Pas de nouveaux tests automatisés d’outillage par défaut. Les essais
matériels restent distincts. Aucun flash implicite. Les conteneurs sont éphémères,
le dépôt est monté en lecture seule et la compilation n’a pas de réseau.

Chaque fonction des scripts doit avoir un commentaire court en français au-dessus,
qui explique son rôle et les arguments lorsque leur sens n’est pas évident.
Chaque script doit aussi expliquer son rôle dans un commentaire en tête de fichier.

Exception locale autorisée par l’utilisateur : les chemins de cet outillage et
l’image figée peuvent être des constantes dans le script commun. Les chemins
locaux sont relatifs au dépôt, aucun chemin personnel absolu n’est inscrit.
Cette exception remplace ici la règle globale des variables d’environnement,
sans l’étendre aux autres zones.

`docs/keymap.md` reprend les sept schémas du README, avec les légendes alignées
sur les bindings de `dongle`. Ils montrent la base Windows et les couches communes,
les adaptations OS sont résumées séparément. Les points indiquent les bindings
transparents, `∅` les touches sans action. Garder les schémas cohérents avec la source.
Ne pas modifier le README actuel avant la promotion finale de l’index.

`docs/configuration-materielle.md` distingue le matériel, le trajet des touches
et l’alimentation. Ses trois architectures doivent préciser l’état réel : câble
compilé, dongle utilisé sur le Mac mais pas encore réintégré au build, Bluetooth
direct à ajouter. Les capacités natives ZMK ne sont pas des profils déjà testés.

## TODO

Voir [docs/TODO.md](docs/TODO.md), source unique du suivi.

## Ideas / backlog

Consigner les idées dans [docs/TODO.md](docs/TODO.md), sans dupliquer les listes.
