// The tour's Gym and league stops: a pin with its badge on every Gym town,
// and the league circuit: a plate for each league and a dotted trail from
// INDIGO PLATEAU and from the Hoenn League to the Sevii Masters. The places
// come from `landmarks.json`, which the builder reads from the game; the badge
// art from the trainer card.
import type { Bounds } from "./data"
import { DATA } from "./data"
import type { Roster } from "./lineup"
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

export type LeagueId = "indigo" | "masters" | "hoenn"

export type League = {
  id: LeagueId
  name: string
  x: number
  y: number
  /** Its four halls and the Champion's Room, with each one's battle condition (`league_halls.c`). */
  halls?: { name: string; condition: string }[]
}

export type Landmarks = {
  badges: Gym[]
  circuit: League[]
  /** The trainers a league event's five are drawn from, with what the draw reads. */
  roster?: Roster
}

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

/** The Trainer Rating a player holding `badges` badges has (`league_circuit.c`): from all regions together. */
export const trainerRating = (badges: number) => (badges <= 8 ? 10 * badges : 80 + 5 * (badges - 8))

/** A league wants the player at this Trainer Rating or more: any 8 badges. */
export const QUALIFYING_RATING = 80

/** Whether a badge (by place in the sheet: Johto's eight, Kanto's, then Hoenn's) is from Hoenn. */
const fromHoenn = (badge: number) => badge >= 16

/**
 * Which leagues know a player holding `earned` badges (`league_circuit.c`):
 * the Trainer Rating counts badges from every region together, 80 of it
 * (any 8 badges) qualifies, and each league also wants a badge of its own
 * side. The Sevii Masters needs wins, which the page has none of.
 */
export const callingFor = (earned: ReadonlySet<number>): Record<LeagueId, boolean> => {
  const qualified = trainerRating(earned.size) >= QUALIFYING_RATING
  const held = [...earned]
  return {
    indigo: qualified && held.some((badge) => !fromHoenn(badge)),
    hoenn: qualified && held.some(fromHoenn),
    masters: false,
  }
}

/**
 * The league whose call rings next, from those that know the player and
 * haven't called yet on this page (`ChooseCallingLeague` in
 * `league_events.c`): the one that called least recently (none has, so none
 * wins on that), then the one with the most badges of its side (Indigo
 * counts Kanto's and Johto's, Hoenn Hoenn's), then Indigo.
 */
