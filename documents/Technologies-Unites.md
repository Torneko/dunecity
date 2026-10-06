# Technologies des unités — Dune Legacy Tornie 1.0.533

Ce relevé inclut **Vanilla, Tornie, Tornie Lite et Jericho**. Il provient des fichiers ObjectData et des règles du moteur de la version 1.0.533, basée sur la 1.0.524-26 corrigée.

Le **niveau minimal** ci-dessous est calculé pour construire une chaîne de production à partir d’un chantier de construction, sans usine capturée ni bâtiment avancé offert par la carte. Il inclut les niveaux des bâtiments prérequis et la possibilité de les améliorer. Les crédits, les surfaces de construction et les limites d’unités restent nécessaires. Une carte ou une sauvegarde personnalisée peut modifier ces données.

Le niveau d’amélioration est le nombre d’améliorations de l’usine : 0 = aucune, 1 = une, 2 = deux, etc. Ce nombre est distinct du niveau technologique de la partie. Les niveaux supérieurs conservent les déblocages antérieurs.

Légende : **H** = Harkonnen, **A** = Atreides, **O** = Ordos, **F** = Fremen, **S** = Sardaukar, **M** = Mercenaires, **N** = Neutres, **R** = Rebelles, **C** = Corruptique, **W** = Wildspade, **K** = Kleshmersh, **T** = Tharpique.

Vanilla propose neuf factions, dont Kleshmersh (K) avec les technologies de Neutral (N). Tornie et Jericho proposent les douze factions nommées et leurs campagnes. Tornie Lite présente les six factions H/A/O/F/S/M.

## Vue par niveau — factions des campagnes et menus principaux

