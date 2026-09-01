import { describe, expect, it } from "webanvil/test"

import {
  commands,
  encodeCommandRequest,
  keepCoordinate,
  keepMap,
  parseAbi,
  parseStateSnapshot,
  type CommandRequest,
  type SessionAbi,
} from "./protocol"

const abi: SessionAbi = {
  requestSize: 424,
  resultSize: 16,
  stateSize: 344,
  requestStatusOffset: 87,
  resultStatusOffset: 14,
  flagsOffset: 0x1270,
  varsOffset: 0x1340,
}

const abiBytes = (version = 7): Uint8Array => {
  const bytes = new Uint8Array(16)
  const view = new DataView(bytes.buffer)
  for (const [index, value] of [
    version,
    abi.requestSize,
    abi.resultSize,
    abi.stateSize,
    abi.requestStatusOffset,
    abi.resultStatusOffset,
    abi.flagsOffset,
    abi.varsOffset,
  ].entries())
    view.setUint16(index * 2, value, true)
  return bytes
}

const request = (): CommandRequest => ({
  requestId: 0x12345678,
  command: commands.arrange,
  mapGroup: keepMap,
  mapNum: keepMap,
  x: keepCoordinate,
  y: keepCoordinate,
  rngSeed: 1,
  useRngSeed: true,
  vars: [],
  flags: [],
  checkpoint: 2,
  facing: 2,
  textSpeed: 3,
  party: [{ species: 131, moves: [57, 0, 0, 0], level: 25, egg: false, fainted: true }],
  bagItems: [{ item: 4, quantity: 1 }],
  pcSlots: [{ box: 3, slot: 7, mon: { species: 16, moves: [19, 0, 0, 0], level: 8, egg: false } }],
  wildMon: { species: 155, moves: [0, 0, 0, 0], level: 5, egg: false },
  currentBox: 3,
  hmsOverwrite: true,
})

describe("game-session v7 protocol", () => {
  it("accepts only the exact versioned ABI layout", () => {
    expect(parseAbi(abiBytes())).toEqual(abi)
    expect(() => parseAbi(abiBytes(6))).toThrow("Unsupported test ROM ABI")
  })

  it("encodes party-only fainted state, generic items, bounded PC slots, and a wild fixture", () => {
    const bytes = encodeCommandRequest(abi, request())
    const view = new DataView(bytes.buffer)

    expect(view.getUint32(0, true)).toBe(0x12345678)
    expect(bytes[86]).toBe(commands.arrange)
    expect(view.getUint16(88, true)).toBe(131)
    expect(view.getUint16(90, true)).toBe(57)
    expect(bytes[98]).toBe(25)
    expect(bytes[104]).toBe(1)
    expect(view.getUint16(208, true)).toBe(4)
    expect(view.getUint16(210, true)).toBe(1)
    expect(view.getUint16(240, true)).toBe(16)
    expect(bytes[256]).toBe(3)
    expect(bytes[257]).toBe(7)
    expect(view.getUint16(400, true)).toBe(155)
    expect(bytes[410]).toBe(5)
    expect(Array.from(bytes.slice(240, 256))).not.toContain(1)
    expect(Array.from(bytes.slice(260, 400))).toEqual(Array(140).fill(0))
    expect(Array.from(bytes.slice(416, 424))).toEqual([1, 1, 1, 3, 1, 0, 0, 0])
  })

  it("preserves invalid fixture values for ROM-side negative validation", () => {
    const invalid = request()
    invalid.bagItems[0]!.quantity = 0
    invalid.pcSlots[0]!.box = 14
    invalid.pcSlots[0]!.slot = 30
    invalid.party[0]!.species = 0xffff
    const bytes = encodeCommandRequest(abi, invalid)
    const view = new DataView(bytes.buffer)

    expect(view.getUint16(88, true)).toBe(0xffff)
    expect(view.getUint16(210, true)).toBe(0)
    expect(bytes[256]).toBe(14)
    expect(bytes[257]).toBe(30)
  })

  it("decodes only the requested PC slots from semantic state", () => {
    const bytes = new Uint8Array(abi.stateSize)
    const view = new DataView(bytes.buffer)
    bytes[332] = 1
    view.setUint16(176, 131, true)
    view.setUint16(178, 57, true)
    bytes[186] = 25
    bytes[188] = 2
    bytes[189] = 9

    expect(parseStateSnapshot(bytes).pcSlots).toEqual([
      {
        box: 2,
        slot: 9,
        species: 131,
        moves: [57, 0, 0, 0],
        level: 25,
        egg: false,
      },
    ])
  })
})
