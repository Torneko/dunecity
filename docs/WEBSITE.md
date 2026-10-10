## Doublefinery artwork revision r5 — 2026-10-10

Publication verified: tagged source `d29b80cb5579d99c9cf40eb3bf9f2a86c9fc13be`, website `d0901d65dd9710de37fa51f95fb2ac6ddce43972`. The revised user-supplied sprite is byte-identical in all four mods, all game packages, both PNG archives and the live gallery. All 28 release asset digests match local files; all nine package URLs returned HTTP 200. Game regression suites were not rerun.

Linux packaging: https://github.com/Torneko/dunelegacy-tornie/actions/runs/38068492379 — success. Pages: https://github.com/Torneko/dunelegacy-tornie/actions/runs/38068770242 — success.

Artwork revision r5: the corrected Doublefinery sheet supplied on October 10 is copied byte-for-byte into all four mods. Native dimensions: 80 × 224 pixels (seven 80 × 32 frames), without resizing. The existing 1.0.538 packages and FR/EN website are refreshed with this image.

## Published 1.0.538 follow-up — 2026-10-10

Publication confirmed: release tag source ed14895dca2063dacb5ebcbcb98f67a81575f0c6, website 9af05f94036a1f4da958f9d291aea4c0b24514aa. All 28 release asset digests match the local publication files; all nine package download URLs returned HTTP 200. Local website checks: 104; live website checks: 52.

Linux packaging: https://github.com/Torneko/dunelegacy-tornie/actions/runs/38028622609 — success; compiled game code matches the tagged source (only the French guide changed afterward). Pages: https://github.com/Torneko/dunelegacy-tornie/actions/runs/38029333585 — success. Both public languages and the offline ZIP include the exact final sprite.

The existing 1.0.538 release and tag now include the Doublefinery follow-up (native
80×224 final sprite, network protocol 9, save format 9831). French/English Pages and
offline downloads are updated together from the recovered gh-pages baseline 2f6ee0b6.
Both co-op PCs need the replaced packages. Prior game validation is retained as
historical evidence; game suites were not rerun after the final sprite replacement.
Publication/package and website validation reports accompany the release.

## Published 1.0.538 baseline — 2026-10-09

The French/English website was updated from published `origin/gh-pages`
`ec822ddbb4a195c864d2658fb4dfe6b2b62b31d6`. Tornie confirmed the latest build and
two-PC co-op work and authorized publication. The public site, exact release
downloads and offline ZIP now identify 1.0.538.
The obsolete LOCAL labels in both HTML entries were removed after publication,
including the header, sidebar and search description. The corrected public site
and matching offline ZIP passed 16 additional desktop/mobile label checks.
Game packages and tagged source are unchanged.

- Release: https://github.com/Torneko/dunelegacy-tornie/releases/tag/v1.0.538
- Tagged/tested source: `c39a276e1bb4a8749b055d6283df3e067dec7e71`; merged through PR #10.
- Website: `2f6ee0b62a5d8b4bdf6fe9ed9cd440a848dde8ff` on `gh-pages`.
- Windows/Linux build, engine, installed FR/EN runtime and locale checks: https://github.com/Torneko/dunelegacy-tornie/actions/runs/37981715769 — success.
- GitHub Pages deployment: https://github.com/Torneko/dunelegacy-tornie/actions/runs/38005836548 — success.
- Browser checks: 180 local/file/HTTP desktop/mobile checks, 198 local feature checks, 60 live route checks and 66 live feature checks, with no browser errors.
- All twenty release downloads are publicly accessible and match GitHub SHA-256 digests. Four published guides and eight new PNGs were checked against the local site.
- Windows ZIP SHA-256: `dce9fc42e20178218d17bf46b161eac6cc7d1a4fddeabb6cdfe488478cd3fd80`.
- Offline website ZIP SHA-256: `4ed9d75ba7a8daab2144b3363afef46127b2e0f5f3bc6bd49c49c8c02257903d`.

Both languages cover optional allied Easy Mode (mission 2, +500 credits at that
mission start, -25 purchase prices), five optional extra units per present enemy
per mission, reinforcements for both allies and protocol 7. There are 67 achievements
and 119 gallery PNGs. Six supplied production portraits distinguish 1, 3 and 5
infantry in Barracks/WOR/Worfinery, with white price text, in Tornie, Jericho and
their Lite editions. Selected-unit portraits retain their existing images. The
Worfinery displays Harvester last while preserving production queues and saves.
Wildspade/Tornie and Rebels/Jericho use their supplied banners and updated colors.

