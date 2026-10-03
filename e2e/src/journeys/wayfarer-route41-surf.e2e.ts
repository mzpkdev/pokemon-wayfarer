import { afterEach, beforeAll, beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols, type SkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

// Route 40's bottom row (12-20, 60) is a row of warps on water into Route
// 41's open water (37-45, 12), which reaches Cianwood City's seam. The walker
// graph models it as an off-screen water link; this journey proves the player
// can make the same crossing: Surf south from Route 40 over the warp row,
// land on Route 41, and surf on into Cianwood City.
//
// The route avoids New Game objects; trainers are marked as already battled
// so their sight lines don't stop the journey, and wild encounters are off.
const TRAINER_FLAGS_START = 0x500
const TRAINER_FLAGS_END = 0x85f

type Segment = [Direction, number, number]

const route41: Segment[] = [
  ["down", 39, 23],
  ["right", 62, 23],
  ["down", 62, 34],
  ["left", 61, 34],
  ["down", 61, 43],
  ["right", 64, 43],
  ["down", 64, 55],
  ["left", 44, 55],
  ["down", 44, 60],
  ["left", 37, 60],
  ["down", 37, 61],
  ["left", 31, 61],
  ["down", 31, 66],
  ["left", 30, 66],
  ["down", 30, 68],
  ["left", 29, 68],
  ["down", 29, 69],
  ["left", 13, 69],
  ["up", 13, 59],
  ["left", 0, 59],
]

describe.sequential("Wayfarer Route 40 water warps to Cianwood", () => {
  let game: GameSession
  let symbols: SkyEmuSymbols

  beforeAll(async () => {
    symbols = await readSkyEmuSymbols(requireSymbolsPath())
  })
  beforeEach(async () => {
    game = await GameSession.launch()
  })
  afterEach(async () => {
    await game?.close()
  })

  const markTrainersBattled = async () => {
    const client = (game as unknown as { running: RunningSkyEmu }).running.client
    const abi = await client.readBytes(symbols.address("gE2ETestAbi"), 16)
    const flagsOffset = new DataView(abi.buffer, abi.byteOffset, abi.byteLength).getUint16(12, true)
    const pointer = await client.readBytes(symbols.address("gSaveBlock1Ptr"), 4)
    const saveBlock1 = new DataView(pointer.buffer, pointer.byteOffset, pointer.byteLength).getUint32(0, true)
    const first = TRAINER_FLAGS_START >> 3
    const last = TRAINER_FLAGS_END >> 3
    await client.writeBytes(saveBlock1 + flagsOffset + first, new Uint8Array(last - first + 1).fill(0xff))
  }

  // One tile in a direction, retrying while a wandering swimmer is in the way.
  const step = async (direction: Direction) => {
    const before = (await game.state.read()).player
    for (let attempt = 0; attempt < 20; attempt++) {
      await game.player.move(direction)
      for (let frame = 0; frame < 40; frame += 4) {
        await game.wait.frames(4)
        const state = await game.state.read()
        if (state.player.x !== before.x || state.player.y !== before.y) return
      }
    }
    throw new Error(`Did not surf ${direction} from ${before.x},${before.y}`)
  }

  const surfTo = async (direction: Direction, x: number, y: number) => {
    for (let tiles = 0; tiles < 80; tiles++) {
      const { player } = await game.state.read()
      if (player.x === x && player.y === y) return
      await step(direction)
    }
    throw new Error(`Did not reach ${x},${y}`)
  }

  it("surfs over Route 40's warp row onto Route 41 and on into Cianwood City", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: { map: "route-40", x: 14, y: 22 } },
      story: { flags: { disableEncounters: true } },
      party: [{ species: "lapras", level: 50, moves: ["surf"] }],
      determinism: { textSpeed: "instant", rngSeed: 1 },
    })
    await markTrainersBattled()

    // Mount Surf facing the water south of the beach.
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await game.wait.until(
      (state) => state.dialogue.message === "want-to-use-surf" && !state.dialogueOpen,
      "Surf prompt",
    )
    await game.wait.frames(12)
    await game.controls.press("a")
    await game.wait.until((state) => state.dialogue.message === "player-used-surf", "Surf confirmation")
    await game.dialogue.waitForClosed()
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.ready && state.player.surfing && state.player.y === 23,
      "on the water at (14, 23)",
      3_600,
    )

    // South to the warp row; the step onto (14, 60) lands on Route 41 warp 6.
    await surfTo("down", 14, 59)
    await game.player.move("down")
    await game.wait.frames(30)
    // The warp fires when the player presses on into it from its tile.
    const onWarp = await game.state.read()
    if (onWarp.map.name === "route-40") await game.player.move("down")
    await game.wait.until(
      (state) => state.ready && state.map.name === "route-41",
      "landed on Route 41",
      1_800,
    )
    const landed = await game.state.read()
    expect({ x: landed.player.x, y: landed.player.y, surfing: landed.player.surfing }).toEqual({
      x: 39,
      y: 12,
      surfing: true,
    })

    for (const [direction, x, y] of route41) await surfTo(direction, x, y)

    // One more tile west crosses the seam into Cianwood City.
    await game.player.move("left")
    await game.wait.until(
      (state) => state.ready && state.map.name === "cianwood-city",
      "surfed into Cianwood City",
      1_800,
    )
    const arrived = await game.state.read()
    expect({ x: arrived.player.x, y: arrived.player.y }).toEqual({ x: 36, y: 49 })
  })
})
