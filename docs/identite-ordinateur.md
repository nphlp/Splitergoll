# Reconnaître un ordinateur en USB et Bluetooth

**Diagnostic Mac initial validé sur V2, priorité USB expérimentale développée.** Le rapprochement est aidé par
les applications Mac et Windows, dont le client d’identité Windows reste à développer.
Le [nouveau mode USB prioritaire](priorite-usb.md) utilise cette association, son
essai physique reste à faire. Les profils normaux sont conservés. Le clavier doit
rester utilisable sans application. Un OS identique ou un état Caps Lock commun
ne suffit pas. Les [destinations de saisie](destinations-saisie.md) décrivent le
comportement souhaité. La recherche autonome ci-dessous reste disponible.

## Ce que les données ordinaires apporteraient

| Donnée non conservée | Intérêt pour reconnaître l’ordinateur |
| --- | --- |
| Descripteurs USB, série, ContainerID | Identifient le périphérique, donc le clavier |
| Réglages du port série de diagnostic | Débit, parité et format, sans identité du PC |
| Rapports HID ordinaires | États ou configuration du clavier, sans identifiant d’hôte garanti |
| Adresse d’identité et données d’appairage BLE | Reconnaissent un pair BLE, sans équivalent USB standard trouvé |
| Nom, série ou System ID d’un serveur GATT de l’ordinateur | Disponibilité à vérifier, ne fournit pas seul le rapprochement USB |
| Hash de base GATT | Décrit les services du serveur, pas une identité de machine |

