// The tour's badge stop: a pin with its badge on every Gym town, and the
// league circuit drawn as a dotted trail from INDIGO PLATEAU to the Sevii
// Masters to the Hoenn League. The places come from `landmarks.json`, which
// the builder reads from the game; the badge art from the trainer card.
import type { Bounds } from "./data"
import { DATA } from "./data"
import type { Overlay } from "./render"

export type Gym = {
  /** Place in the badge sheet: Johto's eight, Kanto's eight, then Hoenn's. */
  badge: number
  region: "Johto" | "Kanto" | "Hoenn"
  town: string
  /** The badge's name, as the Gym's lines give it: "BOULDER BADGE". */
  name?: string
  x: number
  y: number
  /** Who leads it, from the game's notable routines; `simulated` ones walk the map. */
  leader?: {
    name: string
    /** Their notable on the map; a duo's first half, with the second in `also`. */
    slug: string
    also?: string
    /** Whether they walk the map, or stand at their Gym. */
    simulated: boolean
    /** Their battle portrait, `leaders/<pic>.png` beside the data. */
    pic?: string
  }
}

export type League = { id: string; name: string; x: number; y: number }

export type Landmarks = { badges: Gym[]; circuit: League[] }

/** The 24 badges, 16x16 each in one row. */
export const BADGES_SRC = "/game/badges.png"

const loading = new Map<string, Promise<Landmarks>>()

/** Loads the landmarks once per base path. Without them the tour just has no badge pins. */
export const loadLandmarks = (base = DATA): Promise<Landmarks> => {
  let landmarks = loading.get(base)
  if (!landmarks) {
    landmarks = fetch(`${base}/landmarks.json`)
      .then((response) => response.json() as Promise<Landmarks>)
      .catch(() => ({ badges: [], circuit: [] }))
    loading.set(base, landmarks)
  }
  return landmarks
}

/** The box round every pin and league, with `room` metatiles to spare. */
export const landmarkBounds = ({ badges, circuit }: Landmarks, room: number): Bounds | null => {
  const points = [...badges, ...circuit]
  if (!points.length) return null
  return {
    left: Math.min(...points.map(({ x }) => x)) - room,
    top: Math.min(...points.map(({ y }) => y)) - room,
    right: Math.max(...points.map(({ x }) => x)) + room,
    bottom: Math.max(...points.map(({ y }) => y)) + room,
  }
}

/** The ring round each region's pins: the gold of Johto's name on the map, Kanto's red, Hoenn's sky. */
const RINGS: Record<Gym["region"], string> = {
  Johto: "rgb(248 208 64)",
  Kanto: "rgb(232 72 56)",
  Hoenn: "rgb(96 176 248)",
}

/** What the tour tells the overlays about the stop. */
export type StopClock = {
  /** How much of the stop is shown, 0 to 1: fades in on arriving and out on leaving. */
  alpha: number
  /** Milliseconds since arriving. */
  elapsed: number
  /** Whether the pins pop in and the dots march, or everything just stands. */
  animate: boolean
  /** The pin (by badge) under the pointer or keyboard focus, held down, and whose card is open. */
  hover?: number | null
  pressed?: number | null
  selected?: number | null
}

/** Sampled points along an arc from `a` to `b`, bowed to one side by `bow` of their distance. */
const arc = (a: League, b: League, bow: number, steps: number) => {
  const dx = b.x - a.x
  const dy = b.y - a.y
  const control = { x: (a.x + b.x) / 2 - dy * bow, y: (a.y + b.y) / 2 + dx * bow }
  return Array.from({ length: steps + 1 }, (_, i) => {
    const t = i / steps
    const u = 1 - t
    return {
      x: u * u * a.x + 2 * u * t * control.x + t * t * b.x,
      y: u * u * a.y + 2 * u * t * control.y + t * t * b.y,
    }
  })
}

