# 02. Planifier les essais selon le matériel

[Documentation](../../README.md) › [Référence technique](../../README.md#référence-technique) › [Suivi](../../README.md#suivi) › 02. Planifier les essais selon le matériel

[← 01. Reprendre le développement](01-reprendre.md) · [03. TODO et idées →](03-todo.md)

V1 et V2 peuvent tester l’USB et le Bluetooth quand ils sont alimentés.
V2 est nécessaire pour les essais de charge, de veille profonde et d’autonomie
sur batterie. Deux ordinateurs et un seul clavier suffisent à un essai de bascule.

| Sujet | Matériel nécessaire | Référence |
| --- | --- | --- |
| Layout, accents et encodeurs | Un clavier et l’ordinateur de l’OS visé | [Layouts par OS](../../README.md#layouts-clavier) |
| USB et jeu Windows | PC Windows, un clavier et ses câbles | [Essais wire](../validation/02-tests-wire.md) |
| Reprise, appairages et latence de bascule | Mac, PC Windows et un clavier en `bluetooth auto` | [Essais connexions](../validation/05-tests-connexions-bluetooth.md), [essais de reprise](../validation/06-tests-reprise-bluetooth.md) |
| Veille, première frappe et autonomie | V2 sur batterie, avec un ordinateur | [Essais V2](../validation/09-tests-batteries-veille.md) |
| Sélecteur V1/V2 dans l’app Mac | Mac et deux claviers exposant les batteries en Bluetooth | [Essai des deux claviers](../validation/10-tests-selection-clavier-macos.md) |
| Priorité USB et app Mac | Mac, V2 et un câble USB gauche. Ajouter un deuxième hôte pour les destinations mixtes | [Protocole de priorité USB](../experimentations/01-priorite-usb.md) |
| Gaming USB et hôte concurrent | PC Windows, un clavier et un Mac pour observer la liaison BLE concurrente | [Gaming USB](../experimentations/02-gaming-usb.md) |
| Détection automatique Linux | Un ordinateur Linux avec accès réel à l’USB et au Bluetooth | [Diagnostics](../../README.md#diagnostics) |
| Retour du dongle | XIAO BLE, deux moitiés et un ordinateur | [Matériel et architecture](../firmware/02-materiel-et-profils.md) |
| Build et flash Windows | PC avec WSL ou Windows selon la commande, un clavier pour le flash réel | [Build local](../firmware/01-build-local.md) |

Avant un test Bluetooth, vérifier le profil flashé et les hôtes réellement
mémorisés. Les appairages et la mémoire d’OS de V1 et V2 sont indépendants.
Les délais courts et les conditions particulières sont précisés dans chaque
protocole. Les tâches ouvertes restent dans le [TODO](03-todo.md).

---

[Documentation](../../README.md) › [Référence technique](../../README.md#référence-technique) › [Suivi](../../README.md#suivi) › 02. Planifier les essais selon le matériel

[← 01. Reprendre le développement](01-reprendre.md) · [03. TODO et idées →](03-todo.md)
