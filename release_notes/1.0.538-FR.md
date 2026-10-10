# Dune Legacy Tornie 1.0.538

## Révision publiée r8 — 10 octobre 2026

Cette révision remplace les paquets de la **1.0.538** existante. Elle rétablit les
portraits illustrés des unités avec toutes les couleurs et adopte la disposition
des bâtiments de l’éditeur fournie par Tornie, dans les quatre mods. Les icônes
de production d’infanterie restent distinctes des portraits des unités sélectionnées.
La dernière planche Doublefinery, avec son trait corrigé, est copiée à l’identique :
80 × 224 pixels, sept images de 80 × 32, sans redimensionnement.

Tornie a validé les portraits et la disposition en jeu et autorisé la publication.
Les vérifications r6/r7 couvrent les versions Windows/Linux, le rendu FR/EN et
le site bilingue. Après cette dernière correction d’image, l’intégrité des paquets
et des PNG est contrôlée ; les suites de jeu ne sont pas répétées. Les rapports
r6/r7 restent identifiés comme vérifications précédentes. Version 1.0.538,
protocole 9 et sauvegardes 9831 inchangés. Mettre à jour les deux PC en coop.

## Disposition locale de l’éditeur r7 — 10 octobre 2026

La palette des bâtiments avancés suit la disposition fournie par Tornie :
éoliennes en haut, Love Factory à droite, Worfinery puis Tech Center à gauche,
Chaos Factory à droite, postes vert/bleu/orange sur une ligne et Doublefinery
tout en bas. Cette disposition concerne Tornie, Jericho et leurs éditions Lite.
Les aperçus gardent leurs pixels natifs. Le correctif des portraits r6 est conservé.
Version 1.0.538, protocole 9 et sauvegardes 9831 restent identiques.

Cette révision est préparée localement ; les téléchargements publics restent à r5.

## Correctif local des portraits — 10 octobre 2026

Les fiches des unités retrouvent leurs portraits illustrés d’origine, quelle que
soit la couleur choisie. Les couleurs personnalisées n’affichent plus une petite
vignette de l’éditeur avec une étoile à la place du portrait. Cela s’applique aux
cinq éditions et aux changements de mod dans la même session.

Les six icônes d’infanterie (individuelle, escouades de trois et de cinq) restent
réservées aux listes de production Caserne/WOR/Worfinery de Tornie, Jericho et de
leurs versions Lite, avec les prix en blanc. La Doublefinery conserve la dernière
planche corrigée fournie par Tornie. Version 1.0.538, protocole 9, sauvegardes 9831.

Vérifications réussies : 122 tests moteur sur Windows et Linux, rendu des portraits avec les 21 couleurs dans les cinq éditions, passage entre mods et icônes de production séparées. Contrôles en français et en anglais, y compris dans l’AppImage finale sous Ubuntu 22.04 / WSL ; 200 contrôles du site local. Mint 22.3 / noyau 7.0 reste à vérifier sur le PC concerné.

Ce correctif est préparé localement. Les téléchargements publics restent ceux
de la révision graphique r5 jusqu’à la prochaine publication.

Révision graphique r5 : la planche Doublefinery corrigée fournie le 10 octobre est copiée à l’identique dans les quatre mods. Dimensions natives : 80 × 224 pixels (sept images de 80 × 32), sans redimensionnement. Les paquets de la 1.0.538 et le site FR/EN sont actualisés avec cette image.

## Correctif intégré à la 1.0.538 — 10 octobre 2026

La 1.0.538 publiée est remplacée par ce correctif, sans changer de numéro de version.
La Doublefinery occupe **5 × 2 cases** : deux quais, 2 500 épices de stockage et deux
moissonneuses offertes, dans la limite des unités autorisées. Disponible au niveau 7
avec IX, Windtrap et Refinery pour Ordos, Mercenary, Neutral et Corruptique dans Tornie,
Jericho et leurs versions Lite ; elle est capturable. Son sprite final de 80 × 224 pixels
est utilisé sans redimensionnement.

Ce correctif rétablit les missiles de Palace aléatoires, corrige les couleurs bonus,
l’alignement des boutons et la transparence des étoiles dans l’éditeur, et renomme
les escouades de cinq. Scoutpost, Flamepost et Chemipost ne sont plus capturables ;
leurs améliorations suivent les plans du chantier capturé ou le donneur du mode Chaos.

