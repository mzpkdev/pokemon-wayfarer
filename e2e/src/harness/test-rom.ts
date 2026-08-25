import { createIsolatedRom, type IsolatedRom } from "./isolated-rom"
import { advance, press } from "./input"
import { type SkyEmuButton, type SkyEmuClient } from "./skyemu"
import { startSkyEmu, type RunningSkyEmu } from "./skyemu-server"
import { readSkyEmuSymbols, type SkyEmuSymbols } from "./symbols"
import { requireRomPath, requireSymbolsPath } from "./utils"

const abiVersion = 1
const expectedRequestSize = 88
const expectedResultSize = 16
const expectedStateSize = 20
const expectedRequestStatusOffset = 87
const expectedResultStatusOffset = 14
const maxPatches = 8
const keepMap = 0xffff
const keepCoordinate = -0x8000
const pendingStatus = 1
const successStatus = 3
const errorStatus = 4
const arrangeCommand = 1
const varsStart = 0x4000

const checkpoints = {
  "bedroom-before-clock": 1,
  "new-bark-after-intro": 2,
  "elm-lab-before-intro": 3,
} as const

const maps = {
  "new-bark-town": { mapGroup: 0, mapNum: 0 },
  "elm-lab": { mapGroup: 1, mapNum: 0 },
  "players-house-1f": { mapGroup: 1, mapNum: 3 },
  "players-bedroom": { mapGroup: 1, mapNum: 4 },
} as const

const storyVars = {
  newBarkTownLabState: 0x4074,
  newBarkTownState: 0x4075,
} as const

const storyFlags = {
  hideSilverInNewBark: 0x04a,
  hideElmLabAide: 0x04b,
  adventureStarted: 0x1d4,
  momVisited: 0x265,
} as const

const directions = {
  down: 1,
  up: 2,
  left: 3,
  right: 4,
} as const

const buttons = {
  a: "A",
  b: "B",
  down: "Down",
  left: "Left",
  r: "R",
  right: "Right",
  start: "Start",
  up: "Up",
} as const satisfies Record<string, SkyEmuButton>

const textSpeeds = {
  fast: 2,
  instant: 3,
} as const

const gamePhases = ["boot", "overworld", "dialogue", "battle"] as const
const arrangePhases = ["none", "validate", "new-game", "state", "warp", "field-ready"] as const
const arrangeErrors = [
  "none",
  "command",
  "checkpoint",
  "map",
  "coordinates",
  "facing",
  "text-speed",
  "var-count",
  "var",
  "flag-count",
  "flag",
] as const

export type TestCheckpoint = keyof typeof checkpoints
export type TestMap = keyof typeof maps
export type TestDirection = keyof typeof directions
export type StoryVar = keyof typeof storyVars
export type StoryFlag = keyof typeof storyFlags

export type TestPosition = {
  map?: TestMap
  x: number
  y: number
}

export type ArrangeTestRom = {
  checkpoint: TestCheckpoint
  player?: {
    facing?: TestDirection
    position?: TestPosition
  }
  story?: {
    flags?: Partial<Record<StoryFlag, boolean>>
    vars?: Partial<Record<StoryVar, number>>
  }
  determinism?: {
    rngSeed?: number
    textSpeed?: keyof typeof textSpeeds
  }
}

export type TestRomState = {
  frame: number
  phase: (typeof gamePhases)[number]
  ready: boolean
  map: {
    mapGroup: number
    mapNum: number
  }
  player: {
    x: number
    y: number
    facing: TestDirection | "unknown"
  }
  controlsLocked: boolean
  scriptActive: boolean
  dialogueOpen: boolean
}

type TestRomAbi = {
  requestSize: number
  resultSize: number
  stateSize: number
  requestStatusOffset: number
  resultStatusOffset: number
  flagsOffset: number
  varsOffset: number
}

type ArrangeResult = {
  requestId: number
  error: number
  status: number
  phase: number
}

const uint16 = (bytes: Uint8Array, offset: number): number =>
  bytes[offset]! | (bytes[offset + 1]! << 8)

const int16 = (bytes: Uint8Array, offset: number): number => {
  const value = uint16(bytes, offset)
  return value & 0x8000 ? value - 0x1_0000 : value
}

const uint32 = (bytes: Uint8Array, offset: number): number =>
  (bytes[offset]! |
    (bytes[offset + 1]! << 8) |
    (bytes[offset + 2]! << 16) |
    (bytes[offset + 3]! << 24)) >>>
  0

const entries = <Name extends string, Value>(
  value: Partial<Record<Name, Value>> | undefined,
): [Name, Value][] => Object.entries(value ?? {}) as [Name, Value][]

