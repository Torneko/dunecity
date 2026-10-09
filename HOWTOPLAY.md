# Playing Dune Legacy Tornie

In the host lobby, **Load a save** opens **Manual** (`mpsave/`, `.dls` files) and **Automatic** (`coop/`, `.dls` checkpoints) tabs. The save must be a co-op campaign for the active mod. Loading closes the old lobby and opens a new one for the saved mission; the guest must rejoin. Cancelling or selecting an incompatible file keeps the current lobby. Automatic resume through the `.ini` progress record remains available.

Choose a campaign or start a skirmish through **Single Player -> Custom Game**. The main menu also includes multiplayer, the map editor and the mod selector.

Build your base on rock. A Construction Yard, Wind Traps and a Refinery form the starting economy. Harvesters collect spice, Silos increase storage, and production facilities supply your army. Protect your harvesters, keep enough power for your base, and upgrade your buildings to unlock more technology.

The **Mods** menu switches between Vanilla, **Tornie**, **Tornie Lite**, **Jericho** and **Jericho Lite**. Vanilla provides nine factions, Tornie/Jericho twelve, and both Lite editions six. Jericho Lite combines Tornie Lite technology with Jericho spice. The active mod and game version appear in the main menu.

**Co-op campaign** follows the host faction's original campaign over nine shared stages. Join over LAN/Internet or choose an AI guest. Both allies may choose the same faction, with separate ownership and funds. The host can choose enemy AI type/difficulty and enable **Allied control** (off by default). New intros give the guest matching forces, an extra deployable MCV and a nearby WOR. Both PCs require version 1.0.538 and network protocol 7. Easy Mode and Enemy forces +5 are off by default. Host reinforcements also reach the second ally. See the [English guide](docs/COOP-CAMPAIGN-EN.md) or [guide français](docs/COOP-CAMPAIGN.md).

The former urban simulation has been removed. Its two scenario maps are retained in the original source branch; the supplied maps in this edition use RTS units and structures.

User settings are stored in a separate Dune Legacy Tornie profile. Saves containing the removed urban simulation are rejected; historical RTS save-format identifiers and reserved object IDs are retained for compatibility.

Open **Achievements / Hauts faits** in the main menu for 67 offline awards and
cumulative statistics. Unlock messages appear in the news ticker during play.
Progress is stored in `achievements.ini` beside your user configuration. Keep that
file with your profile when transferring saves. Captures are allowed for Pacifism;
the consumed infantry still counts as a loss for No Casualties. Replays, cheat mode
and single-player immortality do not grant achievements. Old saves remain playable,
but unknown earlier actions cannot qualify for no-destruction or no-loss awards.

Source and issues: [Torneko/dunelegacy-tornie](https://github.com/Torneko/dunelegacy-tornie).

The editor’s **Sabotage** mode makes Soldiers and Troopers, including squads, hunt enemy buildings for capture. In game, the **Sabotage** button appears below **Retreat**; mixed selections apply it only to capturing infantry. Allied and noncapturable buildings are ignored, and lost targets are replaced. Normal capture rules apply: a red-health building changes ownership; otherwise the infantry damages it and is consumed.
