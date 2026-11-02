import art from "~/generated/game-art.json"

// PROF. OAK throwing a Poké Ball and a POKéMON coming out, after the game's own
// release: CreatePokeballSpriteToReleaseMon and SpriteCB_ReleasedMonFlyOut in
// game/src/pokeball.c, PokeBallOpenParticleAnimation in
// game/src/battle_anim_throw.c, and the emerge affine animation in
// game/src/data.c. Units are GBA pixels and 60 Hz frames.

/** The stage canvas; x has room on the left for WOOPER's arc out of the ball. */
export const STAGE = { width: 192, height: 136 }

export const OVAL = { x: 48, y: 104 }
export const OAK = { x: 100, y: 26 }
/** OAK's outstretched hand, where the ball leaves from and returns to. */
export const HAND = { x: 109, y: 76 }
/** Where the ball lands and opens. */
export const BALL_LANDING = { x: 72, y: 112 }
type Bounds = { left: number; right: number; top: number; bottom: number }
const artBounds = (file: string) =>
  (art as Record<string, { art?: Bounds }>)[`people/${file}.png`]!.art!

const oakArt = artBounds("oak")
/** OAK's height in the Pokédex's decimeters, which sets the scale for everyone. */
const OAK_HEIGHT = 17.5
const PIXELS_PER_DECIMETER = (oakArt.bottom - oakArt.top + 1) / OAK_HEIGHT

/** The row the POKéMON stand on: just behind OAK's feet, on the oval. */
export const FLOOR = OAK.y + oakArt.bottom - 2
/** Where a POKéMON stands, unless it is too wide to fit before OAK. */
const MON_X = 72
/** The rightmost column a POKéMON may reach, so it stays clear of OAK's hand. */
const MON_RIGHT = 106

/**
 * The POKéMON OAK sends out, one per throw: WOOPER from his own intro, then
 * the opening's cast. Each is scaled from its Pokédex height (species_info's
 * .height, in decimeters) against OAK's, and `center` and `foot` place its
 * art: the middle of its first frame and the row under its lowest pixel.
 */
export const ROSTER = (
  [
    ["wooper", 4],
    ["torchic", 4],
    ["manectric", 15],
    ["volbeat", 7],
    ["flygon", 20],
  ] as const
).map(([species, height]) => {
  const bounds = artBounds(`mons/${species}`)
  const size = (height * PIXELS_PER_DECIMETER) / (bounds.bottom - bounds.top + 1)
  const halfWidth = ((bounds.right - bounds.left + 1) * size) / 2
  return {
    species,
    size,
    center: (bounds.left + bounds.right + 1) / 2,
    foot: bounds.bottom + 1,
    home: { x: Math.round(Math.min(MON_X, MON_RIGHT - halfWidth)), y: FLOOR },
  }
})

/** The ball's frames: closed, open, then empty once it has vanished. */
export const BALL_FRAME = { closed: 0, open: 1 } as const

/** The release's particle frames: 0, 1, 2, 0 mirrored, 2, 1 (sAnim_RegularBall). */
export const PARTICLE_FRAMES: { frame: number; flip: boolean }[] = [
  { frame: 0, flip: false },
  { frame: 1, flip: false },
  { frame: 2, flip: false },
  { frame: 0, flip: true },
  { frame: 2, flip: false },
  { frame: 1, flip: false },
]

/** RGB(31, 22, 30), the Poké Ball's open fade color, in 8-bit channels. */
export const OPEN_FADE = "rgb(248 176 240)"

const THROW = 28
const BOUNCE = 8
/** The game waits 32 frames between the ball appearing and it opening. */
const WAIT = 32
const OPEN = 5
const EMERGE = 32
const IDLE = 330
const RECALL = 12
const CLOSE = 6

/** When each beat starts, in frames from the start of the loop. */
export const BEATS = (() => {
  const throwAt = 30
  const bounceAt = throwAt + THROW
  const openAt = bounceAt + BOUNCE + WAIT
  const outAt = openAt + EMERGE
  const recallAt = outAt + IDLE
  const closeAt = recallAt + RECALL
  const returnAt = closeAt + CLOSE
  const restAt = returnAt + THROW
  return {
    throwAt,
    bounceAt,
    openAt,
    outAt,
    recallAt,
    closeAt,
    returnAt,
    restAt,
    loop: restAt + 90,
  }
})()

export type Ball = { x: number; y: number; frame: number; rotation: number }
export type Particle = { x: number; y: number; frame: number; flip: boolean }
export type Mon = {
  /** Index into ROSTER. */
  species: number
  /** The middle of its feet. */
  x: number
  y: number
  /** Of its 64x64 picture: its size times how far it has grown out of the ball. */
  scale: number
  tint: number
  frame: number
}

export type ReleaseState = { ball: Ball | null; particles: Particle[]; mon: Mon | null }

const lerp = (from: number, to: number, t: number) => from + (to - from) * t