const parseAbi = (bytes: Uint8Array): TestRomAbi => {
  const version = uint16(bytes, 0)
  const abi: TestRomAbi = {
    requestSize: uint16(bytes, 2),
    resultSize: uint16(bytes, 4),
    stateSize: uint16(bytes, 6),
    requestStatusOffset: uint16(bytes, 8),
    resultStatusOffset: uint16(bytes, 10),
    flagsOffset: uint16(bytes, 12),
    varsOffset: uint16(bytes, 14),
  }
  if (
    version !== abiVersion ||
    abi.requestSize !== expectedRequestSize ||
    abi.resultSize !== expectedResultSize ||
    abi.stateSize !== expectedStateSize ||
    abi.requestStatusOffset !== expectedRequestStatusOffset ||
    abi.resultStatusOffset !== expectedResultStatusOffset
  ) {
    throw new Error(
      `Unsupported test ROM ABI: version=${version}, request=${abi.requestSize}, result=${abi.resultSize}, state=${abi.stateSize}`,
    )
  }
  return abi
}

const parseResult = (bytes: Uint8Array): ArrangeResult => ({
  requestId: uint32(bytes, 0),
  error: uint16(bytes, 12),
  status: bytes[14]!,
  phase: bytes[15]!,
})

const facingName = (facing: number): TestDirection | "unknown" =>
  (Object.entries(directions).find(([, value]) => value === facing)?.[0] as
    | TestDirection
    | undefined) ?? "unknown"

export class TestRom {
  readonly checkpoint = {
    load: async (checkpoint: TestCheckpoint, overrides: Omit<ArrangeTestRom, "checkpoint"> = {}) =>
      this.arrange({ checkpoint, ...overrides }),
  }

  readonly dialogue = {
    state: async () => {
      const state = await this.state.read()
      return { open: state.dialogueOpen }
    },
    waitForOpen: async (maxFrames = 1_200) =>
      this.wait.until((state) => state.dialogueOpen, "open dialogue", maxFrames),
    waitForClosed: async (maxFrames = 1_200) =>
      this.wait.until((state) => !state.dialogueOpen, "closed dialogue", maxFrames),
  }

  readonly player = {
    interact: async () => press(this.client, "A"),
    move: async (direction: TestDirection) => press(this.client, buttons[direction], 3, 1),
    press: async (button: keyof typeof buttons) => press(this.client, buttons[button]),
  }

  readonly state = {
    read: async (): Promise<TestRomState> => {
      const bytes = await this.client.readBytes(
        this.symbols.address("gE2ETestState"),
        this.abi.stateSize,
      )
      return {
        frame: uint32(bytes, 0),
        map: { mapGroup: uint16(bytes, 4), mapNum: uint16(bytes, 6) },
        player: {
          x: int16(bytes, 8),
          y: int16(bytes, 10),
          facing: facingName(bytes[17]!),
        },
        phase: gamePhases[bytes[12]!] ?? "boot",
        ready: bytes[13] === 1,
        controlsLocked: bytes[14] === 1,
        scriptActive: bytes[15] === 1,
        dialogueOpen: bytes[16] === 1,
      }
    },
  }

  readonly story = {
    flag: async (name: StoryFlag): Promise<boolean> => {
      const saveBlock = await this.client.readUint32LE(this.symbols.address("gSaveBlock1Ptr"))
      const id = storyFlags[name]
      const byte = await this.client.readBytes(
        saveBlock + this.abi.flagsOffset + Math.floor(id / 8),
        1,
      )
      return (byte[0]! & (1 << (id % 8))) !== 0
    },
    var: async (name: StoryVar): Promise<number> => {
      const saveBlock = await this.client.readUint32LE(this.symbols.address("gSaveBlock1Ptr"))
      const id = storyVars[name]
      return this.client.readUint16LE(saveBlock + this.abi.varsOffset + (id - varsStart) * 2)
    },
  }

  readonly wait = {
    frames: async (frames: number) => advance(this.client, frames),
    forMap: async (map: TestMap, maxFrames = 1_200) => {
      const expected = maps[map]
      await this.wait.until(
        (state) =>
          state.ready &&
          state.map.mapGroup === expected.mapGroup &&
          state.map.mapNum === expected.mapNum,
        `ready map ${map}`,
        maxFrames,
      )
    },
    forReady: async (maxFrames = 1_200) =>
      this.wait.until((state) => state.ready, "ready overworld", maxFrames),
    until: async (
      predicate: (state: TestRomState) => boolean | Promise<boolean>,
      description: string,
      maxFrames = 1_200,
    ): Promise<void> => {
      for (let elapsed = 0; elapsed <= maxFrames; elapsed += 2) {
        const state = await this.state.read()
        if (await predicate(state)) return
        await advance(this.client, 2)
      }
      throw new Error(`${description} not reached in ${maxFrames} frames; ${await this.describe()}`)
    },
  }

  private requestId = 0

  private constructor(
    private readonly client: SkyEmuClient,
    private readonly symbols: SkyEmuSymbols,
    private readonly abi: TestRomAbi,
    private readonly running: RunningSkyEmu,
    private readonly rom: IsolatedRom,
  ) {}