const easeOut = (t: number) => 1 - (1 - Math.min(1, Math.max(0, t))) ** 3

/** Pins pop in one after the other from here, in milliseconds, and the trail starts drawing after them. */
const PIN_DELAY_MS = 150
const PIN_STAGGER_MS = 40
const TRAIL_DELAY_MS = 1100
const TRAIL_MS = 2600
/** Dots on the trail, in CSS pixels: the gap between them and how fast they march. */
const DOT_GAP = 12
const DOT_SPEED = 28

/**
 * The overlays for the stop, in drawing order: the trail, then the pins. They
 * read `clock` as they draw, so the tour updates it on every frame.
 */
export const badgeOverlays = (
  { badges, circuit }: Landmarks,
  sheet: CanvasImageSource,
  clock: StopClock,
): Overlay[] => {
  // Each leg bows the same way round, so the circuit reads as one sweep.
  const legs = circuit.slice(1).map((to, i) => arc(circuit[i]!, to, -0.14, 64))
  const trail = legs.flat()
  const lengths = trail.map((point, i) =>
    i ? Math.hypot(point.x - trail[i - 1]!.x, point.y - trail[i - 1]!.y) : 0,
  )
  const total = lengths.reduce((sum, length) => sum + length, 0)
  // How far along the trail each league is, 0 to 1.
  const reach = circuit.map((league) => {
    let best = 0
    let bestDistance = Infinity
    let run = 0
    trail.forEach((point, i) => {
      run += lengths[i]!
      const distance = Math.hypot(point.x - league.x, point.y - league.y)
      if (distance < bestDistance) {
        bestDistance = distance
        best = run
      }
    })
    return total ? best / total : 0
  })
  const revealed = () => (clock.animate ? easeOut((clock.elapsed - TRAIL_DELAY_MS) / TRAIL_MS) : 1)

  const drawTrail: Overlay = (context, { toScreen, ratio }) => {
    if (!trail.length || clock.alpha <= 0) return
    const upTo = revealed() * total
    const path = new Path2D()
    let run = 0
    for (const [i, point] of trail.entries()) {
      run += lengths[i]!
      if (run > upTo) break
      const { x, y } = toScreen(point.x, point.y)
      if (i) path.lineTo(x, y)
      else path.moveTo(x, y)
    }
    context.globalAlpha = clock.alpha
    context.lineCap = "round"
    context.lineJoin = "round"
    context.setLineDash([0, DOT_GAP * ratio])
    context.lineDashOffset = clock.animate ? -((clock.elapsed / 1000) * DOT_SPEED * ratio) : 0
    // A dark dot under each bright one, offset like the game's text shadow.
    context.translate(ratio, ratio)
    context.lineWidth = 5 * ratio
    context.strokeStyle = "rgb(0 0 0 / 0.55)"
    context.stroke(path)
    context.translate(-ratio, -ratio)
    context.lineWidth = 4 * ratio
    context.strokeStyle = "rgb(255 232 120)"
    context.stroke(path)
  }

  const drawLeagues: Overlay = (context, { toScreen, ratio }) => {
    if (clock.alpha <= 0) return
    context.setLineDash([])
    context.textAlign = "center"
    context.textBaseline = "middle"
    const progress = revealed()
    for (const [i, league] of circuit.entries()) {
      // A league lights as the trail reaches it.
      if (progress < reach[i]! - 0.001) continue
      const { x, y } = toScreen(league.x, league.y)
      const radius = 10 * ratio
      context.globalAlpha = clock.alpha
      context.fillStyle = "rgb(24 32 56)"
      context.beginPath()
      context.arc(x, y, radius + 2 * ratio, 0, Math.PI * 2)
      context.fill()
      context.fillStyle = "rgb(255 214 0)"
      context.beginPath()
      context.arc(x, y, radius, 0, Math.PI * 2)
      context.fill()
      context.font = `${Math.round(14 * ratio)}px "Wayfarer Text", monospace`
      context.fillStyle = "rgb(24 32 56)"
      context.fillText(String(i + 1), x, y + ratio)
      // The name sits below, or above the last stop, which has room there.
      context.font = `${Math.round(16 * ratio)}px "Wayfarer Text", monospace`
      const below = i < circuit.length - 1
      const labelY = y + (below ? 1 : -1) * (radius + 12 * ratio)
      context.fillStyle = "rgb(0 0 0 / 0.75)"
      context.fillText(league.name, x + ratio * 1.5, labelY + ratio * 1.5)
      context.fillStyle = "rgb(248 248 248)"
      context.fillText(league.name, x, labelY)
    }
    context.globalAlpha = 1
  }

  const drawPins: Overlay = (context, { toScreen, ratio }) => {
    if (clock.alpha <= 0) return
    const wide = context.canvas.width / ratio > 700
    // Whole multiples of the 16 pixel badge, so it stays crisp.
    const icon = Math.round(16 * ratio * (wide ? 2 : 1))
    context.imageSmoothingEnabled = false
    for (const gym of badges) {
      const pop = clock.animate
        ? easeOut((clock.elapsed - PIN_DELAY_MS - gym.badge * PIN_STAGGER_MS) / 360)
        : 1
      if (pop <= 0) continue
      const { x, y } = toScreen(gym.x, gym.y)
      // A pin grows under the pointer, dips while held, and wears gold while
      // its card is open; the rest step back then.
      const hovered = clock.hover === gym.badge
      const pressed = clock.pressed === gym.badge
      const selected = clock.selected === gym.badge
      const others = clock.selected != null && !selected
      const grow = pressed ? 0.92 : hovered || selected ? 1.18 : 1
      const size = icon * (0.5 + 0.5 * pop) * (pressed ? 0.92 : 1)
      const radius = icon * 0.78 * grow
      context.globalAlpha = clock.alpha * pop * (others && !hovered ? 0.55 : 1)
      if (hovered || selected) {
        context.save()
        context.shadowColor = selected ? "rgb(255 214 0 / 0.9)" : "rgb(255 255 255 / 0.7)"
        context.shadowBlur = 10 * ratio
      }
      context.fillStyle = pressed
        ? "rgb(12 16 32 / 0.95)"
        : hovered || selected
          ? "rgb(48 64 112 / 0.95)"
          : "rgb(24 32 56 / 0.92)"
      context.strokeStyle = selected
        ? "rgb(255 214 0)"
        : hovered
          ? "rgb(248 248 248)"
          : RINGS[gym.region]
      context.lineWidth = Math.max(2, Math.round((hovered || selected ? 3 : 2) * ratio))
      context.beginPath()
      context.arc(x, y, radius * (0.5 + 0.5 * pop), 0, Math.PI * 2)
      context.fill()
      context.stroke()
      if (hovered || selected) context.restore()
      context.drawImage(sheet, gym.badge * 16, 0, 16, 16, x - size / 2, y - size / 2, size, size)
    }
    // The hovered pin's town, over it.
    const named = badges.find((gym) => gym.badge === clock.hover)
    if (named) {
      const { x, y } = toScreen(named.x, named.y)
      context.globalAlpha = clock.alpha
      context.font = `${Math.round(16 * ratio)}px "Wayfarer Text", monospace`
      context.textAlign = "center"
      context.textBaseline = "bottom"
      const above = y - icon * 0.78 * 1.18 - 6 * ratio
      context.fillStyle = "rgb(0 0 0 / 0.8)"
      context.fillText(named.town, x + ratio * 1.5, above + ratio * 1.5)
      context.fillStyle = "rgb(248 248 248)"
      context.fillText(named.town, x, above)
      context.textBaseline = "alphabetic"
    }
    context.globalAlpha = 1
  }

  return [drawTrail, drawPins, drawLeagues]
}
