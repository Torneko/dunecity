# Wildspade mentat — 1.0.535

Background supplied by Tornie: `mods/Tornie/data/MentatCat_Wildspade.png` (640 × 400), copied byte for byte into both full mods. Dedicated `[Mentat 9]` overrides replace the former Neutral alias. Jericho resolves its runtime slot through the faction identity before choosing assets.

Generated atlases: `MentatCat_Wildspade_Eyes.png` and `MentatCat_Wildspade_Mouth.png`, in `mods/Tornie/data/` and `mods/Jericho/data/`. Built-in **imagegen** was used with a transparent background, not the CLI/API fallback. Runtime cropping, nearest-neighbour fitting and alpha blending are declared in `mod.ini`; no background file is rewritten. Five frames follow the existing blink/look and speech timing. Resting eye/mouth patches use the original background pixels.

## Prompt set

Use case: precise-object-edit. Input: exact purple pixel-art cat mentat reference. Preserve the entire 640 × 400 room/background unchanged. Generate only a separate facial-overlay strip. Match the cat's viewpoint, purple fur, green eyes, pixel-art style, proportions and lighting. True transparent alpha outside the patches. No text, grid, body, room or ears. Exactly five equal horizontal cells, no gaps; opaque fur patches cover the old facial features.

Eyes: logical 100 × 50 patch for x132/y135. Five states: normal, look left, look right, look down, closed blink.

Mouth: logical 100 × 50 patch for x147/y188. Five states: tiny opening, medium opening, wider opening, rounded opening, closed rest.

The engine uses the untouched background for normal eyes, closed mouth and final mouth rest; the generated states supply gaze, blink and speaking variants. Mouth talking uses the first three generated openings. Tests capture all states for visual review and compare idle/background pixels exactly.
