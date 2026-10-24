# Essais wire sur Mac et Windows

Utiliser le Clavier V1 sans batterie, avec les deux UF2 du même build `wire`.
Les [schémas de touches](keymap.md) restent la référence. La bascule d’OS décrite
ci-dessous concerne `multi`. Les profils fixes utilisent directement leur OS.

## Restriction Bluetooth du nouveau wire

Le correctif bloque les annonces de la centrale vers les ordinateurs, sans
changer les keymaps ni la liaison BLE entre moitiés. Le retour utilisateur après
le protocole ci-dessous est positif sur V1, sans nouvelle capture instrumentée.

1. Flasher la gauche avec le nouveau `wire windows`. Si la droite est identique
   au build précédent, son reflash n’est pas nécessaire. Ne pas effacer les bonds.
2. Brancher la gauche au PC Windows et alimenter la droite par son USB. Taper
   quelques lettres de chaque côté, puis vérifier Shift entre les deux moitiés.
3. Garder le Bluetooth du Mac actif : V1 ne doit plus s’y reconnecter comme clavier.
   L’ancienne entrée peut rester enregistrée, mais doit être déconnectée.
4. Redémarrer la gauche et vérifier à nouveau saisie USB des deux côtés et
   absence de reconnexion Bluetooth au Mac. V2 peut rester utilisé sur le Mac.

La droite garde ses annonces de transport split : ce test ne demande pas une
radio entièrement éteinte. Les variantes debug et usb-trace ajoutent uniquement
leur diagnostic USB, sans réautoriser les connexions Bluetooth de la gauche.

## Résultats confirmés

Sur le firmware migré vers Zephyr 4.1, l’utilisateur confirme sur V1 en `wire macos`
la saisie, Shift, les encodeurs, copier/coller, la déconnexion/reconnexion et
l’accès aux couches. Le triple appui droit active aussi le pavé numérique.
Il confirme aussi le verrouillage Shift, les accents essayés (dont le triple appui
`à/â/À`) et l’ordre des frappes alternées entre moitiés. La latence lui paraît
imperceptible, sans mesure instrumentée. L’accès au bootloader par L1 + L2 puis
clic sur l’encodeur gauche et le retour au firmware par reset sont confirmés.

Les résultats suivants concernent ZMK 0.3.0.

Le flash et la saisie des deux moitiés par USB sur le Mac sont confirmés par
l’utilisateur, avec la variante de diagnostic. Le clic d’encodeur qui affiche
l’OS paraît plus rapide qu’avec l’ancien firmware.

Le flash automatique avec `make flash wire macos right` puis `left` et le
fonctionnement des deux moitiés avec ce profil fixe sont aussi confirmés.
Le flash de `wire windows`, gaming inclus, et son fonctionnement sur V1
sont également confirmés. Le parcours détaillé des couches de jeu reste à vérifier.

Une latence était ressentie à droite avec `wire macos debug`. Après un passage par
`wire windows` normal, puis `wire macos` normal, l’écart est devenu imperceptible.
Comparer normal/debug à keymap identique avant d’en attribuer la cause aux logs ou au BLE.
La lecture des deux consoles USB est validée sur V1 en `wire macos debug`.
Le parcours complet des touches et couches sur les deux OS reste à faire.
Voir [le diagnostic de latence](latence.md).

## Préparer le clavier

1. Flasher la droite puis la gauche avec les deux UF2 du même build `wire`.
2. Ne pas flasher `settings_reset` tant qu’aucun problème d’appairage n’est établi.
3. Brancher les deux USB au Mac. Désactiver temporairement le Bluetooth du Mac
   pour vérifier que la saisie passe bien par USB. Cela ne coupe pas le BLE entre moitiés.
4. Vérifier quelques lettres de chaque côté avant de tester les couches.

Le câble gauche transporte la saisie et l’alimentation, le droit fournit l’alimentation.
Sans batterie, retirer les deux USB éteint complètement le clavier.

## Valider la migration sur V1 avec macOS

Utiliser `wire macos` normal, sans logs USB. Les résultats déjà confirmés sont indiqués plus haut.

1. **Saisie et liaison entre moitiés** : tester chaque touche, maintenir une lettre
   puis relâcher, combiner Shift droit avec une lettre gauche. Aucun appui ne doit rester bloqué.
2. **Couches**, puis **tap-dances** : suivre les deux protocoles distincts ci-dessous.
3. **Encodeurs** : tourner les deux boutons lentement dans chaque sens,
   puis plus vite. Tester leurs clics, le volume et la navigation selon la couche.
4. **Macros macOS** : vérifier copier/coller, sélection, navigation par mot,
   accents et symboles utilisés au quotidien.
5. **Redémarrage et reconnexion** : retirer les deux USB, puis les rebrancher.
   Les deux côtés doivent fonctionner et la keymap fixe doit rester macOS,
   sans effacement des réglages.
6. **Bootloader, en dernier** : en tertiaire, cliquer l’encodeur gauche pour
   vérifier l’apparition de `NICENANO`. Un reset physique simple doit permettre
   de reprendre la saisie. Vérifier aussi le double appui physique pour accéder
   au bootloader. Ne pas utiliser le clic droit de cette couche, qui efface les appairages BLE.

