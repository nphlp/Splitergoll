# Journal d’activité sur batterie

Le diagnostic dédié conserve jusqu’à **128 événements en RAM** : activité,
présence USB, fenêtre de recherche et résultat d’écriture de préférence.
Il ne conserve aucune frappe, adresse, clé ou identité d’ordinateur.
Les pertes sont signalées et font refuser une capture, sans écrasement silencieux.

## Lire le journal

Le firmware doit avoir été préparé avec le module `diagnostics/power/firmware`
et `CONFIG_SPLITERGOLL_POWER_TRACE=y`. Ce module reste absent des builds usuels,
même de `ble-trace`. Ce n’est pas un nouveau profil matériel.

Avec la gauche en USB et son port explicitement identifié :

```sh
make ble power /dev/cu.usbmodem123
```

Cette commande envoie seulement `P`, lit le journal et le conserve dans
`.local/power-traces/`. Elle ne sélectionne aucun ordinateur et n’efface rien.
Deux lectures gardent les mêmes événements, avec éventuellement de nouveaux à la fin.

## Essai sur batterie

1. Lire un premier journal et noter son identifiant `boot`.
2. Garder les batteries ON, débrancher les deux USB et effectuer le scénario convenu.
3. Rebrancher la gauche sans reset, avant le sommeil profond, puis relire.
4. Vérifier le même `boot`, l’absence de pertes et l’ordre des événements.

Un identifiant différent invalide la continuité : le contrôleur a redémarré.
Le sommeil profond et les coupures effacent ce journal RAM. On ne l’utilise pas
pour mesurer une consommation ou une latence. ON/OFF batterie reste indiqué par
l’utilisateur, aucun GPIO ne permet de le lire.

`ACTIVITY` vaut actif, inactif ou sommeil. `WINDOW` porte les millisecondes
restantes et l’état des annonces. `SAVE` porte le slot et le résultat de
l’écriture existante : zéro signifie succès. Un appel dédupliqué sans écriture
ne crée pas de faux événement de sauvegarde.
L’état USB « énuméré » peut être celui du diagnostic CDC, sans saisie USB active.

Le diagnostic ponctuel utilise recherche et IDLE à dix secondes, SLEEP à dix
minutes pour permettre la lecture après l’essai. Les délais publics restent inchangés.
L’essai court sur V2 est positif : même démarrage et aucune perte après retrait
puis retour USB. Sur batterie, la fenêtre se ferme puis la gauche passe en IDLE.
Le retour USB ne la réveille pas. Aucune nouvelle écriture n’était nécessaire
pour la préférence Mac inchangée. La droite et SLEEP ne sont pas observés par ce journal.
