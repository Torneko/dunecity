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

Current synchronized baseline: game **1.0.533** (`e1ce006ded1584164873e7416d95547670c384ec`), game branch also includes Linux packaging commit `d0aaf2cf803740a5c224562d69d6112ad91b6602`; published site `bee81be888632a5181c31c73638a81c451172a53`. The site contains 30 unit entries, 29 buildings, 46 achievements and 106 gallery PNGs. The Fremen house-confirmation banner remains an open visual issue according to Tornie's next-update note. The 6 October website correction aligns the publication wording and this known issue in both languages; game behavior is unchanged.

Preparation 1.0.534: `prepare/1.0.534` holds the engine update; `prepare/site-1.0.534` holds the matching bilingual site. The local preview adds Chaos documentation, updated Wildspade routes and two original windtrap previews (108 PNGs). The original Fremen confirmation banner is verified by runtime rendering. Keep public gh-pages on 1.0.533 until releasing the game; replace preview download links with the 1.0.534 release links when publishing.
