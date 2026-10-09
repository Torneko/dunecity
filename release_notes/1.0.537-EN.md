# Dune Legacy Tornie 1.0.537

- In the host lobby, **Load a save** opens **Manual** (`mpsave/`, `.dls` files) and **Automatic** (`coop/`, `.dls` checkpoints) tabs. The save must be a co-op campaign for the active mod. Loading closes the old lobby and opens a new one for the saved mission; the guest must rejoin. Cancelling or selecting an incompatible file keeps the current lobby. Automatic resume through the `.ini` progress record remains available.
- The co-op lobby supports selecting each enemy AI's type and difficulty. Campaign AI remains the initial choice; qBot, Mentat, SmartBot and AI Player are also available.
- An AI may replace the second player, owning its separate base, army and credits on the host's team. Mission transitions no longer wait for an absent human guest.
- **Allied control**, disabled by default, lets allies issue orders to each other's units and structures while preserving ownership and separate funds. With it disabled, commands remain separate.
- New co-op intros give the guest a WOR on free rock as close as possible to the host WOR, or the host Construction Yard when there is no WOR. Matching starting units and the extra deployable MCV remain.
- An enemy sharing either ally's faction is reassigned to another available faction while keeping its forces, position and campaign role. Both allies and all opponents use distinct colors; lobby color choices refresh immediately.
- In-game How to Play now has French and English text, including Jericho Lite and co-op options.
- Wildspade mentat: mouth moved one pixel right and up; eye and mouth animations are slightly darker at render time. Original background and idle pixels remain intact.

Co-op choices persist in progress records and checkpoints. Older records remain readable and default to allied control disabled when the field is absent. New layouts apply to a new campaign or the next mission, not existing checkpoint positions.

Both PCs require **1.0.537**, the same mod and network protocol **6**. Version 1.0.536 clients cannot join. Tornie has tried co-op on two PCs and confirmed loading saves from the lobby. Automated Windows/Linux checks cover the new settings and their persistence. Extended playtesting and the Mint 22.3/kernel 7.0 compatibility check remain open.
