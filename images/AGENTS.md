# Photos publiques

Les six photos WebP de ce dossier illustrent le README. Leurs noms décrivent le
contenu, en minuscules et kebab-case, sans numéro de prise de vue ni date.
Mettre à jour leurs références dans le même changement qu’un renommage.

## Métadonnées et validation

Publier sans blocs EXIF, XMP ou ICC. Vérifier le profil couleur avant de retirer
ICC : le profil sRGB générique des six photos actuelles peut être retiré sans
conversion. Ne pas appliquer aveuglément cette suppression à un autre espace couleur.
Privilégier un retrait des blocs sans réencodage, avec conservation des dimensions,
du flux image compressé et des pixels décodés. Vérifier aussi l’orientation.
Les attributs étendus macOS ne sont pas versionnés par Git.

Le contrôle des six fichiers publiés confirme un unique bloc VP8, sans donnée
annexe ni octets après le conteneur RIFF. Flux compressés et pixels décodés sont
identiques aux originaux. macOS appose encore com.apple.provenance localement,
hors contenu WebP et hors Git. Ne pas confondre cet attribut avec un bloc embarqué.

## Historique

L’audit en lecture seule des références locales trouve onze objets image, dont
les six photos actuelles et deux anciennes photos WebP. Aucun EXIF, XMP ou GPS
n’est détecté. Les huit WebP historiques ont seulement un profil ICC.
Cet audit ne couvre pas les objets inaccessibles, les copies ou caches distants.
Ne pas présenter des données personnelles comme découvertes sans preuve.

Renommer ou nettoyer le checkout ne purge pas les anciennes versions dans Git.
Le chantier de purge est au [TODO](../docs/TODO.md). Son périmètre, notamment les
branches historiques à préserver, doit être validé avant toute réécriture.
