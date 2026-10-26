# Tester l’identité USB/Bluetooth sur Mac

**Premier essai positif sur Mac + V2.** L’utilisateur confirme le résultat
« Même ordinateur déclaré ». Le retrait/rebranchement USB et la relance du
diagnostic restent à tester. Avec V2, l’app annonce un même
identifiant local par USB et Bluetooth. Le clavier compare les deux déclarations.
La saisie reste en Bluetooth, sans nouvelle priorité USB ni fusion de destinations.
Ce guide concerne le profil `identity`. La vraie bascule se teste séparément avec
[usb-priority](priorite-usb.md).

## Préparer

```sh
make build bluetooth auto identity
make app build
```

La nouvelle app s’installe avec la commande habituelle. Elle ferme et remplace
proprement l’app installée, puis la relance :

```sh
make app install
```

Passer uniquement la gauche en bootloader, puis :

```sh
make flash bluetooth auto left identity
```

La droite est identique au firmware normal et ne nécessite pas de flash.
Ne pas flasher de reset : les appairages et leurs OS sont conservés.
L’inactivité reste à deux minutes, la veille profonde à dix minutes.

## Vérifier sur V2

1. Garder les batteries sur ON, la gauche en USB au Mac et les deux moitiés
   utilisables en Bluetooth. Choisir V2 comme clavier suivi dans l’app.
2. Dans le menu de l’app, choisir **Ouvrir l’app**. Le diagnostic est intégré à
   la fenêtre principale, avec les batteries et le sélecteur.
3. Attendre l’annonce sur les deux liaisons. Résultat attendu :
   **Même ordinateur déclaré**. La frappe doit rester habituelle.
4. Débrancher seulement l’USB gauche, sans éteindre sa batterie. Sous environ
   cinq secondes, l’USB doit être absent et la correspondance non établie.
5. Rebrancher cet USB au même Mac : la correspondance doit revenir.
6. Fermer puis rouvrir la fenêtre de l’app : elle doit retrouver le même résultat.

Le nouveau firmware expose aussi l’**OS actif** et la liaison de **Saisie** en
lecture seule. Avec ce profil, la saisie reste en Bluetooth, même avec le câble.
L’ancien diagnostic reste compatible mais indique **Non communiqué** pour ces
deux champs. Après mise à jour, une reconnexion Bluetooth ordinaire peut être
nécessaire pour découvrir cette nouvelle caractéristique.

Déplier **Diagnostic** pour arrêter/relancer les échanges ou **Copier le diagnostic**.
La copie fournit les états à partager, sans identifiant ou frappe.
Si le service est absent après un flash, tenter d’abord une reconnexion Bluetooth
ordinaire et rouvrir le diagnostic. Ne pas effacer les appairages pour ce seul message.

## Ce qui est conservé

Le compagnon d’identité crée au premier usage du clavier suivi
`~/Library/Application Support/Splitergoll/host-identity.json`, privé à l’utilisateur.
Le fichier explique son usage et survit au remplacement de l’app. Ce n’est ni
un mot de passe, ni une identité matérielle, ni une clé d’appairage.

Les annonces restent en RAM dans le clavier et expirent après quinze secondes
sans renouvellement. La nouvelle app les entretient en arrière-plan pour permettre
la priorité USB, même fenêtre fermée. **Arrêter**, quitter l’app ou le sommeil du
Mac suspendent les échanges. Avec le profil `identity`, aucun changement de bonds,
de keymap ou de destination n’est effectué.

Ce test compare une identité **déclarée**. Un fichier copié peut imiter une
installation, ce n’est donc pas une preuve de sécurité. Deux pairs qui déclarent
le même identifiant simultanément donnent un conflit. Aucun rapprochement automatique
de saisie n’est autorisé par ce premier lot.

Pour revenir au firmware normal, remettre seulement la gauche en bootloader :

```sh
make flash bluetooth auto left
```

## Plus tard sur Windows

Tester d’abord le portable à largeur réduite préparé à la maison. Le client
d’identité Windows reste à développer : l’exécutable actuel lit les batteries.
L’essai avec deux ordinateurs, dont deux du même OS, reste distinct de ce test Mac.
