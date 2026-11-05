// What the map draws that the game never does: the ripples on the water and
// the surf along the coast, the ships' lanes, wakes and names (and the
// train's), and the arrow over a Gym Leader's head. They help read the world from far out. Close in, where
// the picture is the game's own pixels at their own size or larger, they
// step aside, so what is left looks like the game.
import { ramp } from "./noise"

/** CSS pixels per metatile up to which the extras show in full, and from which they are gone: a game pixel a pixel. */
const EXTRAS_FULL = 12
const EXTRAS_GONE = 16

/** How much of the extras show at a zoom (CSS pixels per metatile): 1 far out, 0 close in. */
export const extrasAt = (zoom: number) => 1 - ramp(zoom, EXTRAS_FULL, EXTRAS_GONE)
