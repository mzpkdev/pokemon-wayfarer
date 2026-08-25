import {
  checkpoints,
  directions,
  maps,
  storyFlags,
  storyVars,
  textSpeeds,
  type Checkpoint,
  type Direction,
  type GameMap,
  type StoryFlag,
  type StoryVar,
  type TextSpeed,
} from "../catalog"
import {
  arrangeErrors,
  arrangePhases,
  arrangeStatuses,
  encodeArrangeRequest,
  keepCoordinate,
  keepMap,
  maxPatches,
  parseArrangeResult,
} from "../protocol"
import { type SessionRuntime } from "../runtime"
import { describeState, type StateApi } from "./state"

export type PlayerPosition = {
  map?: GameMap
  x: number
  y: number
}

export type ArrangeGame = {
  checkpoint: Checkpoint
  player?: {
    facing?: Direction
    position?: PlayerPosition
  }
  story?: {
    flags?: Partial<Record<StoryFlag, boolean>>
    vars?: Partial<Record<StoryVar, number>>
  }
  determinism?: {
    rngSeed?: number
    textSpeed?: TextSpeed
  }
}

export type ArrangeApi = {
  arrange: (options: ArrangeGame) => Promise<void>
  checkpoint: {
    load: (checkpoint: Checkpoint, overrides?: Omit<ArrangeGame, "checkpoint">) => Promise<void>
  }
}

const entries = <Name extends string, Value>(
  value: Partial<Record<Name, Value>> | undefined,
): [Name, Value][] => Object.entries(value ?? {}) as [Name, Value][]

export const createArrangeApi = (runtime: SessionRuntime, state: StateApi): ArrangeApi => {
  let nextRequestId = 0

  const waitForResult = async (requestId: number, maxFrames = 3_600): Promise<void> => {
    const address = runtime.address("gE2ETestResult")
    let result = parseArrangeResult(await runtime.readBytes(address, runtime.abi.resultSize))

    for (let elapsed = 0; elapsed <= maxFrames; elapsed += 2) {
      await runtime.advance(2)
      result = parseArrangeResult(await runtime.readBytes(address, runtime.abi.resultSize))
      if (result.status !== arrangeStatuses.success && result.status !== arrangeStatuses.error)
        continue
      if (result.requestId !== requestId) {
        throw new Error(`Test ROM returned request ${result.requestId}, expected ${requestId}`)
      }
      if (result.status === arrangeStatuses.error) {
        const phase = arrangePhases[result.phase] ?? `unknown-${result.phase}`
        const error = arrangeErrors[result.error] ?? `unknown-${result.error}`
        throw new Error(`Test ROM arrangement failed during ${phase}: ${error}`)
      }
      return
    }

    const phase = arrangePhases[result.phase] ?? `unknown-${result.phase}`
    throw new Error(
      `Test ROM arrangement timed out during ${phase}; ${describeState(await state.read())}`,
    )
  }

  const arrange = async (options: ArrangeGame): Promise<void> => {
    const vars = entries(options.story?.vars)
    const flags = entries(options.story?.flags)
    if (vars.length > maxPatches) {
      throw new Error(`Test ROM supports at most ${maxPatches} var overrides`)
    }
    if (flags.length > maxPatches) {
      throw new Error(`Test ROM supports at most ${maxPatches} flag overrides`)
    }

    const position = options.player?.position
    const map = position?.map ? maps[position.map] : undefined
    const requestId = ++nextRequestId
    const bytes = encodeArrangeRequest(runtime.abi, {
      requestId,
      mapGroup: map?.mapGroup ?? keepMap,
      mapNum: map?.mapNum ?? keepMap,
      x: position?.x ?? keepCoordinate,
      y: position?.y ?? keepCoordinate,
      rngSeed: options.determinism?.rngSeed ?? 1,
      vars: vars.map(([name, value]) => ({ id: storyVars[name], value })),
      flags: flags.map(([name, value]) => ({ id: storyFlags[name], value })),
      checkpoint: checkpoints[options.checkpoint],
      facing: directions[options.player?.facing ?? "up"],
      textSpeed: textSpeeds[options.determinism?.textSpeed ?? "instant"],
    })

    const requestAddress = runtime.address("gE2ETestRequest")
    await runtime.writeBytes(requestAddress, bytes)
    await runtime.writeBytes(
      requestAddress + runtime.abi.requestStatusOffset,
      Uint8Array.of(arrangeStatuses.pending),
    )
    await waitForResult(requestId)
  }

  return {
    arrange,
    checkpoint: {
      load: (checkpoint, overrides = {}) => arrange({ checkpoint, ...overrides }),
    },
  }
}
