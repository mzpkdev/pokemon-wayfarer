// Wayfarer's world, assembled from the region maps the game draws (Hoenn,
// Johto and Kanto, the Sevii Islands) on one shared ocean, and the camera that
// pulls back across it as the page scrolls. Units are the maps' own pixels.
import art from "~/generated/game-art.json"

type Placed = { x: number; y: number }
type Size = { width: number; height: number }

const manifest = art as unknown as Record<string, Size & Placed>
const sizeOf = (file: string) => manifest[`world/${file}.png`]!

/** The sea's two-color stripe, as the region maps draw it. */
export const SEA = (manifest["world/hoenn.png"] as unknown as { sea: number[][] }).sea.map(
  ([r, g, b]) => `rgb(${r} ${g} ${b})`,
)

/** Where each region map sits in the world, by its top-left corner. */
export const REGIONS = [
  { file: "johto-kanto", x: 0, y: 0 },
  { file: "hoenn", x: 0, y: 160 },
  { file: "sevii-123", x: 252, y: 118 },
  { file: "sevii-45", x: 318, y: 162 },
  { file: "sevii-67", x: 262, y: 204 },
].map((region) => {
  const { width, height } = sizeOf(region.file)
  return { ...region, width, height }
})

export const WORLD = {
  width: Math.max(...REGIONS.map(({ x, width }) => x + width)),
  height: Math.max(...REGIONS.map(({ y, height }) => y + height)),
}

const region = (file: string) => REGIONS.find((placed) => placed.file === file)!

/**
 * The middle of a region map cell, in world pixels. Cells count from the
 * map's top-left corner the way the game's layouts do; the game draws them
 * one column and two rows in (MAPCURSOR_X_MIN, MAPCURSOR_Y_MIN in region_map.c).
 */
export const cell = (file: string, column: number, row: number): Placed => {
  const { x, y } = region(file)
  const trimmed = manifest[`world/${file}.png`]!
  return { x: x + (column + 1) * 8 + 4 - trimmed.x, y: y + (row + 2) * 8 + 4 - trimmed.y }
}

/** LITTLEROOT TOWN, where the camera starts (MAPSEC_LITTLEROOT_TOWN is at 4, 11). */
export const LITTLEROOT = cell("hoenn", 4, 11)

/**
 * A part of the world to fit on screen, and where it sits in the space it is
 * given when its shape leaves room over: 0 hugs the top or left, 0.5 centers.
 */
export type Frame = {
  x: number
  y: number
  width: number
  height: number
  alignX: number
  alignY: number
}

const around = ({ x, y }: Placed, width: number, height: number): Frame => ({
  x: x - width / 2,
  y: y - height / 2,
  width,
  height,
  alignX: 0.5,
  alignY: 0.5,
})

/**
 * What the camera frames, in order: LITTLEROOT, all of Hoenn, the whole
 * world. The world hugs the screen's top-left corner, where the Johto map's
 * cut edges meet the screen's own.
 */
export const SHOTS: Frame[] = [
  around(LITTLEROOT, 56, 40),
  { ...around({ x: region("hoenn").x + 124, y: region("hoenn").y + 75 }, 264, 166) },
  { x: 0, y: 0, width: WORLD.width + 16, height: WORLD.height + 12, alignX: 0, alignY: 0 },
]

/**
 * Scroll progress (0 to 1) to the frame on screen: a hold on each shot, and
 * eased pulls between them, the size changing geometrically so the zoom
 * feels even.
 */
const BEATS = [
  { from: 0, to: 0.12, shot: 0 },
  { from: 0.12, to: 0.42, shot: 0, next: 1 },
  { from: 0.42, to: 0.52, shot: 1 },
  { from: 0.52, to: 0.86, shot: 1, next: 2 },
  { from: 0.86, to: 1, shot: 2 },
]

