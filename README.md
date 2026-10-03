# Dune Legacy Tornie

**Dune Legacy Tornie** is an independent Dune Legacy-derived real-time strategy project focused on classic Dune II gameplay plus Tornie's engine, faction, campaign, balance, graphics, audio and editor extensions.

This repository is intentionally separated from **DuneCity** and **Dune2R**. The former DuneCity city-building simulation is not part of this project.

Project repository: https://github.com/Torneko/dunelegacy-tornie

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
- Tornie mod loading through `mods/Tornie/`.
- Custom units including Rocket Trike, Flame Tank, Elite Launcher and Elite Siege Tank.
- Advanced Windtrap variants, Worfinery, Tech Center and Scoutpost.
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

## Original Dune II data

Dune Legacy Tornie loads Dune II game data from its data paths, together with `LEGACY.PAK` and the Tornie assets.

## License and credits

The project remains distributed under the GNU General Public License version 2 or later, following the licensing of its Dune Legacy code base.

Credits remain due to the original Dune Legacy contributors, Westwood Studios for Dune II, and all contributors whose work remains in this derived codebase.

**Tornie / Tornie Panther** maintains the Tornie-specific direction, assets and gameplay changes in this project.

The retained code also includes contributions from the DuneCity development history. Its original authorship and license notices remain in the source history and credits.

## Optional Discord presence

Set `DUNELEGACY_DISCORD_APP_ID` to the application ID of your own Discord application to enable Rich Presence. Without it, Rich Presence stays disabled. This edition does not use the former project's Discord application.
