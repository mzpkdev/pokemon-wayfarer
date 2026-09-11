# Rejected action art candidates

Historical record: the user subsequently approved four-style v1 with Gold,
Kris, Brendan, and May. Red and Leaf are deferred, so this art gap no longer
blocks the four-style release. It remains a requirement before either deferred
style can be registered. The saved IDs 3 and 4 remain reserved and invalid.

These are rejected imagegen outputs, retained for review. They are not game assets
and must not be referenced by production graphics tables. Neither candidate was
loaded in an emulator. Rejected PNGs are retained locally in
`/tmp/trainer-appearance-art-candidates/`; they are not committed game assets.

Both attempts used the built-in imagegen tool with these local references:

- `game/graphics/object_events/pics/people/red/red_bike.png` (Red identity).
- `game/graphics/object_events/pics/people/brendan/acro_bike.png` (27-frame action contract).

The required output is an indexed 4-bit PNG, 864 by 32 pixels, containing 27
horizontal 32 by 32 frames, with the FRLG player palette and transparent index 0.

## Attempt 1

Prompt requested an identity-preserving edit: replace Brendan with Red while
keeping every pose, frame position, wheel position and animation order. It
specified 864 by 32 pixels, 27 frames, no scaling, antialiasing, labels or margins,
and enumerated the 16 colors from `player_frlg.pal`. Red's cap, vest, trousers,
shoes and backpack were to match the Red bike reference.

The output was 2059 by 764 pixels, RGB, with large margins and a textured
background. Visual inspection showed incorrect direction order and failure to
preserve the reference trick poses. Rejected.

## Attempt 2

Exact correction prompt:

> Edit target image 2 ONLY: it is 864x32 pixels, 27 square frames each 32x32. Preserve this exact image size, palette of 16 colors, native pixel grid, solid purple background, and EVERY existing pose, direction, bicycle position, and frame spacing. Change only the rider's identity from Brendan to Red, whose outfit and head are in reference image 1. Do not redraw the bicycles or rearrange frames. No additional background padding, no scaling, no texture. This output will be loaded directly by an emulator as 27 native 32x32 frames; wrong dimensions or pose ordering makes it unusable. Copy reference 2's native 864x32 canvas exactly. Red has a red cap, black hair, red vest and blue pants; not Brendan's white hat.

The output again has large margins, RGB encoding, a textured background, and
incorrect direction/pose ordering. It is not a pixel-exact edit of the source.
Rejected. Resizing and palette quantization alone cannot repair the incorrect
animation poses.

## Requirement for deferred styles

Red and Leaf still need authored and visually verified Acro tricks, underwater,
and watering art. Inspection of Red's existing surf and item sheets against
Brendan's underwater and watering sheets found no same-character frame-only
normalization that supplies the missing submerged body or watering can/pour
poses. Existing aliases do not demonstrate support.
