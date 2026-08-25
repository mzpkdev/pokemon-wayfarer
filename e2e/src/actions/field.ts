import { press, advance } from "./input"
import {
  readMapLocation,
  readPlayerElevation,
  readPlayerPosition,
  type MapLocation,
  type PlayerPosition,
} from "./map"
import { type SkyEmuButton, type SkyEmuClient } from "../harness/skyemu"
import { type SkyEmuSymbols } from "../harness/symbols"

export type FieldDirection = Extract<SkyEmuButton, "Down" | "Left" | "Right" | "Up">

const taskSize = 0x28
const tasksCount = 16

export const isPlayerMovementIdle = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
): Promise<boolean> => (await client.readBytes(symbols.address("gPlayerAvatar") + 3, 1))[0] === 0

export const areFieldControlsUnlocked = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
): Promise<boolean> => {
  const [locked, scriptStatus] = await Promise.all([
    client.readBytes(symbols.address("sLockFieldControls"), 1),
    client.readBytes(symbols.address("sGlobalScriptContextStatus"), 1),
  ])

  return locked[0] === 0 && scriptStatus[0] === 2
}

export const isCallback = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  callback: string,
): Promise<boolean> =>
  (await client.readUint32LE(symbols.address("gMain") + 4)) === (symbols.address(callback) | 1)

export const isTaskActive = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  task: string,
): Promise<boolean> => {
  const expected = symbols.address(task) | 1
  const tasks = symbols.address("gTasks")

  for (let id = 0; id < tasksCount; id++) {
    const address = tasks + id * taskSize
    const [functionAddress, active] = await Promise.all([
      client.readUint32LE(address),
      client.readBytes(address + 4, 1),
    ])
    if (active[0] !== 0 && functionAddress === expected) return true
  }

  return false
}

const describeField = async (client: SkyEmuClient, symbols: SkyEmuSymbols): Promise<string> => {
  const [map, position, elevation, locked, scriptStatus, callback] = await Promise.all([
    readMapLocation(client, symbols),
    readPlayerPosition(client, symbols),
    readPlayerElevation(client, symbols),
    client.readBytes(symbols.address("sLockFieldControls"), 1),
    client.readBytes(symbols.address("sGlobalScriptContextStatus"), 1),
    client.readUint32LE(symbols.address("gMain") + 4),
  ])
  return `map=${map.mapGroup}:${map.mapNum}, position=${position.x}:${position.y}, elevation=${elevation}, lock=${locked[0]}, script=${scriptStatus[0]}, callback=0x${callback.toString(16)}`
}

export const waitFor = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  predicate: () => Promise<boolean>,
  description: string,
  maxFrames = 1_200,
  stepFrames = 2,
): Promise<void> => {
  for (let elapsed = 0; elapsed <= maxFrames; elapsed += stepFrames) {
    if (await predicate()) return
    await advance(client, Math.min(stepFrames, maxFrames - elapsed + 1))
  }

  throw new Error(
    `${description} not reached in ${maxFrames} frames; ${await describeField(client, symbols)}`,
  )
}

export const waitForCallback = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  callback: string,
  maxFrames = 1_200,
): Promise<void> =>
  waitFor(
    client,
    symbols,
    () => isCallback(client, symbols, callback),
    `callback ${callback}`,
    maxFrames,
  )

export const waitForControlsUnlocked = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  maxFrames = 1_200,
): Promise<void> =>
  waitFor(
    client,
    symbols,
    () => areFieldControlsUnlocked(client, symbols),
    "unlocked field controls",
    maxFrames,
  )

export const waitForControlsLocked = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  maxFrames = 1_200,
): Promise<void> =>
  waitFor(
    client,
    symbols,
    async () => !(await areFieldControlsUnlocked(client, symbols)),
    "locked field controls",
    maxFrames,
  )

export const waitForMap = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  expected: MapLocation,
  maxFrames = 1_200,
): Promise<void> =>
  waitFor(
    client,
    symbols,
    async () => {
      const actual = await readMapLocation(client, symbols)
      return actual.mapGroup === expected.mapGroup && actual.mapNum === expected.mapNum
    },
    `map ${expected.mapGroup}:${expected.mapNum}`,
    maxFrames,
    4,
  )

const waitForPlayerIdle = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  position: PlayerPosition,
): Promise<boolean> => {
  if (!(await isPlayerMovementIdle(client, symbols))) return false
  const current = await readPlayerPosition(client, symbols)
  return current.x === position.x && current.y === position.y
}

export const moveTo = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  target: Partial<PlayerPosition>,
  maxPulses = 500,
): Promise<void> => {
  if (target.x === undefined && target.y === undefined)
    throw new Error("moveTo needs an x or y coordinate")
  await waitForControlsUnlocked(client, symbols)

  for (let pulse = 0; pulse < maxPulses; pulse++) {
    const current = await readPlayerPosition(client, symbols)
    const atX = target.x === undefined || current.x === target.x
    const atY = target.y === undefined || current.y === target.y
    if (atX && atY) {
      if (await waitForPlayerIdle(client, symbols, current)) return
      await advance(client, 1)
      continue
    }

    const direction: FieldDirection =
      target.x !== undefined && current.x !== target.x
        ? current.x < target.x
          ? "Right"
          : "Left"
        : current.y < (target.y as number)
          ? "Down"
          : "Up"
    await press(client, direction, 3, 1)
  }

  throw new Error(
    `position ${target.x ?? "*"}:${target.y ?? "*"} not reached after ${maxPulses} movement pulses; ${await describeField(client, symbols)}`,
  )
}

export const moveUntil = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  direction: FieldDirection,
  predicate: () => Promise<boolean>,
  description: string,
  maxPulses = 300,
): Promise<void> => {
  for (let pulse = 0; pulse <= maxPulses; pulse++) {
    if (await predicate()) return
    await press(client, direction, 3, 1)
  }

  throw new Error(
    `${description} not reached after ${maxPulses} ${direction} movement pulses; ${await describeField(client, symbols)}`,
  )
}

export const pressUntil = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  button: SkyEmuButton,
  predicate: () => Promise<boolean>,
  description: string,
  maxPulses = 120,
): Promise<void> => {
  for (let pulse = 0; pulse <= maxPulses; pulse++) {
    if (await predicate()) return
    await press(client, button, 1, 60)
  }

  throw new Error(
    `${description} not reached after ${maxPulses} ${button} pulses; ${await describeField(client, symbols)}`,
  )
}
