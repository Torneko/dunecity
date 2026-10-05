# Local achievements

Dune Legacy Tornie provides 43 offline achievements. Open **Achievements**
or **Hauts faits** from the main menu to view locked awards and cumulative stats.
Unlocks use the existing in-game news ticker. Steam is not required.

## Eligibility and rules

Campaigns, skirmishes, custom games and multiplayer use events for the local
player's house. Multiplayer achievements observe deterministic simulation events;
their profile writes and notifications are local and never enter the command queue,
simulation random generator or network checksums. Shared-house players observe that
house's aggregate actions. Campaign achievements require campaign mode.
Replays, cheat sessions and single-player immortality do not grant progression.
Leaving a match grants neither a victory nor a defeat.

- Pacifism checks attributed enemy destruction. Captures, allied attacks and worm
  attacks do not count as player kills. True Pacifist additionally checks damage.
- No Mercy requires winning campaign mission 1 with every opposing unit and
  structure gone. Harvesters, carryalls and walls still count as remaining forces.
- Roadkill requires five infantry crushed by one vehicle within two simulation
  seconds, including the boundary. Vehicle counts cannot be combined.
- True Colors compares the resolved initial color against the chosen faction's
  standard color, and requires a custom or multiplayer victory.
- Campaign completion means winning final mission 22. Master of Arrakis requires
  the eight specified base campaigns. House Collector requires victories with all
  twelve named factions, including Wildspade, Kleshmersh, Tharpique and Corruptique.
  Victories from different mods accumulate by faction identity. Repeated victories
  with one faction do not substitute for another. Completing any faction's campaign
  grants Conquer Arrakis. Previously earned House Collector awards remain unlocked.
- No Casualties counts consumed capture infantry, as well as destroyed units.
  Healthy MCV deployment and departing logistics aircraft are not losses.
- Worm Hunter credits player-attributed half-health defeats, including retreat with respawning enabled.
- Spice credits count refinery deposits, not starting funds or captured credits.
  Red/green awards observe actual harvester collection before the tile changes.
- Flame Master requires 50 cumulative Flame Tank kills. Tornie Arsenal requires
  three distinct produced exclusive types in one game. Spice Collector requires
  two distinct harvested types. These provisional thresholds are configurable.
- Against the Odds requires a custom/multiplayer win against at least one opposing
  Hard or Brutal AI; an allied AI does not qualify. Factory production counts units;
  Starport purchases do not count as production.

## Persistence and save compatibility

`achievements.ini` lives beside the user's `Dune Legacy.ini`, in the independent
Tornie profile. It stores lifetime statistics, unlock timestamps, campaign flags,
per-run high water marks and checkpoint history. Writes replace the file atomically.
An unknown/newer or damaged profile is preserved and the window reports the issue.

Game save streams, format versions and item IDs are unchanged. A checkpoint is
stored separately in the local profile, keyed by the exact save-file digest. When
that save is loaded, match conditions and simulation-cycle crushing windows resume.
The run's high water marks prevent the same events from being counted again when
replaying a checkpoint. Keep this profile when moving saves to another machine.

Older saves, or saves received without the associated local profile, still load.
Their unknown prior history prevents Pacifism, True Pacifist and No Casualties from
being awarded for that resumed match. Other applicable events and wins still count;
pre-load refined spice is not imported into new statistics. Awards and lifetime
totals already earned survive loading an earlier save.

## Extending the catalog

`config/Achievements.ini` defines stable IDs, English/French names and descriptions,
rule, statistic, threshold and `Secret`. Definitions override the compiled fallback
catalog; additional IDs using an existing rule can be added. `Secret=true` displays
`???` and a hidden description until unlocked. The default 43 awards are visible.
Keep the fallback in `include/Achievements/AchievementDefaults.h` synchronized when
changing built-in definitions, so a missing external catalog keeps the same rules.

`AchievementManager` owns all rules and profile state. `AchievementEvents` translates
engine events without changing their behavior. Adding a new statistic or rule belongs
there rather than in `Game.cpp`. Statistics remain available for future catalog entries.

## Validation

Build `dunelegacy` and `dunelegacy_tests`, then run CTest. Test-enabled engine builds
also support `--verify-achievements`, which includes `--verify-mods`. Use isolated
`DUNELEGACY_USER_DIR` (Windows) or `XDG_CONFIG_HOME` (Linux), SDL dummy audio/video,
and `DUNELEGACY_SMOKE_DIR` pointing to an existing output folder.
The integration check exercises real captures, final-building victory, damage
attribution, crushing, MCV deployment, worm removal, harvesting, save/reload and SDL
window rendering. Optional `DUNELEGACY_OLD_SAVE` checks a previous-version save.

## 1.0.531 additions

- `RULER_OF_ARRAKIS`: win a campaign mission with its displayed final score >= 1000.
  The profile stores `BestCampaignScore`, never the sum of repeated score screens.
- `WILDSPADE_COMMANDER`, `KLESHMERSH_COMMANDER`, `THARPIQUE_COMMANDER`,
  `CORRUPTIQUE_COMMANDER`: complete the named campaign in any supported edition.
- `JERICHO_MASTER`: complete all 12 named campaigns specifically in Jericho.
  Campaign completion is keyed by both mod and faction; Tornie victories do not substitute.
- `WORM_HUNTER`: a worm defeated at its half-health retreat threshold counts when
  the last damaging source belongs to the local player, including respawning and
  same-house neutral worms. AI damage and ordinary full-health burrowing do not count.

Old profile entries/checkpoints and game save formats are preserved. Legacy custom
house config adapters remain readable; the game roster exposes named factions.