The release uses the artifacts from the successful build above. The duplicate tag
build was cancelled after its version gate passed to preserve those audited files.
Standard Linux formats, a separate Mint compatibility AppImage, exact tagged
sources, bilingual notes, PNG pack and offline site accompany the release.
The compatibility AppImage passed FR/EN runtime and PulseAudio checks under
Ubuntu 22.04/WSL. Mint 22.3/kernel 7.0 still needs testing on the target PC.

Details and publication hashes are in `release_notes/Publication-Validation-1.0.538.json`
and the accompanying release reports. Use this published `origin/gh-pages` tree
or the matching 1.0.538 offline ZIP as the website baseline on either PC.

## Published 1.0.537 baseline — 2026-10-08

Recovered the current `origin/gh-pages` baseline at `a39cecfa` before updating the local bilingual site. Document allied control (default off), selectable enemy AI, an AI guest, guest intro WOR, enemy faction/color collision handling, revised in-game help and refined Wildspade rendering. Unit technology and prices, solo campaign maps and the 47-achievement catalogue are unchanged. New co-op layouts require a new campaign or next mission; checkpoints retain existing positions. Protocol 6 requires both players to use 1.0.537. Tornie has tried co-op on two PCs and confirmed lobby save loading. Extended playtesting and the Mint 22.3/kernel 7.0 compatibility check remain open. The French/English website and offline ZIP now identify 1.0.537 and embed the exact audited Windows release archive.

- Release: https://github.com/Torneko/dunelegacy-tornie/releases/tag/v1.0.537
- Tagged/tested source: `2e54898202b13bd238064d6920c8bbea5253342d`; merged through PR #9.
- Website: `ec822ddbb4a195c864d2658fb4dfe6b2b62b31d6` on `gh-pages`, recovered from the previous `a39cecfa` baseline.
- Windows/Linux build, engine and installed FR/EN runtime checks: https://github.com/Torneko/dunelegacy-tornie/actions/runs/37870615482 — success.
- GitHub Pages deployment: https://github.com/Torneko/dunelegacy-tornie/actions/runs/37872686746 — success.
- Browser checks: 180 local/file/HTTP desktop/mobile checks plus 60 live checks, including achievement search, filters and language switching.
- Windows ZIP SHA-256: `02d4c20cab6a4366773300326967240c279ece56c0526b2dc1b5268eb05ce9d9`.
- Offline website ZIP SHA-256: `dbb83ec7facd1b7ce68f67e23ad38190c3db7230d18c9419975dd7657fc3049e`.
- Standard Linux CI formats, a separately identified Mint compatibility AppImage, exact tagged sources, bilingual notes and 111 PNGs accompany the release.

The release uses the artifacts from the successful build above. Repeated merge/tag builds of the same code were cancelled after verifying the source tree and tag version, to preserve the audited archives. Publication details are in `release_notes/Publication-Validation-1.0.537.json`. Tornie confirmed lobby save loading works. Extended co-op playtesting and the friend's Mint 22.3/kernel 7.0 check remain open. Use this `origin/gh-pages` tree or the matching 1.0.537 offline ZIP as the website baseline on either PC.

# Documentation website maintenance

Tornie requests that every game update also update the French and English website. This applies to work from either PC.

## Release 1.0.536

The French/English 1.0.536 website was updated from the published `origin/gh-pages` 1.0.535 tree. It covers Easy Mode, Jericho Lite, revised intro opposition, Ornithopter behavior and Wildspade's price, darker mentat overlays, 47 achievements and the independent nine-mission cooperative campaign. Raider Trike and Rocket Trike prices use black numbers on their light portraits; added backgrounds/borders are removed for all prices and the other prices retain their usual color. Jericho Lite enables the four Jericho spice families by default with matching colors and effects; its six opening missions copy Tornie Lite except for their remapped opponents. New co-op campaigns follow the host faction's original campaign, with 405 variants from 45 sources, a nearby deployable MCV and matching starting forces for the guest, original opponents (up to five in Vanilla) and shared harvest objectives. Older sessions keep the 45 previous layouts. Guides retain identical faction choices, separate ownership/progress/checkpoints and protocol 5. Solo Commander/Chaos Conqueror awards are distinguished from faction victories that also support multiplayer.

The local/offline site passed 180 desktop/mobile browser checks across both languages, source files, the ZIP extraction and HTTP preview, with no JavaScript, missing-image or overflow failures. The release package is `DuneLegacyTornie-Site-Local-1.0.536.zip`. Published French/English entries and download links now identify 1.0.536. A human co-op match between two computers remains pending; the release includes co-op for playtesting. Publication details and package hashes are recorded below and in `release_notes/Publication-Validation-1.0.536.json`.