**Coop : mettre à jour les deux PC**, protocole **9**, sauvegardes **9831**.
Les sauvegardes publiées antérieures restent lisibles. Les prototypes 9830 avec une
Doublefinery de 4 × 2 sont étendus si la cinquième colonne est libre ; sinon le chargement
est refusé sans écraser la carte ni le fichier d’origine.

Les vérifications moteur Windows/Linux et les contrôles de rendu ont réussi avant
le remplacement final du sprite. À la demande de Tornie, les tests du reste du jeu
n’ont pas été relancés après ce remplacement : compilation et intégrité des paquets
ont été vérifiées. Le contrôle sur le PC Mint 22.3 / noyau 7.0 concerné reste à faire.

### Correction des portraits et de la Worfinery

Les six PNG fournis distinguent Soldat/Trooper individuel, escouade de 3 et
escouade de 5. Seules les listes de production Caserne/WOR/Worfinery de Tornie, Jericho
et leurs versions Lite utilisent ces portraits ; les prix sont affichés en blanc.
Les cartes, technologies et fiches du site montrent les variantes correspondantes.
Dans la Worfinery, la moissonneuse apparaît en dernier. Cet ordre d’affichage
conserve les productions en cours et les commandes sauvegardées.
Tornie a testé la coop 1.0.538 sur deux PC et confirmé son bon fonctionnement.
Le test Mint 22.3 / noyau 7.0 reste à faire sur le PC concerné.

## Nouveautés 1.0.538

Deux options de lobby sont désactivées par défaut : **Mode facile** et **Forces ennemies +5**.
Le mode facile commence une nouvelle campagne à la mission 2. Chacun des deux alliés,
humain ou IA, reçoit 500 crédits supplémentaires au départ de cette mission seulement.
Leurs achats d’unités et de bâtiments coûtent 25 crédits de moins (minimum 1) pendant
la campagne. Les ennemis ne reçoivent ni crédits ni réduction.

Forces ennemies +5 ajoute cinq unités adaptées à la technologie par adversaire présent
à chaque mission, près de ses bâtiments ou forces. Un adversaire arrivant uniquement
par renforts reçoit ses unités sur une bordure opposée à la base principale. Les unités
commencent en garde de zone. Les adversaires absents de la mission ne reçoivent rien.

Les renforts destinés au premier joueur sont aussi envoyés au second allié, avec le
même nombre, le même moment, le même lieu relatif et la même répétition. Une unité
indisponible est adaptée à la faction alliée. Les renforts et forces déjà enregistrés
dans un checkpoint sont restaurés, jamais ajoutés une seconde fois.

**Ensemble sur Arrakis / COOP_CONQUEROR** demande de terminer toutes les étapes de la
campagne commune, avec un humain ou une IA. Le mode facile autorise le saut de l’intro.
Le déblocage reste local et une session terminée ne compte qu’une fois dans le cumul.
Les succès des campagnes solo gardent leurs conditions.

Les deux PC doivent utiliser **1.0.538**, protocole **9**. Les nouvelles sauvegardes
portent la version **9831** ; les anciennes restent lisibles avec leurs forces et
renforts enregistrés. Commencer une nouvelle campagne ou passer à la mission suivante
pour profiter des nouveaux renforts. Tornie a confirmé la coop de la version initiale sur deux PC. Mint 22.3 / noyau 7.0 reste à essayer.

## Portraits, bannières et succès

Les nouveaux portraits individuels et d’escouades concernent uniquement les listes Caserne/WOR/Worfinery de Tornie, Jericho et de leurs éditions Lite. Vanilla, le Starport et le portrait de l’unité sélectionnée conservent leurs images. Les PNG fournis sont copiés sans modification.

Wildspade/Tornie utilise la nouvelle bannière et le violet foncé. Rebels/Jericho utilise la nouvelle bannière et le gris foncé. Les autres factions et mods gardent leurs variantes.

Le catalogue passe de 47 à 67 succès : coop complète, Gaspillage de carburant, trois bâtiments ennemis détruits par un seul missile de Palace, scores de 1 500 / 2 000, cumul de 10 000 points et paliers pour chaque compteur. Les dégâts alliés ou non mortels empêchent Gaspillage de carburant. Les explosions d’un même missile sont regroupées, les bâtiments sont comptés une fois. Les scores de missions gagnées sont dédupliqués après rechargement ; les points anciens non enregistrés ne sont pas inventés.
