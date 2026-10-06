# Dune Legacy Tornie 1.0.533

Previous automated checks are recorded in the supplied report. The final Fremen banner fix has been compiled; the full test suite was not rerun at Tornie’s request.

## Vanilla Kleshmersh campaign

The 22 scenarios derive from the original Dune II Harkonnen campaign: player Harkonnen → Kleshmersh, Atreides → Harkonnen, Ordos → Sardaukar, Sardaukar → Mercenary. Original terrain, positions, teams, reinforcements, economy and objectives are preserved.

In this campaign, Harkonnen is dark green, Sardaukar fuchsia, and Mercenary turquoise. Mercenary keeps its Vanilla technology. Enemy palaces choose one random classic power each recharge: missile, Fremen or saboteur. The saboteur uses the normal Ordos/Mercenary behavior.

## Additional squads in Tornie, Tornie Lite and Jericho

- **5 Soldiers**: Barracks, technology level 4, second upgrade, no IX. First order, alongside the existing three-unit order.
- **5 Troopers**: Worfinery only, technology level 7, upgrade and IX. First order, alongside the existing three-unit order.
- **Price**: three times the building’s original-house single-unit price, including captured buildings.
- **Map editor**: the custom section starts with Soldiers, then Troopers. Tiles use the Infantry/Troopers Squad sprites with the blue star. Each placement creates five units in a game; map and reinforcement saves preserve the group.

The conditions apply to all houses that own the building. Normal construction availability is preserved; a house unable to build Barracks can use a supplied or captured one.

## Three new achievements

- **Missile Barrage**: launch at least three palace missiles in one match.
- **Blue Harvest**: collect blue spice with a Harvester.
- **Purple Harvest**: collect purple spice with a Harvester.

The catalogue contains 46 achievements. Missile progress survives saved games.

## Local use

Extract the game ZIP and launch dunelegacy.exe. Start a new game for the new orders; older saves retain their stored rules. New 1.0.533 saves require this game version.

The French and English site includes the new requirements and house prices, plus 106 PNGs including the added Vanilla illustrations. Website: https://torneko.github.io/dunelegacy-tornie/ — Game: https://github.com/Torneko/dunelegacy-tornie/releases/tag/v1.0.533

The editor unit list supports the mouse wheel and a vertical scrollbar when it exceeds the available height, keeping the last units accessible.

The editor’s **Sabotage** mode makes Soldiers and Troopers, including squads, hunt enemy buildings for capture. In game, the **Sabotage** button appears below **Retreat**; mixed selections apply it only to capturing infantry. Allied and noncapturable buildings are ignored, and lost targets are replaced. Normal capture rules apply: a red-health building changes ownership; otherwise the infantry damages it and is consumed.

The Fremen banner is restored in the house-choice confirmation. Its original artwork is preserved; the planet background now uses a compatible image format.
