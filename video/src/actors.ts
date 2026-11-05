// The video's actors: the player, people and Pokémon walking the map the way
// the game moves its own sprites, with the bubbles it puts over their heads.
// A scene's `cast` (in `timeline.ts`) says who stands where when; this file
// poses them for a moment of the scene and draws them. Their sheets are cut
// from the game's art by `scripts/actors.mjs` into `actors/`.
import type { Overlay, Sprite } from "~/world-map/render"

export type Facing = "south" | "north" | "west" | "east"

/**
 * How a sheet's frames are laid out. A person's: standing south, north and
 * west, then two steps each way. A Pokémon's: two steps south, north and west.
 * East is west mirrored. `tiles` is a frame's size on the map, in metatiles.
 */
type Sheet = {
  src: string
  frame: number
  height: number
  /** "facing" has one frame each way at every other frame; "still" only ever shows the frames it is posed in. */
  layout: "person" | "pokemon" | "facing" | "still"
}

const person = (name: string): Sheet => ({
  src: `/actors/${name}.png`,
  frame: 16,
  height: 32,
  layout: "person",
})
const pokemon = (name: string, frame = 32): Sheet => ({
  src: `/actors/${name}.png`,
  frame,
  height: frame,
  layout: "pokemon",
})

export const SHEETS = {
  gold: person("gold-walking"),
  "gold-running": person("gold-running"),
  brock: person("brock"),
  blue: person("blue"),
  surge: person("surge"),
  wallace: person("wallace"),
  clair: person("clair"),
  "bug-catcher": person("bug-catcher"),
  rod: { src: "/actors/rod.png", frame: 16, height: 32, layout: "still" },
  wooloo: pokemon("wooloo"),
  graveler: pokemon("graveler"),
  golem: pokemon("golem"),
  noctowl: pokemon("noctowl"),
  gogoat: pokemon("gogoat"),
  pyroar: pokemon("pyroar"),
  furfrou: pokemon("furfrou"),
  vivillon: pokemon("vivillon"),
  "exeggutor-alola": pokemon("exeggutor-alola", 64),
  luvdisc: pokemon("luvdisc"),
  "luvdisc-surf": pokemon("luvdisc-surf"),
  "gold-fishing": { src: "/actors/gold-fishing.png", frame: 32, height: 32, layout: "still" },
  "gold-surfing": { src: "/actors/gold-surfing.png", frame: 32, height: 32, layout: "facing" },
  ball: { src: "/actors/ball.png", frame: 16, height: 32, layout: "still" },
} satisfies Record<string, Sheet>
export type SheetId = keyof typeof SHEETS

export const EMOTES = {
  exclamation: "/actors/emote-exclamation.png",
  question: "/actors/emote-question.png",
  heart: "/actors/emote-heart.png",
}
export type EmoteId = keyof typeof EMOTES

/** Where an actor is at a moment of the scene; they walk straight from each mark to the next. */
export type Mark<M> = { at: M; x: number; y: number }

export type Actor<M> = {
  /** Their sheet; none for someone the map already draws, who only needs a bubble over them. */
  sprite: SheetId | null
  /**
   * Their marks, in the tiles of `map` when given (from its top-left corner),
   * else in the world grid: they stand on the first until its moment, walk
   * from each to the next, and stand on the last.
   */
  marks: Mark<M>[]
  map?: string
  /** On stage only from this moment, and until this one. */
  from?: M
  to?: M
  /** The way they face standing still: one way, or turning at moments. Walking, they face where they go. */
  face?: Facing | { at: M; to: Facing }[]
  /** Bubbles popping over their head, each held `seconds` (one, unless given). */
  emotes?: { at: M; kind: EmoteId; seconds?: number }[]
  /** Little jumps on the spot, as the game's sprites hop in surprise or joy. */
  hops?: M[]
  /** Frames of their sheet shown from a moment on, over their own: a cast of the rod, say. */
  poses?: { at: M; frame: number }[]
  /** Tiles they are drawn off the ground, as a surfer sits up on what they ride. */
  lift?: number
}

