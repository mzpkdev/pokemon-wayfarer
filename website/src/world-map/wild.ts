// The wild stop: who lives where, one home region after another. Each
// generation has a home (the wild encounters v2 PRD's natives table): Gen I
// in Kanto, II in Johto, III and V in Hoenn, Kalos in the Safari Zones,
// Galar in the Sevii Islands, VII in Alola, Hisui in Sinjoh, and Gen IV
// waits for Sinnoh. A wave pops its home's species where they really live,
// crowding the home region, with a few of the real strays elsewhere, looping,
// while a tally per home counts up beside them. The data is what
// `build/encounters.mjs` writes.
import { DATA, loadWorld } from "./data"
import { ICON, loadEncounters } from "./encounters"
import type { Overlay } from "./render"

/** A home region's wave: its chip, what the card says while it plays, and where on the map it lives. */
export type Home = {
  id: string
  /** The chip's label. */
  chip: string
  /** Its generations, for the chip's label to a screen reader and the card's line. */
  line: string
  /** The `region` of its placed maps in `meta.json`; the Safari Zones use their gates instead. */
  regions: string[]
  /** Of a region shared with another home, its own side: the maps nearer this name's label in `meta.regions`. */
  side?: string
  /** Not in the wild yet: drawn as silhouettes. */
  later?: boolean
}

export const HOMES: Home[] = [
  {
    id: "kanto",
    chip: "KANTO",
    line: "Gen I, at home in Kanto.",
    regions: ["Johto and Kanto"],
    side: "KANTO",
  },
  {
    id: "johto",
    chip: "JOHTO",
    line: "Gen II, at home in Johto.",
    regions: ["Johto and Kanto"],
    side: "JOHTO",
  },
  {
    id: "hoenn",
    chip: "HOENN",
    line: "Gen III, and more of Gen V the further east you go.",
    regions: ["Hoenn"],
  },
  {
    id: "safari",
    chip: "SAFARI",
    line: "Kalos lives in the three Safari Zones, a third in each.",
    regions: [],
  },
  {
    id: "sevii",
    chip: "SEVII",
    line: "The Sevii Islands are Galar: Gen VIII and the Galarian forms.",
    regions: ["Sevii Islands"],
  },
  {
    id: "alola",
    chip: "ALOLA",
    line: "Gen VII and the Alolan forms, in Alola.",
    regions: ["Alola"],
  },
  {
    id: "sinjoh",
    chip: "SINJOH",
    line: "Sinjoh is Hisui: its own species and the Hisuian forms.",
    regions: ["Sinjoh"],
  },
  {
    id: "sinnoh",
    chip: "SINNOH",
    line: "Gen IV waits for Sinnoh, coming later.",
    regions: ["Sinnoh"],
    later: true,
  },
]

/** A home's wave, in ms. */
const WAVE_MS = 1900
/** How long an icon takes to pop, how long it lives, and when within a wave the last starts, in ms. */
const POP_MS = 260
/** The idle hop: a pixel up and back, like a party icon's two frames, this often. */
const HOP_MS = 260
const LIFE_MS = 1650
const SPREAD_MS = 600
/** How long a chip's count takes to tick up, in ms. */
export const TICK_MS = 700
/** The most icons up per wave, on a wide screen and on a narrow one. */
const CAP = 24
const CAP_NARROW = 12
/** Of those, the most away from home: real strays, never more than a few. */
const AWAY = 3
/** Where the strays go in a wave's order, so a narrow screen keeps some too. */
const AWAY_AT = [3, 8, 14]
/** Icons crowd at home, overlapping a little, but keep their distance away. */
const CROWD = 0.7
/** A home with fewer routes than this huddles instead of spreading out. */
const SMALL = 8
/** A huddle's places around its point, in icon sizes (down is nearer). */
const HUDDLE: [number, number][] = [
  [0, 0],
  [-0.75, 0.3],
  [0.75, 0.3],
  [-0.4, -0.45],
  [0.4, -0.45],
]
/** How many silhouettes stand in for a home still to come. */
const LATER = 12
/** An icon's size on screen, in CSS pixels, before rounding to a whole multiple of the atlas. */
const SIZE = 64
const SIZE_NARROW = 32

/** An icon on the map: a stray away from its home, or a silhouette of one not in the wild yet. */
type Icon = {
  x: number
  y: number
  slot: number
  away?: boolean
  later?: boolean
  /** Off its point by this many icon sizes, on screen: a place in a huddle. */
  offset?: [number, number]
}

