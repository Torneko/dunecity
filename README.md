# Dune Legacy Tornie

**Dune Legacy Tornie** is an independent Dune Legacy-derived real-time strategy project focused on classic Dune II gameplay plus Tornie's engine, faction, campaign, balance, graphics, audio and editor extensions.

This repository is intentionally separated from **DuneCity** and **Dune2R**. The former DuneCity city-building simulation is not part of this project.

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

Dune Legacy Tornie requires original Dune II game data files. Those copyrighted game data files are not part of the source-code project and should be supplied separately by the user.

## License and credits

The project remains distributed under the GNU General Public License version 2 or later, following the licensing of its Dune Legacy code base.

Credits remain due to the original Dune Legacy contributors, Westwood Studios for Dune II, and all contributors whose work remains in this derived codebase.

**Tornie / Tornie Panther** maintains the Tornie-specific direction, assets and gameplay changes in this project.
