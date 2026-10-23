# Planifier selon le matériel disponible

Le matériel disponible est **Mac + Windows + V1 + V2 à domicile**.
V1 est revenu en `wire windows` normal pour jouer. Les prochains essais de
connexion seront préparés à la reprise, V2 peut rester le clavier de saisie.
Le suivi détaillé reste dans le
[TODO](TODO.md). Ce tableau sert à choisir un chantier que l’on peut réellement
terminer, sans confondre développement et validation matérielle.

## Quel clavier faut-il ?

V1 et V2 peuvent servir aux essais USB ou Bluetooth avec une alimentation USB.
V2 est nécessaire pour tester les batteries, l’autonomie et la veille profonde
sans alimentation externe. Pour relier Mac et Windows, **un seul clavier suffit** :
il n’est pas nécessaire d’avoir V1 et V2 ensemble.

## Ce qu’on peut terminer avec Mac et V2

| Sujet | Essais possibles maintenant | Limite ou préparation |
| --- | --- | --- |
| Deux batteries après migration | Lecture et affichage revalidés sur V2 | Éclairs sous USB et tirets à la déconnexion confirmés. La capacité réelle sous USB reste inaccessible |
| Veille et réveil | Reprise courte et réveil par la gauche confirmés. Compléter première frappe, encodeurs et OS après réveil | La droite seule ne réveille pas la centrale en veille profonde. Essais sur batterie, sans USB |
| Politique de veille et autonomie | Comparer veille profonde et centrale maintenue en inactivité, puis décider du compromis | Demande un firmware d’essai et des mesures sur une durée suffisante |
| Keymap et matériel côté Mac | Compléter touches, accents, couches, encodeurs, bootloader et redémarrage | La validation Windows avec gaming reste indépendante |
| [Application macOS](application-macos.md) | Affichage des niveaux, tirets à la déconnexion et éclairs sous USB confirmés sur V2 | Compléter reprise, sommeil du Mac et ouverture de session. Couche, OS et réglages avancés reportés |
| Correction manuelle de l’OS mémorisé | Développer et tester correction, sauvegarde, reconnexion et redémarrage sur Mac | Choisir d’abord le raccourci et préparer la mémoire d’OS de V2. Une vérification Windows restera nécessaire |

Le profil courant retenu pour V2 est `bluetooth macos` normal : keymap fixe,
batteries et veille, sans mémoire d’OS ni fenêtres expérimentales. Les essais courts
ont utilisé un diagnostic temporaire. Revenir au firmware normal sans nouvel appairage.

## Ce qu’il vaut mieux valider avec Mac et Windows ensemble

Dans ce tableau, « un clavier » signifie **V1 ou V2**. V1 peut fonctionner en BLE
grâce à ses USB d’alimentation. V2 peut fonctionner en USB avec le profil adapté.

| Sujet | Matériel pour terminer | Ce qu’on peut préparer sans Windows |
| --- | --- | --- |
| Fenêtres de reconnexion de deux minutes | Mac + Windows + un clavier, deux appairages avec OS mémorisés | Comportements principaux validés sur V1 au délai de dix secondes. Vérifier les 120 secondes face à IDLE, puis les nouveaux bonds |
| Sélection automatique et retour au précédent pendant la fenêtre de démarrage | Mac + Windows + un clavier. V2 ajoute l’essai après vraie veille profonde | Sauvegarde immédiate, persistance et retour Windows après Mac confirmés. Vraie veille profonde et délai normal restent distincts |
| Accueil d’un nouvel appareil sans détourner la saisie | Mac + Windows + un clavier | Cadrage et tests des slots libres/pleins et des demandes concurrentes. Intégré et testé en logiciel. Valider ensemble appairage initial et reconnexion |
| Réduire le délai de bascule | Mac + Windows + un clavier | Inspecter les gardes et préparer les mesures. Comparer les deux sens avec le même firmware |
| Destinations USB/BLE et priorité USB | Mac + Windows + un clavier | Concevoir l’association des identités. Tester ensuite branchement, retrait, repli et absence de doublon. Deux PC Windows seraient utiles pour éprouver l’identité au-delà de l’OS |
| Gaming USB et fermeture du BLE vers les hôtes | Windows + un clavier, avec Mac pour le scénario BLE simultané | Logique et tests automatisés après le lot destinations. Vérifier ensuite saisie Windows, déconnexion Mac et reprise, sans couper le BLE entre moitiés |

On peut avancer sur le code et ses tests sans les deux ordinateurs. En revanche,
ces transitions restent **non validées matériellement** tant que le scénario complet
n’a pas été joué. Garder une même version de firmware pendant la séance Mac/Windows
évite d’attribuer à l’OS une différence introduite entre deux builds.

Un essai Mac seul des fenêtres est possible mais partiel. Sur V2, vérifier d’abord
l’OS mémorisé : la mémoire créée sur V1 ne s’y transfère pas. S’il manque, préparer
avec accord un nouvel appairage hôte. Le nouvel `auto` peut alors apprendre l’OS
directement. Les essais batterie et veille n’en ont pas besoin.

## Autres sujets du backlog

| Sujet | Matériel ou condition |
| --- | --- |
| Migration KSCAN vers input | Développement possible sur Mac, puis touches et encodeurs sur un clavier. V2 pour le réveil, Windows pour la non-régression finale |
| Outils de build et flash Windows | Windows, avec un clavier pour valider le flash réel |
| Détection Linux | Un hôte Linux avec accès USB/BLE réel, à ajouter à nos essais Mac/Windows |
| Retour du dongle | XIAO BLE + deux moitiés + un ordinateur. Mac suffit pour commencer, V2 pour tester sur batterie |
| Documentation et entretien du build | Mac seul suffit pour commencer, sans manipulation du clavier |

## Ordre conseillé

1. Lire [le point de reprise](reprise.md), vérifier le matériel disponible, puis
   tester le délai normal de deux minutes face à IDLE et à la sauvegarde. Compléter
   les nouveaux appairages séparément, sans refaire les bascules déjà validées.
2. Concevoir l’identité commune des destinations USB/BLE, puis développer Gaming USB
   avec mémoire du mode. Clarifier les cas de retrait USB et de sortie volontaire
   avant le code, avec les deux ordinateurs pour les transitions.
3. Sur V2, compléter première frappe, encodeurs et autonomie sur batterie.
4. Compléter la reprise de l’app après sommeil du Mac et l’ouverture de session.

La documentation décrit les résultats observés, les essais encore partiels et les
choix restant à faire. Un firmware compilé n’est pas une validation sur le clavier.