const ease = (t: number) => (t < 0.5 ? 4 * t * t * t : 1 - (-2 * t + 2) ** 3 / 2)
const mix = (from: number, to: number, t: number) => from + (to - from) * t
const mixGeometric = (from: number, to: number, t: number) => from * (to / from) ** t

export const frameAt = (progress: number): Frame & { beat: number } => {
  const p = Math.min(1, Math.max(0, progress))
  const index = BEATS.findIndex(({ to }) => p <= to)
  const beat = BEATS[index === -1 ? BEATS.length - 1 : index]!
  const from = SHOTS[beat.shot]!
  if (beat.next === undefined) return { ...from, beat: beat.shot }
  const to = SHOTS[beat.next]!
  const t = ease((p - beat.from) / (beat.to - beat.from))
  const width = mixGeometric(from.width, to.width, t)
  const height = mixGeometric(from.height, to.height, t)
  const centerX = mix(from.x + from.width / 2, to.x + to.width / 2, t)
  const centerY = mix(from.y + from.height / 2, to.y + to.height / 2, t)
  return {
    x: centerX - width / 2,
    y: centerY - height / 2,
    width,
    height,
    alignX: mix(from.alignX, to.alignX, t),
    alignY: mix(from.alignY, to.alignY, t),
    beat: t < 0.5 ? beat.shot : beat.next,
  }
}

/**
 * The ferry ports, from their region map cells (region_map_layout_*.h).
 * MELEMELE has no map of its own; the S.S. ANNE sails off the edge toward it.
 */
const PORTS = {
  olivine: cell("johto-kanto", 3, 6),
  vermilion: cell("johto-kanto", 24, 7),
  slateport: cell("hoenn", 8, 10),
  lilycove: cell("hoenn", 18, 3),
  oneIsland: cell("sevii-123", 1, 8),
  sevenIsland: cell("sevii-67", 5, 8),
  melemele: { x: WORLD.width + 80, y: 120 },
}
type Port = keyof typeof PORTS

/** Open-sea turning points, so the lanes go around the land, not over it. */
const SEA_POINTS = {
  olivineSound: { x: 27, y: 96 },
  johtoCoast: { x: 34, y: 132 },
  strait: { x: 150, y: 140 },
  kantoBay: { x: 210, y: 124 },
  seviiReach: { x: 240, y: 150 },
  hoennNorth: { x: 190, y: 172 },
  hoennEast: { x: 250, y: 250 },
  hoennSouth: { x: 200, y: 302 },
  slateportReach: { x: 110, y: 302 },
  slateportBerth: { x: 76, y: 292 },
  mossdeepSound: { x: 215, y: 215 },
  seviiNorth: { x: 300, y: 140 },
  seviiSouth: { x: 400, y: 282 },
  sevenReach: { x: 270, y: 232 },
}
type Point = Port | keyof typeof SEA_POINTS

const point = (name: Point) =>
  name in PORTS ? PORTS[name as Port] : SEA_POINTS[name as keyof typeof SEA_POINTS]

/** The way between two ports, through the sea points between them. */
const LEGS: Record<string, Point[]> = {
  "olivine-vermilion": ["olivine", "olivineSound", "johtoCoast", "strait", "kantoBay", "vermilion"],
  "vermilion-slateport": [
    "vermilion",
    "kantoBay",
    "seviiReach",
    "hoennEast",
    "hoennSouth",
    "slateportReach",
    "slateportBerth",
    "slateport",
  ],
  "slateport-lilycove": [
    "slateport",
    "slateportBerth",
    "slateportReach",
    "hoennSouth",
    "hoennEast",
    "mossdeepSound",
    "lilycove",
  ],
  "lilycove-olivine": ["lilycove", "hoennNorth", "strait", "johtoCoast", "olivineSound", "olivine"],
  "vermilion-oneIsland": ["vermilion", "kantoBay", "seviiReach", "oneIsland"],
  "oneIsland-melemele": ["oneIsland", "seviiNorth", "melemele"],
  "melemele-sevenIsland": ["melemele", "seviiSouth", "sevenIsland"],
  "sevenIsland-vermilion": ["sevenIsland", "sevenReach", "seviiReach", "kantoBay", "vermilion"],
  "lilycove-sevenIsland": ["lilycove", "mossdeepSound", "hoennEast", "sevenIsland"],
  "sevenIsland-olivine": [
    "sevenIsland",
    "sevenReach",
    "seviiReach",
    "strait",
    "johtoCoast",
    "olivineSound",
    "olivine",
  ],
  "olivine-slateport": [
    "olivine",
    "olivineSound",
    "johtoCoast",
    "strait",
    "hoennNorth",
    "hoennEast",
    "hoennSouth",
    "slateportReach",
    "slateportBerth",
    "slateport",
  ],
}

