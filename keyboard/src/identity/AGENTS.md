# Identité d’hôte diagnostique

## Périmètre

Option `SPLITERGOLL_HOST_IDENTITY`, seulement gauche `bluetooth auto identity`.
Aucun changement du sélecteur, du transport HID clavier, de la keymap, des bonds
ou des namespaces NVS. ZMK_USB reste désactivé, USB_DEVICE_STACK et USB_DEVICE_HID
exposent une collection vendor sans Input/Output report. GATT s’ajoute aux services.
La droite et les profils normaux ne compilent aucun de ces adaptateurs.

## Protocole v1

Toutes les trames font 64 octets. USB Feature report sans Report ID, usage page
FF31 et usage 5350. Sur Mac les données n’ont pas de préfixe Report ID. Un futur
client Windows devra traiter le préfixe zéro imposé par son API HID séparément.
Service BLE 7d2f6767-4c2b-4d13-a528-80b87ac03810, caractéristique finissant par 3811.

| Octets | Réponse GET / READ | Annonce SET / WRITE |
| --- | --- | --- |
| 0..3 | SGID | SGID |
| 4,5,6,7 | version 1, type 1, USB=0/BLE=1, état | version 1, type 2, canal, zéro |
| 8..23 | jeton aléatoire du démarrage | écho exact |
| 24..39 | contexte propre à la liaison | écho exact |
| 40..55 | zéros, jamais l’identité d’hôte | UUID aléatoire de l’app, octets réseau |
| 56,57 | masque associé et masque des annonces BLE fraîches | zéros |
| 58,59,60 | USB prêt, annonce USB fraîche, slot BLE ou FF pour USB | zéros |
| 61..63 | zéros | zéros |

États : 0 en attente, 1 identité commune déclarée, 2 différente, 3 conflit.
Une identité déclarée par plusieurs pairs BLE frais crée un conflit et retire
tous les bits de correspondance. Les réponses ne révèlent aucun UUID d’hôte.
Les annonces expirent exactement après 15000 ms par soustraction uint32, sans timer.

Le jeton de démarrage provient de sys_csrand_get avant activation USB. Les contextes
de liaison sont uniques dans ce démarrage par tag de canal et compteur monotone,
sans crypto ou allocation dans un callback USB. Un compteur épuisé refuse le lien.
Ce challenge empêche le rejeu accidentel d’un ancien contexte, pas l’usurpation
d’identité par un programme malveillant. Copier le fichier utilisateur peut imiter
une identité sans qu’un second pair soit présent : ne jamais présenter ce protocole
comme une authentification ni déclencher une fusion de saisie dans ce premier lot.

## Adaptateurs

- USB : initialisation 90 avant ZMK 96, un wrapper usb_enable conserve le callback
  natif et le transmet exactement une fois. Reset, retrait et suspend révoquent le
  contexte, resume d’un appareil configuré en crée un nouveau. Ne pas appeler
  usb_enable une seconde fois. L’échec d’initialisation ne coupe pas le clavier BLE.
- BLE : rôle périphérique LE, niveau de sécurité L2 minimum, bond public retrouvé
  via bt_foreach_bond et profil ZMK connu. Le split est exclu. Permissions READ/WRITE
  ENCRYPT et PREPARE_WRITE, quatre fragments préparatoires possibles. Les préparations
  ne modifient rien, seule la trame complète assemblée est acceptée. CMD sans réponse refusé.
- Un spinlock protège le moteur borné, sans NVS ou activité ZMK. Connexion réutilisée,
  déconnexion et sécurité perdue révoquent l’ancien propriétaire. Aucun identifiant
  d’hôte, adresse BLE ou contenu de frappe n’est journalisé.

## Validation

Le moteur pur et le vrai adaptateur ont des suites C ASan/UBSan. Elles couvrent
formats/tailles/réserves, expiration et wrap, refus split/non chiffré/non appairé,
fragments non appliqués, callbacks natifs préservés, reconnexions et conflits.
Les frontières radio/USB sont doublées. La compilation nRF réelle vérifie les API
et les gardes, sans prouver l’énumération sur Mac. Les C de cette zone ont -Werror.
Le guide physique est docs/diagnostic-identite.md. Aucun flash ou effacement réalisé
par l’agent. Ne pas déduire une priorité USB de la seule présence de l’interface vendor.

Contrôles réalisés : 26 suites C ASan/UBSan et 112 cas AWK passent. PowerShell est
explicitement non exécuté, absent de PATH. Le C identité compile en ARM avec -Werror,
la paire identity passe les gardes et SHA-256. Le témoin bluetooth auto normal
est identique sur les deux moitiés au build du 24 octobre, et la droite identity
est identique à la droite normale. Les seuls avertissements observés du firmware
restent KSCAN et le drapeau du moteur Docker, pas un diagnostic dans ce nouveau C.
Les conteneurs éphémères sont libérés et les services préexistants préservés.
Ce constat logiciel ne valide pas le transfert USB/GATT physique.

Premier essai physique rapporté par l’utilisateur sur Mac + V2 : les deux annonces
sont présentes et le résultat est « Même ordinateur déclaré ». Cela valide ce
parcours de comparaison, sans prouver une authentification ni une priorité USB.
Retrait/rebranchement USB et relance du diagnostic restent à vérifier.

## Ideas / backlog

Client Windows après validation Mac. Avant un coordinateur de saisie : traitement
des identités copiées et des installations multi-utilisateurs, portée durable de
l’association, renouvellement et repli sans app. Ne pas réutiliser les wrappers
USB-auto et BLE-auto simultanément comme deux propriétaires de sélection.