| Niveau | Vanilla | Tornie | Tornie Lite | Jericho |
|---|---|---|---|---|
| 1 | Aucune nouvelle unité fabriquée | Aucune nouvelle unité fabriquée | Aucune nouvelle unité fabriquée | Aucune nouvelle unité fabriquée |
| 2 | Soldat (A/O/F/S/M/N/R/K); Trooper (H); Trike (A/F/M/N/R/K); Raider Trike (O/F/M/N/R/K) | Soldat (A/O/F/S/M/N/R/C/W/K/T); Escouade de soldats (A/O/F/S/M/N/R/C/W/K/T); Trooper (H/N); Trike (A/F/S/M/C/T); Raider Trike (O/F/S/M/R/C/K/T) | Soldat (A/O/F/S/M); Escouade de soldats (A/O/F/S/M); Trooper (H); Trike (A/F/S/M); Raider Trike (O/F/S/M); Rocket Trike (M); Sonic Trike (A/F) | Soldat (A/O/F/S/M/N/R/C/W/K/T); Escouade de soldats (A/O/F/S/M/N/R/C/W/K/T); Trooper (H); Trike (A/F/S/M/C/T); Raider Trike (O/F/S/M/R/C/K/T) |
| 3 | Trike (S); Raider Trike (S); Quad (H/A/O/F/S/M/N/R/K) | Trooper (C/W); Raider Trike (W); Quad (H/A/O/F/S/M/C/T); Rocket Trike (N/W); Sonic Trike (R/K) | Quad (H/A/O/F/S/M); Rocket Trike (H) | Trooper (N/C/W); Raider Trike (W); Quad (H/A/O/F/S/M/C/T); Rocket Trike (N/W); Sonic Trike (R/K) |
| 4 | Moissonneuse (H/A/O/F/S/M/N/R/K); VCM (H/A/O/F/S/M/N/R/K); Char (H/A/O/F/S/M/N/R/K) | Escouade de troopers (H/C); Quad (N); Moissonneuse (H/A/O/F/S/M/N/R/C/W/K/T); VCM (H/A/O/F/S/M/N/R/C/W/K/T); Char (H/A/O/F/S/M/R/C/W/K/T) | Escouade de troopers (H); Moissonneuse (H/A/O/F/S/M); Harvestank (O); VCM (H/A/O/F/S/M); Char (H/A/O/F/S/M) | Escouade de troopers (H/C); Quad (N); Moissonneuse (H/A/O/F/S/M/N/R/C/W/K/T); VCM (H/A/O/F/S/M/N/R/C/W/K/T); Char (H/A/O/F/S/M/R/C/W/K/T) |
| 5 | Trooper (O/F/S/M/N/R/K); Lance-missiles (H/A/O/F/S/M/N/R/K); Carryall (H/A/O/F/S/M/N/R/K) | Trooper (O/F/S/M/R/K/T); Escouade de troopers (O/F/S/M/N/R/W/K/T); Sonic Trike (C/T); Char (N); Lance-missiles (H/A/O/F/S/M/N/R/C/W/K/T); Carryall (H/A/O/F/S/M/N/R/C/W/K/T) | Trooper (O/F/S/M); Escouade de troopers (O/F/S/M); Lance-missiles (H/A/O/F/S/M); Carryall (H/A/O/F/S/M) | Trooper (O/F/S/M/R/K/T); Escouade de troopers (O/F/S/M/N/R/W/K/T); Sonic Trike (C/T); Char (N); Lance-missiles (H/A/O/F/S/M/N/R/C/W/K/T); Carryall (H/A/O/F/S/M/N/R/C/W/K/T) |
| 6 | Char de siège (H/A/F/S/M/N/R/K) | Trooper (A); Escouade de troopers (A); Char de siège (H/A/O/F/S/M/N/R/C/W/K/T); Char lance-flammes (W); Lance-missiles élite (W); Carryall chimique (W) | Trooper (A); Escouade de troopers (A); Char de siège (H/A/O/F/S/M) | Trooper (A); Escouade de troopers (A); Char de siège (H/A/O/F/S/M/N/R/C/W/K/T); Char lance-flammes (W); Lance-missiles élite (W) |
| 7 | Char de siège (O); Devastator (H/F/S/M/N/R/K); Deviator (O); Char sonique (A/F/S/M/N/R/K); Ornithoptère (A/O/F/S/M/N/R/K) | Devastator (H/S/M/C); Deviator (O/M/T); Char sonique (A/S/R); Char lance-flammes (H/F/R/K); Lance-missiles élite (A/N/T); Char de siège élite (O/F/N/C/K); Ornithoptère (A/O/F/S/M/N/R/C/W/K/T) | Devastator (H/S/M); Deviator (O/M); Char sonique (A/S); Char lance-flammes (H/F); Lance-missiles élite (A); Char de siège élite (O/F); Ornithoptère (A/O/F/S/M) | Devastator (H/S/M/C); Deviator (O/M/T); Char sonique (A/S/R); Char lance-flammes (H/F/R/K); Lance-missiles élite (A/N/T); Char de siège élite (O/F/N/C/K); Carryall chimique (W); Ornithoptère (A/O/F/S/M/N/R/C/W/K/T) |
| 8 | Aucune nouvelle unité fabriquée | Aucune nouvelle unité fabriquée | Aucune nouvelle unité fabriquée | Aucune nouvelle unité fabriquée |
| 9 | Aucune nouvelle unité fabriquée | Harvestank (R/C/K); Char de siège chimique (C/T) | Char de siège chimique (M) | Harvestank (R/C/K); Char de siège chimique (C/T) |

Au niveau 1, la raffinerie fournit normalement une moissonneuse lors de son placement. Elle est distincte de la fabrication régulière d’une moissonneuse en usine.

## Liste complète : factions, usines et améliorations

La mention « — » signifie absence de fabrication régulière pour cette unité. Des commandes au Spatioport, offres aléatoires, pouvoirs du Palais ou unités placées sur la carte peuvent néanmoins en fournir.

### Vanilla