/** A thing thrown in an arc from one tile to another over a span, like a Poké Ball handed over. */
export type Toss<M> = {
  sprite: SheetId
  from: { x: number; y: number }
  to: { x: number; y: number }
  map?: string
  span: { from: M; to: M }
  /** How high the arc rises at its middle, in tiles. */
  height?: number
}

/** An actor as drawn at one moment: which frame, where, and the bubble over them. */
export type Posed = {
  sprite: SheetId | null
  frame: number
  flip: boolean
  /** Their tile: they stand on its bottom edge, centered. */
  x: number
  y: number
  /** Tiles off the ground, hopping or thrown. */
  lift: number
  emote?: { kind: EmoteId; lift: number }
}

/** Seconds a hop lasts, and how high it goes, in tiles. */
const HOP_SECONDS = 0.28
const HOP_HEIGHT = 0.5
/** Seconds a bubble takes to pop up, and how far it overshoots, in tiles. */
const EMOTE_POP_SECONDS = 0.16
const EMOTE_POP = 0.4
/** Seconds between a Pokémon's two frames: it bobs on the spot as followers do. */
const BOB_SECONDS = 0.32

const facingOf = (dx: number, dy: number): Facing =>
  Math.abs(dx) > Math.abs(dy) ? (dx < 0 ? "west" : "east") : dy < 0 ? "north" : "south"

const ROW = { south: 0, north: 1, west: 2, east: 2 } as const

/** The frame for a facing: `step` is 0 standing, 1 or 2 mid-stride. */
const frameOf = (sheet: Sheet, facing: Facing, step: 0 | 1 | 2, bob: 0 | 1) => {
  const row = ROW[facing]
  if (sheet.layout === "person") return step === 0 ? row : 3 + row * 2 + (step - 1)
  if (sheet.layout === "pokemon") return row * 2 + (step === 0 ? bob : step - 1)
  if (sheet.layout === "facing") return row * 2
  return 0
}

/**
 * The scene's actors `seconds` into it, their moments already in seconds:
 * those on stage, posed, ordered back to front.
 */
