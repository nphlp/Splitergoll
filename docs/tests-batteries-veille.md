# Tester batteries et veille sur V2

Diagnostic ponctuel sur le Mac, avec la keymap macOS fixe. Les fichiers temporaires
restent dans `.local/flash/`, les sources et logs dans `.local/diagnostic/power-test/`.
Les commandes de build habituelles et leurs délais restent inchangés.
Pendant les attentes, utiliser le clavier intégré du Mac pour échanger : taper
sur V2 relance les délais. Attendre la confirmation d’ouverture des consoles
avant de commencer les gestes destinés aux logs.

| Réglage | Diagnostic | Bluetooth normal |
| --- | --- | --- |
| Inactivité | 10 secondes | 2 minutes |
| Veille profonde | 60 secondes | 10 minutes |
| Mesure batterie pendant l’activité | 5 secondes | 60 secondes |
| Logs USB | Oui, chaque moitié | Non |

Flasher la droite puis la gauche avec les deux UF2 du diagnostic, sans firmware
`settings_reset`. Les appairages sont conservés. Revenir aux firmwares normaux
après les essais, les logs et les mesures plus fréquentes modifiant la consommation.

## 1. Batteries et logs, avec USB

1. Garder les deux USB branchés et les deux interrupteurs sur ON pour charger.
2. Repérer les deux ports série et lire les consoles après le signal de disponibilité.
3. Après confirmation d’ouverture, taper A puis P toutes les quatre secondes
   pendant vingt secondes pour garder les deux moitiés actives.
4. Relever tension en millivolts, pourcentage et erreurs éventuelles dans les logs.
   Sur V2, l’USB donne environ 5 V et 100 %, sans représenter l’état des cellules.
5. Après fermeture des consoles, débrancher les deux USB en gardant ON. Appuyer sur
   une touche de chaque côté, attendre cinq secondes avec le diagnostic, puis lancer
   `make bat read`. Garder les USB débranchés pour les deux essais suivants.

Le pilote journalise déjà la lecture ADC, la tension calculée et le pourcentage.
Il ne mesure pas le courant de charge. Les mesures s’arrêtent en inactivité : un
pourcentage inchangé après dix secondes sans frappe peut être une valeur conservée.
Comparer les niveaux de charge avec des relevés sur batterie seule, USB débranchés,
avant et après la charge. `make bat avg` peut relire plusieurs fois la même valeur.

## 2. Inactivité courte, sur batterie

1. Garder les interrupteurs ON, les deux USB débranchés et les consoles fermées.
2. Taper une lettre de chaque côté, puis attendre 15 secondes sans utiliser le clavier.
3. Taper une seule fois A à gauche et P à droite. Noter si chaque première lettre
   arrive immédiatement. Les 15 secondes dépassent l’inactivité de 10 secondes
   du diagnostic et restent sous sa veille profonde de 60 secondes.

Sur batterie, l’utilisateur rapporte une reprise immédiate à droite, sans latence
perçue. Le délai exact de cette attente n’est pas chronométré dans son retour.
Un premier essai après 15 secondes sous USB était également immédiat.
Le délai normal retenu pour les prochains builds est deux minutes d’inactivité.

## 3. Veille profonde, sur batterie

1. Fermer les lecteurs USB. Garder les interrupteurs sur ON et débrancher les deux USB.
2. Taper une lettre de chaque côté, puis attendre 70 secondes sans utiliser le clavier.
3. Appuyer une fois sur A à gauche, attendre cinq secondes, puis appuyer une seconde
   fois sur A. Noter ce qu’a produit chaque appui. Faire ensuite la même séquence
   avec P à droite pour observer son réveil local, la gauche étant déjà réveillée.
4. Retaper de chaque côté, attendre à nouveau 70 secondes, puis faire P, attendre
   cinq secondes et refaire P, sans toucher la gauche. Faire ensuite A, attendre
   cinq secondes et refaire A, puis tester P à nouveau. Noter chaque appui séparément.

Les USB étant débranchés, il n’y a pas de capture USB de la veille profonde.
Les logs RAM ne constituent pas un historique persistant après le réveil.
Le délai d’activité est local à chaque moitié. Le premier appui peut servir au réveil,
et la droite seule ne peut pas réveiller une centrale dont la radio est coupée.
Sur batterie après migration, l’utilisateur attend environ une minute puis fait
plusieurs appuis à droite sans résultat. Après des appuis à gauche, les deux côtés
refonctionnent en 2 à 5 secondes selon son estimation. Le retour confirme le réveil
local de la centrale et la limite du réveil depuis la droite seule. La première
frappe, les encodeurs et toutes les sources de réveil ne sont pas validés séparément.

Après les essais, reflasher `bluetooth macos` normal sur les deux moitiés.
Voir [veille et réveil](veille.md) et [les mesures de batterie](batteries.md).
