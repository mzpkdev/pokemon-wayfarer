import { type SkyEmuClient } from "../harness/skyemu"

const saveBlock1PointerAddress = 0x03002ef4
const saveBlock1LocationOffset = 0x8
const objectEventsAddress = 0x020009a0
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

export const readMapLocation = async (client: SkyEmuClient): Promise<MapLocation> => {
  const saveBlock1Address = await client.readUint32LE(saveBlock1PointerAddress)
  const location = await client.readBytes(saveBlock1Address + saveBlock1LocationOffset, 2)

  return { mapGroup: location[0]!, mapNum: location[1]! }
}

const readPlayerObjectEvent = async (client: SkyEmuClient): Promise<Uint8Array> => {
  const objectEvents = await client.readBytes(objectEventsAddress, objectEventSize * objectEventsCount)

  for (let index = 0; index < objectEventsCount; index++) {
    const offset = index * objectEventSize
    if ((objectEvents[offset + objectEventPlayerFlagOffset]! & 1) === 0) continue

    return objectEvents.slice(offset, offset + objectEventSize)
  }

  throw new Error("SkyEmu did not expose the player object event")
}

export const readPlayerPosition = async (client: SkyEmuClient): Promise<PlayerPosition> => {
  const objectEvent = await readPlayerObjectEvent(client)
  const position = objectEvent.slice(objectEventPositionOffset, objectEventPositionOffset + 4)

  return {
    x: position[0]! | (position[1]! << 8),
    y: position[2]! | (position[3]! << 8),
  }
}

export const readPlayerElevation = async (client: SkyEmuClient): Promise<number> => {
  const objectEvent = await readPlayerObjectEvent(client)

  return objectEvent[objectEventElevationOffset]! & 0x0f
}
