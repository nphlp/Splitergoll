# USB prioritaire sur le même ordinateur

**Mode expérimental développé, essai physique à faire sur Mac + V2.**
La saisie passe sur USB lorsque l’app confirme que le câble et le profil Bluetooth
sélectionné correspondent au même ordinateur. Retirer le câble rétablit Bluetooth.
La keymap reste celle de l’appareil sélectionné, sans changement des touches.

## Préparer

```sh
make build bluetooth auto usb-priority
make app build
make app install
```

Passer uniquement la gauche en bootloader, puis :

```sh
make flash bluetooth auto left usb-priority
```

La droite reste identique au firmware Bluetooth normal. Aucun reset des réglages
ni nouvel appairage nécessaire. Inactivité : deux minutes, veille profonde : dix.

## Ce que montre l’app

Une fois les deux annonces reconnues, pour le même Mac :

| Clavier | Liaison | État |
| --- | --- | --- |
| Splitergoll V2 | USB | Sélectionné |
| Splitergoll V2 | Bluetooth | Désactivé |

« Désactivé » concerne **la saisie sur cette liaison**. La connexion Bluetooth reste
présente pour l’identité, les batteries et le choix d’un autre ordinateur. Le BLE
entre les deux moitiés reste également actif.

Le simple branchement ne suffit pas : le canal USB doit être énuméré et les deux
annonces fraîches doivent correspondre au profil sélectionné. Un chargeur ne crée
pas de sortie USB. Sélectionner un autre ordinateur garde sa saisie en Bluetooth,
même si le câble reste branché au Mac.

## Rôle de l’app

L’app renouvelle l’identité toutes les cinq secondes, même fenêtre fermée. Elle
reprend après le sommeil du Mac et respecte **Arrêter** dans les détails du diagnostic.
Si elle est quittée, si l’annonce expire ou si le rapprochement échoue, la sortie
revient en Bluetooth. Le clavier reste donc utilisable sans app, mais la priorité
USB de ce mode en dépend. L’expiration intervient après quinze secondes, puis
attend les relâchements nécessaires. Un lien remplacé est révoqué immédiatement.

Une transition normale attend la fin des touches et boutons maintenus, des macros
et des rapports HID actifs. Elle ne coupe pas un Shift au milieu d’une frappe.
Les changements de sortie n’ouvrent pas une fenêtre d’appairage supplémentaire.
Un retrait brutal du câble peut interrompre l’appui en cours : relâcher puis retaper.

## Premier essai

1. Garder les batteries sur ON, la gauche reliée au Mac en USB et V2 connecté en BLE.
   Lancer l’app et suivre V2. Attendre **USB : Sélectionné**, puis taper des deux côtés.
2. Débrancher uniquement l’USB gauche : Bluetooth doit reprendre et permettre de taper.
3. Rebrancher : USB doit être sélectionné après le prochain échange, sans nouveau flash.
4. Fermer seulement la fenêtre, attendre vingt secondes et la rouvrir : USB doit rester
   sélectionné. Quitter l’app est un test distinct qui doit provoquer le repli Bluetooth.

Ces essais restent à réaliser. Les tests logiciels ne prouvent ni l’énumération
réelle des deux interfaces USB, ni le comportement de tous les hubs ou ordinateurs.

Le client d’identité Windows reste à développer. Le rapprochement est déclaratif,
pas une authentification : ne pas copier le fichier d’identité sur un autre poste.
Une duplication visible est refusée. Les installations multi-utilisateurs et les
identifiants copiés hors présence simultanée restent à traiter avant généralisation.
Le Gaming USB avec arrêt des connexions et sa mémorisation reste un lot séparé.

Pour revenir au firmware Bluetooth normal, flasher seulement la gauche avec
`make flash bluetooth auto left`. Les appairages sont conservés.
