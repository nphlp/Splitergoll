# 03. Veille et réveil

[Documentation](../README.md) › [Utiliser le clavier](../README.md#utiliser-le-clavier) › 03. Veille et réveil

[← 02. Batteries et recharge](02-batteries.md) · [04. Application macOS →](04-application-macos.md)

Avec le firmware `bluetooth` normal, le clavier entre en **inactivité** après
deux minutes. Sur batterie, la **veille profonde** est prévue après dix minutes
depuis la dernière activité. L’USB laisse l’inactivité fonctionner et maintient la
gauche hors de la veille profonde tant qu’elle est alimentée. Le profil `wire`
désactive la veille profonde.

- Après une pause, appuyer sur une touche pour reprendre la saisie.
- Si le clavier est en veille profonde, le réveiller avec une **touche de gauche**
  et laisser la connexion revenir. La droite seule ne réveille pas la gauche.
- Si les batteries restent figées après une pause, réveiller le clavier avant
  de relire les niveaux. Le firmware suspend ses mesures en inactivité.

Sur V2, les essais courts ont observé la reprise immédiate après inactivité et
une reprise en deux à cinq secondes après sollicitation de la gauche en veille
profonde. Les essais ont utilisé des délais raccourcis.

[États d’alimentation et délais](../technique/firmware/11-etats-alimentation.md)
· [Protocole et résultats sur V2](../technique/validation/09-tests-batteries-veille.md)

---

[Documentation](../README.md) › [Utiliser le clavier](../README.md#utiliser-le-clavier) › 03. Veille et réveil

[← 02. Batteries et recharge](02-batteries.md) · [04. Application macOS →](04-application-macos.md)
