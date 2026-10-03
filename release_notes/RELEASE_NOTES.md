# Dune Legacy Tornie 1.0.525

Based on corrected DuneCity Tornie **1.0.524-26**, commit
`80799faba1b66c64887286446f5ed6b0df985226` from `Torneko/dunecity-tornie`.

The independent RTS now includes the corrected base's engine, campaign, unit,
structure, palette, voice, French audio and relocatable Linux resource improvements.
Tornie, Tornie Lite and Jericho are bundled and selectable with their complete payloads.
Captured faction technology and the strengthened Harkonnen opening missions are retained.

The project identity is Dune Legacy Tornie. The intro's bottom text and menu footer
use that name. The DuneCity first-launch activation popup, promotional panel and
city simulation are removed. Update checks use this project's repository and user
profiles are separate. Original source attribution and stable gameplay IDs remain.

The supplied RTS maps keep moderate, deterministic red and green spice pockets:
117 terrain maps and 25 legacy seed scenarios, about 10% of normal spice replaced.
Vanilla normalizes these variants to ordinary spice as in the corrected base.
Mod campaigns retain the corrected base's terrain and gameplay parameters.

Validation: Windows and Linux builds; 59 engine test cases (3378 assertions);
142 supplied maps; 2074 exact mod payload checksums; repeated activation,
78 opening/final campaign scenario loads and new-object save/load checks in English
and French using the actual Windows engine and SDL dummy drivers.
Manual visual playthroughs, long games, Android builds and live multiplayer are
not covered by these automated checks.
