// The game's own weather over the maps that have it (`weather` in meta.json,
// from each map's map.json): rain and thunderstorms along Johto's west coast
// and at the Lake of Rage, snow on Mt. Silver's routes and in Sinnoh's north,
// Route 228's sandstorm, the shade under the forests and the leaves on
// Bellchime Trail. Each is drawn within its map's rectangle, in game pixels,
// from a hash of the map, the drop and the time, so nothing is stored. The
// haze fades in from the map's edges, except where the next map has weather
// too, so a rainy coast reads as one.
import type { Meta, Weather } from "./data"
import { roll, ramp } from "./noise"
import type { Overlay } from "./render"

/** CSS pixels per metatile from which the drops, flakes and streaks show, and where they are all there. */
const DROPS_FROM = 2
const DROPS_FULL = 4
/** How many drops a map gets per thousand CSS pixels of its area on screen, by weather. */
const DENSITY: Record<Weather, number> = {
  rain: 1.1,
  storm: 1.5,
  snow: 0.7,
  sand: 0.9,
  shade: 0,
  leaves: 0.25,
}
/** The most drops one map draws in a frame. */
const MOST = 700
/** The haze each weather lays over its map: a color and how strong, there from far out. */
const HAZE: Record<Weather, [string, number]> = {
  rain: ["rgb(24 40 96)", 0.22],
  storm: ["rgb(16 24 72)", 0.24],
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

type Spot = {
  x: number
  y: number
  w: number
  h: number
  weather: Weather
  seed: number
  /** Which edges fade in: not the ones another map with weather lies along. */
  fade: { left: boolean; top: boolean; right: boolean; bottom: boolean }
}

/** Whether two spots lie along each other's edge: `b` across `a`'s `side`, overlapping it. */
const along = (a: Spot, b: Spot, side: "left" | "top" | "right" | "bottom") => {
  const across =
    side === "left" || side === "right"
      ? b.y < a.y + a.h && b.y + b.h > a.y
      : b.x < a.x + a.w && b.x + b.w > a.x
  if (!across) return false
  if (side === "left") return b.x + b.w === a.x
  if (side === "right") return b.x === a.x + a.w
  if (side === "top") return b.y + b.h === a.y
  return b.y === a.y + a.h
}

/** The weather overlay, on a clock (`now`, in ms); still at one moment without `animate`. */
export const weatherOverlay = (meta: Meta, animate: boolean, now: () => number): Overlay => {
  const spots: Spot[] = meta.maps.flatMap((map, seed) =>
    map.weather
      ? [
          {
            x: map.x,
            y: map.y,
            w: map.width,
            h: map.height,
            weather: map.weather,
            seed,
            fade: { left: true, top: true, right: true, bottom: true },
          },
        ]
      : [],
  )
  for (const spot of spots)
    for (const other of spots)
      if (other !== spot)
        for (const side of ["left", "top", "right", "bottom"] as const)
          if (along(spot, other, side)) spot.fade[side] = false
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
      const w = Math.round(b.x) - x
      const h = Math.round(b.y) - y
      const { weather, seed } = spot
      context.save()
      context.beginPath()
      context.rect(x, y, w, h)
      context.clip()
      const [haze, strength] = HAZE[weather]
      if (strength > 0) {
        // The haze fades in from the map's edges, so it doesn't lie on the
        // world as a hard-edged box where the map runs over open sea; an
        // edge another map's weather lies along stays full, so the two read
        // as one.
        const most = Math.round(Math.min(FEATHER * scale, Math.min(w, h) / 4))
        const f = {
          left: spot.fade.left ? most : 0,
          top: spot.fade.top ? most : 0,
          right: spot.fade.right ? most : 0,
          bottom: spot.fade.bottom ? most : 0,
        }
        const clear = haze.replace(")", " / 0)")
        context.globalAlpha = strength
        context.fillStyle = haze
        context.fillRect(x + f.left, y + f.top, w - f.left - f.right, h - f.top - f.bottom)
        // The four edges, each a strip fading out, and the corners between them.
        const innerX = x + f.left
        const innerY = y + f.top
        const innerW = w - f.left - f.right
        const innerH = h - f.top - f.bottom
        for (const [size, x0, y0, x1, y1, rx, ry, rw, rh] of [
          [f.left, x, 0, x + f.left, 0, x, innerY, f.left, innerH],
          [f.right, x + w, 0, x + w - f.right, 0, x + w - f.right, innerY, f.right, innerH],
          [f.top, 0, y, 0, y + f.top, innerX, y, innerW, f.top],
          [f.bottom, 0, y + h, 0, y + h - f.bottom, innerX, y + h - f.bottom, innerW, f.bottom],
        ] as const) {
          if (!size) continue
          const edge = context.createLinearGradient(x0, y0, x1, y1)
          edge.addColorStop(0, clear)
          edge.addColorStop(1, haze)
          context.fillStyle = edge
          context.fillRect(rx, ry, rw, rh)
        }
        for (const [fx, fy, cx, cy, rx, ry] of [
          [f.left, f.top, innerX, innerY, x, y],
          [f.right, f.top, innerX + innerW, innerY, innerX + innerW, y],
          [f.left, f.bottom, innerX, innerY + innerH, x, innerY + innerH],
          [f.right, f.bottom, innerX + innerW, innerY + innerH, innerX + innerW, innerY + innerH],
        ] as const) {
          if (!fx || !fy) continue
          const corner = context.createRadialGradient(cx, cy, 0, cx, cy, fx)
          corner.addColorStop(0, haze)
          corner.addColorStop(1, clear)
          context.fillStyle = corner
          context.fillRect(rx, ry, fx, fy)
        }
      }
      if (weather === "storm") {
        // A flash now and then, the map lit white for a moment.
        const since = (t + seed * 1733) % FLASH_EVERY_MS
        if (animate && t > 0 && since < FLASH_MS) {
          context.globalAlpha = 0.45 * (1 - since / FLASH_MS)
          context.fillStyle = "rgb(255 255 255)"
          context.fillRect(x, y, w, h)
        }
      }
      if (drops > 0 && DENSITY[weather] > 0) {
        const count = Math.min(
          MOST,
          Math.round(((w * h) / (ratio * ratio) / 1000) * DENSITY[weather] * drops),
        )
        context.globalAlpha = drops
        // Each drop runs a loop of its own: a position in the map from its
        // number, and a phase of its fall from the time.
        for (let k = 0; k < count; k++) {
          const ox = roll(seed, k, 1)
          const oy = roll(seed, k, 2)
          const phase = roll(seed, k, 3)
          if (weather === "rain" || weather === "storm") {
            // Falling to the south-west, fast and slanted, as the overworld's rain does.
            const fall = ((t / 600 + phase) % 1) * 1.2
            const dx = x + Math.round(((ox - fall * 0.22 + 1) % 1) * w)
            const dy = y + Math.round(((oy + fall) % 1) * h)
            context.fillStyle = "rgb(200 220 255)"
            context.fillRect(dx, dy, dot, dot * 5)
            context.fillRect(dx + dot, dy - dot * 2, dot, dot * 3)
          } else if (weather === "snow") {
            const fall = (t / 5200 + phase) % 1
            const sway = Math.sin((t / 1400 + phase) * Math.PI * 2) * 3 * px
            const dx = x + Math.round(((ox + 1) % 1) * w + sway)
            const dy = y + Math.round(((oy + fall) % 1) * h)
            // A flake of two or three game pixels over a shadow, so it shows on snow too.
            const big = roll(seed, k, 4) > 0.5
            const size = big ? dot * 3 : dot * 2
            context.fillStyle = "rgb(72 88 136 / 0.55)"
            context.fillRect(dx + dot, dy + dot, size, size)
            context.fillStyle = "rgb(255 255 255)"
            context.fillRect(dx, dy, size, size)
          } else if (weather === "sand") {
            // Blowing west, flat and quick.
            const blow = (t / 700 + phase) % 1
            const dx = x + Math.round(((ox - blow + 1) % 1) * w)
            const dy = y + Math.round(((oy + blow * 0.08) % 1) * h)
            // Grit, dark and light by turns, so it shows on the sand and the rock alike.
            context.fillStyle = roll(seed, k, 4) > 0.5 ? "rgb(255 244 208)" : "rgb(112 72 32)"
            context.fillRect(dx, dy, dot * 7, dot)
          } else if (weather === "leaves") {
            // Tumbling down and east, turning, in the trail's golds and greens.
            const fall = (t / 4200 + phase) % 1
            const turn = Math.sin((t / 700 + phase) * Math.PI * 2)
            const dx = x + Math.round(((ox + fall * 0.35) % 1) * w + turn * 3 * px)
            const dy = y + Math.round(((oy + fall) % 1) * h)
            context.fillStyle = roll(seed, k, 5) > 0.5 ? "rgb(240 200 72)" : "rgb(120 184 56)"
            context.fillRect(dx, dy, turn > 0 ? dot * 2 : dot, dot)
          }
        }
      }
      context.restore()
    }
    context.globalAlpha = 1
  }
}
