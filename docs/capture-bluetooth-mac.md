# Capturer le Bluetooth depuis le Mac

La gauche expose ses traces sur USB avec le firmware `ble-trace`. Les USB peuvent
rester branchés au Mac pendant que le clavier communique en Bluetooth avec Mac
ou Windows. Aucun script PowerShell ni transfert de fichiers n’est nécessaire.
La keymap du diagnostic reste fixe, même si l’ordinateur Bluetooth change.

## Observer Windows

Les deux appareils sont déjà appairés : Mac en emplacement 0, Windows en 1.
Ces index correspondent à nos essais, ils ne sont pas imposés par le firmware.

1. Garder les USB branchés au Mac, sans reset ni mode bootloader.
2. Couper temporairement le Bluetooth du Mac et de Windows pour libérer la liaison.
3. Depuis le terminal du Mac, sélectionner l’emplacement Windows :

   ```sh
   make ble-profile /dev/cu.usbmodem1101 1
   ```

4. Réactiver le Bluetooth de Windows et reconnecter son entrée `Splitergoll`.
5. Attendre dix secondes, puis lire depuis le Mac :

   ```sh
   make ble-capture /dev/cu.usbmodem1101
   ```

Adapter le port à la gauche présente. Les dix secondes sont une marge de capture,
pas un délai imposé au clavier. Les fichiers restent dans `.local/ble-traces/`.
Le préfixe `macos` et le champ `host=macos` désignent le **collecteur USB**,
même lorsque l’ordinateur Bluetooth observé est Windows. Noter cet hôte séparément.

## Revenir au Mac

1. Couper le Bluetooth de Windows, garder celui du Mac coupé.
2. Exécuter `make ble-profile /dev/cu.usbmodem1101 0`.
3. Réactiver le Bluetooth du Mac. La saisie doit reprendre sans nouvel appairage.

Pour observer ce retour, attendre dix secondes puis relancer `make ble-capture`.
Avant une capture réalisée par l’agent, annoncer que le clavier est prêt.

## Connexion ou nouvel appairage ?

Ces étapes testent une **reconnexion**. Un simple reset ou un flash ordinaire
conserve les appairages. Reproduire un premier appairage demande un protocole
d’effacement ciblé du lien testé, avec accord préalable. Ne pas utiliser de reset
global pour cette mesure. L’[alternative PowerShell](capture-bluetooth-windows.md)
reste disponible si les USB sont branchés au PC Windows.
