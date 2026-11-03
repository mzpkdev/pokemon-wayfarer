// The game's own weather over the maps that have it (`weather` in meta.json,
// from each map's map.json): rain and thunderstorms along Johto's west coast
// and at the Lake of Rage, snow on Mt. Silver's routes and in Sinnoh's north,
// Route 228's sandstorm, the shade under the forests and the leaves on
// Bellchime Trail. Each is drawn within its map's rectangle, in game pixels,
// from a hash of the map, the drop and the time, so nothing is stored. The
// haze fades in from the map's edges, except along the stretch where the
// next map has weather too, so a rainy coast reads as one.
import type { Meta, Weather } from "./data"
import { roll, ramp } from "./noise"
import type { Overlay } from "./render"

/** CSS pixels per metatile from which the drops, flakes and streaks show, and where they are all there. */
const DROPS_FROM = 2
const DROPS_FULL = 4
/** How many drops a map gets per thousand CSS pixels of it on screen, by weather. */
const DENSITY: Record<Weather, number> = {
  rain: 1.1,
  storm: 1.5,
  snow: 0.7,
  sand: 0.9,
  shade: 0,
  leaves: 0.25,
}
/** Metatiles across the lattice the drops hang on, so their number follows what's on screen. */
const LATTICE = 8
/** The most drops one lattice cell holds. */
const MOST = 160
/** The haze each weather lays over its map: a color and how strong, there from far out. The storm's is the rain's, so the two join without a step. */
const HAZE: Record<Weather, [string, number]> = {
  rain: ["rgb(24 40 96)", 0.22],
  storm: ["rgb(24 40 96)", 0.22],
  snow: ["rgb(232 240 255)", 0.12],
  sand: ["rgb(214 178 96)", 0.3],
  shade: ["rgb(0 8 40)", 0.24],
  leaves: ["rgb(0 0 0)", 0],
}
/** How far in from a map's edge its haze fades, in metatiles, at most: a quarter of its narrower side. */
const FEATHER = 14
/** How often a storm flashes, in ms, and how long a flash lasts. */
const FLASH_EVERY_MS = 5200
const FLASH_MS = 90

type Side = "left" | "top" | "right" | "bottom"

/** A stretch of a spot's side another spot with weather lies along: `from` to `to` along it, in metatiles. */
type Join = { side: Side; from: number; to: number }

type Spot = {
  x: number
  y: number
  w: number
  h: number
  weather: Weather
  seed: number
  joins: Join[]
}

/** Where `b` lies along `a`'s `side`, if it does. */
const joinOf = (a: Spot, b: Spot, side: Side): Join | undefined => {
  const vertical = side === "left" || side === "right"
  const from = vertical ? Math.max(a.y, b.y) : Math.max(a.x, b.x)
  const to = vertical ? Math.min(a.y + a.h, b.y + b.h) : Math.min(a.x + a.w, b.x + b.w)
  if (to <= from) return undefined
  const touching =
    side === "left"
      ? b.x + b.w === a.x
      : side === "right"
        ? b.x === a.x + a.w
        : side === "top"
          ? b.y + b.h === a.y
          : b.y === a.y + a.h
  return touching ? { side, from, to } : undefined
}

/** A box on the canvas, in device pixels. */
type Box = { x: number; y: number; w: number; h: number }

/**
 * Fills `box` with `color`, fading in from its edges over `feather` device
 * pixels, so it doesn't lie on the world as a hard-edged shape where a map
 * runs over open sea. Along `joins` (in device pixels along each side) the
 * edge stays full, so two maps' weather reads as one.
 */
