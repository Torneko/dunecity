# Dune Legacy Tornie 1.0.528

Fixes the Jericho house colors when launching with Jericho already selected.
Wildspade, Kleshmersh and Tharpique now receive their faction colors immediately,
as they already did after switching mods. The map editor also restores default
faction colors before creating its interface, regardless of previous custom-game
team colors. Map ownership, gameplay balance and save formats remain unchanged.

Real-engine regression checks reproduce the old startup mismatch and verify
faction mappings, interface colors and editor icon palettes at startup, after mod
switches, after custom-game colors and after map save/load. Tornie, Tornie Lite,
Jericho and vanilla are covered; Chaos Factory and achievement checks are retained.

## Previous release: 1.0.527

Fixes the Chaos Factory's Starport fallback in the map editor after a mod switch.
Its dedicated 3x2 preview is now rebuilt with the other mod-dependent structures.
The structure sprite loader also accepts its bundled RGBA PNG through the existing
game-palette/team-color pipeline instead of rejecting it. Original PNG assets,
balance, item IDs, achievement profiles and save formats are preserved.

The real-engine regression check verifies the preview and sprite atlas after
startup, mod switches, game initialization and save/load. It checks every campaign
faction slot at all three zooms, and compares the rendered editor preview pixel for
pixel with the active structure frame. The previous build reproduced the fallback
failure. Full achievement and campaign integration checks remain enabled.

## Previous release: 1.0.526

Adds 37 internal offline achievements, a main-menu window with cumulative statistics
and discreet news-ticker unlock notifications. Definitions and French/English text
are configurable in `config/Achievements.ini`; secret awards are supported.

Rules cover Pacifism, True Pacifist, captures, all-force elimination, same-vehicle
Roadkill, custom faction colors, eight campaigns, production, Palace powers, worms
and modded spice. Player damage attribution excludes allies and environmental kills.
Final-building capture is recorded before synchronous victory checks.

Progress is stored atomically in the independent user profile. Local save checkpoints
resume eligibility and avoid recounting the same events. Game save streams and
versions are unchanged, including loading previous-version RTS saves. Untracked
earlier save history cannot grant no-destruction or no-loss achievements.

Validation: 79 engine test cases (3507 assertions), plus real-engine event, window
rendering and save/reload integration. Test builds expose `--verify-achievements`;
Windows CI now checks both build and installed French/English packages.

See [complete rules and persistence](docs/ACHIEVEMENTS.md).

## Previous release: 1.0.525

Based on corrected DuneCity Tornie **1.0.524-26**, commit
`80799faba1b66c64887286446f5ed6b0df985226` from `Torneko/dunecity-tornie`.

The independent RTS now includes the corrected base's engine, campaign, unit,
structure, palette, voice, French audio and relocatable Linux resource improvements.
Tornie, Tornie Lite and Jericho are bundled and selectable with their complete payloads.
Captured faction technology and the strengthened Harkonnen opening missions are retained.

The project identity is Dune Legacy Tornie. The intro's bottom text and menu footer
use that name. The DuneCity first-launch activation popup, promotional panel and
city simulation are removed. Update checks use this project's repository and user
profiles are separate. Original source attribution and stable gameplay IDs remain.

PAK archive reads now protect the shared file cursor while graphics and sounds
load in parallel. This fixes an intermittent French startup failure reproduced
by a concurrent-read regression test. Windows CI also exercises the installed
package in both French and English.

The supplied RTS maps keep moderate, deterministic red and green spice pockets:
117 terrain maps and 25 legacy seed scenarios, about 10% of normal spice replaced.
Vanilla normalizes these variants to ordinary spice as in the corrected base.
Mod campaigns retain the corrected base's terrain and gameplay parameters.

Validation: Windows and Linux builds; 60 engine test cases (3384 assertions);
142 supplied maps; 2074 exact mod payload checksums; repeated activation,
78 opening/final campaign scenario loads and new-object save/load checks in English
and French using the actual Windows engine, including its installed package,
and SDL dummy drivers.
Manual visual playthroughs, long games, Android builds and live multiplayer are
not covered by these automated checks.