export const poseCast = (
  actors: Actor<number>[],
  tosses: Toss<number>[],
  seconds: number,
  origin: (map: string | undefined) => { x: number; y: number },
): Posed[] => {
  const posed: Posed[] = []
  for (const actor of actors) {
    if (actor.from !== undefined && seconds < actor.from) continue
    if (actor.to !== undefined && seconds >= actor.to) continue
    const sheet: Sheet = SHEETS[actor.sprite ?? "gold"]
    const marks = actor.marks.toSorted((a, b) => a.at - b.at)
    const first = marks[0]
    if (!first) continue
    const base = origin(actor.map)
    let x = first.x
    let y = first.y
    // The leg under way, when it goes somewhere, and the way of the last leg that did.
    let walking: Facing | null = null
    let heading: Facing | null = null
    let walked = 0
    let arrived = -Infinity
    const next = marks.findIndex((mark) => seconds < mark.at)
    const last = marks[marks.length - 1]!
    if (next === -1) {
      x = last.x
      y = last.y
    } else if (next > 0) {
      const a = marks[next - 1]!
      const b = marks[next]!
      const t = (seconds - a.at) / (b.at - a.at)
      x = a.x + (b.x - a.x) * t
      y = a.y + (b.y - a.y) * t
    }
    for (let i = 1; i < marks.length; i++) {
      const a = marks[i - 1]!
      const b = marks[i]!
      if (a.at > seconds) break
      if (a.x === b.x && a.y === b.y) continue
      heading = facingOf(b.x - a.x, b.y - a.y)
      if (seconds < b.at) {
        walking = heading
        walked = (Math.hypot(b.x - a.x, b.y - a.y) * (seconds - a.at)) / (b.at - a.at)
      } else arrived = b.at
    }
    // Standing, they face as told (a turn made since they arrived), else the way they walked in.
    let facing: Facing = walking ?? heading ?? "south"
    if (!walking && typeof actor.face === "string") facing = actor.face
    else if (!walking && typeof actor.face === "object") {
      const turn = actor.face.findLast((t) => t.at <= seconds)
      if (turn && (turn.at >= arrived || !heading)) facing = turn.to
    }
    // Two strides a tile, each a step then a stand, as the game walks.
    const stride = Math.floor(walked * 4) % 4
    const step: 0 | 1 | 2 = walking ? (stride === 0 ? 1 : stride === 2 ? 2 : 0) : 0
    const bob = (Math.floor(seconds / BOB_SECONDS) % 2) as 0 | 1
    const hop = actor.hops?.findLast((at) => at <= seconds)
    const hopT = hop === undefined ? 1 : (seconds - hop) / HOP_SECONDS
    const lift = hopT < 1 ? Math.sin(Math.PI * hopT) * HOP_HEIGHT : 0
    const emote = actor.emotes?.findLast(
      (e) => e.at <= seconds && seconds < e.at + (e.seconds ?? 1),
    )
    const pop = emote ? (seconds - emote.at) / EMOTE_POP_SECONDS : 1
    const pose = actor.poses?.findLast((p) => p.at <= seconds)
    posed.push({
      sprite: actor.sprite,
      frame:
        pose?.frame ??
        frameOf(
          sheet,
          facing,
          step,
          sheet.layout === "pokemon" && walking ? ((Math.floor(walked * 2) % 2) as 0 | 1) : bob,
        ),
      flip: facing === "east",
      x: base.x + x,
      y: base.y + y,
      lift: lift + (actor.lift ?? 0),
      emote: emote && {
        kind: emote.kind,
        lift: pop < 1 ? Math.sin(Math.PI * pop) * EMOTE_POP : 0,
      },
    })
  }
  for (const toss of tosses) {
    if (seconds < toss.span.from || seconds > toss.span.to) continue
    const base = origin(toss.map)
    const t = (seconds - toss.span.from) / (toss.span.to - toss.span.from)
    posed.push({
      sprite: toss.sprite,
      frame: 0,
      flip: false,
      x: base.x + toss.from.x + (toss.to.x - toss.from.x) * t,
      y: base.y + toss.from.y + (toss.to.y - toss.from.y) * t,
      lift: Math.sin(Math.PI * t) * (toss.height ?? 1.5),
    })
  }
  return posed.toSorted((a, b) => a.y - b.y)
}

export type CastImages = {
  sheets: Record<SheetId, HTMLImageElement>
  emotes: Record<EmoteId, HTMLImageElement>
}

/** The posed actors as sprites on the map: each stands on its tile's bottom edge, centered on it. */
export const castSprites = (posed: Posed[], images: CastImages): Sprite[] =>
  posed.flatMap((actor) => {
    if (!actor.sprite) return []
    const sheet: Sheet = SHEETS[actor.sprite]
    const width = sheet.frame / 16
    const height = sheet.height / 16
    return {
      image: images.sheets[actor.sprite],
      source: { x: actor.frame * sheet.frame, y: 0, width: sheet.frame, height: sheet.height },
      x: actor.x + 0.5 - width / 2,
      y: actor.y + 1 - height - actor.lift,
      width,
      height,
      flip: actor.flip,
    }
  })

/** The bubbles over the actors' heads, drawn over everything on the map. */
export const emotesOverlay =
  (posed: Posed[], images: CastImages): Overlay =>
  (context, { scale, toScreen }) => {
    context.imageSmoothingEnabled = false
    for (const actor of posed) {
      if (!actor.emote) continue
      const sheet: Sheet = SHEETS[actor.sprite ?? "gold"]
      // A person's head is near the top of their frame; the bubble sits just over it.
      const top = actor.y + 1 - sheet.height / 16 - actor.lift - 0.75 - actor.emote.lift
      const at = toScreen(actor.x, top)
      context.drawImage(
        images.emotes[actor.emote.kind],
        Math.round(at.x),
        Math.round(at.y),
        Math.round(scale),
        Math.round(scale),
      )
    }
  }