const hazeFill = (
  context: CanvasRenderingContext2D,
  color: string,
  { x, y, w, h }: Box,
  feather: number,
  joins: { side: Side; from: number; to: number }[],
) => {
  const f = Math.min(feather, Math.floor(Math.min(w, h) / 4))
  const clear = color.replace(")", " / 0)")
  context.fillStyle = color
  context.fillRect(x + f, y + f, w - f * 2, h - f * 2)
  if (f > 0) {
    // The four edges, each a strip fading out, and the corners between them.
    for (const [x0, y0, x1, y1, rx, ry, rw, rh] of [
      [x, 0, x + f, 0, x, y + f, f, h - f * 2],
      [x + w, 0, x + w - f, 0, x + w - f, y + f, f, h - f * 2],
      [0, y, 0, y + f, x + f, y, w - f * 2, f],
      [0, y + h, 0, y + h - f, x + f, y + h - f, w - f * 2, f],
    ] as const) {
      const edge = context.createLinearGradient(x0, y0, x1, y1)
      edge.addColorStop(0, clear)
      edge.addColorStop(1, color)
      context.fillStyle = edge
      context.fillRect(rx, ry, rw, rh)
    }
    for (const [cx, cy, rx, ry] of [
      [x + f, y + f, x, y],
      [x + w - f, y + f, x + w - f, y],
      [x + f, y + h - f, x, y + h - f],
      [x + w - f, y + h - f, x + w - f, y + h - f],
    ] as const) {
      const corner = context.createRadialGradient(cx, cy, 0, cx, cy, f)
      corner.addColorStop(0, color)
      corner.addColorStop(1, clear)
      context.fillStyle = corner
      context.fillRect(rx, ry, f, f)
    }
  }
  // The joined stretches, filled back to full strength out to the edge.
  context.fillStyle = color
  for (const { side, from, to } of joins) {
    if (side === "left") context.fillRect(x, from, f, to - from)
    else if (side === "right") context.fillRect(x + w - f, from, f, to - from)
    else if (side === "top") context.fillRect(from, y, to - from, f)
    else context.fillRect(from, y + h - f, to - from, f)
  }
}