export type Wild = {
  counts: number[]
  /** Per wave, the icons in the order they pop. */
  waves: Icon[][]
  /** The icon atlas, once decoded, and its silhouettes for the homes still to come. */
  icons: HTMLImageElement
  silhouettes: HTMLCanvasElement | undefined
  columns: number
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
export const timeline = () => {
  const durations = HOMES.map(() => WAVE_MS)
  const starts = durations.map((_, i) => durations.slice(0, i).reduce((a, b) => a + b, 0))
  return { durations, starts, cycle: durations.reduce((a, b) => a + b, 0) }
}

/** The wave playing `elapsed` ms into the cycle, and how far into it. */
export const waveAt = (elapsed: number) => {
  const { starts, durations, cycle } = timeline()
  const t = ((elapsed % cycle) + cycle) % cycle
  const wave = starts.findLastIndex((start) => start <= t)
  return { wave, into: t - starts[wave]!, length: durations[wave]! }
}

/** The clock's start that has `wave` begin at `now`. */
export const startOf = (wave: number, now: number) => now - timeline().starts[wave]!

/** Farthest-point sampling: up to `count` of `points`, spread out, from a start that varies by `seed`. */
const spreadOut = <T extends { x: number; y: number }>(
  points: T[],
  count: number,
  seed: number,
) => {
  if (!points.length) return []
  const picked = [points[hash(seed) % points.length]!]
  const nearest = points.map((p) => Math.hypot(p.x - picked[0]!.x, p.y - picked[0]!.y))
  while (picked.length < Math.min(count, points.length)) {
    let best = 0
    nearest.forEach((d, i) => {
      if (d > nearest[best]!) best = i
    })
    const point = points[best]!
    picked.push(point)
    points.forEach((other, i) => {
      nearest[i] = Math.min(nearest[i]!, Math.hypot(other.x - point.x, other.y - point.y))
    })
  }
  return picked
}

/** One of `choices`, by weight, unused ones first. */
const choose = (choices: [slot: number, weight: number][], used: Set<number>, seed: number) => {
  const fresh = choices.filter(([slot]) => !used.has(slot))
  const pool = fresh.length ? fresh : choices
  const total = pool.reduce((sum, [, weight]) => sum + weight, 0)
  let pick = (hash(seed) / 0x100000000) * total
  for (const [slot, weight] of pool) if ((pick -= weight) < 0) return slot
  return pool.at(-1)![0]
}

/** A huddle around a point, in icon sizes, so a home that's a speck at world zoom still crowds. */
const huddle = (
  anchor: { x: number; y: number },
  choices: [number, number][],
  used: Set<number>,
  seed: number,
) =>
  HUDDLE.map((offset, i) => {
    const slot = choose(choices, used, seed + i * 3)
    used.add(slot)
    return { ...anchor, slot, offset }
  })

/** The atlas in black: who isn't in the wild yet, the way the Pokédex shows one unseen. */
const silhouette = (icons: HTMLImageElement) => {
  if (!icons.naturalWidth) return undefined
  const canvas = document.createElement("canvas")
  canvas.width = icons.naturalWidth
  canvas.height = icons.naturalHeight
  const context = canvas.getContext("2d")!
  context.drawImage(icons, 0, 0)
  context.globalCompositeOperation = "source-in"
  context.fillStyle = "rgb(16 16 24)"
  context.fillRect(0, 0, canvas.width, canvas.height)
  return canvas
}

/** Loads the encounters, the atlas and the placed maps, and picks each wave's icons. */
export const loadWild = async (base = DATA): Promise<Wild> => {
  const icons = new Image()
  icons.src = `${base}/icons.png`
  const [encounters, world] = await Promise.all([
    loadEncounters(base),
    loadWorld(base),
    icons.decode().catch(() => {}),
  ])
  const placed = new Map(world.meta.maps.map((map) => [map.name, map]))
  const homes = new Map(encounters.homes.map((home) => [home.id, new Set(home.slots)]))

  // Each placed map's wild Pokémon as roster entries, with how often they
  // come up across its methods and times of day.
  const routes = Object.entries(encounters.maps).flatMap(([name, place]) => {
    const map = placed.get(name)
    if (!map || !place.e) return []
    const weights = new Map<number, number>()
    for (const table of Object.values(place.e))
      for (const slots of Object.values(table))
        for (const [species, percent] of slots) {
          const entry = encounters.entry[species] ?? -1
          if (entry >= 0) weights.set(entry, (weights.get(entry) ?? 0) + Math.max(1, percent))
        }
    return [{ map, weights }]
  })
  const centre = (map: (typeof world.meta.maps)[number]) => ({
    x: map.x + map.width / 2,
    y: map.y + map.height / 2,
  })
  const waves: Icon[][] = []
  // Kanto and Johto share one piece of land: a map is on the side whose label is nearer.
  const label = new Map(world.meta.regions.map((region) => [region.name, region]))
  const sides = HOMES.flatMap((home) => (home.side ? [home.side] : []))
  const isHome = (home: Home) => (map: (typeof world.meta.maps)[number]) => {
    if (!home.regions.includes(map.region ?? "")) return false
    if (!home.side) return true
    const { x, y } = centre(map)
    const distance = (name: string) => {
      const at = label.get(name)
      return at ? Math.hypot(at.x - x, at.y - y) : Infinity
    }
    return sides.every((side) => distance(home.side!) <= distance(side))
  }
  HOMES.forEach((home, wave) => {
    const inHome = isHome(home)
    const slots = homes.get(home.id) ?? new Set<number>()
    const used = new Set<number>()
    const seed = (wave + 1) * 1009
    const at = (weights: Map<number, number>) => [...weights].filter(([slot]) => slots.has(slot))
    let crowd: Icon[]
    if (home.id === "safari") {
      // Each Safari Zone's own Kalos, huddled at its gate.
      crowd = encounters.safari.flatMap(({ near, slots: zone }, z) => {
        const map = placed.get(near)
        if (!map) return []
        return huddle(
          centre(map),
          zone.map((slot) => [slot, 1] as [number, number]),
          used,
          seed + z * 101,
        )
      })
    } else if (home.later) {
      // Nobody lives there yet: some of the roster over its maps, in silhouette.
      const maps = world.meta.maps.filter(inHome)
      const choices = [...slots].map((slot) => [slot, 1] as [number, number])
      crowd = spreadOut(maps.map(centre), LATER, seed).map((point, i) => {
        const slot = choose(choices, used, seed + i)
        used.add(slot)
        return { ...point, slot, later: true }
      })
    } else {
      const homeRoutes = routes.filter(({ map, weights }) => inHome(map) && at(weights).length)
      const points = homeRoutes.map(({ map, weights }) => ({ ...centre(map), weights }))
      crowd =
        homeRoutes.length >= SMALL
          ? spreadOut(points, CAP - AWAY, seed).map(({ x, y, weights }, i) => {
              const slot = choose(at(weights), used, seed + i * 3)
              used.add(slot)
              return { x, y, slot }
            })
          : // A small home huddles on a few of its routes instead.
            spreadOut(points, 3, seed).flatMap(({ x, y, weights }, r) =>
              huddle({ x, y }, at(weights), used, seed + r * 17),
            )
    }
    // The real strays: this home's species where they live away from it.
    const awayRoutes = routes.filter(({ map, weights }) => !inHome(map) && at(weights).length)
    const away = spreadOut(
      awayRoutes.map(({ map, weights }) => ({
        x: map.x + map.width / 2,
        y: map.y + map.height / 2,
        weights,
      })),
      AWAY,
      seed + 5,
    ).map(({ x, y, weights }, i) => {
      const slot = choose(at(weights), used, seed + 400 + i)
      used.add(slot)
      return { x, y, slot, away: true }
    })
    const ordered = [...crowd]
    away.forEach((icon, i) => ordered.splice(Math.min(AWAY_AT[i]!, ordered.length), 0, icon))
    waves.push(ordered)
  })

  return {
    counts: HOMES.map((home) => homes.get(home.id)?.size ?? 0),
    waves,
    icons,
    silhouettes: HOMES.some((home) => home.later) ? silhouette(icons) : undefined,
    columns: encounters.columns,
  }
}

/** Overshoots a little on the way up: 0 to 1 over `t` in [0, 1]. */
const pop = (t: number) => {
  const c = 2.2
  return 1 + (c + 1) * (t - 1) ** 3 + c * (t - 1) ** 2
}

/** The icons over the map: popping in their wave, or all still when `still` is set. */
export const wildOverlay = (
  wild: Wild,
  clock: WildClock,
  animate: boolean,
  now: () => number,
): Overlay => {
  return (context, projection) => {
    if (!wild.icons.complete || !wild.icons.naturalWidth) return
    const narrow = context.canvas.width / projection.ratio < 700
    const cap = narrow ? CAP_NARROW : CAP
    // A whole multiple of the atlas' pixels, so the icons stay crisp.
    const k = Math.max(1, Math.round(((narrow ? SIZE_NARROW : SIZE) * projection.ratio) / ICON))
    const size = ICON * k
    context.imageSmoothingEnabled = false
    const place = (icon: Icon) => {
      const at = projection.toScreen(icon.x, icon.y)
      const [dx, dy] = icon.offset ?? [0, 0]
      return { x: at.x + dx * size, y: at.y + dy * size }
    }
    // Home crowds may overlap a little; a stray too close to one already up is left out.
    const spread = (icons: Icon[]) => {
      const kept: Icon[] = []
      const spots: { x: number; y: number }[] = []
      for (const icon of icons) {
        const at = place(icon)
        const room = size * (icon.away ? 1.15 : CROWD)
        if (spots.some((spot) => Math.hypot(spot.x - at.x, spot.y - at.y) < room)) continue
        spots.push(at)
        kept.push(icon)
        if (kept.length === cap) break
      }
      // Front to back: the lower on the map, the nearer.
      return kept
        .map((icon, order) => ({ icon, order }))
        .toSorted((a, b) => place(a.icon).y - place(b.icon).y)
    }
    const draw = (icon: Icon, alpha: number, scale: number, lift: number) => {
      const at = place(icon)
      const w = Math.max(k, Math.round(size * scale))
      const x = Math.round(at.x - w / 2)
      // Each hops a pixel on its own beat, so the crowd never moves as one.
      const hop = animate && Math.floor((now() + icon.slot * 97) / HOP_MS) % 2 ? k : 0
      const y = Math.round(at.y - w - lift * projection.ratio) - hop
      // A flat shadow in whole pixels, the way the overworld draws one: three
      // rows, the middle widest.
      context.globalAlpha = alpha * 0.35
      context.fillStyle = "#000"
      const cx = Math.round(at.x)
      const cy = Math.round(at.y - 3 * k)
      for (const [dy, width] of [
        [0, 0.36],
        [1, 0.56],
        [2, 0.36],
      ] as const) {
        const half = Math.round((w * width) / 2 / k) * k
        context.fillRect(cx - half, cy + dy * k, half * 2, k)
      }
      context.globalAlpha = alpha
      const sheet = icon.later && wild.silhouettes ? wild.silhouettes : wild.icons
      context.drawImage(
        sheet,
        (icon.slot % wild.columns) * ICON,
        Math.floor(icon.slot / wild.columns) * ICON,
        ICON,
        ICON,
        x,
        y,
        w,
        w,
      )
      // A "?" over each one still to come.
      if (icon.later) {
        const text = Math.max(10, Math.round(w * 0.45))
        context.font = `${text}px "Wayfarer Text", monospace`
        context.textAlign = "center"
        context.textBaseline = "bottom"
        const qx = Math.round(at.x)
        const qy = y + Math.round(w * 0.42)
        context.fillStyle = "rgb(0 0 0 / 0.8)"
        context.fillText("?", qx + k, qy + k)
        context.fillStyle = "rgb(248 248 248)"
        context.fillText("?", qx, qy)
      }
    }
    // A held wave stays up (popping in once when it's picked); with reduced
    // motion the first wave, or the picked one, just stands.
    if (!animate || clock.still !== null) {
      const wave = clock.still ?? 0
      const age = animate ? now() - clock.start : Infinity
      const icons = spread(wild.waves[wave]!)
      icons.forEach(({ icon, order }) => {
        const own = age - (order / Math.max(1, icons.length)) * SPREAD_MS
        if (own < 0) return
        const t = Math.min(1, own / POP_MS)
        draw(icon, 1, Math.max(0.05, pop(t)), own < POP_MS ? (1 - t) * 10 : 0)
      })
      context.globalAlpha = 1
      return
    }
    const { wave, into } = waveAt(now() - clock.start)
    const icons = spread(wild.waves[wave]!)
    icons.forEach(({ icon, order }) => {
      const age = into - (order / Math.max(1, icons.length)) * SPREAD_MS
      if (age < 0 || age > LIFE_MS) return
      const t = Math.min(1, age / POP_MS)
      // It bounces as it lands and rises a little as it goes.
      const fade = Math.min(1, (LIFE_MS - age) / 300)
      const bounce = age < POP_MS ? (1 - t) * 10 : 0
      draw(icon, Math.max(0, fade), Math.max(0.05, pop(t)), bounce + (age / LIFE_MS) * 6)
    })
    context.globalAlpha = 1
  }
}
