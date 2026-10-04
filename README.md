# Dune Legacy Tornie

**Dune Legacy Tornie** is an independent Dune Legacy-derived real-time strategy project focused on classic Dune II gameplay plus Tornie's engine, faction, campaign, balance, graphics, audio and editor extensions.

This repository is intentionally separated from **DuneCity** and **Dune2R**. The former DuneCity city-building simulation is not part of this project.

Project repository: https://github.com/Torneko/dunelegacy-tornie

Version **1.0.525** incorporates the corrected **DuneCity Tornie 1.0.524-26**
source at `80799faba1b66c64887286446f5ed6b0df985226` from
`Torneko/dunecity-tornie`. Its RTS changes, campaigns and mod assets are retained;
the standalone identity and removal of city simulation are applied on top.

The supplied maps contain small, reproducible pockets of Tornie red and green spice.
About 10% of normal spice tiles are replaced, preserving thin/thick terrain, blooms,
rock, units, buildings and scenario coordinates. Version-2 maps store the new terrain
directly; the 25 seed-based legacy scenarios opt in through `SpiceVariantPercent`
and `SpiceVariantSeed` in `[MAP]`. Unmarked maps keep their original spice.
The optional loader rule is capped at 15%. Run
`python scripts/add-spice-variants.py --check` to validate the supplied maps;
`--write` applies the conversion to new, unmarked maps without converting existing
maps twice.

## Main goals

Version **1.0.526** adds 37 internal offline achievements, cumulative local
statistics, a main-menu achievement window and discreet unlock messages.
Campaign, custom and multiplayer events share a central manager. Game save formats
remain unchanged. See [achievement rules and persistence](docs/ACHIEVEMENTS.md).

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
- Selectable Tornie, Tornie Lite and Jericho mods with complete bundled resources.
- Tornie Lite's six-house campaigns and Jericho's Wildspade, Kleshmersh and Tharpique factions.
- Custom units including Rocket Trike, Flame Tank, Elite Launcher and Elite Siege Tank.
- Advanced Windtrap variants, Worfinery, Tech Center, Scoutpost, Love Factory,
  Chaos Factory, Flamepost, Chemipost and chemical vehicles from the corrected base.
- Captured faction technology, French audio fixes and relocatable Linux resources.
- Custom house palettes, portraits, voices and campaign data.
- Updated map editor support for Tornie terrain and gameplay content.
- Multiplayer, save/load and AI work inherited from the Dune Legacy-derived engine.

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
This checks repeated mod switches, the opening/final scenarios of every campaign,
new object creation and save/load. It does not replace manual gameplay or network testing.

## Original Dune II data

Dune Legacy Tornie loads Dune II game data from its data paths, together with `LEGACY.PAK` and the Tornie assets.

## License and credits

The project remains distributed under the GNU General Public License version 2 or later, following the licensing of its Dune Legacy code base.

Credits remain due to the original Dune Legacy contributors, Westwood Studios for Dune II, and all contributors whose work remains in this derived codebase.

**Tornie / Tornie Panther** maintains the Tornie-specific direction, assets and gameplay changes in this project.

The retained code also includes contributions from the DuneCity development history. Its original authorship and license notices remain in the source history and credits.

## Optional Discord presence

Set `DUNELEGACY_DISCORD_APP_ID` to the application ID of your own Discord application to enable Rich Presence. Without it, Rich Presence stays disabled. This edition does not use the former project's Discord application.
