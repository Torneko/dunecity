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

Current synchronized release: **1.0.534**, tagged game source `640c00d5538af3deb368fbd06d6c71a15123a0f6`; published website `1a6ea2917dc4e7f8e3b527c7e8cb44188dfdf182`. Game PR #4 and website PR #5 are merged. Both public entries and the offline site ZIP match this release: 30 unit entries, 29 buildings, 46 achievements and 108 PNGs.

The 1.0.534 site documents Chaos Mode and the updated Wildspade technology gates. The 2×3 and 3×2 advanced Windtrap entries use the original `Tornie_AdvancedWindtrap_icon.png` game portrait, shared by these variants; their sprites and editor previews remain in the gallery. The original Fremen house-confirmation banner is verified in the actual game rendering.

Release downloads: `DuneLegacyTornie-1.0.534-Windows-x64.zip`, four Linux package formats, sources, French/English notes and `DuneLegacyTornie-Site-Local-1.0.534.zip`. The offline website embeds the exact Windows release archive. Previous preview packages are retained locally for reference and are not release download targets.

Validation: release CI https://github.com/Torneko/dunelegacy-tornie/actions/runs/37491579704 succeeded; Pages deployment https://github.com/Torneko/dunelegacy-tornie/actions/runs/37493840528 succeeded. The actual Windows CI archive was checked in French and English; public FR/EN website checks cover 36 desktop/mobile routes and 12 Windtrap portrait cases, in addition to offline/local checks. Published Windows/PNG download hashes match the verified package. Manual Wildspade checks and a live two-PC multiplayer match remain pending according to Tornie's playtest report.

## Prepared preview 1.0.535

The `prepare/1.0.535` game branch and `prepare/site-1.0.535` website branch rebuild Tornie/Jericho campaigns and add the animated Wildspade mentat. The preview has 24 campaign plans, 528 rebuilt scenarios and 111 PNGs. Public release 1.0.534 remains the publication baseline until this preview is approved for release. Offline preview downloads use `DuneLegacyTornie-1.0.535-Windows-Preview.zip`; they must not be presented as public release assets. See [campaign plans](CAMPAIGNS.md) and [mentat sources/prompts](WILDSPADE-MENTAT.md).