| Unité | Niveau minimal et factions | Production et conditions |
|---|---|---|
| Soldat (Soldier) | **2** : A/O/F/S/M/N/R/K | Caserne ; amélioration **0** (H) ; bâtiment non constructible par cette faction, capture ou placement sur carte nécessaire<br>Caserne ; amélioration **0** (A/O/F/S/M/N/R/K) |
| Escouade de soldats (Infantry) | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Trooper | **2** : H; **5** : O/F/S/M/N/R/K | WOR ; amélioration **0** (H/O/F/S/M/N/R/K)<br>WOR ; amélioration **0** (A) ; bâtiment non constructible par cette faction, capture ou placement sur carte nécessaire |
| Escouade de troopers (Troopers) | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Trike | **2** : A/F/M/N/R/K; **3** : S | Usine légère ; amélioration **0** (A/F/S/M/N/R/K) |
| Raider Trike | **2** : O/F/M/N/R/K; **3** : S | Usine légère ; amélioration **0** (O/F/S/M/N/R/K) |
| Quad | **3** : H/A/O/F/S/M/N/R/K | Usine légère ; amélioration **0** (H)<br>Usine légère ; amélioration **1** (A/O/F/S/M/N/R/K) |
| Rocket Trike | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Sonic Trike | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Moissonneuse (Harvester) | **4** : H/A/O/F/S/M/N/R/K | Usine lourde ; amélioration **0** (H/A/O/F/S/M/N/R/K) |
| Harvestank (Rebel Harvester) | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| VCM (MCV) | **4** : H/A/O/F/S/M/N/R/K | Usine lourde ; amélioration **1** (H/A/O/F/S/M/N/R/K) |
| Char (Tank) | **4** : H/A/O/F/S/M/N/R/K | Usine lourde ; amélioration **0** (H/A/O/F/S/M/N/R/K) |
| Lance-missiles (Launcher) | **5** : H/A/O/F/S/M/N/R/K | Usine lourde ; amélioration **2** (H/A/O/F/S/M/N/R/K) |
| Char de siège (Siege Tank) | **6** : H/A/F/S/M/N/R/K; **7** : O | Usine lourde ; amélioration **3** (H/A/F/S/M/N/R/K)<br>Usine lourde ; amélioration **2** (O) |
| Devastator | **7** : H/F/S/M/N/R/K | Usine lourde ; amélioration **0** ; requis : IX (H/F/S/M/N/R/K) |
| Deviator | **7** : O | Usine lourde ; amélioration **0** ; requis : IX (O) |
| Char sonique (Sonic Tank) | **7** : A/F/S/M/N/R/K | Usine lourde ; amélioration **0** ; requis : IX (A/F/S/M/N/R/K) |
| Char lance-flammes (Flame Tank) | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Lance-missiles élite (Elite Launcher) | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Char de siège élite (Elite Siege Tank) | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Char de siège chimique (Chemical Siege Tank) | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Carryall | **5** : H/A/O/F/S/M/N/R/K | Usine high-tech ; amélioration **0** (H/A/O/F/S/M/N/R/K) |
| Carryall chimique (Chemical Carryall) | — | Aucune chaîne de fabrication régulière pour les factions de ce mode. |
| Ornithoptère (Ornithopter) | **7** : A/O/F/S/M/N/R/K | Usine high-tech ; amélioration **1** ; requis : IX (A/O/F/S/M/N/R/K) |
| Saboteur | — | Pouvoir du Palais, pas une production d’usine. |
| Frégate (Frigate) | — | Transport automatique des commandes au Spatioport, pas une unité à fabriquer. |
| Ver des sables (Sandworm) | — | Créature de la carte, pas une unité à fabriquer. |

#### Niveaux des bâtiments de production

| Bâtiment | Premier niveau de construction et factions |
|---|---|
| Caserne | **2** : A/O/F/S/M/N/R/K |
| Usine légère | **2** : A/O/F/M/N/R/K; **3** : H/S |
| Usine lourde | **4** : H/A/O/F/S/M/N/R/K |
| Usine high-tech | **5** : H/A/O/F/S/M/N/R/K |
| WOR | **2** : H; **5** : O/F/S/M/N/R/K |
| IX | **7** : H/A/O/F/S/M/N/R/K |
| Spatioport | **6** : H/A/O/F/S/M/N/R/K |
| Palace | **8** : H/A/O/F/S/M/N/R/K |

### Tornie

