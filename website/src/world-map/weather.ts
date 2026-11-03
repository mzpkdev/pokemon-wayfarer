// The game's own weather over the maps that have it (`weather` in meta.json,
// from each map's map.json): rain and thunderstorms along Johto's west coast
// and at the Lake of Rage, snow on Mt. Silver's routes and in Sinnoh's north,
// Route 228's sandstorm, the shade under the forests and the leaves on
// Bellchime Trail. Each is drawn within its map's rectangle, in game pixels,
// from a hash of the map, the drop and the time, so nothing is stored.
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
  fog: 0.12,
  ash: 0.5,
  shade: 0,
  leaves: 0.25,
}
/** The most drops one map draws in a frame. */
const MOST = 700
/** The haze each weather lays over its map: a color and how strong, there from far out. */
const HAZE: Record<Weather, [string, number]> = {
  rain: ["rgb(24 40 96)", 0.22],
  storm: ["rgb(16 24 72)", 0.3],
  snow: ["rgb(232 240 255)", 0.12],
  sand: ["rgb(214 178 96)", 0.3],
  fog: ["rgb(236 240 248)", 0.32],
  ash: ["rgb(96 96 104)", 0.26],
  shade: ["rgb(0 8 40)", 0.24],
  leaves: ["rgb(0 0 0)", 0],
}
/** How far in from a map's edge its haze fades, in metatiles, at most: a quarter of its narrower side. */
const FEATHER = 10
/** How often a storm flashes, in ms, and how long a flash lasts. */
const FLASH_EVERY_MS = 5200
const FLASH_MS = 90

type Spot = { x: number; y: number; w: number; h: number; weather: Weather; seed: number }

/** The weather overlay, on a clock (`now`, in ms); still at one moment without `animate`. */
export const weatherOverlay = (meta: Meta, animate: boolean, now: () => number): Overlay => {
  const spots: Spot[] = meta.maps.flatMap((map, seed) =>
    map.weather
      ? [{ x: map.x, y: map.y, w: map.width, h: map.height, weather: map.weather, seed }]
      : [],
  )
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
        // world as a hard-edged box where the map runs over open sea.
        const feather = Math.round(Math.min(FEATHER * scale, Math.min(w, h) / 4))
        const clear = haze.replace(")", " / 0)")
        context.globalAlpha = strength
        context.fillStyle = haze
        context.fillRect(x + feather, y + feather, w - feather * 2, h - feather * 2)
        for (const [x0, y0, x1, y1, rx, ry, rw, rh] of [
          [x, y + feather, x + feather, y + feather, x, y + feather, feather, h - feather * 2],
          [
            x + w,
            y + feather,
            x + w - feather,
            y + feather,
            x + w - feather,
            y + feather,
            feather,
            h - feather * 2,
          ],
          [x + feather, y, x + feather, y + feather, x + feather, y, w - feather * 2, feather],
          [
            x + feather,
            y + h,
            x + feather,
            y + h - feather,
            x + feather,
            y + h - feather,
            w - feather * 2,
            feather,
          ],
        ] as const) {
          const edge = context.createLinearGradient(x0, y0, x1, y1)
          edge.addColorStop(0, clear)
          edge.addColorStop(1, haze)
          context.fillStyle = edge
          context.fillRect(rx, ry, rw, rh)
        }
        // The corners, lit from the inside corner out.
        for (const [cx, cy, rx, ry] of [
          [x + feather, y + feather, x, y],
          [x + w - feather, y + feather, x + w - feather, y],
          [x + feather, y + h - feather, x, y + h - feather],
          [x + w - feather, y + h - feather, x + w - feather, y + h - feather],
        ] as const) {
          const corner = context.createRadialGradient(cx, cy, 0, cx, cy, feather)
          corner.addColorStop(0, haze)
          corner.addColorStop(1, clear)
          context.fillStyle = corner
          context.fillRect(rx, ry, feather, feather)
        }
      }
      if (weather === "storm") {
        // A flash now and then, the map lit white for a moment.
        const since = (t + seed * 1733) % FLASH_EVERY_MS
        if (since < FLASH_MS && animate) {
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
          } else if (weather === "fog") {
            // Banks of mist, long and slow, drifting east.
            const drift = (t / 14000 + phase) % 1
            const dx = x + Math.round(((ox + drift) % 1) * w)
            const dy = y + Math.round(oy * h)
            context.fillStyle = "rgb(255 255 255)"
            context.globalAlpha = drops * 0.16
            context.fillRect(dx - dot * 14, dy, dot * 28, dot * 3)
            context.globalAlpha = drops
          } else if (weather === "ash") {
            const fall = (t / 7000 + phase) % 1
            const sway = Math.sin((t / 2200 + phase) * Math.PI * 2) * 2 * px
            const dx = x + Math.round(ox * w + sway)
            const dy = y + Math.round(((oy + fall) % 1) * h)
            context.fillStyle = "rgb(190 190 190)"
            context.fillRect(dx, dy, dot, dot)
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
