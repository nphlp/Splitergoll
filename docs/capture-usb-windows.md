# Capturer l’USB normal sur Windows

Cette procédure observe le firmware normal sans lui ajouter d’interface USB.
Elle a permis de corriger la détection Windows de `wire auto`, qui fonctionnait
uniquement avec `usb-trace` sur le PC. La saisie et la détection Windows puis macOS
sont confirmées sur V1 sans port série, mais un délai irrégulier au branchement
sur Mac reste étudié dans [le guide auto](keymap-auto.md). L’absence de port COM est normale
sans l’interface série de diagnostic.

Le même [capture.ps1](../diagnostics/os-signature/capture.ps1) propose `-UsbEvents`.
Il utilise Logman et Tracerpt, intégrés à Windows, pour enregistrer puis décoder une
trace ETW. Aucun flash ni pilote à installer. Ce mode nécessite PowerShell en
administrateur pour accéder aux événements USB système, contrairement au lecteur COM.
Voir [la procédure Microsoft](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/how-to-capture-a-usb-event-trace).

## Procédure

1. Garder `wire auto` normal sur les deux moitiés. Débrancher seulement l’USB gauche
   du PC avant de lancer la capture. La droite peut rester alimentée.
2. Copier le script mis à jour dans le dossier de son choix. Ouvrir PowerShell en
   administrateur et se placer dans ce dossier.
3. Lancer :

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\capture.ps1 -UsbEvents
```

4. Au message **CAPTURE ACTIVE**, brancher la gauche au PC Windows. Ne rien taper
   pendant les vingt secondes. Le script arrête ensuite sa session et décode la trace.
5. Transmettre les fichiers `windows-usb-....xml` et `windows-usb-...-summary.txt`,
   créés directement dans le dossier courant. Garder le `.etl` brut pour une éventuelle analyse.

Aucun numéro COM n’est nécessaire : le firmware normal n’expose pas ce port.
Le mode `-Port COMx` reste destiné au firmware `usb-trace`.

## Ce que cette capture permet

Les événements USB et l’énumération peuvent montrer les requêtes réellement faites
par Windows et les erreurs éventuelles. Le filtre `HeadersBusTrace` collecte les
transferts sans leurs paquets de données. Les événements décrivent aussi d’autres
périphériques USB du PC, ces fichiers restent privés et sont ignorés par Git.
Le résumé permet de contrôler les pertes avant de tirer une conclusion.

Si Tracerpt signale des événements hors schéma, cela concerne le décodage du fichier,
pas le flash. Le script utilise désormais `-lr`, le décodage au mieux prévu par
[Microsoft](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/tracerpt).
Conserver le brut ETL même si les en-têtes nécessaires sont lisibles dans le XML.

La capture côté Windows ne donne pas directement l’état interne du classifieur.
On ne remplace pas un résultat inconnu par Windows par défaut. La comparaison avec
les signatures précédentes doit précéder toute modification de règle.

La capture reçue contient 2 075 événements sans perte déclarée. Les 22 demandes
SETUP du clavier montrent des noms lus sur 255 puis 262 octets, sans le sondage sur
4 exigé auparavant. Leur rejeu reproduit « inconnu » avant correction, puis Windows
après ajout de ce motif. L’utilisateur confirme ensuite la saisie et la détection
Windows puis macOS avec le même firmware normal, sans reflash entre les hôtes.

Le nom de session est unique et l’arrêt est dans `finally`, y compris après erreur.
Si le processus PowerShell est fermé brutalement, utiliser la commande d’arrêt
exacte affichée au début. Elle cible uniquement cette session de diagnostic.
L’option `-ets` ne crée pas de collecte planifiée persistante, voir
[Logman](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/logman-start-stop).
