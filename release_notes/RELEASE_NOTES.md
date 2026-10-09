# Dune Legacy Tornie 1.0.537

In the host lobby, **Load a save** opens **Manual** (`mpsave/`, `.dls` files) and **Automatic** (`coop/`, `.dls` checkpoints) tabs. The save must be a co-op campaign for the active mod. Loading closes the old lobby and opens a new one for the saved mission; the guest must rejoin. Cancelling or selecting an incompatible file keeps the current lobby. Automatic resume through the `.ini` progress record remains available.

Optional allied control (off by default), selectable enemy AI type/difficulty and an AI guest, separate bases and funds, guest intro WOR, enemy faction/color collision handling, French/English How to Play including Jericho Lite, and refined Wildspade eye/mouth rendering. See `docs/COOP-CAMPAIGN-EN.md` and `docs/COOP-CAMPAIGN.md`.

Both PCs require 1.0.537 and protocol 6. Save version 9828 protects the new cooperative options from older executables; old saves remain readable. Tornie has tried co-op on two PCs and confirmed loading saves from the lobby. Automated Windows/Linux checks cover the new options; extended playtesting and the Mint 22.3/kernel 7.0 compatibility check remain open.

---

# Dune Legacy Tornie 1.0.536

Version 1.0.536. Co-op is available for playtesting; a human match between two computers remains unconfirmed.

- Co-op now follows the original campaign of the host’s faction: nine stages, preserved terrain and opponents, and a partner with an MCV near the Construction Yard and matching starting forces. Harvest objectives are shared. A live two-computer match remains unconfirmed.
- Separate co-op progress/checkpoints, resuming with new participant names and explicit readiness between missions. Lobby choices are locked during launch; a disconnected partner cancels the countdown. Messages from an earlier mission are discarded.
- Easy Mode is off by default and exclusive to solo campaigns: begin at mission 2 with 500 extra credits for the player's starting house; its unit/building purchases cost 25 fewer credits, minimum 1, throughout the campaign.
- Jericho Lite: six Tornie Lite factions and technology, all four Jericho spice families enabled by default with the same colors and effects, including in the editor and generator. Its six opening missions copy Tornie Lite’s terrain, economy, structures and forces; only the opponents follow Jericho Lite’s distinct plan.
- Tornie and Jericho intros include five extra dispersed enemies, bringing the total to 17 without an initial rush. Two bonus Tanks, nine Troopers and two Special Unit Spawns remain.
- Ornithopter attack orders continue hunting after their target disappears. Wildspade Ornithopters cost 550 credits.
- Builder price contrast panels are removed. Raider Trike and Rocket Trike prices use black numbers directly on their light portraits, with no background or border; other prices retain their usual color. Long descriptions in the Mods menu remain wrapped. Cat mentat eye/mouth overlays are slightly darker.
- 47 achievements: Chaos Conqueror requires completing a solo campaign played with Chaos Mode from its start. Co-op progress does not complete a faction's solo campaign.
- Continuing an older campaign adds opponents required by the next map, including Wildspade's third final-mission opponent.

For co-op playtesting, use this same build and mod on both computers with different player names. Open Co-op Campaign, create/join a lobby and select factions. Check independent bases, victory followed by both players choosing Ready, and resuming progress. Local transport checks are complete; a human match between two computers remains pending.

Extract the entire Windows ZIP and run `dunelegacy.exe`. The bilingual website is updated for this version and also supplied as an offline ZIP.