/** The ball's flight between two points: a parabola, turning a quarter every 3 frames. */
const flight = (from: { x: number; y: number }, to: { x: number; y: number }, t: number): Ball => ({
  x: Math.round(lerp(from.x, to.x, t)),
  y: Math.round(lerp(from.y, to.y, t) - 4 * 28 * t * (1 - t)),
  frame: BALL_FRAME.closed,
  rotation: Math.floor((t * THROW) / 3) % 4,
})

/** gSineTable's Sin(index, amplitude), with 256 steps to a full turn. */
const sin = (index: number, amplitude: number) =>
  Math.trunc(Math.sin((index * Math.PI * 2) / 256) * amplitude)

/** The emerge affine animation: 0x28 of 0x100, then 0x12 more each frame for 12 frames. */
const emergeScale = (frame: number) => Math.min(0x100, 0x28 + 0x12 * Math.min(frame, 12)) / 0x100

/** Everything on the stage `frame` frames into the loop. */
export const releaseAt = (loopFrame: number): ReleaseState => {
  const f = loopFrame % BEATS.loop
  const species = Math.floor(loopFrame / BEATS.loop) % ROSTER.length
  const { home, size } = ROSTER[species]!
  const { throwAt, bounceAt, openAt, outAt, recallAt, closeAt, returnAt, restAt } = BEATS

  if (f < throwAt || f >= restAt) return { ball: null, particles: [], mon: null }

  if (f < bounceAt)
    return { ball: flight(HAND, BALL_LANDING, (f - throwAt) / THROW), particles: [], mon: null }

  if (f < openAt) {
    const bounce =
      f - bounceAt < BOUNCE ? Math.round(Math.sin(((f - bounceAt) / BOUNCE) * Math.PI) * 4) : 0
    return {
      ball: { ...BALL_LANDING, y: BALL_LANDING.y - bounce, frame: BALL_FRAME.closed, rotation: 0 },
      particles: [],
      mon: null,
    }
  }

  if (f < recallAt) {
    const since = f - openAt
    const ball = since < OPEN ? { ...BALL_LANDING, frame: BALL_FRAME.open, rotation: 0 } : null

    // One particle a frame for 16 frames, in eight directions, each flying out
    // 2px a frame until 50px from the ball (PokeBallOpenParticleAnimation_Step2).
    const particles: Particle[] = []
    for (let spawned = 0; spawned < 16 && spawned <= since; spawned++) {
      const radius = (since - spawned) * 2
      if (radius >= 50) continue
      const angle = (spawned % 8) * 32
      const cycle = PARTICLE_FRAMES[(since - spawned) % PARTICLE_FRAMES.length]!
      particles.push({
        x: BALL_LANDING.x + sin(angle, radius),
        y: BALL_LANDING.y - 5 + sin(angle + 64, radius),
        ...cycle,
      })
    }

    // The POKéMON flies from the ball to its place over 32 frames, pushed up
    // and left by up to 32px on the way, while it grows and its ball color fades.
    const progress = Math.min(since * 4, 128)
    const arc = progress < 128 ? -Math.trunc(sin(progress, 256) / 8) : 0
    const mon: Mon = {
      species,
      x: Math.round(lerp(BALL_LANDING.x, home.x, progress / 128)) + arc,
      y: Math.round(lerp(BALL_LANDING.y, home.y, progress / 128)) + arc,
      scale: emergeScale(since) * size,
      tint: Math.max(0, 1 - since / EMERGE),
      // The front animation's second frame, held briefly on arrival, then a blink now and then.
      frame: f < outAt ? 0 : (f - outAt) % 90 < 70 ? 0 : 1,
    }
    if (f >= outAt && f - outAt < 24) mon.frame = 1
    return { ball, particles, mon }
  }

  if (f < closeAt) {
    // Called back: tinted again and shrinking into the ball.
    const t = (f - recallAt) / RECALL
    return {
      ball: t > 0.5 ? { ...BALL_LANDING, frame: BALL_FRAME.open, rotation: 0 } : null,
      particles: [],
      mon: {
        species,
        x: Math.round(lerp(home.x, BALL_LANDING.x, t)),
        y: Math.round(lerp(home.y, BALL_LANDING.y, t)),
        scale: lerp(1, 0x28 / 0x100, t) * size,
        tint: Math.min(1, t * 2),
        frame: 0,
      },
    }
  }

  if (f < returnAt)
    return {
      ball: { ...BALL_LANDING, frame: BALL_FRAME.closed, rotation: 0 },
      particles: [],
      mon: null,
    }

  return { ball: flight(BALL_LANDING, HAND, (f - returnAt) / THROW), particles: [], mon: null }
}

/** The settled scene, for visitors who prefer less motion: WOOPER out beside OAK. */
export const restingRelease: ReleaseState = {
  ball: null,
  particles: [],
  mon: { species: 0, ...ROSTER[0]!.home, scale: ROSTER[0]!.size, tint: 0, frame: 0 },
}
