# Choisir le clavier suivi par l’app

Le menu **Clavier suivi** permet de choisir les batteries de V1 ou V2 lorsque
plusieurs Splitergoll sont connectés au Mac. Il ne change pas la destination des
frappes. Le sélecteur est développé et testé automatiquement, l’essai avec les
deux claviers reste à faire. L’app est installée selon l’utilisateur et V2 seul
est bien sélectionné dans le menu. Les essais complémentaires sont reportés
à sa demande pour reprendre le firmware.

## Choisir un clavier

Dans la fenêtre, cliquer sur une ligne du tableau des connexions, ou ouvrir **Clavier suivi**
dans la barre des menus, puis cliquer sur son nom.
Une coche indique le choix dans le menu. Dans le tableau, elle indique la liaison
de saisie sélectionnée, pas le clavier suivi. Le nom du clavier suivi apparaît
au-dessus des batteries et dans l’infobulle,
sans élargir l’indicateur compact des batteries.
Les deux lignes USB et Bluetooth d’un même clavier suivent le même choix. Leur
état est informatif : cliquer sur USB ne force pas une bascule de saisie.

Sans choix enregistré, l’app suit l’unique clavier compatible connecté. Si elle
le suit déjà, l’arrivée d’un second ne change rien. Si plusieurs sont disponibles
au départ, choisir dans le menu. Le choix explicite est conservé après fermeture
ou redémarrage de l’app.

Si le clavier choisi est absent, son nom reste affiché et les niveaux deviennent
des tirets. L’app ne suit pas silencieusement l’autre clavier. Pour en changer,
le sélectionner dans le menu. Au changement, les anciennes mesures sont vidées
avant de lire les nouvelles. Le diagnostic USB/Bluetooth suit le même choix,
avec invalidation de ses anciens résultats. Il continue fenêtre fermée pour
maintenir la priorité USB du clavier suivi. Il ne s’applique pas aux autres claviers.

Le nom est un libellé, l’identifiant CoreBluetooth local au Mac reconnaît le
clavier. Un renommage conserve donc le choix. Deux noms identiques sont distingués
par un suffixe d’identifiant dans le menu.

## Essai V1/V2

Les deux claviers doivent être déjà connectés au Mac en Bluetooth et exposer leurs
services batterie. Le **wire strict de V1 ne convient pas** : préparer son profil
Bluetooth avant cet essai, sans effacer les appairages. L’app ne lance ni scan ni
nouvel appairage. La lecture par USB seul reste un autre sujet.

1. Choisir V2, puis V1. Vérifier le nom et les deux niveaux à chaque fois.
   V1 alimenté en USB peut annoncer 100 %, donc afficher les éclairs.
2. Revenir à V2, quitter puis rouvrir l’app. V2 doit rester choisi.
3. Éteindre V2 : son nom reste sélectionné, avec deux tirets malgré V1 disponible.
   Rallumer V2 : ses niveaux reviennent. Ne pas éteindre le clavier utilisé pour répondre.

Les tests automatisés couvrent sélection, persistance, renommage, ambiguïtés,
menu et rejet des réponses d’anciennes sessions. Ils ne remplacent pas ce contrôle
radio réel. L’anomalie antérieure d’affichage n’avait pas de cause confirmée.
