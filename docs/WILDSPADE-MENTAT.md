## Ajustement 1.0.537

La bouche utilise maintenant l’ancrage `(175, 200)`, un pixel à droite et vers le haut par rapport à 1.0.536. Les deux ancrages des yeux restent `(134, 137)` et `(198, 139)` ; ils sont vérifiés dans les captures des cinq états. Les overlays actifs utilisent `Eyes Brightness = 245` et `Mouth Brightness = 245` (sur 255). La teinte diminue légèrement à l’affichage, sans repeindre les PNG, changer leur transparence ni modifier le fond ou les poses de repos. Le nez, les moustaches et l’épaule restent préservés.

# Wildspade mentat — 1.0.535

Background supplied by Tornie: `mods/Tornie/data/MentatCat_Wildspade.png` (640 × 400), copied byte for byte into both full mods. Dedicated `[Mentat 9]` overrides replace the former Neutral alias. Jericho resolves its runtime slot through the faction identity before choosing assets.

Generated atlases: `MentatCat_Wildspade_Eyes.png` and `MentatCat_Wildspade_Mouth.png`, in `mods/Tornie/data/` and `mods/Jericho/data/`. Built-in **imagegen** was used with a transparent background, not the CLI/API fallback. Runtime cropping, separately anchored eye/mouth patches, nearest-neighbour fitting and alpha blending are declared in `mod.ini`; no background file is rewritten. Five frames follow the existing blink/look and speech timing. Resting eye/mouth patches use the original background pixels.

## Prompt set

Use case: precise-object-edit. Input: exact purple pixel-art cat mentat reference. Preserve the entire 640 × 400 room/background unchanged. Generate only a separate facial-overlay strip. Match the cat's viewpoint, purple fur, green eyes, pixel-art style, proportions and lighting. True transparent alpha outside the patches. No text, grid, body, room or ears. Exactly five equal horizontal cells, no gaps; opaque fur patches cover the old facial features.

Original generation request: eyes in a logical 100 × 50 patch for x132/y135. Five states: normal, look left, look right, look down, closed blink.

Original generation request: mouth in a logical 100 × 50 patch for x147/y188. Five states: tiny opening, medium opening, wider opening, rounded opening, closed rest.

The engine uses the untouched background for normal eyes, closed mouth and final mouth rest; the generated states supply gaze, blink and speaking variants. Mouth talking uses the first three generated openings. Tests capture all states for visual review and compare idle/background pixels exactly.

## Alignment and foreground correction

The initial broad overlays shifted the features. The original atlases are retained byte for byte; this correction changes native UI composition rather than repainting the input. Eyes use two fixed feature anchors (left x134/y137, 43×29; right x198/y139, 31×25), with per-cell source crops compensating generated padding. The mouth uses x174/y201, 40×23, leaving the original nose and whisker roots untouched. Idle eyes and closed mouth still use exact background pixels.

`Eyes Patch` / `Mouth Patch`: destination `x,y,w,h` relative to the animation canvas, followed by one source `x,y,w,h` per horizontal atlas cell, separated by semicolons. Source x is relative to its cell. Layout counts, dimensions and actual surface bounds are checked before rendering. Other mentats keep their existing strip loader.

`Foreground Polygon` supplies a native-coordinate silhouette. The engine copies only the original shoulder pixels inside this outline into an alpha surface, then draws it after the briefing animation, like Paul's foreground. No new bitmap is generated and the background file remains unchanged. Foregrounds are selected by faction asset identity, including Jericho's runtime mapping.

Verification now renders the actual `BriefingMenu`, including its Heavy Factory animation and buttons, in both mods. Tests compare original nose/whisker pixels, iris anchors, the overlapping shoulder, video visibility, idle pixels and Paul's existing foreground. The five speaking/looking states are captured for visual review.