| Unité | Niveau minimal et factions | Production et conditions |
|---|---|---|
| Soldat (Soldier) | **2** : A/O/F/S/M/N/R/C/W/K/T | Caserne ; amélioration **0** (H) ; bâtiment non constructible par cette faction, capture ou placement sur carte nécessaire<br>Caserne ; amélioration **0** (A/O/F/S/M/N/R/C/W/K/T) |
| Escouade de soldats (Infantry) | **2** : A/O/F/S/M/N/R/C/W/K/T | Caserne ; amélioration **1** (A/O/F/S/M/N/R/C/W/K/T) |
| Trooper | **2** : H/N; **3** : C/W; **5** : O/F/S/M/R/K/T; **6** : A | WOR ; amélioration **0** (H/A/O/F/S/M/N/R/C/W/K/T)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M/N/R/C/W/K/T) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Escouade de troopers (Troopers) | **4** : H/C; **5** : O/F/S/M/N/R/W/K/T; **6** : A | WOR ; amélioration **1** (H/A/O/F/S/M/N/R/C/W/K/T)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M/N/R/C/W/K/T) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Trike | **2** : A/F/S/M/C/T | Usine légère ; amélioration **0** (A/F/S/M/C/T) |
| Raider Trike | **2** : O/F/S/M/R/C/K/T; **3** : W | Usine légère ; amélioration **0** (O/F/S/M/R/C/W/K/T) |
| Quad | **3** : H/A/O/F/S/M/C/T; **4** : N | Usine légère ; amélioration **0** (H)<br>Usine légère ; amélioration **1** (A/O/F/S/M/N/C/T) |
| Rocket Trike | **3** : N/W | Usine légère ; amélioration **0** (N)<br>Usine légère ; amélioration **1** (W) |
| Sonic Trike | **3** : R/K; **5** : C/T | Usine légère ; amélioration **1** (R/K)<br>Usine légère ; amélioration **2** (C/T) |
| Moissonneuse (Harvester) | **4** : H/A/O/F/S/M/N/R/C/W/K/T | Usine lourde ; amélioration **0** (H/A/O/F/S/M/N/R/C/W/K/T)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M/N/R/C/W/K/T) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Harvestank (Rebel Harvester) | **9** : R/C/K | Usine lourde ; amélioration **0** ; requis : IX (R/C/K) |
| VCM (MCV) | **4** : H/A/O/F/S/M/N/R/C/W/K/T | Usine lourde ; amélioration **1** (H/A/O/F/S/M/N/R/C/W/K/T) |
| Char (Tank) | **4** : H/A/O/F/S/M/R/C/W/K/T; **5** : N | Usine lourde ; amélioration **0** (H/A/O/F/S/M/N/R/C/W/K/T) |
| Lance-missiles (Launcher) | **5** : H/A/O/F/S/M/N/R/C/W/K/T | Usine lourde ; amélioration **2** (H/A/O/F/S/M/N/R/C/W/K/T) |
| Char de siège (Siege Tank) | **6** : H/A/O/F/S/M/N/R/C/W/K/T | Usine lourde ; amélioration **3** (H/A/F/S/M/R/C/W/K/T)<br>Usine lourde ; amélioration **2** (O/N) |
| Devastator | **7** : H/S/M/C | Usine lourde ; amélioration **0** ; requis : IX (H/S/M/C) |
| Deviator | **7** : O/M/T | Usine lourde ; amélioration **0** ; requis : IX (O/M/T) |
| Char sonique (Sonic Tank) | **7** : A/S/R | Usine lourde ; amélioration **0** ; requis : IX (A/S/R) |
| Char lance-flammes (Flame Tank) | **6** : W; **7** : H/F/R/K | Usine lourde ; amélioration **0** ; requis : IX (H/F/R/W/K) |
| Lance-missiles élite (Elite Launcher) | **6** : W; **7** : A/N/T | Usine lourde ; amélioration **0** ; requis : IX (A/N/W/T) |
| Char de siège élite (Elite Siege Tank) | **7** : O/F/N/C/K | Usine lourde ; amélioration **0** ; requis : IX (O/F/N/C/K) |
| Char de siège chimique (Chemical Siege Tank) | **9** : C/T | Usine lourde ; amélioration **4** (C/T) |
| Carryall | **5** : H/A/O/F/S/M/N/R/C/W/K/T | Usine high-tech ; amélioration **0** (H/A/O/F/S/M/N/R/C/W/K/T) |
| Carryall chimique (Chemical Carryall) | **6** : W | Usine high-tech ; amélioration **2** ; requis : IX (A) ; **amélioration 2 inaccessible normalement**<br>Usine high-tech ; amélioration **2** ; requis : IX (W) |
| Ornithoptère (Ornithopter) | **7** : A/O/F/S/M/N/R/C/W/K/T | Usine high-tech ; amélioration **1** ; requis : IX (A/O/F/S/M/N/R/C/W/K/T) |
| Saboteur | — | Pouvoir du Palais, pas une production d’usine. |
| Frégate (Frigate) | — | Transport automatique des commandes au Spatioport, pas une unité à fabriquer. |
| Ver des sables (Sandworm) | — | Créature de la carte, pas une unité à fabriquer. |

#### Niveaux des bâtiments de production

