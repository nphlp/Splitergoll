# Planifier selon le matériel disponible

Au bureau, nous avons le **Mac et V2**. Le suivi détaillé reste dans le
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
| Deux batteries après migration | Lecture revalidée sur batterie : gauche 63 %, droite 67 % | Sous USB, le faux 100 % vient de la mesure de l’alimentation. L’affichage reste à traiter |
| Veille et réveil | Reprise courte et réveil par la gauche confirmés. Compléter première frappe, encodeurs et OS après réveil | La droite seule ne réveille pas la centrale en veille profonde. Essais sur batterie, sans USB |
| Politique de veille et autonomie | Comparer veille profonde et centrale maintenue en inactivité, puis décider du compromis | Demande un firmware d’essai et des mesures sur une durée suffisante |
| Keymap et matériel côté Mac | Compléter touches, accents, couches, encodeurs, bootloader et redémarrage | La validation Windows avec gaming reste indépendante |
| [Application macOS](application-macos.md) | Premier lot : deux batteries dans une fenêtre et la barre des menus, reprise et actualisation | Lecture existante réutilisable, limites USB à afficher. Couche, OS et réglages avancés reportés. Tester consommation et sommeil du Mac |
| Correction manuelle de l’OS mémorisé | Développer et tester correction, sauvegarde, reconnexion et redémarrage sur Mac | Choisir d’abord le raccourci et préparer la mémoire d’OS de V2. Une vérification Windows restera nécessaire |

Le profil courant retenu pour V2 est `bluetooth macos` normal : keymap fixe,
batteries et veille, sans mémoire d’OS ni fenêtres expérimentales. Les essais courts
ont utilisé un diagnostic temporaire. Revenir au firmware normal sans nouvel appairage.

## Ce qu’il vaut mieux valider avec Mac et Windows ensemble

Dans ce tableau, « un clavier » signifie **V1 ou V2**. V1 peut fonctionner en BLE
grâce à ses USB d’alimentation. V2 peut fonctionner en USB avec le profil adapté.

| Sujet | Matériel pour terminer | Ce qu’on peut préparer sans Windows |
| --- | --- | --- |
| Fenêtres de reconnexion de 10 secondes | Mac + Windows + un clavier, deux appairages avec OS mémorisés | Code compilé et contrôlé. Attendre les deux hôtes pour tester les deux ordres de retour, la fermeture et la réouverture |
| Sélection automatique et retour au précédent sous 5 secondes | Mac + Windows + un clavier. V2 ajoute l’essai après vraie veille profonde | Logique et tests automatisés, puis essai groupé avec les deux ordres et des frappes pendant la fenêtre |
| Accueil d’un nouvel appareil sans détourner la saisie | Mac + Windows + un clavier | Cadrage et tests des slots libres/pleins et des demandes concurrentes. Intégration après validation des reconnexions connues |
| Réduire le délai de bascule | Mac + Windows + un clavier | Inspecter les gardes et préparer les mesures. Comparer les deux sens avec le même firmware |
| Destinations USB/BLE et priorité USB | Mac + Windows + un clavier | Concevoir l’association des identités. Tester ensuite branchement, retrait, repli et absence de doublon. Deux PC Windows seraient utiles pour éprouver l’identité au-delà de l’OS |
| Gaming USB et fermeture du BLE vers les hôtes | Windows + un clavier, avec Mac pour le scénario BLE simultané | Logique et tests automatisés après le lot destinations. Vérifier ensuite saisie Windows, déconnexion Mac et reprise, sans couper le BLE entre moitiés |

On peut avancer sur le code et ses tests sans les deux ordinateurs. En revanche,
ces transitions restent **non validées matériellement** tant que le scénario complet
n’a pas été joué. Garder une même version de firmware pendant la séance Mac/Windows
évite d’attribuer à l’OS une différence introduite entre deux builds.

Un essai Mac seul des fenêtres est possible mais partiel. Sur V2, il demanderait
d’abord un appairage avec le diagnostic fixe pour enregistrer son OS, puis le passage
à `auto`. La mémoire créée sur V1 ne se transfère pas à V2. Ce détour n’est pas
nécessaire pour les essais batterie et veille.

## Autres sujets du backlog

| Sujet | Matériel ou condition |
| --- | --- |
| Migration KSCAN vers input | Développement possible sur Mac, puis touches et encodeurs sur un clavier. V2 pour le réveil, Windows pour la non-régression finale |
| Outils de build et flash Windows | Windows, avec un clavier pour valider le flash réel |
| Détection Linux | Un hôte Linux avec accès USB/BLE réel, à ajouter à nos essais Mac/Windows |
| Retour du dongle | XIAO BLE + deux moitiés + un ordinateur. Mac suffit pour commencer, V2 pour tester sur batterie |
| Documentation et entretien du build | Mac seul suffit pour commencer, sans manipulation du clavier |

## Ordre conseillé

1. Avec Mac + V2 : la lecture des deux batteries et les reprises principales sont
   revalidées. Compléter première frappe, encodeurs et OS après réveil si nécessaire.
2. À partir des résultats : décider de la politique de veille, puis mesurer l’autonomie.
   L’affichage des batteries ou la correction d’OS sont des chantiers indépendants possibles.
3. Dès que Mac + Windows sont disponibles : valider le lot de reconnexion déjà committé,
   avant d’ajouter l’accueil des nouveaux appareils et les destinations USB/BLE.

La documentation décrit les résultats observés, les essais encore partiels et les
choix restant à faire. Un firmware compilé n’est pas une validation sur le clavier.