## Published 1.0.536 baseline — 2026-10-07

- Release: https://github.com/Torneko/dunelegacy-tornie/releases/tag/v1.0.536
- Tagged/tested source: `4c26d6cc6887ab9fd0735269366829b057a52a63`; merged into `dunelegacy-tornie` through PR #8.
- Website: `a39cecfa007131e107b184eec5092b84960599d8` on `gh-pages`.
- Windows/Linux build, engine, installed FR/EN runtime and locale checks: https://github.com/Torneko/dunelegacy-tornie/actions/runs/37697974701 — success.
- GitHub Pages deployment: https://github.com/Torneko/dunelegacy-tornie/actions/runs/37700593303 — success.
- Offline/site browser verification: 180 local/file/HTTP desktop/mobile checks and 24 live FR/EN checks, plus seven live download checks.
- Windows ZIP SHA-256: `18efee50d0de8e7817ea1ba19e1e01c2f97d959761e6694aa98a64e79f28c669`.
- Offline website ZIP SHA-256: `594cd784b6bdac5c2413b7e26725632cf69223c5f9aebb701f3082de76091729`.

The release uses the archives from the successful build above. The website and offline ZIP embed that exact Windows archive. Creating the tag started a second build of identical code; its version check passed, then the duplicate compilation/upload was cancelled to preserve these audited files. Source tags, release assets and their GitHub SHA-256 digests were checked. Reports accompany the release and are tracked under `release_notes/`.

A human two-computer co-op match remains pending, as accepted by Tornie for this publication. Both players need 1.0.536 and the same mod. The release documents co-op as available for playtesting.

Use this `origin/gh-pages` tree or the 1.0.536 offline package as the next website baseline on either PC.

## Current locations

Local follow-up to 1.0.538: the working preview is `work/site-538-followup/dist`
beside this source checkout. Its baseline is published `gh-pages` commit
`2f6ee0b6`. It documents the 5×2 Doublefinery, local protocol 9/save format 9831,
121 PNGs and the test downloads. This local patch has not been published.
Use the follow-up preview when preparing its eventual publication, and recheck
the public branch before merging website changes from another PC.

- Published site: https://torneko.github.io/dunelegacy-tornie/
- English entry: https://torneko.github.io/dunelegacy-tornie/index-en.html
- GitHub Pages source: the root of the `gh-pages` branch, separate from the game source branch `dunelegacy-tornie`.
- Offline package: `DuneLegacyTornie-Site-Local-<version>.zip` in the matching GitHub release. Its website files are under `DuneLegacyTornie-Site-Local/dist/`.

Before editing, fetch both branches and read the matching release notes and validation report. Preserve unrelated local changes. Retrieve the latest site from `origin/gh-pages` or the release package; previous PCs may retain older website generators outside this repository.

## Updating the content

Derive the documentation from the actual engine rules and active-mod configuration. Include special runtime gates, building upgrades, prerequisite chains and faction-specific overrides; copying `ObjectData.ini` alone is insufficient.

Update both entries and renderers (`index.html`, `index-en.html`, `assets/app.js`, `assets/app-en.js`), shared data (`assets/site-data.js`), English data (`assets/site-en-data.js`) and the accompanying downloadable documents as needed. Preserve the FR/EN switch and its selected page, faction and mod.

For each game update, revise the displayed version, release links, affected unit/building descriptions and technology progression. Keep the faction/campaign roster, achievement catalogue and gallery consistent with the game. Use the corresponding original game PNGs for icons, sprites and banners, retaining mod-specific variants. Achievement pages describe rules; they do not display invented personal unlock progress.

Keep the offline package functional through `file://`, including its local downloads and launchers. The published site uses public release/download links where appropriate. Preserve both French and English documents and asset catalogues.

## Verification and release

Check both languages in a real browser on desktop and mobile widths: navigation, affected fiches/technology routes, language switching, images, search/filter controls and downloads. Check offline access and browser errors. Validate that the displayed version and download targets match the release, and that the gallery images exist.

When releasing the game, include the updated offline site ZIP and publish the corresponding site through the configured `gh-pages` source. Verify the resulting Pages deployment and the live French and English entries. Report the checks actually performed and any unresolved game issues; a previous validation report does not validate a later build.