Le [ContainerID Microsoft](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-containerids-in-windows)
regroupe les fonctions du périphérique. Les [Feature Reports HID](https://www.kernel.org/doc/html/next/hid/hid-transport.html)
sont demandés par le logiciel hôte : en déclarer un ne force pas l’OS à y écrire
une identité. Les champs [DIS](https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/DIS_v1.2/out/en/index-en.html)
sont optionnels. Dans nos captures, les lectures de ces champs visent le clavier.
Le [Database Hash](https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Core-62/out/en/host/generic-attribute-profile--gatt-.html)
détecte les changements des services GATT.

## Deux mécanismes transmettent réellement une adresse d’hôte

**Apple Magic Keyboard.** Apple confirme l’[appairage automatique par câble](https://support.apple.com/en-gb/119917).
L’[étude originale de SkySafe](https://github.com/skysafe/reblog/blob/main/cve-2024-0230/README.md)
décrit l’envoi par USB de l’adresse Bluetooth du Mac et de données d’appairage.
C’est un protocole propre aux appareils Apple, pas le comportement d’un clavier
HID générique. Son déclenchement pour notre matériel et sa compatibilité avec
macOS actuel et le BLE ZMK ne sont pas établis. L’étude documente aussi des
vulnérabilités corrigées : ne pas supposer le même comportement des versions actuelles.

**Microsoft Seamless Pairing pour stylets.** Le [guide Microsoft](https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/seamless-pairing-implementation-guide)
prévoit un Host Bluetooth Report, contenant adresse et type d’adresse du PC,
écrit par un service Windows intégré. Il exige une collection HID de numériseur
et une activation explicite via une extension INF. Ce n’est donc pas disponible
pour notre clavier par simple déclaration d’un champ, sans configuration hôte.

Ces deux exemples prouvent l’existence d’échanges d’identité utiles. Ils ne
prouvent pas qu’un même mécanisme soit utilisable sur nos deux OS et contrôleurs.

## Autres pistes autonomes

**Nom du poste par une interface réseau USB.** Windows prévoit
[OID_GEN_MACHINE_NAME](https://learn.microsoft.com/en-us/windows-hardware/drivers/network/oid-gen-machine-name)
pour transmettre son nom au pilote réseau. DHCP peut aussi transporter un
[nom de poste](https://www.rfc-editor.org/info/rfc4702/), de façon optionnelle.
Ce seraient de nouvelles fonctions USB, pas des données cachées du clavier HID.
Le passage effectif du nom jusqu’au firmware RNDIS, le support macOS et la lecture
du nom du pair BLE restent à vérifier. Deux postes peuvent avoir le même nom :
cette piste ne fournit donc pas seule une identité fiable. Ne pas créer de réseau
ou modifier les routes de l’ordinateur pour cette recherche documentaire.

**Réactions HID corrélées.** Observer plusieurs changements synchronisés serait
plus instructif que comparer deux états Caps Lock identiques. Toutefois,
[InputStick](https://github.com/inputstick/InputStickAPI-iOS/blob/master/README.md)
rapporte des comportements différents de synchronisation des indicateurs selon
l’OS. Cela reste une hypothèse, sans garantie sur nos machines. Pas de frappes
synthétiques ni de rapprochement durable à partir de ce seul indice.

**Présenter le clavier comme une radio Bluetooth USB.** Le
[mode HCI USB de Zephyr](https://docs.zephyrproject.org/latest/samples/bluetooth/hci_usb/README.html)
expose la radio du microcontrôleur à l’ordinateur. Il ne révèle pas l’identité de
sa radio interne. Il changerait profondément la gestion de notre radio et du
split, sans résoudre directement l’identité commune : ce n’est pas un raccourci.

## Limites des pistes autonomes

Une reprise de ces pistes exigerait d’abord de vérifier les conditions d’accès
aux protocoles. Si une voie adaptée existe, comparer l’adresse reçue par USB à l’identité BLE
résolue du pair appairé, jamais à une adresse radio temporaire. Il faudra vérifier
que les deux représentent bien la même radio, notamment avec plusieurs adaptateurs.
L’identité d’une radio n’est pas un identifiant universel de la machine.

Les noms Logitech ne sont pas une alternative autonome : [Solaar](https://github.com/pwr-Solaar/Solaar/blob/master/lib/logitech_receiver/hidpp20.py)
écrit lui-même le nom du poste dans le périphérique. Un programme hôte reste un
recours explicite, aucune installation ou modification système n’est faite ici.
Une capture plus large sur le firmware actuel n’a pas de champ commun identifié
à récupérer. Aucun flash, changement HID ou nouvel appairage demandé à ce stade.

## Voie retenue : identifiant fourni par l’application

**Diagnostic Mac initial validé.** L’app crée un identifiant aléatoire par installation
utilisateur et annonce la même valeur par USB vendor HID et BLE chiffré pour le
clavier suivi. Elle entretient ces annonces en arrière-plan pour la priorité USB.
Arrêter dans les détails, quitter ou mettre le Mac en veille suspend les échanges.
Le [protocole d’essai](diagnostic-identite.md)
donne les commandes et les résultats attendus. Le client Windows reste à développer.
Un programme hôte est nécessaire pour fournir cette identité sur chaque OS.
Le clavier ne peut pas créer directement un fichier sur l’ordinateur par HID.
L’application écrirait dans son propre dossier de données, avec les droits de
l’utilisateur, sans commandes simulées au clavier ni droits administrateur.

Le fichier JSON explique son origine et indique que ce n’est ni un mot de passe
ni une clé d’appairage. Il est conservé dans les données de l’utilisateur et
réutilisé après remplacement de l’app. Un fichier corrompu n’est pas régénéré
silencieusement. Un autre ordinateur aura sa propre identité après installation.

Précautions à intégrer au futur protocole :

- Identifiant limité à Splitergoll, sans numéro de série matériel ni diffusion
  dans les annonces Bluetooth ou les journaux ordinaires.
- Échange BLE sur une liaison appairée et chiffrée, messages strictement validés,
  sans chemins de fichiers ni commandes exécutables fournis par le périphérique.
- Aucun rapprochement à partir de l’identifiant seul comme preuve de sécurité :
  une copie du fichier permettrait de revendiquer la même identité.
- Une identité clonée sur deux postes pourrait diriger les frappes vers le mauvais
  ordinateur. Prévoir une régénération et un traitement explicite des conflits.

Le stockage identifie une installation utilisateur, pas nécessairement toute la
machine. Multi-utilisateur, identités copiées et associations durables restent
à régler avant d’autoriser la fusion des destinations. Le clavier reste utilisable
sans app. Aucun changement automatique de destination n’est développé à ce stade.

Le protocole v1 compare des annonces en RAM et les expire après quinze secondes.
Chaque liaison doit répondre au contexte du démarrage courant, renouvelé à la
reconnexion. Les réponses ne contiennent pas les identifiants d’hôte. Deux pairs
déclarant le même UUID créent un conflit. Une copie de fichier reste une usurpation
possible si l’autre installation n’est pas visible : « même ordinateur déclaré »
n’est pas une preuve d’authenticité. Cette limite doit être traitée avant toute
fusion automatique de saisie et sa priorité USB.
