# Campaigns / Campagnes — 1.0.535

Two independent plans; one occurrence of each faction in each opponent role. / Deux plans indépendants ; chaque faction apparaît une fois dans chaque rôle.

## Tornie

| Campaign / Campagne | First / Premier | Second / Deuxième | Final / Final |
|---|---|---|---|
| Harkonnen | Atreides | Sardaukar | Rebels |
| Atreides | Ordos | Mercenary | Corruptique |
| Ordos | Fremen | Neutral | Wildspade |
| Fremen | Sardaukar | Rebels | Kleshmersh |
| Sardaukar | Mercenary | Corruptique | Tharpique |
| Mercenary | Neutral | Wildspade | Harkonnen |
| Neutral | Rebels | Kleshmersh | Atreides |
| Rebels | Corruptique | Tharpique | Ordos |
| Corruptique | Wildspade | Harkonnen | Fremen |
| Wildspade | Kleshmersh | Atreides | Sardaukar |
| Kleshmersh | Tharpique | Ordos | Mercenary |
| Tharpique | Harkonnen | Fremen | Neutral |

## Jericho

| Campaign / Campagne | First / Premier | Second / Deuxième | Final / Final |
|---|---|---|---|
| Harkonnen | Ordos | Mercenary | Wildspade |
| Atreides | Fremen | Neutral | Kleshmersh |
| Ordos | Sardaukar | Rebels | Tharpique |
| Fremen | Mercenary | Corruptique | Harkonnen |
| Sardaukar | Neutral | Wildspade | Atreides |
| Mercenary | Rebels | Kleshmersh | Ordos |
| Neutral | Corruptique | Tharpique | Fremen |
| Rebels | Wildspade | Harkonnen | Sardaukar |
| Corruptique | Kleshmersh | Atreides | Mercenary |
| Wildspade | Tharpique | Ordos | Neutral |
| Kleshmersh | Harkonnen | Fremen | Rebels |
| Tharpique | Atreides | Sardaukar | Corruptique |

## Preserved rules / Règles conservées

22 scenarios per campaign: the original terrain and three-route region progression remain. The third opponent replaces the original imperial/Sardaukar role in the final scenario. Player credits, quota and buildings remain. All 24 first missions have **two bonus Tanks, three Troopers orders (nine individual Troopers) and two Special orders**. Existing regular vehicles remain; the old Wildspade-only bonus is replaced, not stacked. Starting Soldier/Infantry markers remain absent. Special orders use the existing faction pool and deterministic game RNG. Later missions keep their original forces.

22 scénarios par campagne : terrains et progression des régions conservés. Le troisième adversaire remplace le rôle impérial/Sardaukar dans la finale. Crédits, quota et bâtiments conservés. Les 24 premières missions offrent **deux Tanks en bonus, trois escouades Troopers (neuf fantassins) et deux Special**. Les autres véhicules ordinaires sont conservés ; l’ancien bonus Wildspade est remplacé sans cumul. Aucun Soldier/Infantry initial. Les Special utilisent le tirage déterministe propre à la faction. Les forces des missions suivantes sont conservées.

Opening enemies are reduced from eighteen to twelve orders. The six added enemies are removed; remaining enemy Special markers become ordinary Tanks, and Hunt orders become Area Guard to avoid immediate map-wide pursuit. Ambush encounters remain. Original border entries are moved inside the playable map so all twelve enemies actually deploy. This targets moderate opening pressure; extended campaign balance still depends on playtesting.

Les ennemis des intros passent de dix-huit à douze ordres : retrait des six ennemis supplémentaires, Special ennemis remplacés par des Tanks ordinaires, et ordres Hunt remplacés par Area Guard pour limiter les attaques immédiates. Les embuscades sont conservées. Les entrées hors bordure sont replacées dans la carte pour que les douze ennemis apparaissent réellement. L’objectif est une pression initiale modérée ; l’équilibrage complet reste à apprécier en jeu.

The two opponent sets for a given player are disjoint: six different opponents across the two mods. No fourth opponent is introduced. Region owners and localized briefing names follow the same plan. Vanilla and Tornie Lite campaigns are unchanged. Existing saved games keep their stored scenario; start a new campaign to use the rebuilt maps.

Pour une même faction, les deux mods offrent six adversaires différents. Aucun quatrième adversaire. Régions et noms dans les briefings suivent le même plan. Campagnes Vanilla et Tornie Lite conservées. Les sauvegardes existantes gardent leur scénario ; commence une nouvelle campagne pour utiliser les nouvelles cartes.

## Maintenance

Source: `config/CampaignPlans.json`. Runtime briefing adapters: `mods/{Tornie,Jericho}/data/CampaignPlan.ini`. Run `python scripts/check-campaign-regions.py`. The one-shot `scripts/rebuild-campaigns.py` reconstructs from immutable `v1.0.534` and applies the revised intro policy. `scripts/tune-campaign-openings.py` can separately reapply only the openings from immutable original-release commit `fa2a5de3d23507f95332e243d060eeb419ce8fd5`; it is idempotent and preserves later missions. Do not use the replaced `v1.0.535` tag as its historical baseline.
