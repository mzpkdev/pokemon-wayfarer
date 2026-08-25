import { type SkyEmuClient } from "../harness/skyemu"
import { type SkyEmuSymbols } from "../harness/symbols"

const saveBlock1LocationOffset = 0x8
const saveBlock1VarsOffset = 0x1abc
const varsStart = 0x4000
const varsEnd = 0x40ff
const objectEventSize = 0x24
const objectEventPlayerFlagOffset = 0x2
const objectEventElevationOffset = 0xb
const objectEventPositionOffset = 0x10
const objectEventsCount = 16

export const newBarkPlayersHouse2F = { mapGroup: 1, mapNum: 4 }

export type MapLocation = {
  mapGroup: number
  mapNum: number
}

export type PlayerPosition = {
  x: number
  y: number
}

export const readMapLocation = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
): Promise<MapLocation> => {
  const saveBlock1Address = await client.readUint32LE(symbols.address("gSaveBlock1Ptr"))
  const location = await client.readBytes(saveBlock1Address + saveBlock1LocationOffset, 2)

  return { mapGroup: location[0]!, mapNum: location[1]! }
}

const readPlayerObjectEvent = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
): Promise<Uint8Array> => {
  const objectEvents = await client.readBytes(
    symbols.address("gObjectEvents"),
    objectEventSize * objectEventsCount,
  )

  for (let index = 0; index < objectEventsCount; index++) {
    const offset = index * objectEventSize
    if ((objectEvents[offset + objectEventPlayerFlagOffset]! & 1) === 0) continue

    return objectEvents.slice(offset, offset + objectEventSize)
  }

  throw new Error("SkyEmu did not expose the player object event")
}

export const readPlayerPosition = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
): Promise<PlayerPosition> => {
  const objectEvent = await readPlayerObjectEvent(client, symbols)
  const position = objectEvent.slice(objectEventPositionOffset, objectEventPositionOffset + 4)

  return {
    x: position[0]! | (position[1]! << 8),
    y: position[2]! | (position[3]! << 8),
  }
}

export const readPlayerElevation = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
): Promise<number> => {
  const objectEvent = await readPlayerObjectEvent(client, symbols)

  return objectEvent[objectEventElevationOffset]! & 0x0f
}

export const readSavedVar = async (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  id: number,
): Promise<number> => {
  if (!Number.isInteger(id) || id < varsStart || id > varsEnd) {
    throw new Error(
      `SkyEmu saved variable ID must be between 0x${varsStart.toString(16)} and 0x${varsEnd.toString(16)}`,
    )
  }

  const saveBlock1Address = await client.readUint32LE(symbols.address("gSaveBlock1Ptr"))
  return client.readUint16LE(saveBlock1Address + saveBlock1VarsOffset + (id - varsStart) * 2)
}