| Bâtiment | Premier niveau de construction et factions |
|---|---|
| Caserne | **2** : A/O/F/S/M/N/R/C/W/K/T |
| Usine légère | **2** : A/O/F/S/M/R/C/K/T; **3** : H/N/W |
| Usine lourde | **4** : H/A/O/F/S/M/N/R/C/W/K/T |
| Usine high-tech | **5** : H/A/O/F/S/M/N/R/C/W/K/T |
| WOR | **2** : H/N; **3** : C/W; **5** : O/F/S/M/R/K/T; **6** : A |
| Worfinery | **5** : H/O/F/S/M/N/R/C/W/K/T; **6** : A |
| IX | **6** : W; **7** : H/A/O/F/S/M/N/R/C/K/T |
| Spatioport | **6** : H/A/O/F/S/M/N/R/C/W/K/T |
| Palace | **8** : H/A/O/F/S/M/N/R/C/W/K/T |
| Tech Center | **9** : H/A/O/F/S/M/N/R/C/W/K/T |
| Love Factory | **9** : H/A/O/F/S/M/N/R/C/W/K/T |
| Chaos Factory | **9** : H/A/O/F/S/M/N/R/C/W/K/T |

### Tornie Lite

| Unité | Niveau minimal et factions | Production et conditions |
|---|---|---|
| Soldat (Soldier) | **2** : A/O/F/S/M | Caserne ; amélioration **0** (H) ; bâtiment non constructible par cette faction, capture ou placement sur carte nécessaire<br>Caserne ; amélioration **0** (A/O/F/S/M) |
| Escouade de soldats (Infantry) | **2** : A/O/F/S/M | Caserne ; amélioration **1** (A/O/F/S/M) |
| Trooper | **2** : H; **5** : O/F/S/M; **6** : A | WOR ; amélioration **0** (H/A/O/F/S/M)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Escouade de troopers (Troopers) | **4** : H; **5** : O/F/S/M; **6** : A | WOR ; amélioration **1** (H/A/O/F/S/M)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Trike | **2** : A/F/S/M | Usine légère ; amélioration **0** (A/F/S/M) |
| Raider Trike | **2** : O/F/S/M | Usine légère ; amélioration **0** (O/F/S/M) |
| Quad | **3** : H/A/O/F/S/M | Usine légère ; amélioration **0** (H)<br>Usine légère ; amélioration **1** (A/O/F/S/M) |
| Rocket Trike | **2** : M; **3** : H | Usine légère ; amélioration **0** (H/M) |
| Sonic Trike | **2** : A/F | Usine légère ; amélioration **0** (A/F) |
| Moissonneuse (Harvester) | **4** : H/A/O/F/S/M | Usine lourde ; amélioration **0** (H/A/O/F/S/M)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Harvestank (Rebel Harvester) | **4** : O | Usine lourde ; amélioration **0** (O) |
| VCM (MCV) | **4** : H/A/O/F/S/M | Usine lourde ; amélioration **1** (H/A/O/F/S/M) |
| Char (Tank) | **4** : H/A/O/F/S/M | Usine lourde ; amélioration **0** (H/A/O/F/S/M) |
| Lance-missiles (Launcher) | **5** : H/A/O/F/S/M | Usine lourde ; amélioration **2** (H/A/O/F/S/M) |
| Char de siège (Siege Tank) | **6** : H/A/O/F/S/M | Usine lourde ; amélioration **3** (H/A/F/S/M)<br>Usine lourde ; amélioration **2** (O) |
| Devastator | **7** : H/S/M | Usine lourde ; amélioration **0** ; requis : IX (H/S/M) |
| Deviator | **7** : O/M | Usine lourde ; amélioration **0** ; requis : IX (O/M) |
| Char sonique (Sonic Tank) | **7** : A/S | Usine lourde ; amélioration **0** ; requis : IX (A/S) |
| Char lance-flammes (Flame Tank) | **7** : H/F | Usine lourde ; amélioration **0** ; requis : IX (H/F) |
| Lance-missiles élite (Elite Launcher) | **7** : A | Usine lourde ; amélioration **0** ; requis : IX (A) |
| Char de siège élite (Elite Siege Tank) | **7** : O/F | Usine lourde ; amélioration **0** ; requis : IX (O/F) |
| Char de siège chimique (Chemical Siege Tank) | **9** : M | Usine lourde ; amélioration **4** (M) |
| Carryall | **5** : H/A/O/F/S/M | Usine high-tech ; amélioration **0** (H/A/O/F/S/M) |
| Carryall chimique (Chemical Carryall) | — | Usine high-tech ; amélioration **2** ; requis : IX (A) ; **amélioration 2 inaccessible normalement** |
| Ornithoptère (Ornithopter) | **7** : A/O/F/S/M | Usine high-tech ; amélioration **1** ; requis : IX (A/O/F/S/M) |
| Saboteur | — | Pouvoir du Palais, pas une production d’usine. |
| Frégate (Frigate) | — | Transport automatique des commandes au Spatioport, pas une unité à fabriquer. |
| Ver des sables (Sandworm) | — | Créature de la carte, pas une unité à fabriquer. |