Historical synchronized release: **1.0.535**, tag `v1.0.535`, revised tagged game source `9ca89b3394a0190cf8db05c0538c771fa2e6e78c`; published website `66bcd8722e58cc757573c9220fb0dfb5a8d5d19c`. The existing 1.0.535 was replaced at Tornie's explicit request on 2026-10-06; no new version was created. The initial campaign rebuild was published through merged PRs #6 and #7; the revised opening balance is commit `9ca89b33`. Both public entries and the offline site ZIP match this revision: 30 unit entries, 29 buildings, 46 achievements and 111 PNGs.

The site documents all 24 rebuilt Tornie/Jericho campaigns and 528 scenarios, the balanced twelve-faction opponent plans and the corrected animated cat mentat. Every first mission now has two bonus Tanks relative to the original 1.0.534 forces, three Troopers squads (nine infantry) and two Special Unit Spawns. The earlier Wildspade support was replaced by this common policy. First-mission opposition is reduced to twelve enemies with Area Guard replacing initial Hunt orders; ambush encounters remain. Ordinary vehicles, terrain, economy and buildings are preserved; later mission forces are unchanged. See [campaign plans](CAMPAIGNS.md) and [mentat sources/prompts](WILDSPADE-MENTAT.md). Redownload 1.0.535 and start a new campaign for these forces. Existing saves remain readable with their stored maps. Vanilla and Tornie Lite campaign plans are preserved.

Chaos Mode and the Wildspade technology gates from 1.0.534 remain documented. The 2×3 and 3×2 advanced Windtrap entries use the original `Tornie_AdvancedWindtrap_icon.png` game portrait; sprites and editor previews remain in the gallery. The original Fremen house-confirmation banner is verified in the actual game rendering.

Release: https://github.com/Torneko/dunelegacy-tornie/releases/tag/v1.0.535. Downloads include `DuneLegacyTornie-1.0.535-Windows-x64.zip`, four Linux package formats, tagged sources, French/English notes, 111 PNGs and `DuneLegacyTornie-Site-Local-1.0.535.zip`. The offline website embeds the exact revised Windows CI release archive (SHA-256 `ae491377231f8fec51ac597a251883f64f984350d65afb39c320f64c16055175`). Website scripts and the Windows link carry the revision query `rev=9ca89b33` to refresh cached copies. Prior previews are retained locally for reference. Release 1.0.534 remains available separately.

Validation: revised release CI https://github.com/Torneko/dunelegacy-tornie/actions/runs/37529197044 succeeded; revised Pages deployment https://github.com/Torneko/dunelegacy-tornie/actions/runs/37531119434 succeeded. Windows/Linux engine tests pass: 97 cases and 6,867 assertions. The extracted revised Windows CI archive passed French/English runtime checks, each loading 838 scenarios, including all 528 rebuilt maps and a save from the original 1.0.535. Runtime checks verify all 24 first-mission policies: two bonus Tanks, nine actual Troopers, two resolved Special spawns and twelve enemies without an initial rush. All 2,206 mod files match tagged sources exactly in Windows and Linux payloads. A profile containing the original 1.0.535 managed mods was automatically reseeded to the revised maps despite the unchanged version; its personal campaign file and previous save bytes were preserved.

The published site passed 36 desktop/mobile routes, eight campaign views and twelve Windtrap portrait cases, in addition to 54 offline/local routes, eight affected fiches and 24 portrait checks. All 111 PNG hashes, five public website downloads and nineteen release asset hashes were verified. Reports attached to the release include `Validation-1.0.535.json`, `Campaign-Opening-Balance-1.0.535.json`, `Same-Version-Profile-Update-1.0.535.json` and `Publication-Validation-1.0.535.json`. Tornie authorized publication of this replacement; the new opening balance has automated checks and still benefits from human playtesting. Extended all-faction campaign balance and a live two-PC multiplayer match remain broader playtesting work.

For the next update on either PC, use this published `origin/gh-pages` tree or the matching offline ZIP as the website baseline. Fetch both source branches explicitly if the local clone has a single-branch fetch refspec; do not recover the site from an older preview generator. Historical campaign tooling intentionally uses immutable baseline commits: initial 1.0.535 source `fa2a5de3d23507f95332e243d060eeb419ce8fd5` and original 1.0.534. The published tag now points to `9ca89b33`.

Local artwork revision 538-followup-r4 uses Tornie’s final 80×224 Doublefinery sheet in all four mods and the FR/EN gallery/downloads. Gameplay, network protocol and save format are unchanged; remaining game tests were not rerun at Tornie’s request.
