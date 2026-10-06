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

22 scenarios per campaign: the original terrain and three-route region progression remain. The third opponent replaces the original imperial/Sardaukar role in the final scenario. Player credits, quota, buildings, other units, orders and coordinates are preserved. Human Soldier/Infantry starting markers are removed only from scenario 1. Authored special ground vehicles become `Special` markers, resolved by the existing faction pool and deterministic game RNG.

22 scénarios par campagne : terrains et progression des régions conservés. Le troisième adversaire remplace le rôle impérial/Sardaukar dans la finale. Crédits, quota, bâtiments, autres unités, ordres et positions conservés. Seuls les marqueurs Soldier/Infantry du joueur sont retirés du scénario 1. Les véhicules terrestres spéciaux deviennent des marqueurs `Special`, résolus par le système déterministe existant.

The two opponent sets for a given player are disjoint: six different opponents across the two mods. No fourth opponent is introduced. Region owners and localized briefing names follow the same plan. Vanilla and Tornie Lite campaigns are unchanged. Existing saved games keep their stored scenario; start a new campaign to use the rebuilt maps.

Pour une même faction, les deux mods offrent six adversaires différents. Aucun quatrième adversaire. Régions et noms dans les briefings suivent le même plan. Campagnes Vanilla et Tornie Lite conservées. Les sauvegardes existantes gardent leur scénario ; commence une nouvelle campagne pour utiliser les nouvelles cartes.

## Maintenance

Source: `config/CampaignPlans.json`. Runtime briefing adapters: `mods/{Tornie,Jericho}/data/CampaignPlan.ini`. Run `python scripts/check-campaign-regions.py`. The one-shot `scripts/rebuild-campaigns.py` deliberately reconstructs from immutable `v1.0.534`, not from a previously remapped output.