#### Niveaux des bâtiments de production

| Bâtiment | Premier niveau de construction et factions |
|---|---|
| Caserne | **2** : A/O/F/S/M |
| Usine légère | **2** : A/O/F/S/M; **3** : H |
| Usine lourde | **4** : H/A/O/F/S/M |
| Usine high-tech | **5** : H/A/O/F/S/M |
| WOR | **2** : H; **5** : O/F/S/M; **6** : A |
| Worfinery | **5** : H/O/F/S/M; **6** : A |
| IX | **7** : H/A/O/F/S/M |
| Spatioport | **6** : H/A/O/F/S/M |
| Palace | **8** : H/A/O/F/S/M |
| Tech Center | **9** : H/A/O/F/S/M |
| Love Factory | **9** : H/A/O/F/S/M |
| Chaos Factory | **9** : H/A/O/F/S/M |

### Jericho

| Unité | Niveau minimal et factions | Production et conditions |
|---|---|---|
| Soldat (Soldier) | **2** : A/O/F/S/M/W/K/T/N/R/C | Caserne ; amélioration **0** (H) ; bâtiment non constructible par cette faction, capture ou placement sur carte nécessaire<br>Caserne ; amélioration **0** (A/O/F/S/M/W/K/T/N/R/C) |
| Escouade de soldats (Infantry) | **2** : A/O/F/S/M/W/K/T/N/R/C | Caserne ; amélioration **1** (A/O/F/S/M/W/K/T/N/R/C) |
| Trooper | **2** : H; **3** : W/N/C; **5** : O/F/S/M/K/T/R; **6** : A | WOR ; amélioration **0** (H/A/O/F/S/M/W/K/T/N/R/C)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M/W/K/T/N/R/C) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Escouade de troopers (Troopers) | **4** : H/C; **5** : O/F/S/M/W/K/T/N/R; **6** : A | WOR ; amélioration **1** (H/A/O/F/S/M/W/K/T/N/R/C)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M/W/K/T/N/R/C) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Trike | **2** : A/F/S/M/T/C | Usine légère ; amélioration **0** (A/F/S/M/T/C) |
| Raider Trike | **2** : O/F/S/M/K/T/R/C; **3** : W | Usine légère ; amélioration **0** (O/F/S/M/W/K/T/R/C) |
| Quad | **3** : H/A/O/F/S/M/T/C; **4** : N | Usine légère ; amélioration **0** (H)<br>Usine légère ; amélioration **1** (A/O/F/S/M/T/N/C) |
| Rocket Trike | **3** : W/N | Usine légère ; amélioration **1** (W)<br>Usine légère ; amélioration **0** (N) |
| Sonic Trike | **3** : K/R; **5** : T/C | Usine légère ; amélioration **1** (K/R)<br>Usine légère ; amélioration **2** (T/C) |
| Moissonneuse (Harvester) | **4** : H/A/O/F/S/M/W/K/T/N/R/C | Usine lourde ; amélioration **0** (H/A/O/F/S/M/W/K/T/N/R/C)<br>Worfinery ; amélioration **0** (H/A/O/F/S/M/W/K/T/N/R/C) ; aucun niveau/amélioration supplémentaire pour l’unité |
| Harvestank (Rebel Harvester) | **9** : K/R/C | Usine lourde ; amélioration **0** ; requis : IX (K/R/C) |
| VCM (MCV) | **4** : H/A/O/F/S/M/W/K/T/N/R/C | Usine lourde ; amélioration **1** (H/A/O/F/S/M/W/K/T/N/R/C) |
| Char (Tank) | **4** : H/A/O/F/S/M/W/K/T/R/C; **5** : N | Usine lourde ; amélioration **0** (H/A/O/F/S/M/W/K/T/N/R/C) |
| Lance-missiles (Launcher) | **5** : H/A/O/F/S/M/W/K/T/N/R/C | Usine lourde ; amélioration **2** (H/A/O/F/S/M/W/K/T/N/R/C) |
| Char de siège (Siege Tank) | **6** : H/A/O/F/S/M/W/K/T/N/R/C | Usine lourde ; amélioration **3** (H/A/O/F/S/M/W/K/T/R/C)<br>Usine lourde ; amélioration **2** (N) |
| Devastator | **7** : H/S/M/C | Usine lourde ; amélioration **0** ; requis : IX (H/S/M/C) |
| Deviator | **7** : O/M/T | Usine lourde ; amélioration **0** ; requis : IX (O/M/T) |
| Char sonique (Sonic Tank) | **7** : A/S/R | Usine lourde ; amélioration **0** ; requis : IX (A/S/R) |
| Char lance-flammes (Flame Tank) | **6** : W; **7** : H/F/K/R | Usine lourde ; amélioration **0** ; requis : IX (H/F/W/K/R) |
| Lance-missiles élite (Elite Launcher) | **6** : W; **7** : A/T/N | Usine lourde ; amélioration **0** ; requis : IX (A/W/T/N) |
| Char de siège élite (Elite Siege Tank) | **7** : O/F/K/N/C | Usine lourde ; amélioration **0** ; requis : IX (O/F/K/N/C) |
| Char de siège chimique (Chemical Siege Tank) | **9** : T/C | Usine lourde ; amélioration **4** (T/C) |
| Carryall | **5** : H/A/O/F/S/M/W/K/T/N/R/C | Usine high-tech ; amélioration **0** (H/A/O/F/S/M/W/K/T/N/R/C) |
| Carryall chimique (Chemical Carryall) | **7** : W | Usine high-tech ; amélioration **2** ; requis : IX (W) |
| Ornithoptère (Ornithopter) | **7** : A/O/F/S/M/W/K/T/N/R/C | Usine high-tech ; amélioration **1** ; requis : IX (A/O/F/S/M/W/K/T/N/R/C) |
| Saboteur | — | Pouvoir du Palais, pas une production d’usine. |
| Frégate (Frigate) | — | Transport automatique des commandes au Spatioport, pas une unité à fabriquer. |
| Ver des sables (Sandworm) | — | Créature de la carte, pas une unité à fabriquer. |

