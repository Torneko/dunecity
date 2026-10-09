# Common co-op campaign — 1.0.537

[Guide français](COOP-CAMPAIGN.md)

The main menu's **Co-op campaign** entry supports two players over LAN or Internet. Each player freely chooses a faction and controls a separate base, army and credit balance on the same team. Allies may choose the same faction while keeping separate ownership and funds. Optional allied control permits shared commands.

Co-op follows the nine stages of the original campaign for the faction chosen by the host at launch. Its terrain, structures, opposing forces and objectives are retained. The source campaign stays fixed when resuming; progress and checkpoints remain independent of solo.

Available factions: 12 in Tornie and Jericho, 6 in Tornie Lite and Jericho Lite, 9 in Vanilla. The 45 source campaigns provide 405 co-op variants, with nine stages each.

| Edition | Available factions | Co-op missions |
|---|---:|---:|
| Vanilla | 9 | 9 |
| Tornie | 12 | 9 |
| Jericho | 12 | 9 |
| Tornie Lite | 6 | 9 |
| Jericho Lite | 6 | 9 |

## Hosting and joining

1. Run the **same 1.0.537 build** on both computers. Its network protocol is version **6**. Version 1.0.536 uses a different network protocol and cannot join this lobby.
2. Activate the same edition in **Mods**. Use different player names.
3. Open **Co-op campaign**, then choose the LAN or Internet browser. The host creates a game and selects **New campaign**. The second player joins through the browser or address connection.
4. Connect a human guest or select an AI in the second slot, then choose factions. The host can select each enemy AI’s type and difficulty. Identical factions are allowed. Both players are allies; the opponent slots remain AI-controlled.
5. The host starts after versions, rules and mod resources have synchronized.

Opponents follow the original map: early stages may have a single opponent. Mod finales keep their three roles; some Vanilla finales have four or five opponents. An enemy using either ally’s faction receives another available faction while keeping its role and forces. Both allies and all opponents use distinct colors.

The main player keeps the original base and starting units. The partner receives matching forces and credits, plus an extra MCV on free rock near the main Construction Yard, to deploy a separate base. New intros also provide a WOR on free rock as close as possible to the host WOR, or Construction Yard when the host has no WOR. Special spawns and unavailable units use an equivalent compatible with the partner’s faction.

Harvest objectives are shared: spice stored by both players is added together, with a common target of twice the solo quota. Both computers calculate the same team victory.

Co-op campaigns created with the previous maps retain their recorded layout. Start a new campaign to receive the original maps and MCV starting position.

## Victory, defeat and resuming

Victory and defeat apply to the team. If one allied base is destroyed, the partner may continue while the team retains forces under the map's rules.

Both players confirm readiness after a mission. A victory advances the common campaign; a defeat retries the current mission without advancing progress. Winning the ninth mission completes the campaign. Leaving or disconnecting does not unlock another mission.

Progress and checkpoints are stored in `coop/` beside the user's `Dune Legacy.ini`:

```text
coop/
    <session>.ini   common progress and settings
    <session>.dls   mission checkpoint
```

The session identifier distinguishes common campaigns. Solo progress and saves remain separate. Keep the progress record and checkpoint in the profile used to resume.

To resume, the host activates the campaign's edition, opens **Co-op campaign**, selects **Resume a common campaign** and chooses its `.ini` record. The partner joins the lobby. An available referenced checkpoint resumes the saved mission; without one, the progress record restarts the current mission. Human names can be reassigned in the lobby while the bases remain separate.

Progress writes use a temporary file and keep the previous complete record during replacement. A damaged record can recover from that backup. Completed campaigns remain recorded; create a new campaign to start again.

In the host lobby, **Load a save** opens **Manual** (`mpsave/`, `.dls` files) and **Automatic** (`coop/`, `.dls` checkpoints) tabs. The save must be a co-op campaign for the active mod. Loading closes the old lobby and opens a new one for the saved mission; the guest must rejoin. Cancelling or selecting an incompatible file keeps the current lobby. Automatic resume through the `.ini` progress record remains available.

## Allied control and AI

**Allied control**, off by default, enables orders to allied units and buildings: movement, combat, production, placement, repair and abilities. Production uses the owner’s funds; houses and statistics stay separate. Without this option, each player controls only their own objects.

The host can select a qBot, Mentat, SmartBot or AI Player ally in the second slot, and choose enemy AI types and difficulties. Campaign AI remains the initial enemy choice. These choices and allied control persist across missions and resumes. Difficulty levels depend on the selected AI family.

## Options and achievements

**Easy Mode** remains exclusive to solo campaigns. Its mission-2 start, 500-credit bonus and purchase discounts do not apply to co-op. Both players and their opponents pay the edition's normal prices.

The lobby offers **Chaos Mode**, off by default, in the four mods; Vanilla disables it. The draw uses a shared seed and both computers use the same rules. Saves preserve the mission's rules.

**Commander** awards, campaign conquest and **Chaos Conqueror / `CHAOS_CONQUEROR`** require solo campaigns. Completing the common campaign does not complete a faction's solo campaign. Other achievements follow their ordinary multiplayer rules and local-player events; achievement profiles remain local.

## Validation status and maintenance

Tornie has tested co-op on two PCs. Revision 1.0.537 fixes the reported issues and adds the new options. Tornie confirmed that loading saves from the lobby works. Settings and resumes pass automated Windows/Linux checks; extended playtesting remains useful.

Original-campaign templates live in `coop/faction<id>/coopNN.ini` under `data/` for Vanilla and `mods/<mod>/campaign/` for the mods. The 45 previous root templates remain for older sessions. `scripts/build-coop-campaign.py --check` validates the 405 source variants and previous maps; `--write` updates only co-op templates and checksums. `config/CoopCampaignTerrain.json` is generated with the game’s `MapSeed.cpp` algorithm.

The `Campaign/CoopCampaignSession` model stores the session, participants and progress. Its `[COOP]` context is embedded in `GameInitSettings` map data, which already travels through the network and game saves. Runtime ownership and the chosen faction's rules column are separate, allowing identical faction selections. `Campaign/CoopCampaignRuntime` retains the connection between missions; transitions validate the partner, session and stage before advancing.