/** The weather overlay, on a clock (`now`, in ms); still at one moment without `animate`. */
export const weatherOverlay = (meta: Meta, animate: boolean, now: () => number): Overlay => {
  const spots: Spot[] = meta.maps.flatMap((map, seed) =>
    map.weather
      ? [{ x: map.x, y: map.y, w: map.width, h: map.height, weather: map.weather, seed, joins: [] }]
      : [],
  )
  for (const spot of spots)
    for (const other of spots)
      if (other !== spot)
        for (const side of ["left", "top", "right", "bottom"] as const) {
          const join = joinOf(spot, other, side)
          if (join) spot.joins.push(join)
        }
  const frozen = now()
  return (context, projection) => {
    if (!spots.length) return
    const { scale, ratio, toScreen } = projection
    const zoom = scale / ratio
    const t = animate ? now() : frozen
    const { width, height } = context.canvas
    const drops = ramp(zoom, DROPS_FROM, DROPS_FULL)
    // A game pixel in device pixels, never less than one.
    const px = Math.max(1, scale / 16)
    const dot = Math.max(1, Math.round(px))
    for (const spot of spots) {
      const a = toScreen(spot.x, spot.y)
      const b = toScreen(spot.x + spot.w, spot.y + spot.h)
      if (b.x < 0 || b.y < 0 || a.x > width || a.y > height) continue
      const x = Math.round(a.x)
      const y = Math.round(a.y)
      const box = { x, y, w: Math.round(b.x) - x, h: Math.round(b.y) - y }
      const { weather, seed } = spot
      context.save()
      context.beginPath()
      context.rect(box.x, box.y, box.w, box.h)
      context.clip()
      const feather = Math.round(FEATHER * scale)
      const joins = spot.joins.map(({ side, from, to }) => {
        const vertical = side === "left" || side === "right"
        const p = vertical ? toScreen(0, from).y : toScreen(from, 0).x
        const q = vertical ? toScreen(0, to).y : toScreen(to, 0).x
        return { side, from: Math.round(p), to: Math.round(q) }
      })
      const [haze, strength] = HAZE[weather]
      if (strength > 0) {
        context.globalAlpha = strength
        hazeFill(context, haze, box, feather, joins)
      }
      if (weather === "storm") {
        // A flash now and then, the map lit white for a moment.
        const since = (t + seed * 1733) % FLASH_EVERY_MS
        if (animate && t > 0 && since < FLASH_MS) {
          context.globalAlpha = 0.45 * (1 - since / FLASH_MS)
          hazeFill(context, "rgb(255 255 255)", box, feather, joins)
        }
      }
      if (drops > 0 && DENSITY[weather] > 0) {
        // The drops hang on a lattice over the map, so only the cells on
        // screen are drawn and there are as many close in as far out. Each
        // runs a loop of its own: a place in its cell from its number, and a
        // phase of its fall from the time.
        const cell = LATTICE * scale
        const perCell = ((cell * cell) / (ratio * ratio) / 1000) * DENSITY[weather] * drops
        const firstI = Math.max(0, Math.floor((0 - a.x) / cell))
        const firstJ = Math.max(0, Math.floor((0 - a.y) / cell))
        const lastI = Math.min(Math.ceil(spot.w / LATTICE), Math.ceil((width - a.x) / cell))
        const lastJ = Math.min(Math.ceil(spot.h / LATTICE), Math.ceil((height - a.y) / cell))
        context.globalAlpha = drops
        for (let j = firstJ; j < lastJ; j++)
          for (let i = firstI; i < lastI; i++) {
            const cx = a.x + i * cell
            const cy = a.y + j * cell
            const here = Math.floor(roll(seed, i, j) * 0xffff)
            // A fraction of a drop is a chance of one.
            const count = Math.min(
              MOST,
              Math.floor(perCell) + (roll(here, 0) < perCell % 1 ? 1 : 0),
            )
            for (let k = 1; k <= count; k++) {
              const ox = roll(here, k, 1)
              const oy = roll(here, k, 2)
              const phase = roll(here, k, 3)
              if (weather === "rain" || weather === "storm") {
                // Falling to the south-west, fast and slanted, as the overworld's rain does.
                const fall = ((t / 600 + phase) % 1) * 1.2
                const dx = Math.round(cx + ((ox - fall * 0.22 + 1) % 1) * cell)
                const dy = Math.round(cy + ((oy + fall) % 1) * cell)
                context.fillStyle = "rgb(200 220 255)"
                context.fillRect(dx, dy, dot, dot * 5)
                context.fillRect(dx + dot, dy - dot * 2, dot, dot * 3)
              } else if (weather === "snow") {
                const fall = (t / 5200 + phase) % 1
                const sway = Math.sin((t / 1400 + phase) * Math.PI * 2) * 3 * px
                const dx = Math.round(cx + ox * cell + sway)
                const dy = Math.round(cy + ((oy + fall) % 1) * cell)
                // A flake of two or three game pixels over a shadow, so it shows on snow too.
                const size = roll(here, k, 4) > 0.5 ? dot * 3 : dot * 2
                context.fillStyle = "rgb(72 88 136 / 0.55)"
                context.fillRect(dx + dot, dy + dot, size, size)
                context.fillStyle = "rgb(255 255 255)"
                context.fillRect(dx, dy, size, size)
              } else if (weather === "sand") {
                // Blowing west, flat and quick.
                const blow = (t / 700 + phase) % 1
                const dx = Math.round(cx + ((ox - blow + 1) % 1) * cell)
                const dy = Math.round(cy + ((oy + blow * 0.08) % 1) * cell)
                // Grit, dark and light by turns, so it shows on the sand and the rock alike.
                context.fillStyle = roll(here, k, 4) > 0.5 ? "rgb(255 244 208)" : "rgb(112 72 32)"
                context.fillRect(dx, dy, dot * 7, dot)
              } else if (weather === "leaves") {
                // Tumbling down and east, turning, in the trail's golds and greens.
                const fall = (t / 4200 + phase) % 1
                const turn = Math.sin((t / 700 + phase) * Math.PI * 2)
                const dx = Math.round(cx + ((ox + fall * 0.35) % 1) * cell + turn * 3 * px)
                const dy = Math.round(cy + ((oy + fall) % 1) * cell)
                context.fillStyle = roll(here, k, 5) > 0.5 ? "rgb(240 200 72)" : "rgb(120 184 56)"
                context.fillRect(dx, dy, turn > 0 ? dot * 2 : dot, dot)
              }
            }
          }
      }
      context.restore()
    }
    context.globalAlpha = 1
  }
}
