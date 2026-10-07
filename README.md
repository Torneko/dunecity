# Dune Legacy Tornie

**Dune Legacy Tornie** is an independent Dune Legacy-derived real-time strategy project focused on classic Dune II gameplay plus Tornie's engine, faction, campaign, balance, graphics, audio and editor extensions.

This repository is intentionally separated from **DuneCity** and **Dune2R**. The former DuneCity city-building simulation is not part of this project.

Project repository: https://github.com/Torneko/dunelegacy-tornie

Game documentation: [Français](https://torneko.github.io/dunelegacy-tornie/) · [English](https://torneko.github.io/dunelegacy-tornie/index-en.html). Keep both languages aligned with each game update; see [website maintenance](docs/WEBSITE.md).

Version **[1.0.536](https://github.com/Torneko/dunelegacy-tornie/releases/tag/v1.0.536)** adds cooperative campaigns, Easy Mode and Jericho Lite. Windows compilation and automated checks have passed locally. A live cooperative match between two computers still needs validation; co-op is included for playtesting.

The main menu now provides a [common cooperative campaign](docs/COOP-CAMPAIGN-EN.md) ([guide français](docs/COOP-CAMPAIGN.md)) over LAN or Internet. Two players each control their own base and army on the same team, and may choose the same faction. Nine shared missions have their own progress and checkpoints, independent of solo campaigns. New campaigns follow the original campaign of the host’s faction, with 405 co-op variants from 45 source campaigns. The guest starts with matching units and credits plus an MCV near the host’s Construction Yard; harvest objectives use a shared quota. Available factions: nine in Vanilla, twelve in Tornie/Jericho and six in each Lite edition. Older co-op sessions retain their previous layouts. Both computers need the **same 1.0.536 build**, using network protocol **5**.

Other 1.0.536 changes:

- **Jericho Lite** uses Tornie Lite's six factions and technology, with all four Jericho spice families enabled by default and the same colors and effects, including in the editor and map generator. Its six opening missions copy Tornie Lite's terrain, economy, structures and forces, with opponents remapped to Jericho Lite's distinct solo campaign plan.
- **Easy Mode**, off by default and exclusive to solo campaigns, starts a new campaign at mission 2. Only the player's house receives 500 extra credits at this start and pays 25 fewer credits for unit/building purchases throughout the campaign, with a minimum price of 1. Opponents, custom games and co-op retain normal prices.
- Human-issued Ornithopter attack orders continue hunting after the initial target is gone. Wildspade Ornithopters cost **550** in Tornie/Jericho.
- Each full-mod solo intro adds five dispersed border sentries for **17 enemies**, retaining the two bonus Tanks, nine Troopers and two Special spawns; no initial Hunt rush is added.
- Builder price contrast panels are removed. Raider Trike and Rocket Trike prices use black numbers directly on their light portraits, with no background or border; other prices retain their usual color. Long Mods-menu descriptions wrap, and Wildspade's original eye/mouth overlays are slightly darker while preserving their dimensions and alpha.
- **Chaos Conqueror / Maître du chaos** brings the catalogue to **47** achievements. It requires completing a solo campaign started with Chaos Mode. Commander and solo campaign completion awards remain tied to solo; the separate co-op campaign does not complete a faction's solo campaign.

Version **1.0.535** includes [rebuilt Tornie/Jericho campaigns](docs/CAMPAIGNS.md): twelve campaigns per mod, three balanced opponent roles, disjoint plans between mods and preserved terrain/economy. Its revised first missions give every faction **two bonus Tanks, three Troopers squads (nine infantry) and two faction-specific Special spawns**. Other original vehicles remain. Each intro has twelve enemies: the six extra enemies are removed, enemy Special markers become ordinary Tanks and initial Hunt orders become Area Guard. Regions and briefings follow the same plan. [Wildspade’s cat mentat](docs/WILDSPADE-MENTAT.md) uses Tornie’s original background, separately anchored eye/mouth animations and an overlapping shoulder foreground. Start a new campaign to receive the revised opening forces.

Version **1.0.534** adds optional [Chaos Mode](docs/CHAOS-MODE.md): one same-level donor faction per production/defence building type and Palace, for campaigns, custom games and multiplayer in the Tornie mods. The draw is deterministic and saved; Vanilla keeps its normal rules. Wildspade gets IX at technology 7, Chemical Carryall at 6 after the first Hightech upgrade without IX, and Ornithopter at 7 after the second upgrade with IX. The original Fremen banner is composited in the house confirmation screen and refreshed on mod changes. The bilingual website uses the original game portraits for the 2×3 and 3×2 windtrap icons, with their sprites and editor previews in the gallery.

Version **1.0.525** incorporates the corrected **DuneCity Tornie 1.0.524-26**
source at `80799faba1b66c64887286446f5ed6b0df985226` from
`Torneko/dunecity-tornie`. Its RTS changes, campaigns and mod assets are retained;
the standalone identity and removal of city simulation are applied on top.

The supplied maps contain small, reproducible pockets of Tornie red, green, purple and blue spice.
About 10% of normal spice tiles are replaced, preserving thin/thick terrain, blooms,
rock, units, buildings and scenario coordinates. Version-2 maps store the new terrain
directly; the 25 seed-based legacy scenarios opt in through `SpiceVariantPercent`
and `SpiceVariantSeed` and `SpiceVariantVersion=2` in `[MAP]`. Unmarked maps keep their original spice.
The optional loader rule is capped at 15%. Run
`python scripts/add-spice-variants.py --check` to validate the supplied maps;
`--write` applies the conversion to new, unmarked maps without converting existing
maps twice.

Version **1.0.533** rebuilds vanilla Kleshmersh from the original Harkonnen campaign
in `SCENARIO.PAK`: player Harkonnen becomes Kleshmersh, Atreides becomes Harkonnen,
Ordos becomes Sardaukar, and Sardaukar becomes Mercenary. Original terrain, coordinates,
starting economy, teams, reinforcements and objectives are preserved. Only this campaign
uses dark-green Harkonnen, fuchsia Sardaukar and turquoise Mercenary. Mercenary retains
its full vanilla object data. Enemy palaces randomly select one classic power (missile,
Fremen or saboteur) whenever ready, using the deterministic game RNG and existing cooldowns.
Their encoded ready power and colors survive save/load; other campaigns keep their powers.

Tornie, Jericho and Tornie Lite gain a separate **Troopers (5)** purchase at the top of
the Worfinery list. At technology level 7, owning IX allows one Worfinery upgrade.
The upgraded building produces five ordinary Troopers per order for three times the
original house's single-Trooper price; the three-Trooper order remains available.
The Barracks also gains a separate first **Soldiers (5)** order after its second upgrade
at technology level 4, without IX, for three times the original house's single-Soldier
price. Its three-Soldier order remains available wherever it was previously supported.
Both squads lead the editor custom-unit section, Soldiers then Troopers, using their
Infantry/Troopers Squad tiles with the existing blue star. One marker expands into
five ordinary units; map saves and reinforcement orders preserve the group.
The editor unit list supports the mouse wheel and a vertical scrollbar when it exceeds the available height, keeping the last units accessible.
WOR, Starport and random factory offers cannot produce these new orders. The purchase ID
is appended as 74 (Troopers) and 75 (Soldiers), with save version 9825: older saves remain readable with their stored
rules, while older executables reject new saves. Start a new game for the new purchase.

Version 1.0.533 added three awards, bringing the catalogue at that time to 46: **Missile Barrage** (three local-player palace
missiles in one match), **Blue Harvest** and **Purple Harvest** (actual harvester collection).
Achievement checkpoints preserve the missile count and spice awards across game saves.

## Main goals

The local 1.0.536 project includes 47 internal offline achievements, cumulative local
statistics, a main-menu achievement window and discreet unlock messages.
Campaign, custom and multiplayer events share a central manager. Achievement
profiles remain separate from mission saves. See [achievement rules and persistence](docs/ACHIEVEMENTS.md).

Version **1.0.527** corrects Chaos Factory graphics and its map-editor preview
after switching mods, with render checks across faction colors and zoom levels.

Version **1.0.528** restores Jericho faction colors on startup and resets the
editor to faction defaults after custom games. Editor palettes and map ownership
are checked across all four mods, including map save/load.

Version **1.0.529** includes normal, green, red, purple and blue spice in generated
fields and random blooms across Tornie, Tornie Lite and Jericho. The existing 20%
variant budget is shared equally between the four colors. The editor's load dialog
now lists supplied solo/multiplayer maps, personal maps and active-mod campaigns.

Version **1.0.530** fixes Windows builds with precompiled headers enabled:
the three UTF-8 achievement sources compile separately from the legacy-encoded
MSVC precompiled header, preserving their accented text.

Version **1.0.531** integrates twelve named campaign factions in Tornie and
Jericho. Vanilla adds brown Kleshmersh as its ninth selectable faction with
Neutral's tech tree and all 22 cloned campaign maps: Harkonnen in missions 1–10,
Sardaukar in 11–21 and Rebels in 22. Runtime/save IDs and legacy faction adapters
remain unchanged. Jericho Corruptique retains yellow and its supplied herald.

The campaign score achievement requires a displayed victorious score of at least
1000. Four faction commanders and Master of Jericho extend the catalog. Worm
Hunter counts player-attributed half-health defeats even with respawning enabled.
Purple spice heals harvesters by at most one HP per 1008 simulated milliseconds.
All 142 supplied maps use the four-color variant algorithm while unmarked/older
maps preserve their original rules. Campaign region files use canonical faction
ownership keys and CP850 translations; their routes are checked automatically.

Version **1.0.532** supplies Kleshmersh's own voice asset in vanilla, including
its selection name in the English, French and German interfaces. Jericho Neutral
and Rebels use their canonical banners rather than legacy Wildspade/Kleshmersh
aliases. House Collector counts victories with all twelve named factions; previous
statistics and unlocked awards are retained.

- Preserve the Dune Legacy RTS foundation.
- Keep Tornie-specific gameplay and quality-of-life improvements.
- Support the extended house roster, including Neutral and Rebels.
- Keep Tornie campaigns, custom units, structures, graphics, palettes and voices.
- Maintain Windows and Linux builds from the same CMake source tree.
- Keep the project independent so DuneCity and Dune2R can evolve separately.

## Included Tornie features

The exact set evolves with the project, but the current source includes Tornie additions such as:

- Extended house and campaign support.
- Neutral and Rebels support.
- Selectable Tornie, Tornie Lite, Jericho and Jericho Lite mods with complete bundled resources.
- Tornie Lite's six-house campaigns and Jericho's Wildspade, Kleshmersh and Tharpique factions.
- Custom units including Rocket Trike, Flame Tank, Elite Launcher and Elite Siege Tank.
- Advanced Windtrap variants, Worfinery, Tech Center, Scoutpost, Love Factory,
  Chaos Factory, Flamepost, Chemipost and chemical vehicles from the corrected base.
- Captured faction technology, French audio fixes and relocatable Linux resources.
- Custom house palettes, portraits, voices and campaign data.
- Updated map editor support for Tornie terrain and gameplay content.
- Multiplayer, save/load and AI work inherited from the Dune Legacy-derived engine.
- A separate nine-mission co-op campaign with two independent allied bases, including matching faction choices.

## What is not included

- DuneCity city simulation, city zones, roads, taxes, city overlays or Micropolis gameplay.
- Dune2R as a bundled mod or project.

Legacy numeric IDs and a few reserved save-format bytes may remain internally where required to avoid shifting later Tornie IDs or corrupting compatible save streams. They do not enable DuneCity gameplay.

## Building

### Windows

A CMake + vcpkg build is supported.

```powershell
cmake -B build "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release --parallel
```

The executable is:

```text
dunelegacy.exe
```

### Linux

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run:

```bash
./build/bin/dunelegacy
```

## Configuration

The main configuration template is:

```text
config/Dune Legacy.ini
```

User configuration is kept separately from old DuneCity configuration directories.

For an isolated Windows profile, set `DUNELEGACY_USER_DIR` to the desired
profile directory. Linux uses `XDG_CONFIG_HOME` and the `DuneLegacyTornie` subdirectory.

## Validation

Configure with `DUNELEGACY_BUILD_TESTS=ON` and the vcpkg `tests` feature (or an
installed Catch2 3), then run CTest. `python scripts/check-bundled-mods.py`
checks the campaign resources, presentation assets and exact payload checksums.
Test-enabled builds also accept `--verify-mods`: use an isolated profile,
`SDL_VIDEODRIVER=dummy`, `SDL_AUDIODRIVER=dummy`, `SDL_RENDER_DRIVER=software`,
and `DUNELEGACY_SMOKE_DIR` pointing to an existing temporary directory.
This checks repeated mod switches, all 528 Tornie/Jericho scenarios, the Wildspade
briefing/foreground and starting defence, new object creation and save/load.
It does not replace manual gameplay or network testing.

## Original Dune II data

Dune Legacy Tornie loads Dune II game data from its data paths, together with `LEGACY.PAK` and the Tornie assets.

## License and credits

The project remains distributed under the GNU General Public License version 2 or later, following the licensing of its Dune Legacy code base.

Credits remain due to the original Dune Legacy contributors, Westwood Studios for Dune II, and all contributors whose work remains in this derived codebase.

**Tornie / Tornie Panther** maintains the Tornie-specific direction, assets and gameplay changes in this project.

The retained code also includes contributions from the DuneCity development history. Its original authorship and license notices remain in the source history and credits.

## Optional Discord presence

Set `DUNELEGACY_DISCORD_APP_ID` to the application ID of your own Discord application to enable Rich Presence. Without it, Rich Presence stays disabled. This edition does not use the former project's Discord application.

The editor’s **Sabotage** mode makes Soldiers and Troopers, including squads, hunt enemy buildings for capture. In game, the **Sabotage** button appears below **Retreat**; mixed selections apply it only to capturing infantry. Allied and noncapturable buildings are ignored, and lost targets are replaced. Normal capture rules apply: a red-health building changes ownership; otherwise the infantry damages it and is consumed.

The Fremen banner is restored in the house-choice confirmation. Its original artwork is preserved; the planet background now uses a compatible image format.
