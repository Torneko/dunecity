# Dune Legacy Tornie 1.0.538

## Follow-up included in 1.0.538 — October 10, 2026

The published 1.0.538 packages are replaced by this patch without a version bump.
The **5 × 2 Doublefinery** has two unloading bays, 2,500 spice storage and two free
Harvesters, subject to unit limits. Ordos, Mercenary, Neutral and Corruptique can
build it at technology 7 with IX, Windtrap and Refinery in Tornie, Jericho and both
Lite editions; it can be captured. The final 80 × 224 sprite sheet is used without resizing.

The patch fixes random Palace missile targeting, bonus colors, editor button alignment
and star transparency, and names the five-person squads. Scoutpost, Flamepost and
Chemipost cannot be captured; upgrades follow captured-yard plans or the Chaos donor.

**Update both co-op PCs**: network protocol **9**, save format **9831**.
Earlier published saves remain readable. Prototype 9830 saves with a 4 × 2 Doublefinery
expand when the fifth column is free; otherwise loading is refused without overwriting
the map or original file.

Windows/Linux engine checks and rendering checks passed before the final sprite swap.
As requested by Tornie, game regression suites were not repeated after that swap;
compilation and package integrity were checked. Testing on the affected Mint 22.3 /
kernel 7.0 PC remains pending.

### Portrait and Worfinery correction

The six supplied PNGs distinguish single Soldiers/Troopers, squads of 3 and
squads of 5. Only the Barracks/WOR/Worfinery production lists in Tornie, Jericho and their
Lite editions use these portraits; their price text is white. The website's
cards, technology views and detail pages show the matching variants.
The Worfinery displays the Harvester last. This display order preserves current
production and saved orders. Tornie tested co-op 1.0.538 on two PCs and confirmed
it works. Mint 22.3 / kernel 7.0 still needs testing on the target computer.

## Changes in 1.0.538

Two lobby options are off by default: **Easy Mode** and **Enemy forces +5**.
Easy Mode starts a new campaign at mission 2. Each ally, human or AI, gets 500 extra
credits at that mission's start only. Their unit and building purchases cost 25 credits
less (minimum 1) throughout the campaign. Enemies receive no bonus or discount.

Enemy forces +5 adds five technology-appropriate units per present opponent in each
mission, near its base or army. Reinforcement-only opponents start on a map edge opposite
the main allied base. Extra units use Area Guard. Absent opponents receive no units.

Host reinforcements also reach the second ally with the same number, timing, relative
drop location and repeat rule. Unavailable units receive a faction-compatible equivalent.
Recorded checkpoint reinforcements and forces are restored without being added twice.

**Together on Arrakis / COOP_CONQUEROR** requires every common-campaign stage, with
a human or AI ally. Easy Mode may waive the intro. Unlocks stay local; each completed
session counts once. Solo campaign awards retain their requirements.

Both PCs require **1.0.538**, protocol **9**. New saves use **9831**; older saves remain
readable with their recorded forces and reinforcements. Start a new campaign or reach
the next mission for the new reinforcement behavior. Tornie confirmed two-PC co-op works. Mint 22.3 / kernel 7.0 still needs testing.

## Portraits, banners and achievements

New single/squad portraits apply only to Barracks/WOR/Worfinery production lists in Tornie, Jericho and their Lite editions. Vanilla, Starport and selected-unit portraits keep their images. Supplied PNGs are copied unchanged.

Wildspade/Tornie uses its new banner and dark purple. Rebels/Jericho uses its new banner and dark grey. Other factions and mods retain their variants.

The catalogue grows from 47 to 67 awards: a complete co-op campaign, Fuel Waste, three enemy buildings destroyed with one palace missile, scores of 1,500 / 2,000, a 10,000-point total, and cumulative-counter milestones. Friendly hits and nonlethal damage exclude Fuel Waste. All impacts of one missile are grouped; buildings count once. Won mission scores are deduplicated after reloading; unrecorded historical scores are not invented.
