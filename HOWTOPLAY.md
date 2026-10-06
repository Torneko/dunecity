# Playing Dune Legacy Tornie

Choose a campaign or start a skirmish through **Single Player -> Custom Game**. The main menu also includes multiplayer, the map editor and the mod selector.

Build your base on rock. A Construction Yard, Wind Traps and a Refinery form the starting economy. Harvesters collect spice, Silos increase storage, and production facilities supply your army. Protect your harvesters, keep enough power for your base, and upgrade your buildings to unlock more technology.

The **Mods** menu switches between Vanilla, **Tornie**, **Tornie Lite** and **Jericho**. Tornie Lite provides six campaigns; Tornie and Jericho provide nine. Jericho includes Wildspade, Kleshmersh and Tharpique. The active mod and game version appear in the main menu.

The former urban simulation has been removed. Its two scenario maps are retained in the original source branch; the supplied maps in this edition use RTS units and structures.

User settings are stored in a separate Dune Legacy Tornie profile. Saves containing the removed urban simulation are rejected; historical RTS save-format identifiers and reserved object IDs are retained for compatibility.

Open **Achievements / Hauts faits** in the main menu for 46 offline awards and
cumulative statistics. Unlock messages appear in the news ticker during play.
Progress is stored in `achievements.ini` beside your user configuration. Keep that
file with your profile when transferring saves. Captures are allowed for Pacifism;
the consumed infantry still counts as a loss for No Casualties. Replays, cheat mode
and single-player immortality do not grant achievements. Old saves remain playable,
but unknown earlier actions cannot qualify for no-destruction or no-loss awards.

Source and issues: [Torneko/dunelegacy-tornie](https://github.com/Torneko/dunelegacy-tornie).

The editor’s **Sabotage** mode makes Soldiers and Troopers, including squads, hunt enemy buildings for capture. In game, the **Sabotage** button appears below **Retreat**; mixed selections apply it only to capturing infantry. Allied and noncapturable buildings are ignored, and lost targets are replaced. Normal capture rules apply: a red-health building changes ownership; otherwise the infantry damages it and is consumed.
