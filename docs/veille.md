# Veille et réveil

## Délais

| État | `bluetooth` | Comportement prévu |
| --- | --- | --- |
| Inactivité | 30 secondes | Connexion BLE conservée |
| Veille profonde | 15 minutes | Réveil local par une touche, puis reconnexion |

Le profil `wire` désactive la veille profonde. Le profil `bluetooth` l’active et
la matrice est déclarée comme source de réveil.
[Fonctionnement ZMK](https://zmk.dev/docs/features/low-power-states).

## Résultats sur le clavier Mac

Avec des délais temporairement raccourcis à 10 secondes et une minute, sur batterie
et sans USB. Cette configuration d’essai est désormais retirée :

- Après 10 secondes d’inactivité, la saisie reprend immédiatement.
- Après une minute, chaque moitié se réveille avec une touche, sans reset.
  Le premier appui sert au réveil et n’écrit rien. Un second appui permet la saisie
  une fois la liaison rétablie, en environ 3 secondes lors de l’essai.
- La droite réveillée seule ne peut rien transmettre tant que la gauche dort.
  Réveiller la gauche permet à la droite de se reconnecter et de fonctionner.
- Le choix macOS revient à Windows après la veille profonde. Le réveil redémarre
  le firmware, dont l’état des couches est en mémoire. La sélection d’OS n’est pas
  sauvegardée et Windows est la base de démarrage actuelle.

Les essais portent sur des touches représentatives, pas sur toutes les touches
et les encodeurs. La conservation de l’OS et une reprise depuis n’importe quelle
moitié restent des améliorations à décider.

## Essai au chrono

Utiliser le profil `bluetooth` normal. Ne pas reflasher le reset pour cet essai.

1. Mettre les deux interrupteurs batterie sur marche, débrancher les deux USB.
2. Taper sur les deux moitiés pour démarrer leurs délais, puis lancer le chrono après la dernière frappe.
3. Après plus de 30 secondes, tester la reprise après inactivité. Cette frappe relance le délai.
4. Repartir de la dernière frappe et attendre plus de 15 minutes sans utiliser les moitiés.
5. Faire un appui bref sur une lettre de gauche, attendre la reconnexion, puis tester la droite.
6. Noter si le réveil fonctionne sans reset et si la première lettre arrive ou sert seulement au réveil.

ZMK bloque la veille profonde dès que l’USB fournit du courant, y compris depuis
un chargeur. Le délai est compté depuis la dernière activité, pas depuis le débranchement.
Son contrôle périodique et la reconnexion ajoutent quelques secondes au délai affiché.

La veille profonde coupe le BLE sur chaque moitié. Une touche de droite ne peut
donc pas réveiller à distance une gauche éteinte. Si l’on souhaite reprendre la
saisie depuis n’importe quelle moitié, garder la centrale en inactivité plutôt
qu’en veille profonde est une option à décider après les essais.

## Revenir aux délais normaux

Reflasher les deux UF2 du profil `bluetooth`, disponibles après `make build bluetooth`,
sans firmware de reset. Cela rétablit l’inactivité à 30 secondes, la veille à
15 minutes et la mesure batterie à 60 secondes. Les appairages sont conservés.
Le retour à ces deux firmwares est confirmé sur le clavier Mac. Les USB restent
branchés pour charger, ce qui bloque la veille profonde pendant la charge.
La première frappe de réveil et le retour à Windows restent les mêmes après
une veille profonde, seul son délai change.

Pour flasher ou redémarrer le contrôleur : [flash du firmware](flash-firmware.md).
