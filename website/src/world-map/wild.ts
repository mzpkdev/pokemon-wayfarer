// The wild stop: who lives on the routes, one generation after another.
// Icons pop out of the routes in waves (I to VIII, then the regional forms,
// then a mix), looping, while a tally of species per generation counts up
// beside them. The data is what `build/encounters.mjs` writes.
import { DATA, loadWorld } from "./data"
import { ICON, loadEncounters } from "./encounters"
import type { Overlay } from "./render"

/** The waves: Gen I to VIII, the regional forms, then a mix of all of them. */
export const WAVES = ["I", "II", "III", "IV", "V", "VI", "VII", "VIII", "FORMS", "ALL"] as const
/** The wave's generation as `encounters.json` gives it: 1 to 8, 0 for regional forms, none for the mix. */
/** A wave with something to show, one without, and the mix at the end, in ms. */
const WAVE_MS = 1400
const EMPTY_WAVE_MS = 800
const FINALE_MS = 1800
/** How long an icon takes to pop, how long it lives, and when within a wave the last starts, in ms. */
const POP_MS = 260
const LIFE_MS = 1200
const SPREAD_MS = 500
/** How long a chip's count takes to tick up, in ms. */
export const TICK_MS = 700
/** The most icons up per wave, on a wide screen and on a narrow one. */
const CAP = 24
const CAP_NARROW = 12
/** An icon's size on screen, in CSS pixels, before rounding to a whole multiple of the atlas. */
const SIZE = 64
const SIZE_NARROW = 32

export type Wild = {
  counts: number[]
  /** Per wave, the icons in the order they pop: farthest-apart routes first. */
  waves: { x: number; y: number; slot: number }[][]
  /** The icon atlas, once decoded. */
  icons: HTMLImageElement
  columns: number
  /** The wave of each generation, for a chip. */
  durations: number[]
}

/**
 * Where the tour is in the cycle: when it began, and, with reduced motion or
 * after a tap in it, the wave held still instead (the loop plays when null).
 */
export type WildClock = { start: number; still: number | null }

const hash = (n: number) => {
  let h = Math.imul(n ^ 0x9e3779b9, 0x85ebca6b)
  h ^= h >>> 13
  h = Math.imul(h, 0xc2b2ae35)
  return (h ^ (h >>> 16)) >>> 0
}

/** The wave starts and the whole cycle's length, in ms. */
export const timeline = (counts: number[]) => {
  const durations = WAVES.map((_, i) =>
    i === WAVES.length - 1 ? FINALE_MS : counts[i] ? WAVE_MS : EMPTY_WAVE_MS,
  )
  const starts = durations.map((_, i) => durations.slice(0, i).reduce((a, b) => a + b, 0))
  return { durations, starts, cycle: durations.reduce((a, b) => a + b, 0) }
}

/** The wave playing `elapsed` ms into the cycle, and how far into it. */
export const waveAt = (counts: number[], elapsed: number) => {
  const { starts, cycle } = timeline(counts)
  const t = ((elapsed % cycle) + cycle) % cycle
  const wave = starts.findLastIndex((start) => start <= t)
  return { wave, into: t - starts[wave]! }
}

/** The clock's start that has `wave` begin at `now`. */
export const startOf = (counts: number[], wave: number, now: number) =>
  now - timeline(counts).starts[wave]!

/** Loads the encounters, the atlas and the placed maps, and picks each wave's routes. */
export const loadWild = async (base = DATA): Promise<Wild> => {
  const icons = new Image()
  icons.src = `${base}/icons.png`
  const [encounters, world] = await Promise.all([
    loadEncounters(base),
    loadWorld(base),
    icons.decode().catch(() => {}),
  ])
  const { roster, counts } = encounters
  const rects = new Map(world.meta.maps.map((map) => [map.name, map]))
  // Every route with its own wild Pokémon is a place for an icon to pop up;
  // the icons are the whole roster of the wave's generation, an illustration
  // of what the world holds, not who lives on that route (the explorer's
  // place cards say that).
  const routes = Object.entries(encounters.maps).flatMap(([name, place]) => {
    const rect = rects.get(name)
    if (!rect || !place.e) return []
    const onLand = "land" in place.e || "rock" in place.e
    return [{ x: rect.x + rect.width / 2, y: rect.y + rect.height / 2, onLand }]
  })
  const landed = routes.filter((route) => route.onLand)
  const waves = WAVES.map((_, wave) => {
    const pool = wave < roster.length ? roster[wave]! : roster.flat()
    if (!pool.length) return []
    const places = landed.length >= CAP ? landed : routes
    // Farthest-point sampling spreads the icons over the world; the start varies per wave.
    const picked = [places[hash(wave + 1) % places.length]!]
    const nearest = places.map((route) =>
      Math.hypot(route.x - picked[0]!.x, route.y - picked[0]!.y),
    )
    while (picked.length < Math.min(CAP, places.length)) {
      let best = 0
      nearest.forEach((d, i) => {
        if (d > nearest[best]!) best = i
      })
      const route = places[best]!
      picked.push(route)
      places.forEach((other, i) => {
        nearest[i] = Math.min(nearest[i]!, Math.hypot(other.x - route.x, other.y - route.y))
      })
    }
    // A different species for each place, drawn from the wave's roster.
    const used = new Set<number>()
    return picked.map((route, i) => {
      let slot = pool[hash(wave * 131 + i) % pool.length]!
      for (let step = 1; used.has(slot) && step < pool.length; step++)
        slot = pool[(hash(wave * 131 + i) + step) % pool.length]!
      used.add(slot)
      return { x: route.x, y: route.y, slot }
    })
  })
  return {
    counts,
    waves,
    icons,
    columns: encounters.columns,
    durations: timeline(counts).durations,
  }
}

