import { type SkyEmuClient } from "../harness/skyemu"

const saveBlock1PointerAddress = 0x03002ef4
const saveBlock1LocationOffset = 0x8

export const newBarkPlayersHouse2F = { mapGroup: 1, mapNum: 4 }

export type MapLocation = {
  mapGroup: number
  mapNum: number
}

export const readMapLocation = async (client: SkyEmuClient): Promise<MapLocation> => {
  const saveBlock1Address = await client.readUint32LE(saveBlock1PointerAddress)
  const location = await client.readBytes(saveBlock1Address + saveBlock1LocationOffset, 2)

  return { mapGroup: location[0]!, mapNum: location[1]! }
}
