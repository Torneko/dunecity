# Dune Legacy Tornie 1.0.532

Cette version intègre les nouvelles factions et campagnes, les corrections d'épice et les hauts faits supplémentaires. Les derniers correctifs de voix et de bannières ont été testés et validés par Torneko.

## Factions, campagnes et présentation

- Tornie et Jericho proposent douze factions nommées et leurs campagnes : Atreides, Harkonnen, Ordos, Fremen, Sardaukar, Mercenary, Neutral, Rebels, Wildspade, Kleshmersh, Tharpique et Corruptique.
- Vanilla ajoute Kleshmersh en brun comme neuvième faction, avec les technologies de Neutral et sa bannière fournie. Ses 22 missions sont clonées de la campagne Neutral : Harkonnen aux missions 1–10, Sardaukar aux missions 11–21 et Rebels à la finale.
- Kleshmersh utilise son propre nom vocal en vanilla, y compris dans l'interface française et les annonces anglaises de déploiement.
- Neutral et Rebels utilisent les bonnes bannières dans Jericho. Corruptique est jaune avec sa bannière fournie. Les Fremen du mod Tornie utilisent la nouvelle bannière fournie.
- Wildspade rencontre trois adversaires : Atreides, Kleshmersh et Ordos.
- Les fichiers de régions ont été contrôlés et corrigés : itinéraires, adversaires, noms de factions et transitions de la campagne Kleshmersh.

## Hauts faits et épices

- Le catalogue comprend 43 hauts faits locaux. « Ruler of Arrakis » exige une victoire de campagne dont le score final affiché atteint au moins 1000.
- Ajout des succès de campagne Wildspade, Kleshmersh, Tharpique et Corruptique, ainsi que « Master of Jericho » pour ses douze campagnes.
- « Collectionneur de maisons / House Collector » exige une victoire avec chacune des douze factions. Les statistiques et succès déjà obtenus sont conservés.
- « Worm Hunter » reconnaît un ver vaincu à la moitié de sa vie et attribué au joueur, même lorsque son retour est activé.
- L'épice violette soigne les récolteurs d'au maximum un point de vie par 1008 millisecondes de simulation, pendant une récolte effective.
- Les 142 cartes fournies utilisent les variantes rouge, verte, violette et bleue en petites poches déterministes.

## Installation et validation

Décompresser entièrement le ZIP Windows et lancer `dunelegacy.exe`. Les mods fournis se mettent à jour automatiquement au lancement ; la progression reste dans le profil utilisateur.

Les contrôles locaux Windows et Linux réussissent : 87 tests unitaires / 3694 assertions, séries de 118 chargements de scénarios, bannières, voix, changements de mod, cartes, hauts faits et chargement d’une sauvegarde 1.0.525.

Les [compilations et contrôles GitHub Windows/Linux](https://github.com/Torneko/dunelegacy-tornie/actions/runs/37297912189) réussissent. Le ZIP Windows final a été téléchargé depuis la release et testé avec des profils neufs en français et en anglais : ses 142 cartes, 2198 fichiers de mods et ressources vanilla correspondent aux sources. Les paquets Linux DEB, RPM, TAR.GZ et AppImage sont également fournis.

Sources : commit `6182a6a0861a9ef6bb60985a3833211c2d4cf2e2`. Le patch fourni contient les changements depuis la version publique 1.0.530.

SHA-256 du ZIP Windows :

```text
c83bcf3aabdee00e45921ad90edf8734b9b84608ca5468990470619bd04a1f1b
```