#### Niveaux des bâtiments de production

| Bâtiment | Premier niveau de construction et factions |
|---|---|
| Caserne | **2** : A/O/F/S/M/W/K/T/N/R/C |
| Usine légère | **2** : A/O/F/S/M/K/T/R/C; **3** : H/W/N |
| Usine lourde | **4** : H/A/O/F/S/M/W/K/T/N/R/C |
| Usine high-tech | **5** : H/A/O/F/S/M/W/K/T/N/R/C |
| WOR | **2** : H; **3** : W/N/C; **5** : O/F/S/M/K/T/R; **6** : A |
| Worfinery | **5** : H/O/F/S/M/W/K/T/N/R/C; **6** : A |
| IX | **6** : W; **7** : H/A/O/F/S/M/K/T/N/R/C |
| Spatioport | **6** : H/A/O/F/S/M/W/K/T/N/R/C |
| Palace | **8** : H/A/O/F/S/M/W/K/T/N/R/C |
| Tech Center | **9** : H/A/O/F/S/M/W/K/T/N/R/C |
| Love Factory | **9** : H/A/O/F/S/M/W/K/T/N/R/C |
| Chaos Factory | **9** : H/A/O/F/S/M/W/K/T/N/R/C |

## Cas particuliers à conserver sur le site

- **Worfinery** : Trooper, escouade de troopers et moissonneuse y sont disponibles dès que ce bâtiment existe, sans prérequis ou amélioration supplémentaire propres à ces unités. Le bâtiment garde ses propres conditions de construction. Cela peut avancer les troopers d’Atreides au niveau 6 dans les mods Tornie.

- **IX chez Wildspade** : son niveau configuré est 1, mais la chaîne demande un Spatioport de niveau 6. Le déblocage normal est donc 6. Les véhicules qui réclament IX peuvent arriver à ce niveau.

- **Carryall chimique** : les règles de production l’autorisent chez Wildspade dans les mods Tornie, et chez Atreides dans Tornie/Tornie Lite. Il exige IX et l’usine high-tech améliorée deux fois. **La configuration actuelle d’Atreides ne permet normalement qu’une amélioration de cette usine : son accès au Carryall chimique est donc bloqué sans modification ou bâtiment déjà amélioré.** Wildspade le débloque normalement au niveau 6 dans Tornie, et au niveau 7 dans Jericho.

