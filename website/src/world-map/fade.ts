// The game's screen fade, as the title scene's warp and the tour's own warps
// both play it: a fade is 8 whole steps from clear to black (deltaY 2 in the
// game's palette.c), not a smooth ramp.

/** The steps in a full fade. */
export const FADE_STEPS = 8

/** Rounds a fade level (0 clear, 1 black) to the game's whole steps, toward black. */
export const stepped = (level: number) =>
  Math.ceil(Math.min(1, Math.max(0, level)) * FADE_STEPS) / FADE_STEPS
