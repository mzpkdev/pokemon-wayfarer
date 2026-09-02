import { encodeObserveRegionMapRequest, encodeObserveRegionMapSectionRequest } from "../protocol"
import { type MailboxApi } from "../mailbox"
import { type SessionRuntime } from "../runtime"

export type RegionMapLayout = "johto" | "combined"

export type RegionMapPoint = { x: number; y: number }

export type RegionMapLocation = RegionMapPoint & { width: number; height: number }

export type ActiveRegionMap = {
  mapSection: number
  mapSectionType: number
  cursor: RegionMapPoint
  playerMarker: RegionMapPoint
  layout: RegionMapLayout | "unknown"
}

export type PokedexRegionMapObservation = {
  mapSection: number
  cursor: RegionMapPoint
  playerMarker: RegionMapPoint
  layout: RegionMapLayout | "unknown"
}

export type RegionMapApi = {
  active: () => Promise<ActiveRegionMap>
  entry: (layout: RegionMapLayout, mapSection: number) => Promise<RegionMapLocation>
  loadedLayout: () => Promise<RegionMapLayout | "unknown">
  observePokedex: () => Promise<PokedexRegionMapObservation>
  sectionAt: (point: RegionMapPoint) => Promise<number>
}

const REGION_MAP_JOHTO = 5
const REGION_MAP_COMBINED = 6
const REGION_MAP_ENTRY_SIZE = 8
const REGION_MAP_PALETTE_OFFSET = 7 * 16 * 2
const REGION_MAP_PALETTE_SIZE = 3 * 16 * 2

const layoutName = (type: number): RegionMapLayout | "unknown" => {
  if (type === REGION_MAP_JOHTO) return "johto"
  if (type === REGION_MAP_COMBINED) return "combined"
  return "unknown"
}

const equalBytes = (left: Uint8Array, right: Uint8Array): boolean =>
  left.length === right.length && left.every((value, index) => value === right[index])

export const createRegionMapApi = (runtime: SessionRuntime, mailbox: MailboxApi): RegionMapApi => {
  const active = async (): Promise<ActiveRegionMap> => {
    const pointer = await runtime.readUint32(runtime.address("sRegionMap"))
    const isEwram = pointer >= 0x0200_0000 && pointer < 0x0204_0000
    const isIwram = pointer >= 0x0300_0000 && pointer < 0x0300_8000
    if (!isEwram && !isIwram)
      throw new Error(`No active region map (sRegionMap=${pointer.toString(16)})`)

    const bytes = await runtime.readBytes(pointer, 0x78)
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength)
    return {
      mapSection: view.getUint16(0, true),
      mapSectionType: view.getUint8(2),
      cursor: { x: view.getUint16(0x54, true), y: view.getUint16(0x56, true) },
      playerMarker: { x: view.getUint16(0x74, true), y: view.getUint16(0x76, true) },
      layout: await api.loadedLayout(),
    }
  }

  const api: RegionMapApi = {
    active,
    entry: async (layout, mapSection) => {
      const table = runtime.address(
        layout === "johto" ? "gRegionMapEntries" : "sRegionMapEntries_JK",
      )
      const bytes = await runtime.readBytes(table + mapSection * REGION_MAP_ENTRY_SIZE, 4)
      return { x: bytes[0]!, y: bytes[1]!, width: bytes[2]!, height: bytes[3]! }
    },
    loadedLayout: async () => {
      const loaded = await runtime.readBytes(
        runtime.address("gPlttBufferUnfaded") + REGION_MAP_PALETTE_OFFSET,
        REGION_MAP_PALETTE_SIZE,
      )
      const [johto, combined] = await Promise.all([
        runtime.readBytes(runtime.address("sRegionMapJohto_Pal"), REGION_MAP_PALETTE_SIZE),
        runtime.readBytes(runtime.address("sRegionMapJK_Pal"), REGION_MAP_PALETTE_SIZE),
      ])
      if (equalBytes(loaded, johto)) return "johto"
      if (equalBytes(loaded, combined)) return "combined"
      return "unknown"
    },
    observePokedex: async () => {
      const result = await mailbox.execute(
        (requestId) => encodeObserveRegionMapRequest(runtime.abi, requestId),
        "observe Pokedex region map",
      )
      const observation = await active()
      return {
        mapSection: result.mapNum,
        cursor: { x: result.x, y: result.y },
        playerMarker: observation.playerMarker,
        layout: layoutName(result.mapGroup),
      }
    },
    sectionAt: async ({ x, y }) => {
      const result = await mailbox.execute(
        (requestId) => encodeObserveRegionMapSectionRequest(runtime.abi, requestId, x, y),
        `observe region-map section at ${x}:${y}`,
      )
      return result.mapNum
    },
  }
  return api
}