- **Spatioport** : à partir de son niveau de construction (6), il peut proposer des unités selon le stock CHOAM sans appliquer les niveaux/améliorations de leur usine habituelle. Le Carryall chimique y est exclu, ainsi que l’ornithoptère en campagne. Ce relevé n’invente pas un stock garanti pour chaque unité.

- **Palais** : niveau 8. Les saboteurs viennent de son pouvoir chez Ordos/Mercenaires ; Atreides/Fremen obtiennent des renforts Fremen. Certains autres pouvoirs apportent des véhicules ou des unités aléatoires.

- **Love Factory / Chaos Factory** : niveau 9 dans les mods Tornie. Ce sont des voies de livraison/offres supplémentaires ; leurs sélections et stocks ne garantissent pas chaque unité. Small/Medium/Heavy/Support Delivery sont des commandes de livraison, pas quatre nouveaux types d’unités combattantes.

- **Frégate** : transporte les achats du Spatioport. **Ver des sables** : dépend des cartes et paramètres. Ils n’ont pas de chaîne d’amélioration d’usine à lister.

- **Capture** : une usine capturée peut donner accès à la technologie de sa faction d’origine et modifier la liste disponible. Les tableaux décrivent une base construite par sa propre faction.

- **Clés en double** : le moteur retient la première occurrence. Cela concerne notamment Builder(C) dans Tornie et Builder(T) dans Jericho pour certains chars IX. Les tableaux reflètent cette lecture effective ; les occurrences ajoutées plus bas dans les fichiers ne remplacent pas les premières.

## Sources

Commit : `6182a6a0861a9ef6bb60985a3833211c2d4cf2e2`. Aucune modification du jeu n’a été effectuée pour produire ce relevé.

Fichiers : `config/ObjectData.ini.default`, `mods/Tornie/ObjectData.ini`, `mods/TornieLite/ObjectData.ini`, `mods/Jericho/ObjectData.ini`, `src/ObjectData.cpp`, `src/structures/BuilderBase.cpp`, `src/globals.cpp`, `src/structures/StarPort.cpp`, `src/structures/Palace.cpp`, `src/structures/LoveFactory.cpp`, `src/structures/ChaosFactory.cpp`.

## Escouades de cinq — 1.0.533

Caserne : niveau 4, deuxième amélioration, sans IX. Worfinery : niveau 7, première amélioration et IX. Les commandes de trois restent disponibles. Prix par maison d’origine du bâtiment :

| Mod | Maison | 5 soldats | 5 troopers |
|---|---|---:|---:|
| Tornie | Harkonnen | 180 | 300 |
| Tornie | Atreides | 180 | 300 |
| Tornie | Ordos | 180 | 300 |
| Tornie | Fremen | 180 | 375 |
| Tornie | Sardaukar | 180 | 450 |
| Tornie | Mercenaires | 180 | 300 |
| Tornie | Neutres | 180 | 300 |
| Tornie | Rebelles | 180 | 300 |
| Tornie | Corruptique | 180 | 225 |
| Tornie | Wildspade | 180 | 300 |
| Tornie | Kleshmersh | 180 | 600 |
| Tornie | Tharpique | 180 | 300 |
| TornieLite | Harkonnen | 180 | 300 |
| TornieLite | Atreides | 180 | 300 |
| TornieLite | Ordos | 180 | 300 |
| TornieLite | Fremen | 180 | 375 |
| TornieLite | Sardaukar | 180 | 450 |
| TornieLite | Mercenaires | 180 | 300 |
| Jericho | Harkonnen | 180 | 300 |
| Jericho | Atreides | 180 | 300 |
| Jericho | Ordos | 180 | 300 |
| Jericho | Fremen | 180 | 375 |
| Jericho | Sardaukar | 180 | 450 |
| Jericho | Mercenaires | 180 | 300 |
| Jericho | Wildspade | 180 | 300 |
| Jericho | Kleshmersh | 180 | 600 |
| Jericho | Tharpique | 180 | 300 |
| Jericho | Neutres | 180 | 300 |
| Jericho | Rebelles | 180 | 300 |
| Jericho | Corruptique | 180 | 225 |
