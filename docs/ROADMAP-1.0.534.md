# Préparation de la version 1.0.534

Demandes transmises par Tornie le 6 octobre 2026 dans `task.zip`, contenant `Notes-pour-demain-2026-10-07.md`. Cette feuille de route conserve les demandes pour la prochaine mise à jour ; elles ne sont pas encore implémentées.

## Wildspade

- IX : accès au niveau technologique 7.
- Chemical : accès au niveau 6 après la première amélioration, sans IX.
- Ornithopter : accès au niveau 7 après la deuxième amélioration.
- Reporter les conditions effectives dans les fiches et la progression du site, en français et en anglais.

Le moteur comporte une règle spéciale pour `Unit_ChemicalCarryall` dans l'usine Hightech (`BuilderBase::updateBuildList`) : elle impose actuellement la deuxième amélioration et IX, en plus des données du mod. Vérifier que « Chemical » désigne bien cette unité avant les changements ; le Chemical Siege Tank est une autre unité. Contrôler les résultats dans les mods où Wildspade est disponible.

## Pièges à vent sur le site

Ajouter les images originales du jeu pour les variantes 2×3 et 3×2, en complément de la variante 3×3. Mettre à jour les fiches et la galerie dans les deux langues.

## Bannière Fremen : problème visuel ouvert

Tornie signale que l'écran « Do you wish to join House Fremen? » affiche la planète sans bannière. La correction publiée dans la 1.0.533 ne doit pas être considérée comme validée visuellement.

Reprendre la composition de cet écran avec la bannière originale et contrôler son affichage réel. La capture citée dans la note se trouve sur le deuxième PC, sous `outputs/Note-Banniere-Fremen-2026-10-07.png` ; elle n'est pas contenue dans `task.zip`.

## Option Chaos Mode

- Désactivée par défaut et indisponible en Vanilla.
- Emprunter les technologies des 12 factions uniquement entre technologies de même niveau.
- Conserver les couleurs et les sons de la faction choisie.
- Inclure uniquement les bâtiments producteurs d'unités, les défenses et les palais ; les autres bâtiments restent hors du mélange.
- Exemple : Harkonnen peut recevoir la Caserne des Ordos, l'Usine lourde des Atreides et le Palais de Wildspade, en conservant son identité Harkonnen.

À définir lors de la conception : le moment du tirage et les modes de partie concernés. L'état choisi devra être déterministe en multijoueur et conservé dans les sauvegardes. Préserver les règles des parties existantes et les identifiants stables.

Pour cette version et les suivantes, mettre à jour le site avec le jeu conformément à [WEBSITE.md](WEBSITE.md).