/** Overshoots a little on the way up: 0 to 1 over `t` in [0, 1]. */
const pop = (t: number) => {
  const c = 2.2
  return 1 + (c + 1) * (t - 1) ** 3 + c * (t - 1) ** 2
}

/** The icons over the map: popping in their wave, or all still when `still` is set. */
export const wildOverlay =
  (wild: Wild, clock: WildClock, animate: boolean, now: () => number): Overlay =>
  (context, projection) => {
    if (!wild.icons.complete || !wild.icons.naturalWidth) return
    const narrow = context.canvas.width / projection.ratio < 700
    const cap = narrow ? CAP_NARROW : CAP
    // A whole multiple of the atlas' pixels, so the icons stay crisp.
    const k = Math.max(1, Math.round(((narrow ? SIZE_NARROW : SIZE) * projection.ratio) / ICON))
    const size = ICON * k
    context.imageSmoothingEnabled = false
    // No two overlap: a route too close to one already up is left out.
    const spread = (icons: Wild["waves"][number]) => {
      const kept: { x: number; y: number; slot: number }[] = []
      const spots: { x: number; y: number }[] = []
      for (const icon of icons) {
        const at = projection.toScreen(icon.x, icon.y)
        if (spots.some((spot) => Math.hypot(spot.x - at.x, spot.y - at.y) < size * 1.15)) continue
        spots.push(at)
        kept.push(icon)
        if (kept.length === cap) break
      }
      return kept
    }
    const draw = (
      icon: { x: number; y: number; slot: number },
      alpha: number,
      scale: number,
      lift: number,
    ) => {
      const at = projection.toScreen(icon.x, icon.y)
      const w = Math.max(k, Math.round(size * scale))
      const x = Math.round(at.x - w / 2)
      const y = Math.round(at.y - w - lift * projection.ratio)
      // A flat shadow in whole pixels, the way the overworld draws one: three
      // rows, the middle widest.
      context.globalAlpha = alpha * 0.35
      context.fillStyle = "#000"
      const row = k
      const cx = Math.round(at.x)
      const cy = Math.round(at.y - 3 * k)
      for (const [dy, width] of [
        [0, 0.36],
        [1, 0.56],
        [2, 0.36],
      ] as const) {
        const half = Math.round((w * width) / 2 / k) * k
        context.fillRect(cx - half, cy + dy * row, half * 2, row)
      }
      context.globalAlpha = alpha
      context.drawImage(
        wild.icons,
        (icon.slot % wild.columns) * ICON,
        Math.floor(icon.slot / wild.columns) * ICON,
        ICON,
        ICON,
        x,
        y,
        w,
        w,
      )
    }
    // A held wave stays up (popping in once when it's picked); with reduced
    // motion the mix, or the picked wave, just stands.
    if (!animate || clock.still !== null) {
      const wave = clock.still ?? WAVES.length - 1
      const icons = spread(wild.waves[wave]!)
      icons.forEach((icon, i) => {
        const age = animate
          ? now() - clock.start - (i / Math.max(1, icons.length)) * SPREAD_MS
          : Infinity
        if (age < 0) return
        const t = Math.min(1, age / POP_MS)
        draw(icon, 1, Math.max(0.05, pop(t)), age < POP_MS ? (1 - t) * 10 : 0)
      })
      context.globalAlpha = 1
      return
    }
    const { wave, into } = waveAt(wild.counts, now() - clock.start)
    const icons = spread(wild.waves[wave]!)
    icons.forEach((icon, i) => {
      const age = into - (i / Math.max(1, icons.length)) * SPREAD_MS
      if (age < 0 || age > LIFE_MS) return
      const t = Math.min(1, age / POP_MS)
      // It bounces as it lands and rises a little as it goes.
      const fade = Math.min(1, (LIFE_MS - age) / 300)
      const bounce = age < POP_MS ? (1 - t) * 10 : 0
      draw(icon, Math.max(0, fade), Math.max(0.05, pop(t)), bounce + (age / LIFE_MS) * 6)
    })
    context.globalAlpha = 1
  }
