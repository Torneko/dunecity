# Correctif publié de la 1.0.538

Demandes de Tornie du 9 octobre 2026. Correctif intégré aux paquets et au tag de la 1.0.538 existante, sans nouveau numéro de version.

## Changements intégrés

- Missiles aléatoires : la sélection de cible vérifie la capacité réellement prête du Palace contrôlé, y compris les plans obtenus par capture. La commande déterministe existante consomme une charge.
- Couleurs bonus : vert foncé dans Jericho/Jericho Lite ; rose foncé Wildspade Jericho dans Tornie/Tornie Lite. Les couleurs standards approuvées sont conservées.
- Éditeur : cellules uniformes, colonnes alignées, zones de clic et défilement conservés. Étoiles de la sidebar composées en RGBA ; sprites de carte préservés.
- Escouades de cinq : **Super Heavy Trooper Squad** et **Super Infantry Squad**, traductions FR/EN. IDs 74/75 et cinq fantassins conservés.
- Scoutpost, Flamepost et Chemipost non capturables. Améliorations suivant les plans du chantier capturé ou le donneur Scoutpost de Chaos : Kleshmersh / Flamepost ; Tharpique / Chemipost dans les mods complets, avec IX et niveau requis.

## Doublefinery

| Propriété | Implémentation |
| --- | --- |
| Éditions | Tornie, Jericho et leurs versions Lite |
| Emprise | **5 × 2 cases**, placement, éditeur et ruines |
| Technologie | Niveau 7 |
| Prérequis | IX + Windtrap + Refinery |
| Plans natifs | Ordos, Mercenary, Neutral, Corruptique |
| Autres factions | Utilisation par capture, plans via chantier capturé autorisé |
| Coût / PV / énergie | 800 / 900 / 60, base doublée approuvée pour les tests |
| Stockage | 2 500 |
| Déchargement | Deux Harvesters simultanément |
| Cadeau | Deux Harvesters, limites de Harvesters et d'unités vérifiées séparément pour chacun |
| Capture | Oui, transfert des deux occupants et de leur cargaison |
| Capture / chargement | Aucun nouveau cadeau |
| Identifiant | 76, ajouté après les IDs publiés |

Portrait et sprite fournis par Tornie, conservés dans chaque mod. La planche corrigée fournie par Tornie mesure 80 × 224 pixels : sept frames de 80 × 32, appliquées à leur taille native sans modifier le fichier source. [Portrait](references/1.0.538/Doublefinery_icon.png).

Correction graphique locale `538-followup-r4` : le redimensionnement des images indexées copie désormais les indices de palette, car SDL ne prend pas en charge leur redimensionnement par `SDL_BlitScaled`. L'éditeur associe explicitement la Doublefinery à son sprite et à son aperçu. Cela corrige le rectangle noir en partie, le bâtiment incomplet sur la carte de l'éditeur et son bouton vide. L'emprise corrigée est **5 × 2**. Les contrôles de rendu couvrent les dix frames, les 21 couleurs, les trois zooms et les pixels du bâtiment réellement placé dans l'éditeur.

Les Lite conservent leurs six factions sélectionnables. Les plans des autres identités restent définis pour les structures capturées et cartes personnalisées.

## Compatibilité et validation

- Nouvelles sauvegardes : **9831**. Anciens IDs et format de la raffinerie classique conservés en lecture ; les nouvelles sauvegardes nécessitent ce correctif.
- Protocole réseau : **9**. Les deux PC doivent utiliser le même correctif et les mêmes mods.
- Compilation et CTest Windows/Linux réussis.
- Vérifications dans les quatre mods : disponibilité, prérequis, cadeaux 0/1/2, limites d'unités et de Harvesters, deux déchargements, sauvegarde/chargement, récupération par Carryall, capture des deux occupants, améliorations obtenues par capture/Chaos et lancement aléatoire.
- Éditeur : grilles et clics à 480/600/900 pixels de hauteur, changement de mod et chargement des 838 scénarios existants.
- Site local FR/EN : fiche Doublefinery, noms, règles, 121 PNGs et téléchargements du correctif. Le site public et l’archive hors ligne sont synchronisés avec ce correctif.

Le rapport `Validation-538-Followup.json` accompagne le paquet local et détaille les contrôles finaux. Restent les tests humains sur deux PC avec le protocole 9 et sur le PC Linux Mint 22.3 / noyau 7.0 concerné. La validation coop précédente porte sur la 1.0.538 initiale, protocole 7.

Correction des dimensions : 5 × 2 cases, dix cases occupées, collisions, sélection, placement et ruines cohérents. La nouvelle planche de 80 × 224 remplit les frames de 80 × 32 sans redimensionnement ni marge ajoutée. Protocole 9 et nouvelles sauvegardes 9831. Les sauvegardes publiées restent lisibles. Les prototypes 9830 contenant une Doublefinery sont étendus si la cinquième colonne est libre ; sinon le chargement est refusé sans écraser les objets ni modifier le fichier original. Libérer cette colonne avec le précédent build de test permet de reprendre la sauvegarde.

Révision graphique r4 : planche finale de Tornie appliquée aux quatre mods et au site local. Règles, protocole 9 et format 9831 conservés. Les tests du reste du jeu ne sont pas relancés à la demande de Tornie.
