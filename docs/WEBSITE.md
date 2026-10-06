# Documentation website maintenance

Tornie requests that every game update also update the French and English website. This applies to work from either PC.

## Current locations

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

Current synchronized release: **1.0.535**, tag `v1.0.535`, revised tagged game source `9ca89b3394a0190cf8db05c0538c771fa2e6e78c`; published website `66bcd8722e58cc757573c9220fb0dfb5a8d5d19c`. The existing 1.0.535 was replaced at Tornie's explicit request on 2026-10-06; no new version was created. The initial campaign rebuild was published through merged PRs #6 and #7; the revised opening balance is commit `9ca89b33`. Both public entries and the offline site ZIP match this revision: 30 unit entries, 29 buildings, 46 achievements and 111 PNGs.

The site documents all 24 rebuilt Tornie/Jericho campaigns and 528 scenarios, the balanced twelve-faction opponent plans and the corrected animated cat mentat. Every first mission now has two bonus Tanks relative to the original 1.0.534 forces, three Troopers squads (nine infantry) and two Special Unit Spawns. The earlier Wildspade support was replaced by this common policy. First-mission opposition is reduced to twelve enemies with Area Guard replacing initial Hunt orders; ambush encounters remain. Ordinary vehicles, terrain, economy and buildings are preserved; later mission forces are unchanged. See [campaign plans](CAMPAIGNS.md) and [mentat sources/prompts](WILDSPADE-MENTAT.md). Redownload 1.0.535 and start a new campaign for these forces. Existing saves remain readable with their stored maps. Vanilla and Tornie Lite campaign plans are preserved.

Chaos Mode and the Wildspade technology gates from 1.0.534 remain documented. The 2×3 and 3×2 advanced Windtrap entries use the original `Tornie_AdvancedWindtrap_icon.png` game portrait; sprites and editor previews remain in the gallery. The original Fremen house-confirmation banner is verified in the actual game rendering.

Release: https://github.com/Torneko/dunelegacy-tornie/releases/tag/v1.0.535. Downloads include `DuneLegacyTornie-1.0.535-Windows-x64.zip`, four Linux package formats, tagged sources, French/English notes, 111 PNGs and `DuneLegacyTornie-Site-Local-1.0.535.zip`. The offline website embeds the exact revised Windows CI release archive (SHA-256 `ae491377231f8fec51ac597a251883f64f984350d65afb39c320f64c16055175`). Website scripts and the Windows link carry the revision query `rev=9ca89b33` to refresh cached copies. Prior previews are retained locally for reference. Release 1.0.534 remains available separately.

Validation: revised release CI https://github.com/Torneko/dunelegacy-tornie/actions/runs/37529197044 succeeded; revised Pages deployment https://github.com/Torneko/dunelegacy-tornie/actions/runs/37531119434 succeeded. Windows/Linux engine tests pass: 97 cases and 6,867 assertions. The extracted revised Windows CI archive passed French/English runtime checks, each loading 838 scenarios, including all 528 rebuilt maps and a save from the original 1.0.535. Runtime checks verify all 24 first-mission policies: two bonus Tanks, nine actual Troopers, two resolved Special spawns and twelve enemies without an initial rush. All 2,206 mod files match tagged sources exactly in Windows and Linux payloads. A profile containing the original 1.0.535 managed mods was automatically reseeded to the revised maps despite the unchanged version; its personal campaign file and previous save bytes were preserved.

The published site passed 36 desktop/mobile routes, eight campaign views and twelve Windtrap portrait cases, in addition to 54 offline/local routes, eight affected fiches and 24 portrait checks. All 111 PNG hashes, five public website downloads and nineteen release asset hashes were verified. Reports attached to the release include `Validation-1.0.535.json`, `Campaign-Opening-Balance-1.0.535.json`, `Same-Version-Profile-Update-1.0.535.json` and `Publication-Validation-1.0.535.json`. Tornie authorized publication of this replacement; the new opening balance has automated checks and still benefits from human playtesting. Extended all-faction campaign balance and a live two-PC multiplayer match remain broader playtesting work.

For the next update on either PC, use this published `origin/gh-pages` tree or the matching offline ZIP as the website baseline. Fetch both source branches explicitly if the local clone has a single-branch fetch refspec; do not recover the site from an older preview generator. Historical campaign tooling intentionally uses immutable baseline commits: initial 1.0.535 source `fa2a5de3d23507f95332e243d060eeb419ce8fd5` and original 1.0.534. The published tag now points to `9ca89b33`.