  static launch = async (): Promise<TestRom> => {
    const rom = await createIsolatedRom(requireRomPath())
    try {
      const symbols = await readSkyEmuSymbols(requireSymbolsPath())
      const running = await startSkyEmu(rom.path)
      try {
        const abi = parseAbi(await running.client.readBytes(symbols.address("gE2ETestAbi"), 16))
        const stateAddress = symbols.address("gE2ETestState")
        let hookFrame = 0
        for (let elapsed = 0; elapsed <= 600; elapsed += 2) {
          hookFrame = await running.client.readUint32LE(stateAddress)
          if (hookFrame > 0) break
          await advance(running.client, 2)
        }
        if (hookFrame === 0) throw new Error("Test ROM hook did not start within 600 frames")
        return new TestRom(running.client, symbols, abi, running, rom)
      } catch (error) {
        await running.stop()
        throw error
      }
    } catch (error) {
      await rom.cleanup()
      throw error
    }
  }

  readonly arrange = async (options: ArrangeTestRom): Promise<void> => {
    const vars = entries(options.story?.vars)
    const flags = entries(options.story?.flags)
    if (vars.length > maxPatches) {
      throw new Error(`Test ROM supports at most ${maxPatches} var overrides`)
    }
    if (flags.length > maxPatches) {
      throw new Error(`Test ROM supports at most ${maxPatches} flag overrides`)
    }

    const bytes = new Uint8Array(this.abi.requestSize)
    const view = new DataView(bytes.buffer)
    const requestId = ++this.requestId
    view.setUint32(0, requestId, true)
    view.setUint16(4, keepMap, true)
    view.setUint16(6, keepMap, true)
    view.setInt16(8, keepCoordinate, true)
    view.setInt16(10, keepCoordinate, true)

    const position = options.player?.position
    if (position?.map) {
      const map = maps[position.map]
      view.setUint16(4, map.mapGroup, true)
      view.setUint16(6, map.mapNum, true)
    }
    if (position) {
      view.setInt16(8, position.x, true)
      view.setInt16(10, position.y, true)
    }

    const rngSeed = options.determinism?.rngSeed ?? 1
    view.setUint32(12, rngSeed, true)
    for (const [index, [name, value]] of vars.entries()) {
      view.setUint16(16 + index * 4, storyVars[name], true)
      view.setUint16(18 + index * 4, value, true)
    }
    for (const [index, [name, value]] of flags.entries()) {
      view.setUint16(48 + index * 4, storyFlags[name], true)
      view.setUint8(50 + index * 4, value ? 1 : 0)
    }
    view.setUint8(80, checkpoints[options.checkpoint])
    view.setUint8(81, directions[options.player?.facing ?? "up"])
    view.setUint8(82, vars.length)
    view.setUint8(83, flags.length)
    view.setUint8(84, textSpeeds[options.determinism?.textSpeed ?? "instant"])
    view.setUint8(85, 1)
    view.setUint8(86, arrangeCommand)

    const requestAddress = this.symbols.address("gE2ETestRequest")
    await this.client.writeBytes(requestAddress, bytes)
    await this.client.writeBytes(
      requestAddress + this.abi.requestStatusOffset,
      Uint8Array.of(pendingStatus),
    )
    await this.waitForResult(requestId)
  }

  readonly close = async (): Promise<void> => {
    await this.running.stop()
    await this.rom.cleanup()
  }

  private readonly describe = async (): Promise<string> => {
    const state = await this.state.read()
    return `phase=${state.phase}, map=${state.map.mapGroup}:${state.map.mapNum}, position=${state.player.x}:${state.player.y}, facing=${state.player.facing}, ready=${state.ready}, controlsLocked=${state.controlsLocked}, scriptActive=${state.scriptActive}, dialogueOpen=${state.dialogueOpen}`
  }

  private readonly waitForResult = async (requestId: number, maxFrames = 3_600): Promise<void> => {
    const address = this.symbols.address("gE2ETestResult")
    let result = parseResult(await this.client.readBytes(address, this.abi.resultSize))

    for (let elapsed = 0; elapsed <= maxFrames; elapsed += 2) {
      await advance(this.client, 2)
      result = parseResult(await this.client.readBytes(address, this.abi.resultSize))
      if (result.status !== successStatus && result.status !== errorStatus) continue
      if (result.requestId !== requestId) {
        throw new Error(`Test ROM returned request ${result.requestId}, expected ${requestId}`)
      }
      if (result.status === errorStatus) {
        const phase = arrangePhases[result.phase] ?? `unknown-${result.phase}`
        const error = arrangeErrors[result.error] ?? `unknown-${result.error}`
        throw new Error(`Test ROM arrangement failed during ${phase}: ${error}`)
      }
      return
    }

    const phase = arrangePhases[result.phase] ?? `unknown-${result.phase}`
    throw new Error(`Test ROM arrangement timed out during ${phase}; ${await this.describe()}`)
  }
}
