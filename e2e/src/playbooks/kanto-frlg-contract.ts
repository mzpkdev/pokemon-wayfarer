import { type Direction, type GameMap } from "../harness/game-session/catalog"
import { type GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"
import { kantoOpeningCollision } from "../fixtures/kanto-opening-collision"

export const sceneFinished = (state: GameState): boolean =>
  state.ready && !state.scriptActive && !state.controlsLocked && !state.dialogueOpen

export const expectFidelityState = (
  state: GameState,
  expected: Partial<{
    map: GameState["map"]["name"]
    x: number
    y: number
    facing: GameState["player"]["facing"]
    phase: number
    partySize: number
  }>,
  context: string,
): void => {
  const observed = {
    map: state.map.name,
    x: state.player.x,
    y: state.player.y,
    facing: state.player.facing,
    phase: state.origin.pallet.phase,
    partySize: state.party.length,
  }
  for (const [key, value] of Object.entries(expected)) {
    if (observed[key as keyof typeof observed] !== value)
      throw new Error(
        `${context}: expected ${key}=${JSON.stringify(value)}, observed ${JSON.stringify(observed)}`,
      )
  }
}

/**
 * Strict field-dialogue transcript. The ROM counts every field message box,
 * so each asserted message must be exactly the next one shown: nothing may be
 * skipped, merged, or shown without an assertion.
 */
export class FieldTranscript {
  private sequence = 0

  constructor(private readonly game: GameSession) {}

  /** Adopts the current message count after setup that is not under test. */
  async sync(): Promise<void> {
    this.sequence = (await this.game.state.read()).dialogue.sequence
  }

  /** Waits for the next message box and asserts its complete expanded text. */
  async expect(expected: string, context: string, maxFrames = 3_000): Promise<GameState> {
    const next = this.sequence + 1
    await this.game.wait.until(
      (state) => state.dialogue.sequence >= next,
      `${context}: next message`,
      maxFrames,
    )
    const state = await this.game.state.read()
    if (state.dialogue.sequence !== next)
      throw new Error(
        `${context}: ${state.dialogue.sequence - next} unasserted message(s) appeared first; latest ${JSON.stringify(state.dialogue.fullText)}`,
      )
    if (state.dialogue.fullText !== expected)
      throw new Error(
        `${context}: FRLG dialogue mismatch\nexpected: ${JSON.stringify(expected)}\nobserved: ${JSON.stringify(state.dialogue.fullText)}`,
      )
    this.sequence = next
    return state
  }

  /** Pages through the current message until it closes or the next one opens. */
  async advance(context: string): Promise<void> {
    // A message is counted when it starts; its box opens a frame later.
    // Auto-closing messages may instead hand straight to the next one.
    await this.game.wait.until(
      (state) =>
        state.dialogueOpen || state.dialogue.sequence > this.sequence || sceneFinished(state),
      `${context}: message box`,
      120,
    )
    for (let press = 0; press < 40; press++) {
      const state = await this.game.state.read()
      if (state.dialogue.sequence > this.sequence) return
      // The box stays visible after its last page until the script's
      // button wait is satisfied; only then is the message finished.
      if (!state.dialogueOpen && !state.dialogue.awaitingButton) {
        // A printed message may still be followed by waitfanfare or other
        // waits before its waitbuttonpress; keep watching for that wait.
        let pending = false
        for (let frame = 0; frame < 120 && !pending; frame += 6) {
          await this.game.wait.frames(6)
          const settled = await this.game.state.read()
          if (settled.dialogue.sequence > this.sequence || sceneFinished(settled)) return
          pending = settled.dialogueOpen || settled.dialogue.awaitingButton
        }
        if (!pending) return
        continue
      }
      if (state.choice.kind !== "none")
        throw new Error(`${context}: unexpected ${state.choice.kind} choice`)
      await this.game.controls.press("a")
      await this.game.wait.frames(8)
    }
    throw new Error(`${context}: message did not finish`)
  }

  /** Asserts and dismisses messages in order. */
  async say(messages: readonly string[], context: string): Promise<void> {
    for (const [index, message] of messages.entries()) {
      const label = messages.length > 1 ? `${context} #${index + 1}` : context
      await this.expect(message, label)
      await this.advance(label)
    }
  }

  /** Asserts a message that ends in a YES/NO box and answers it. */
  async ask(
    message: string,
    answer: "yes" | "no",
    context: string,
    expectedPic?: GameState["presentation"]["displayedMonSpecies"],
  ): Promise<void> {
    await this.expect(message, context)
    // Page through the question; the YES/NO box opens after its last page
    // and ignores input for its first frames, so a late press is harmless.
    for (let press = 0; press < 20; press++) {
      const state = await this.game.state.read()
      if (state.choice.kind === "yes-no") break
      if (state.dialogueOpen) await this.game.controls.press("a")
      await this.game.wait.frames(10)
    }
    await this.game.wait.until((state) => state.choice.kind === "yes-no", `${context}: YES/NO`, 600)
    const state = await this.game.state.read()
    if (state.choice.cursor !== 0 || state.choice.optionCount !== 2)
      throw new Error(`${context}: YES/NO did not default to YES: ${JSON.stringify(state.choice)}`)
    if (expectedPic !== undefined && state.presentation.displayedMonSpecies !== expectedPic)
      throw new Error(
        `${context}: expected ${expectedPic} picture, observed ${state.presentation.displayedMonSpecies}`,
      )
    // The YES/NO task ignores input for its first five frames.
    await this.game.wait.frames(8)
    if (answer === "no") {
      await this.game.controls.press("down")
      await this.game.wait.until((current) => current.choice.cursor === 1, `${context}: NO`, 120)
    }
    await this.game.controls.press("a")
    await this.game.wait.until(
      (current) => current.choice.kind === "none",
      `${context}: ${answer.toUpperCase()} chosen`,
      300,
    )
  }

  /** Waits until the scene ends and proves no message went unasserted. */
  async idle(context: string, maxFrames = 1_800): Promise<GameState> {
    await this.game.wait.until(sceneFinished, `${context}: scene end`, maxFrames)
    const state = await this.game.state.read()
    if (state.dialogue.sequence !== this.sequence)
      throw new Error(`${context}: unasserted message ${JSON.stringify(state.dialogue.fullText)}`)
    return state
  }

  /** Walks to an object (which may wander), talks to it, and asserts the exchange. */
  async talkTo(
    localId: number,
    messages: readonly string[],
    context: string,
    side: Direction = "down",
    near?: Tile,
  ): Promise<void> {
    await this.idle(`${context} (before)`)
    for (let attempt = 0; attempt < 6; attempt++) {
      await approachObject(this.game, localId, context, side, near)
      await this.game.player.interact()
      const opened = await this.game.wait
        .until((state) => state.dialogue.sequence > this.sequence, `${context}: talk`, 90)
        .then(() => true)
        .catch(() => false)
      if (opened) {
        await this.say(messages, context)
        await this.idle(context)
        return
      }
    }
    throw new Error(`${context}: object ${localId} never answered`)
  }

  /** Interacts with whatever the player faces and asserts the full exchange. */
  async interact(messages: readonly string[], context: string): Promise<void> {
    await this.idle(`${context} (before)`)
    await this.game.player.interact()
    await this.say(messages, context)
    await this.idle(context)
  }
}

/**
 * Strict battle transcript over the main battle window, Oak's tutorial
 * window, and Oak's party-menu voiceover.
 */
export class BattleTranscript {
  private sequence = 0
  readonly seen: string[] = []

  constructor(private readonly game: GameSession) {}

  async sync(): Promise<void> {
    this.sequence = (await this.game.state.read()).battle.dialogue.sequence
  }

  private async collect(context: string): Promise<GameState> {
    const state = await this.game.state.read()
    const current = state.battle.dialogue.sequence
    if (current > this.sequence + 1)
      throw new Error(
        `${context}: missed ${current - this.sequence - 1} battle message(s) before ${JSON.stringify(state.battle.dialogue.text)}; seen ${JSON.stringify(this.seen)}`,
      )
    if (current === this.sequence + 1) {
      // Window clears print blank strings; they are not dialogue.
      const text = state.battle.dialogue.text.replace(/\n$/, "")
      if (text.trim()) this.seen.push(text)
      this.sequence = current
    }
    return state
  }

  /** Steps frame by frame, pressing A when text waits, until `done`. */
  async runUntil(
    done: (state: GameState) => boolean,
    context: string,
    maxFrames = 12_000,
  ): Promise<void> {
    let quiet = 0
    for (let frame = 0; frame < maxFrames; frame++) {
      const before = this.sequence
      const state = await this.collect(context)
      if (done(state)) return
      quiet = this.sequence === before ? quiet + 1 : 0
      if (quiet >= 30) {
        await this.game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
        await this.collect(context)
        quiet = 0
      } else await this.game.wait.frames(1)
    }
    throw new Error(
      `${context}: battle did not reach its checkpoint; seen ${JSON.stringify(this.seen)}`,
    )
  }

  /** Asserts the messages seen since `from` exactly, then drops them. */
  expectSeen(expected: readonly string[], context: string): void {
    const observed = this.seen.splice(0)
    if (JSON.stringify(observed) !== JSON.stringify(expected))
      throw new Error(
        `${context}: battle transcript mismatch\nexpected: ${JSON.stringify(expected, null, 1)}\nobserved: ${JSON.stringify(observed, null, 1)}`,
      )
  }
}

// The player (OBJ_EVENT_ID_PLAYER) and the following Pokémon
// (OBJ_EVENT_ID_FOLLOWER), which the player walks through.
const passableLocalIds = new Set([255, 254])

const steps: Record<Direction, readonly [number, number]> = {
  up: [0, -1],
  down: [0, 1],
  left: [-1, 0],
  right: [1, 0],
}
const ledges: Record<string, Direction> = { "^": "up", v: "down", "<": "left", ">": "right" }
// Walkable tiles whose sides refuse steps (MB_IMPASSABLE_*); see the exporter.
const blockedSides: Record<string, readonly Direction[]> = {
  ".": [],
  n: ["up"],
  s: ["down"],
  e: ["right"],
  w: ["left"],
  "1": ["up", "right"],
  "2": ["up", "left"],
  "3": ["down", "right"],
  "4": ["down", "left"],
  "|": ["up", "down"],
  "=": ["left", "right"],
}
const opposite: Record<Direction, Direction> = {
  up: "down",
  down: "up",
  left: "right",
  right: "left",
}

type WalkableMap = keyof typeof kantoOpeningCollision
type Tile = { x: number; y: number }

const route = (
  map: WalkableMap,
  from: Tile,
  to: Tile,
  occupied: ReadonlySet<string>,
  avoid: ReadonlySet<string>,
): Direction[] | undefined => {
  const rows = kantoOpeningCollision[map]
  const key = (tile: Tile) => `${tile.x},${tile.y}`
  const cellAt = (tile: Tile) => rows[tile.y]?.[tile.x] ?? "#"
  const passable = (tile: Tile) =>
    cellAt(tile) in blockedSides && !occupied.has(key(tile)) && !avoid.has(key(tile))
  const sideOpen = (from: Tile, to: Tile, direction: Direction) =>
    !(blockedSides[cellAt(from)] ?? []).includes(direction) &&
    !(blockedSides[cellAt(to)] ?? []).includes(opposite[direction])
  const previous = new Map<string, { from: string; direction: Direction } | null>([
    [key(from), null],
  ])
  const queue: Tile[] = [from]
  while (queue.length) {
    const tile = queue.shift()!
    if (tile.x === to.x && tile.y === to.y) break
    for (const [direction, [dx, dy]] of Object.entries(steps) as [Direction, [number, number]][]) {
      let next = { x: tile.x + dx, y: tile.y + dy }
      const cell = rows[next.y]?.[next.x]
      if (!sideOpen(tile, next, direction)) continue
      if (cell && ledges[cell]) {
        if (ledges[cell] !== direction) continue
        next = { x: next.x + dx, y: next.y + dy }
        if (!passable(next)) continue
      } else if (!passable(next) && !(next.x === to.x && next.y === to.y && !occupied.has(key(next)))) continue
      if (previous.has(key(next))) continue
      previous.set(key(next), { from: key(tile), direction })
      queue.push(next)
    }
  }
  if (!previous.has(key(to))) return undefined
  const path: Direction[] = []
  for (let at = previous.get(key(to)); at;) {
    path.unshift(at.direction)
    at = previous.get(at.from)
  }
  return path
}

/**
 * Walks to a tile on foot, re-planning around live objects each step. The
 * final tile may be a warp or trigger; `avoid` keeps other triggers untouched.
 */
export const walkTo = async (
  game: GameSession,
  target: Tile & { facing?: Direction },
  context: string,
  avoid: readonly Tile[] = [],
): Promise<void> => {
  const avoided = new Set(avoid.map((tile) => `${tile.x},${tile.y}`))
  const trail: string[] = []
  for (let attempt = 0; attempt < 400; attempt++) {
    const state = await game.state.read()
    if (state.dialogueOpen || state.battle.active)
      throw new Error(`${context}: walking opened ${state.battle.active ? "a battle" : "dialogue"}`)
    const map = state.map.name as WalkableMap
    if (!(map in kantoOpeningCollision)) throw new Error(`${context}: no collision data for ${map}`)
    const here = { x: state.player.x, y: state.player.y }
    if (here.x === target.x && here.y === target.y) {
      if (target.facing && state.player.facing !== target.facing) {
        await game.player.move(target.facing)
        // A press during the turn-in-place animation is ignored.
        await game.wait.frames(12)
        continue
      }
      // Coordinates switch to the destination when a step starts; input is
      // ignored until the step animation ends.
      // A walking step lasts 16 frames.
      await game.wait.frames(16)
      return
    }
    const occupied = new Set(
      state.objects
        .filter((object) => object.visible && !passableLocalIds.has(object.localId))
        .map((object) => `${object.x},${object.y}`),
    )
    const path = route(map, here, target, occupied, avoided)
    trail.push(
      `${here.x}:${here.y}->${path?.[0] ?? `none(objects ${state.objects.map((o) => `${o.localId}@${o.x},${o.y}`).join(" ")})`}`,
    )
    if (!path?.length) {
      // A wandering NPC may briefly close the only lane.
      await game.wait.frames(16)
      continue
    }
    await game.player.move(path[0]!)
    await game.wait.frames(2)
    const after = await game.state.read()
    if (after.map.name !== map) return
  }
  throw new Error(
    `${context}: could not reach ${target.x}:${target.y}; last steps ${trail.slice(-12).join(" ")}`,
  )
}

/** Steps across a map edge, door, or stair and waits for the destination. */
export const walkOnto = async (
  game: GameSession,
  direction: Direction,
  destination: GameMap,
  context: string,
): Promise<void> => {
  // The first press may only turn the player toward the exit.
  for (let press = 0; press < 6; press++) {
    if ((await game.state.read()).map.name === destination) break
    await game.player.move(direction)
    await game.wait.frames(20)
  }
  // Arrival scenes may start at once, so only the map change is awaited.
  await game.wait.until((state) => state.map.name === destination, `${context}: arrival`, 1_800)
}

export const visibleObject = (state: GameState, localId: number, context: string) => {
  const object = state.objects.find((candidate) => candidate.localId === localId)
  if (!object?.visible)
    throw new Error(
      `${context}: local object ${localId} was not visible: ${JSON.stringify(state.objects)}`,
    )
  return object
}

/** Walks beside a (possibly wandering) object, facing it. */
export const approachObject = async (
  game: GameSession,
  localId: number,
  context: string,
  side: Direction = "down",
  near?: Tile,
): Promise<void> => {
  // Objects spawn only near the camera; `near` brings a distant one into view.
  const spawned = (state: GameState) =>
    state.objects.some((object) => object.localId === localId && object.visible)
  if (near && !spawned(await game.state.read())) await walkTo(game, near, `${context}: approach`)
  const sides: Direction[] = [
    side,
    ...(["down", "left", "right", "up"] as const).filter((d) => d !== side),
  ]
  for (let attempt = 0; attempt < 12; attempt++) {
    const state = await game.state.read()
    const object = visibleObject(state, localId, context)
    const rows = kantoOpeningCollision[state.map.name as WalkableMap]
    const player = { x: state.player.x, y: state.player.y }
    // Prefer the requested side, but stand where nobody else is standing.
    const free = (direction: Direction) => {
      const [dx, dy] = steps[direction]
      const tile = { x: object.x + dx, y: object.y + dy }
      if ((tile.x === player.x && tile.y === player.y) || !rows) return true
      return (
        (rows[tile.y]?.[tile.x] ?? "#") in blockedSides &&
        !state.objects.some(
          (other) =>
            other.visible &&
            !passableLocalIds.has(other.localId) &&
            other.x === tile.x &&
            other.y === tile.y,
        )
      )
    }
    const chosen = sides.find(free)
    if (!chosen) {
      await game.wait.frames(30)
      continue
    }
    const [dx, dy] = steps[chosen]
    await walkTo(game, { x: object.x + dx, y: object.y + dy, facing: opposite[chosen] }, context)
    const now = visibleObject(await game.state.read(), localId, context)
    const after = (await game.state.read()).player
    if (now.x === after.x - dx && now.y === after.y - dy && !now.moving) return
  }
  throw new Error(`${context}: object ${localId} kept moving away`)
}

/** Approaches a door, stair, or edge tile from its side and steps through. */
export const walkThrough = async (
  game: GameSession,
  warp: Tile,
  direction: Direction,
  destination: GameMap,
  context: string,
  avoid: readonly Tile[] = [],
): Promise<void> => {
  const [dx, dy] = steps[direction]
  await walkTo(game, { x: warp.x - dx, y: warp.y - dy }, context, avoid)
  await walkOnto(game, direction, destination, context)
}

/** Takes one real step (turning first if needed), e.g. onto a coord trigger. */
export const step = async (game: GameSession, direction: Direction, context: string) => {
  const before = await game.state.read()
  for (let press = 0; press < 4; press++) {
    await game.player.move(direction)
    await game.wait.frames(10)
    const now = await game.state.read()
    if (
      now.player.x !== before.player.x ||
      now.player.y !== before.player.y ||
      now.map.name !== before.map.name ||
      now.scriptActive ||
      now.dialogueOpen
    )
      return
  }
  throw new Error(`${context}: could not step ${direction}`)
}