/** The three ships' four-port loops (sLoops in src/wayfarer_ferry.c). */
const LOOPS: Port[][] = [
  ["olivine", "vermilion", "slateport", "lilycove"],
  ["vermilion", "oneIsland", "melemele", "sevenIsland"],
  ["slateport", "lilycove", "sevenIsland", "olivine"],
]

/** Catmull-Rom through the points, so lanes curve like a ship's wake. */
const smooth = (points: Placed[], steps = 8): Placed[] => {
  const result: Placed[] = []
  for (let i = 0; i < points.length - 1; i++) {
    const [p0, p1, p2, p3] = [
      points[i - 1] ?? points[i]!,
      points[i]!,
      points[i + 1]!,
      points[i + 2] ?? points[i + 1]!,
    ]
    for (let s = 0; s < steps; s++) {
      const t = s / steps
      const t2 = t * t
      const t3 = t2 * t
      const at = (a: number, b: number, c: number, d: number) =>
        0.5 *
        (2 * b + (c - a) * t + (2 * a - 5 * b + 4 * c - d) * t2 + (3 * b - a - 3 * c + d) * t3)
      result.push({ x: at(p0.x, p1.x, p2.x, p3.x), y: at(p0.y, p1.y, p2.y, p3.y) })
    }
  }
  result.push(points.at(-1)!)
  return result
}

const legPath = (from: Port, to: Port) => {
  const forward = LEGS[`${from}-${to}`]
  const names = forward ?? LEGS[`${to}-${from}`]!.toReversed()
  return smooth(names.map(point))
}

/** A polyline with its running length, for moving along it at an even pace. */
export type Path = { points: Placed[]; lengths: number[]; length: number }

const measure = (points: Placed[]): Path => {
  const lengths = [0]
  for (let i = 1; i < points.length; i++)
    lengths.push(
      lengths[i - 1]! +
        Math.hypot(points[i]!.x - points[i - 1]!.x, points[i]!.y - points[i - 1]!.y),
    )
  return { points, lengths, length: lengths.at(-1)! }
}

/** Every lane once, for drawing. */
export const LANES: Path[] = Object.values(LEGS).map((names) => measure(smooth(names.map(point))))

/** Each ship's whole loop, for sailing it. */
export const VOYAGES: Path[] = LOOPS.map((ports) =>
  measure(ports.flatMap((port, i) => legPath(port, ports[(i + 1) % ports.length]!))),
)

/** Where along a path `distance` falls, and which way the ship faces there. */
export const along = (path: Path, distance: number): Placed & { heading: number } => {
  const d = ((distance % path.length) + path.length) % path.length
  let i = path.lengths.findIndex((length) => length >= d)
  if (i <= 0) i = 1
  const a = path.points[i - 1]!
  const b = path.points[i]!
  const span = path.lengths[i]! - path.lengths[i - 1]! || 1
  const t = (d - path.lengths[i - 1]!) / span
  return { x: a.x + (b.x - a.x) * t, y: a.y + (b.y - a.y) * t, heading: Math.sign(b.x - a.x) }
}