export const callerFor = (
  earned: ReadonlySet<number>,
  called: ReadonlySet<LeagueId>,
): Exclude<LeagueId, "masters"> | null => {
  const calling = callingFor(earned)
  const hoennBadges = [...earned].filter(fromHoenn).length
  const waiting = (["indigo", "hoenn"] as const)
    .filter((id) => calling[id] && !called.has(id))
    .toSorted((a, b) => {
      const badges = (id: typeof a) => (id === "hoenn" ? hoennBadges : earned.size - hoennBadges)
      return badges(b) - badges(a)
    })
  return waiting[0] ?? null
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
  /** How much of the league circuit is up, 0 (the Gym stop: just pins) to 1. */
  circuit?: number
  /** The badges the visitor has earned, which the pins light; the rest step back. */
  earned?: ReadonlySet<number>
  /** When each earned badge was earned (`performance.now()`), for its burst of sparkles. */
  earnedAt?: ReadonlyMap<number, number>
  /** Which leagues are calling. */
  calling?: Partial<Record<LeagueId, boolean>>
  /** The league whose stop it is, to highlight: its plate and its leg of the trail. */
  league?: LeagueId | null
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

/** Pins pop in one after the other from here, in milliseconds. */
const PIN_DELAY_MS = 150
const PIN_STAGGER_MS = 40
/** How long a leg takes to draw in once its league is calling. */
const LEG_MS = 1400
/** Dots on the trail, in CSS pixels: the gap between them and how fast they march. */
const DOT_GAP = 12
const DOT_SPEED = 28
/** How faint a badge not yet earned is: a little at the Gym stop, more once the leagues are up. */
const FAINT_PIN = 0.6
const FAINTER_PIN = 0.35
/** How long a badge's burst of sparkles lasts when it is earned. */
const BURST_MS = 650

/**
 * A gold four-point sparkle centered on a point, in pixels of `unit`, that
 * twinkles by `phase`; a `burst` (0 to 1) also throws four small ones outward.
 */
const sparkle = (
  context: CanvasRenderingContext2D,
  cx: number,
  cy: number,
  unit: number,
  { alpha, phase, burst }: { alpha: number; phase: number; burst?: number },
) => {
  const u = Math.max(1, Math.round(unit))
  const plus = (x: number, y: number, arm: number, color: string) => {
    context.fillStyle = color
    context.fillRect(Math.round(x - u / 2), Math.round(y - u / 2 - arm * u), u, (arm * 2 + 1) * u)
    context.fillRect(Math.round(x - u / 2 - arm * u), Math.round(y - u / 2), (arm * 2 + 1) * u, u)
  }
  context.save()
  context.shadowBlur = 0
  context.shadowOffsetY = 0
  context.globalAlpha = alpha
  // The twinkle: arms of 2, 1 and 3 pixels in turn.
  const arm = [2, 1, 3, 1][Math.floor(((phase % 4) + 4) % 4)]!
  plus(cx, cy, arm, "rgb(255 226 96)")
  context.fillStyle = "rgb(255 255 255)"
  context.fillRect(Math.round(cx - u / 2), Math.round(cy - u / 2), u, u)
  if (burst !== undefined && burst >= 0 && burst < 1) {
    context.globalAlpha = alpha * (1 - burst)
    const reach = (4 + 22 * burst) * unit
    for (const [dx, dy] of [
      [1, -1],
      [1, 1],
      [-1, 1],
      [-1, -1],
    ] as const)
      plus(cx + (dx * reach) / 1.4, cy + (dy * reach) / 1.4, burst < 0.5 ? 1 : 0, "rgb(255 214 0)")
  }
  context.restore()
}

/**
 * The overlays for the stops, in drawing order: the trail, then the pins,
 * then the league plates. They read `clock` as they draw, so the tour updates
 * it on every frame.
 */
export const badgeOverlays = (
  { badges, circuit }: Landmarks,
  sheet: CanvasImageSource,
  clock: StopClock,
): Overlay[] => {
  // Two legs that converge on the Sevii Masters, each from the league that
  // opens it. Each runs outward-in, so it draws toward Seven Island.
  const masters = circuit.find((league) => league.id === "masters")
  const legs = masters
    ? circuit
        .filter((league) => league.id !== "masters")
        .map((from) => {
          const points = arc(from, masters, -0.1, 64)
          const lengths = points.map((point, i) =>
            i ? Math.hypot(point.x - points[i - 1]!.x, point.y - points[i - 1]!.y) : 0,
          )
          return { id: from.id, points, lengths, total: lengths.reduce((sum, n) => sum + n, 0) }
        })
    : []
  // When each leg's league began calling, to draw it in from there.
  const litSince = new Map<LeagueId, number>()

  const drawTrail: Overlay = (context, { toScreen, ratio }) => {
    const mix = clock.circuit ?? 0
    if (clock.alpha <= 0 || mix <= 0) return
    const now = performance.now()
    context.lineCap = "round"
    context.lineJoin = "round"
    context.setLineDash([0, DOT_GAP * ratio])
    for (const leg of legs) {
      const calling = Boolean(clock.calling?.[leg.id])
      if (calling && !litSince.has(leg.id)) litSince.set(leg.id, now)
      else if (!calling) litSince.delete(leg.id)
      // At a league's stop its leg is the one that shows; the Masters' has both.
      const focused = !clock.league || clock.league === "masters" || clock.league === leg.id
      const emphasis = clock.league ? (clock.league === leg.id ? 1.5 : focused ? 1.15 : 0.8) : 1
      const trace = (upTo: number) => {
        const path = new Path2D()
        let run = 0
        for (const [i, point] of leg.points.entries()) {
          run += leg.lengths[i]!
          const { x, y } = toScreen(point.x, point.y)
          if (run > upTo) {
            const before = leg.points[i - 1]!
            const t = (upTo - (run - leg.lengths[i]!)) / leg.lengths[i]!
            const edge = toScreen(
              before.x + (point.x - before.x) * t,
              before.y + (point.y - before.y) * t,
            )
            path.lineTo(edge.x, edge.y)
            break
          }
          if (i) path.lineTo(x, y)
          else path.moveTo(x, y)
        }
        return path
      }
      // Waiting for its league: the whole leg, dim and still.
      context.lineDashOffset = 0
      context.globalAlpha = clock.alpha * mix * (calling ? 0.4 : focused ? 0.55 : 0.3)
      context.lineWidth = 3 * ratio * emphasis
      context.strokeStyle = "rgb(255 255 255)"
      context.stroke(trace(leg.total))
      if (!calling) continue
      const drawn = clock.animate ? easeOut((now - litSince.get(leg.id)!) / LEG_MS) : 1
      const path = trace(leg.total * drawn)
      context.globalAlpha = clock.alpha * mix * (focused ? 1 : 0.6)
      context.lineDashOffset = clock.animate ? -((clock.elapsed / 1000) * DOT_SPEED * ratio) : 0
      // A dark dot under each bright one, offset like the game's text shadow.
      context.translate(ratio, ratio)
      context.lineWidth = 5 * ratio * emphasis
      context.strokeStyle = "rgb(0 0 0 / 0.55)"
      context.stroke(path)
      context.translate(-ratio, -ratio)
      context.lineWidth = 4 * ratio * emphasis
      context.strokeStyle = "rgb(255 232 120)"
      context.stroke(path)
    }
  }

  // Each league on a plate like the region names: dark, rimmed in gold, and
  // glowing while it calls; grey while it doesn't. The one the tour is at
  // takes a white rim and a wider glow.
  const drawLeagues: Overlay = (context, { toScreen, ratio }) => {
    const mix = clock.circuit ?? 0
    if (clock.alpha <= 0 || mix <= 0) return
    context.setLineDash([])
    context.textAlign = "center"
    context.textBaseline = "middle"
    const wide = context.canvas.width / ratio > 700
    const size = Math.round((wide ? 20 : 16) * ratio)
    context.font = `${size}px "Wayfarer Text", monospace`
    for (const league of circuit) {
      const calling = Boolean(clock.calling?.[league.id])
      const here = clock.league === league.id
      const { x, y } = toScreen(league.x, league.y)
      const padX = Math.round(size * 0.45)
      const padY = Math.round(size * 0.3)
      const boxW = Math.round(context.measureText(league.name).width + padX * 2)
      const boxH = Math.round(size + padY * 2)
      // Above its spot.
      const bx = Math.round(x - boxW / 2)
      const by = Math.round(y - 10 * ratio - boxH)
      const rim = Math.max(2, Math.round(2 * ratio))
      const accent = here ? "rgb(255 255 255)" : calling ? "rgb(255 214 0)" : "rgb(150 156 170)"
      context.globalAlpha = clock.alpha * mix * (calling || here ? 1 : 0.8)
      // The spot the trail reaches.
      context.fillStyle = "rgb(24 32 56)"
      context.beginPath()
      context.arc(x, y, 5 * ratio, 0, Math.PI * 2)
      context.fill()
      context.fillStyle = calling ? "rgb(255 214 0)" : "rgb(150 156 170)"
      context.beginPath()
      context.arc(x, y, 3 * ratio, 0, Math.PI * 2)
      context.fill()
      context.save()
      if (calling || here) {
        const pulse = calling && clock.animate ? 0.75 + 0.25 * Math.sin(performance.now() / 260) : 1
        context.shadowColor = here ? "rgb(255 255 255 / 0.9)" : "rgb(255 214 0 / 0.95)"
        context.shadowBlur = (here ? 22 : 16) * ratio * pulse
      }
      context.fillStyle = "rgb(0 0 0 / 0.35)"
      context.fillRect(bx + rim, by + rim, boxW, boxH)
      context.fillStyle = accent
      context.fillRect(bx, by, boxW, boxH)
      context.restore()
      context.fillStyle = "rgb(8 41 0 / 0.92)"
      context.fillRect(bx + rim, by + rim, boxW - rim * 2, boxH - rim * 2)
      const cx = bx + boxW / 2
      const cy = by + boxH / 2
      context.fillStyle = "rgb(0 0 0 / 0.8)"
      context.fillText(league.name, cx + ratio, cy + ratio * 1.5)
      context.fillStyle = calling || here ? accent : "rgb(190 194 204)"
      context.fillText(league.name, cx, cy + ratio * 0.5)
    }
    context.textBaseline = "alphabetic"
    context.globalAlpha = 1
  }

  const drawPins: Overlay = (context, { toScreen, ratio }) => {
    if (clock.alpha <= 0) return
    const wide = context.canvas.width / ratio > 700
    // Whole multiples of the 16 pixel badge, so it stays crisp.
    const icon = Math.round(16 * ratio * (wide ? 2 : 1))
    const mix = clock.circuit ?? 0
    context.imageSmoothingEnabled = false
    for (const gym of badges) {
      const pop = clock.animate
        ? easeOut((clock.elapsed - PIN_DELAY_MS - gym.badge * PIN_STAGGER_MS) / 360)
        : 1
      if (pop <= 0) continue
      const { x, y } = toScreen(gym.x, gym.y)
      // Just the badge, crisp, over a soft shadow that keeps it readable on
      // the map. Under the pointer it lifts and glows white; held, it
      // settles and darkens; with its card open it glows gold and bobs,
      // while the other earned ones dim. An earned badge is bright with a gold glow
      // and a sparkle; the rest are fainter, more so at the leagues.
      const hovered = clock.hover === gym.badge
      const pressed = clock.pressed === gym.badge
      const selected = clock.selected === gym.badge
      const held = !clock.earned || clock.earned.has(gym.badge)
      const earned = Boolean(clock.earned?.has(gym.badge))
      // While a card is open, the other earned pins step back a little.
      const others = clock.selected != null && !selected && earned
      const faint = held ? 1 : FAINT_PIN - mix * (FAINT_PIN - FAINTER_PIN)
      const size = Math.round(icon * (0.5 + 0.5 * pop))
      const step = Math.round(2 * ratio)
      const bob = selected && clock.animate ? (Math.floor(performance.now() / 320) % 2) * step : 0
      const lift = pressed ? 0 : hovered ? step * 2 : selected ? step + bob : 0
      const left = Math.round(x - size / 2)
      const top = Math.round(y - size / 2 - lift + (pressed ? step / 2 : 0))
      context.globalAlpha = clock.alpha * pop * faint * (others && !hovered ? 0.8 : 1)
      context.save()
      context.shadowColor = selected
        ? "rgb(255 214 0 / 0.95)"
        : hovered
          ? "rgb(255 255 255 / 0.9)"
          : earned
            ? "rgb(255 214 0 / 0.9)"
            : "rgb(0 0 0 / 0.75)"
      context.shadowBlur = (selected || hovered ? 10 : earned ? 8 : 4) * ratio
      context.shadowOffsetY = selected || hovered ? 0 : ratio
      // Darker while held.
      if (pressed) context.filter = "brightness(0.75)"
      context.drawImage(sheet, gym.badge * 16, 0, 16, 16, left, top, size, size)
      context.restore()
      if (earned && pop >= 1) {
        const since = clock.earnedAt?.get(gym.badge)
        sparkle(context, left + size * 0.86, top + size * 0.14, ratio * (wide ? 2 : 1), {
          alpha: clock.alpha * (others && !hovered ? 0.8 : 1),
          phase: clock.animate ? performance.now() / 280 + gym.badge * 0.9 : 0,
          burst:
            clock.animate && since !== undefined
              ? (performance.now() - since) / BURST_MS
              : undefined,
        })
      }
    }
    // The hovered pin's town, over it.
    const named = badges.find((gym) => gym.badge === clock.hover)
    if (named) {
      const { x, y } = toScreen(named.x, named.y)
      context.globalAlpha = clock.alpha
      context.font = `${Math.round(16 * ratio)}px "Wayfarer Text", monospace`
      context.textAlign = "center"
      context.textBaseline = "bottom"
      const above = y - icon / 2 - Math.round(2 * ratio) * 2 - 6 * ratio
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
