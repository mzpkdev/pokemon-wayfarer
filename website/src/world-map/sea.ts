// The living sea: far out, glints of light come and go across the open
// water, the way sun catches waves; close in, where the game's own water
// tiles show, a ripple drifts over each, the way the overworld cycles its
// water. Nothing is stored: a glint or a ripple is a hash of its place and
// the stretch of time, so every frame agrees. Both keep to the water mask.
import { roll, ramp } from "./noise"
import type { Overlay } from "./render"
import { isWaterAt, type Water } from "./water"

/** CSS pixels per metatile below which the glints show in full, and above which they are gone. */
const GLINTS_FULL = 4
const GLINTS_GONE = 7
/** CSS pixels per metatile from which the ripples start, and where they are fully in. */
const RIPPLES_FROM = 6
const RIPPLES_FULL = 10
/** How long a glint lives, in ms, and how many of the lattice's cells are quiet at a time. */
const GLINT_MS = 2600
const GLINT_QUIET = 0.72
/** CSS pixels the glints' lattice is at least apart; the lattice doubles in metatiles to keep it. */
const GLINT_SPACING = 36
/** How long a ripple takes to drift down its tile, in ms. */
const RIPPLE_MS = 2400
/** How many of the water tiles ripple at a time, at most. */
const RIPPLE_SHARE = 0.45
/** How long the surf along a coast takes to wash in and draw back, in ms. */
const SURF_MS = 2800

/** The sea overlay, on a clock (`now`, in ms); still at one moment without `animate`. */
export const seaOverlay = (water: Water, animate: boolean, now: () => number): Overlay => {
  const frozen = now()
  return (context, projection) => {
    const { scale, ratio, left, top } = projection
    const zoom = scale / ratio
    const { width, height } = context.canvas
    const t = animate ? now() : frozen
    const right = left + width / scale
    const bottom = top + height / scale

    const glints = 1 - ramp(zoom, GLINTS_FULL, GLINTS_GONE)
    if (glints > 0) {
      // A lattice in metatiles, doubling so its cells stay at least GLINT_SPACING across.
      let cell = 4
      while (cell * zoom < GLINT_SPACING) cell *= 2
      const period = Math.floor(t / GLINT_MS)
      const dot = Math.max(1, Math.round(ratio))
      context.fillStyle = "rgb(255 255 255)"
      const firstX = Math.floor(left / cell)
      const firstY = Math.floor(top / cell)
      const lastX = Math.floor(right / cell)
      const lastY = Math.floor(bottom / cell)
      for (let j = firstY; j <= lastY; j++)
        for (let i = firstX; i <= lastX; i++) {
          // Each cell's glint comes a little later than the last one's, so
          // they don't all blink together; a cell may sit out a period.
          const lag = roll(i, j, 7) * GLINT_MS
          const k = Math.floor((t - lag) / GLINT_MS)
          if (roll(i, j, k, 1) < GLINT_QUIET) continue
          const life = ((t - lag) / GLINT_MS - k) % 1
          const x = (i + roll(i, j, k, 2)) * cell
          const y = (j + roll(i, j, k, 3)) * cell
          if (!isWaterAt(water, Math.floor(x), Math.floor(y))) continue
          const shine = Math.sin(life * Math.PI)
          const w = dot * (1 + Math.round(shine * 2))
          context.globalAlpha = glints * shine * 0.7
          context.fillRect(
            Math.round((x - left) * scale - w / 2),
            Math.round((y - top) * scale),
            w,
            dot,
          )
        }
      void period
      context.globalAlpha = 1
    }

    const ripples = ramp(zoom, RIPPLES_FROM, RIPPLES_FULL)
    if (ripples > 0) {
      // A game pixel, in device pixels: ripples are drawn on the tiles' own grid.
      const px = Math.max(1, Math.round(scale / 16))
      const share = Math.min(RIPPLE_SHARE, (zoom / 32) * RIPPLE_SHARE + 0.12)
      const firstX = Math.floor(left)
      const firstY = Math.floor(top)
      const lastX = Math.floor(right)
      const lastY = Math.floor(bottom)
      context.fillStyle = "rgb(255 255 255)"
      for (let y = firstY; y <= lastY; y++)
        for (let x = firstX; x <= lastX; x++) {
          if (!isWaterAt(water, x, y)) continue
          const sx = Math.round((x - left) * scale)
          const sy = Math.round((y - top) * scale)
          // Surf where the water meets land: foam along the tile's edge,
          // washing in and drawing back on a beat its stretch of coast
          // shares (a few tiles at a time), each tile a touch off it.
          const beat = roll(x >> 2, y >> 2, 14) + roll(x, y, 15) * 0.12
          const wash = Math.sin(((t / SURF_MS + beat) % 1) * Math.PI * 2)
          if (wash > 0.15) {
            context.globalAlpha = ripples * 0.45 * Math.min(1, (wash - 0.15) * 1.6)
            const reach = Math.max(px, Math.round(px * (1 + 2 * wash)))
            if (!isWaterAt(water, x, y - 1)) context.fillRect(sx, sy, Math.round(scale), reach)
            if (!isWaterAt(water, x, y + 1))
              context.fillRect(sx, sy + Math.round(scale) - reach, Math.round(scale), reach)
            if (!isWaterAt(water, x - 1, y)) context.fillRect(sx, sy, reach, Math.round(scale))
            if (!isWaterAt(water, x + 1, y))
              context.fillRect(sx + Math.round(scale) - reach, sy, reach, Math.round(scale))
          }
          if (roll(x, y, 11) > share) continue
          const phase = roll(x, y, 12)
          const drift = ((t / RIPPLE_MS + phase) % 1) * 16
          const ox = Math.floor(roll(x, y, 13) * 9)
          // A light stroke of a few game pixels, drifting down the tile and
          // round again; a shorter one trails it eight rows under.
          const row = Math.floor(drift)
          context.globalAlpha = ripples * 0.26
          context.fillRect(sx + ox * px, sy + row * px, 6 * px, px)
          context.fillRect(sx + (ox + 2) * px, sy + ((row + 8) % 16) * px, 3 * px, px)
        }
      context.globalAlpha = 1
    }
  }
}