Ces essais couvrent les principaux risques de la migration, sans garantir l’absence
de toute régression. Le gaming Windows et V2 sur batterie (BLE vers le Mac, batteries,
veille et réveil) nécessitent des validations séparées.

### Couches : maintien simple

Une couche est une disposition de touches. Le tap-dance est le mécanisme qui
choisit une action selon le nombre d’appuis. Les deux existaient avant la migration.

Dans les sources, **L1 est à droite**, entre Shift et Entrée.
**L2 est à gauche**, entre Option et Espace. Le pavé reste actif après relâchement,
ce n’est pas un verrouillage du clavier entier.

Dans un document de texte, revenir à la base avant de commencer. Pour chaque ligne,
maintenir les touches indiquées une demi-seconde avant le geste de contrôle,
puis tout relâcher. La « touche Y » désigne toujours sa position physique de base.

| Geste | Contrôle | Résultat attendu |
| --- | --- | --- |
| Maintenir L1 droite | Appuyer sur Y | `"` : couche primaire |
| Maintenir L2 gauche | Appuyer sur Y | `=` : couche secondaire |
| Maintenir L2 gauche | Tourner l’encodeur droit dans un texte de plusieurs lignes | Curseur vertical |
| Maintenir L1 et L2 | Tourner ce même encodeur droit | Curseur horizontal : couche tertiaire |
| Tout relâcher | Appuyer sur Y | `y` : retour à la base |

### Tap-dances : appuis multiples

Enchaîner les appuis rapidement, avec moins de 250 ms entre deux appuis.
Attendre une demi-seconde entre deux essais. Pour un double appui maintenu,
faire « appui, relâchement, appui maintenu », puis attendre une demi-seconde.

| Geste depuis la base | Contrôle | Résultat attendu |
| --- | --- | --- |
| Double appui sur L1 droite, maintenir le second | Appuyer sur Y, puis tout relâcher | `=`, puis Y redonne `y` : secondaire temporaire |
| Double appui sur L2 gauche, maintenir le second | Appuyer sur Y, puis tout relâcher | `"`, puis Y redonne `y` : primaire temporaire |
| Triple appui sur L1 droite, tout relâcher | Attendre puis appuyer sur Y | `=` : pavé numérique persistant |
| Un appui sur cette même L1 droite | Attendre puis appuyer sur Y | `y` : sortie du pavé |
| Double appui sur Shift droit, relâcher | Attendre puis appuyer sur A | `A` : Shift verrouillé |
| Un appui sur Shift droit, relâcher | Attendre puis appuyer sur A | `a` : Shift déverrouillé |

Pour tester un tap-dance de caractères, maintenir L2 gauche une demi-seconde.
La touche physique S donne `é` avec un appui, `É` avec deux appuis rapides.
Attendre une demi-seconde après chaque séquence, sans relâcher L2 entre les deux essais.

La touche physique F ne prévoit que deux actions : `è` puis `ë`. Un troisième
appui lance une nouvelle séquence : `ëè` est donc attendu, comme avant la migration.
La touche V prévoit trois actions : `à`, `â`, `À`. Le délai est de 250 ms depuis
le dernier appui tant que la séquence n’a pas atteint son nombre maximal d’actions,
pas de 250 ms pour l’ensemble des trois appuis. Shift verrouillé reste un maintien
logique de Shift, pas un Caps Lock.

## Parcourir la keymap

Sur chaque ordinateur, utiliser le profil fixe correspondant ou choisir le mode OS
dans `multi`. Garder une disposition d’entrée française adaptée.
L’OS n’est pas détecté automatiquement.

- Vérifier les 42 touches, les deux clics d’encodeur et leurs rotations dans les deux sens.
- Tester Shift droit avec une lettre gauche et les couches activées depuis l’autre moitié.
- Tester les accents et symboles : `é è ê ë à â ù û ç œ É À Ç`, puis `0123456789 () [] {} <> @ # | \ /`.
- Parcourir les couches primaire, secondaire, tertiaire et pavé numérique.
- Vérifier copier/coller et navigation avec Ctrl sous Windows, Command/Option sous macOS.
- Vérifier les deux couches de jeu sous Windows, selon les actions documentées.

Le changement d’OS utilise `L1` et le clic d’encodeur droit : Windows → Linux → macOS
→ Windows. Relâcher `L1` entre deux changements. Les retours du pavé numérique et du
jeu vont actuellement à Windows, c’est une limite connue de `multi`.

## Changer d’ordinateur et redémarrer

Déplacer les deux USB vers Windows, puis refaire les vérifications dans le mode
Windows. Désactiver temporairement son Bluetooth pour confirmer la sortie USB.
Un débranchement complet redémarre le firmware : `multi` revient à Windows,
les profils fixes restent sur leur OS. L’appairage interne doit être conservé.
Vérifier que les deux côtés refonctionnent après reconnexion, sans firmware de reset.

En cas d’écart, noter l’ordinateur, la couche, la touche, le résultat attendu et
le résultat obtenu. Les [logs USB](logs-usb.md) sont disponibles pour le diagnostic.
