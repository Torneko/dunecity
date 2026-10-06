# Dune Legacy Tornie 1.0.532

This release integrates the named factions and campaigns, expanded offline achievements, spice changes and the latest voice and banner corrections.

## Factions and campaigns

- Tornie and Jericho offer twelve named factions and their campaigns: Atreides, Harkonnen, Ordos, Fremen, Sardaukar, Mercenary, Neutral, Rebels, Wildspade, Kleshmersh, Tharpique and Corruptique.
- Vanilla adds brown Kleshmersh as its ninth faction, with Neutral’s technology tree and the supplied brown banner. Its 22 missions clone Neutral’s campaign: Harkonnen in missions 1–10, Sardaukar in 11–21 and Rebels in the final mission.
- Vanilla Kleshmersh uses its own voice name, including faction selection and English deployment announcements.
- Jericho Neutral and Rebels use their correct banners. Corruptique is yellow with the supplied banner. Tornie Fremen use their new supplied banner.
- Wildspade faces three opponents: Atreides, Kleshmersh and Ordos.
- Region files were checked for routes, faction names, opponents and campaign transitions.

## Achievements and spice

- 43 local achievements. Ruler of Arrakis requires a winning campaign mission with a displayed final score of at least 1000.
- Campaign achievements cover Wildspade, Kleshmersh, Tharpique and Corruptique. Master of Jericho requires all twelve Jericho campaigns.
- House Collector requires victories with all twelve factions. Existing statistics and unlocked awards are retained.
- Worm Hunter counts a player-attributed worm defeat at its half-health retreat threshold, including when respawning is enabled.
- Purple spice heals Harvesters and Harvestanks by at most 1 HP per 1008 simulated milliseconds, while actually harvesting.
- The 142 supplied maps use small reproducible pockets of red, green, purple and blue spice in the mods. Generated fields and random blooms can also select all four variants. Vanilla retains normal spice.

## Installation and validation

Extract the entire Windows ZIP and launch dunelegacy.exe. Bundled mods update automatically on launch. User progression is stored in the independent profile.

Windows and Linux GitHub builds pass. Local checks include 87 unit tests, 3694 assertions and runs loading 118 opening/final campaign scenarios. The final Windows ZIP was downloaded and checked with fresh French and English profiles, including banners, voices, maps, achievements and older save loading.

[Published release and downloads](https://github.com/Torneko/dunelegacy-tornie/releases/tag/v1.0.532) · [GitHub build checks](https://github.com/Torneko/dunelegacy-tornie/actions/runs/37297912189)

Source commit: `6182a6a0861a9ef6bb60985a3833211c2d4cf2e2`. The source patch covers changes since public version 1.0.530.

Windows ZIP SHA-256:

```text
c83bcf3aabdee00e45921ad90edf8734b9b84608ca5468990470619bd04a1f1b
```
