# Tester les connexions Bluetooth

Utiliser **V1** pour les essais, ses deux USB pour l’alimentation et sa gauche au
Mac pour le diagnostic. V2 ou le clavier intégré du Mac sert à échanger ici.
Ouvrir un document texte vide sur chaque ordinateur. Conserver les deux appairages :
aucun « Oublier », `ble forget` ou `settings_reset` pendant cette série.

Le diagnostic ponctuel utilise **10 secondes**, les builds normaux **30 secondes**.
Il est dans `.local/flash/ble-window-10s/`, sans nouveau profil public. Le flasher
par son chemin UF2 explicite, uniquement sur la gauche. La droite reste identique.
La limite de fin d’un appairage commencé reste 30 secondes depuis son début.

## Résultats sur V1

Confirmés avec ce diagnostic : allers-retours Mac/Windows et bonnes keymaps,
frappes retenues pendant la transition, attente du relâchement de Shift,
reconnexion puis sélection de Windows après un seul geste, et Mac arrivé en premier
qui garde la sélection quand Windows rejoint. Le reset simple fonctionne.
Les lectures initiales confirment les deux OS sauvegardés et les deux connexions,
sans erreur. Le retour au build normal de trente secondes reste à confirmer.
Les nouveaux appairages et les cas limites du dernier paragraphe restent séparés.

Faire les cas dans cet ordre. Si un cas échoue, noter le résultat avant de passer
au suivant. Tous les gestes concernent V1, avec relâchement complet de L1 et du clic.

## 1. État initial

1. Activer les Bluetooth Mac et Windows, puis flasher la gauche du diagnostic.
2. Si un hôte manque après dix secondes, L1 + clic encodeur droit rouvre la recherche.
3. Vérifier les deux connexions avec `make ble profiles <port>` et les deux OS
   sauvegardés avec `make ble memory <port>`. `make port` explique comment trouver le port.
4. Revenir au Mac par raccourci si nécessaire. Le clic simple droit doit afficher macOS.

Les lectures USB ne changent ni le slot sélectionné ni la fenêtre. Ne pas supposer
que Mac est zéro et Windows un : conserver les indices réellement lus.

## 2. Bascules et saisie immédiate, dans les deux sens

1. Les deux hôtes connectés, faire L1 + clic droit, tout relâcher puis taper
   immédiatement `azertyuiop` dans le document du nouvel ordinateur.
2. Attendre que le texte apparaisse, puis faire un clic simple droit : Windows attendu.
3. Refaire le même essai vers le Mac : texte complet et macOS attendu.
4. Faire trois allers-retours. Noter lettre perdue, texte envoyé à l’ancien hôte,
   touche restée active ou besoin d’un second geste.

La courte garde de fin des anciens comportements reste normale. Taper ne doit
plus repousser indéfiniment la bascule. La phrase exerce les deux moitiés.

## 3. Touche maintenue pendant le geste

1. Depuis le Mac, maintenir Shift, faire le raccourci puis relâcher L1 et le clic,
   en gardant Shift : le clavier doit rester sur le Mac.
2. Relâcher Shift, puis taper `azertyuiop` : le texte doit arriver sur Windows.
3. Tester une lettre sans Shift : aucun modificateur ne doit rester bloqué sur les deux hôtes.

## 4. Fenêtre fermée puis recherche d’un connu

1. Revenir au Mac avec les deux hôtes connectés. Couper le Bluetooth Windows.
2. Attendre **12 secondes depuis le dernier raccourci**, puis réactiver Windows.
3. Attendre quelques secondes : Windows ne doit pas se reconnecter tant que la fenêtre reste fermée.
4. Faire une seule fois le raccourci. Windows doit se reconnecter puis recevoir
   la saisie, sans second geste. Vérifier Windows au clic simple, après la bascule.
5. Refaire dans l’autre sens : sélectionner Windows, couper le Bluetooth Mac,
   attendre 12 secondes, réactiver le Mac, puis rouvrir par un seul geste.

Les douze secondes servent seulement à assurer la fermeture du diagnostic de dix
secondes. Si `WINDOW` indique déjà `remaining_ms=0 advertising=0`, inutile de les
attendre. Pour le build normal, attendre plus de trente secondes depuis l’ouverture.
La fenêtre commence au démarrage ou au raccourci, pas à la dernière frappe.

Quand le Bluetooth Mac est coupé, utiliser son clavier et son trackpad intégrés.
Le passage sur le nouvel hôte attend sa connexion sécurisée et son OS mémorisé.

## 5. Première arrivée avec zéro connexion

1. Couper les Bluetooth des deux ordinateurs. Garder leurs réglages ouverts.
2. Faire le raccourci sur V1, puis réactiver le Mac dans les dix secondes.
3. Le Mac doit devenir utilisable sans second geste. Réactiver ensuite Windows
   pendant la même fenêtre si possible : son arrivée ne doit pas reprendre la sélection.
4. Avec les deux connectés, un nouveau geste doit permettre de sélectionner Windows.

Si la manipulation dépasse dix secondes, rouvrir la fenêtre avant de réessayer.

## 6. Redémarrage et mémoire

1. Les deux Bluetooth actifs, faire un seul appui bref sur reset de gauche.
2. Vérifier les reconnexions pendant la fenêtre. Rouvrir par raccourci si nécessaire.
3. Tester les deux destinations : bon OS, deux moitiés actives, aucun nouvel appairage.
4. Vérifier `stored=macos/windows`, `state=saved` et l’absence d’erreur dans la mémoire.

Ne pas exiger ici le retour prioritaire au précédent hôte pendant la fenêtre de
démarrage : cette politique reste à développer. Le slot préféré ZMK a sa propre
sauvegarde différée.

## 7. Nouvel appairage et cas limites, séparément

L’accueil d’un nouveau bond reste à tester avec un appareil supplémentaire, ou
après accord explicite pour oublier un seul appairage existant. Ne rien effacer
pour les six premiers cas. Lors de ce test : avec un seul hôte prêt, ouvrir la
recherche, appairer l’autre et vérifier sélection automatique, bon OS sauvegardé,
puis conservation des deux associations après redémarrage.

Fin d’appairage après fermeture, slots pleins, dépassement du tampon de saisie
et panne de sauvegarde sont contrôlés en logiciel. Ils ne sont pas déduits des
essais précédents et ne demandent aucune manipulation destructive improvisée.
Gaming USB et destinations mixtes ne font pas encore partie de ce firmware.

Après validation, remettre le build normal à 30 secondes. Noter pour chaque cas :
OK/KO, destination réelle, première lettre reçue et délai approximatif ressenti.
