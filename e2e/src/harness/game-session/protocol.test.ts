import { describe, expect, it } from "webanvil/test"

import {
  commands,
  encodeCommandRequest,
  encodeObserveRegionMapRequest,
  encodeObserveRegionMapSectionRequest,
  encodeObserveVarRequest,
  encodeSetVarRequest,
  encodeSaveRequest,
  encodeWinBattleRequest,
  keepCoordinate,
  keepMap,
  parseAbi,
  parseCommandResult,
  parseStateSnapshot,
  type CommandRequest,
  type SessionAbi,
} from "./protocol"
import { decodeFieldMessageText } from "./features/state"

const abi: SessionAbi = {
  requestSize: 372,
  resultSize: 16,
  stateSize: 1680,
  requestStatusOffset: 87,
  resultStatusOffset: 14,
  flagsOffset: 0x1270,
  varsOffset: 0x1340,
}

const abiBytes = (version = 22): Uint8Array => {
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
  fullPocketMask: 7,
  regionalBadgeCounts: [4, 3, 1],
  leagueClears: [true, false, false],
})

const expectNoFixtureMutations = (bytes: Uint8Array) => {
  expect(Array.from(bytes.slice(88, 356))).toEqual(Array(268).fill(0))
  expect(Array.from(bytes.slice(356))).toEqual(Array(16).fill(0))
}

describe("game-session v22 protocol", () => {
  it("decodes canonical FRLG dialogue glyphs and page breaks for exact assertions", () => {
    expect(
      decodeFieldMessageText([0xca, 0xc9, 0xc5, 0x1b, 0xc7, 0xc9, 0xc8, 0xfe, 0xb4, 0xfb, 0xff]),
    ).toBe("POKéMON\n’\n")
  })

  it("encodes explicit appearance IDs separately from checkpoint defaults", () => {
    expect(encodeCommandRequest(abi, request())[369]).toBe(0)
    for (const id of [1, 2, 5, 6])
      expect(encodeCommandRequest(abi, { ...request(), appearanceId: id })[369]).toBe(id)
  })

  it("encodes the decorated Secret Base fixture in the reserved request tail", () => {
    expect(encodeCommandRequest(abi, request())[370]).toBe(0)
    expect(encodeCommandRequest(abi, { ...request(), decoratedSecretBase: true })[370]).toBe(1)
  })

  it("accepts only the exact versioned ABI layout", () => {
    expect(parseAbi(abiBytes())).toEqual(abi)
    expect(() => parseAbi(abiBytes(16))).toThrow("Unsupported test ROM ABI")
  })

  it("encodes party-only fainted state, generic items, bounded PC slots, and a wild fixture", () => {
    const bytes = encodeCommandRequest(abi, request())
    const view = new DataView(bytes.buffer)

    expect(view.getUint32(0, true)).toBe(0x12345678)
    expect(bytes[86]).toBe(commands.arrange)
    expect(view.getUint16(88, true)).toBe(131)
    expect(view.getUint16(90, true)).toBe(57)
    expect(bytes[98]).toBe(25)
    expect(bytes[100]).toBe(1)
    expect(view.getUint16(184, true)).toBe(4)
    expect(view.getUint16(186, true)).toBe(1)
    expect(view.getUint16(216, true)).toBe(16)
    expect(bytes[228]).toBe(3)
    expect(bytes[229]).toBe(7)
    expect(view.getUint16(344, true)).toBe(155)
    expect(bytes[354]).toBe(5)
    expect(Array.from(bytes.slice(216, 228))).not.toContain(1)
    expect(Array.from(bytes.slice(232, 344))).toEqual(Array(112).fill(0))
    expect(Array.from(bytes.slice(356, 368))).toEqual([1, 1, 1, 3, 1, 7, 4, 3, 1, 1, 0, 0])
    expect(bytes[369]).toBe(0)
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
    expect(view.getUint16(186, true)).toBe(0)
    expect(bytes[228]).toBe(14)
    expect(bytes[229]).toBe(30)
  })

  it("encodes the flash-save command without fixture mutations", () => {
    const bytes = encodeSaveRequest(abi, 17)

    expect(new DataView(bytes.buffer).getUint32(0, true)).toBe(17)
    expect(bytes[86]).toBe(commands.save)
    expectNoFixtureMutations(bytes)
  })

  it("encodes the read-only region-map observation command", () => {
    const bytes = encodeObserveRegionMapRequest(abi, 23)

    expect(new DataView(bytes.buffer).getUint32(0, true)).toBe(23)
    expect(bytes[86]).toBe(commands.observeRegionMap)
    expectNoFixtureMutations(bytes)
  })

  it("encodes a region-map grid lookup without changing the ABI layout", () => {
    const bytes = encodeObserveRegionMapSectionRequest(abi, 24, 25, 7)
    const view = new DataView(bytes.buffer)

    expect(view.getUint32(0, true)).toBe(24)
    expect(view.getInt16(8, true)).toBe(25)
    expect(view.getInt16(10, true)).toBe(7)
    expect(bytes[86]).toBe(commands.observeRegionMapSection)
    expect(bytes).toHaveLength(abi.requestSize)
  })

  it("encodes a read-only banked-variable observation without changing the ABI layout", () => {
    const bytes = encodeObserveVarRequest(abi, 26, 0x7023)
    const view = new DataView(bytes.buffer)

    expect(view.getUint32(0, true)).toBe(26)
    expect(view.getUint16(4, true)).toBe(0x7023)
    expect(bytes[86]).toBe(commands.observeVar)
    expectNoFixtureMutations(bytes)
  })

  it("encodes a bounded banked-variable write without changing the ABI layout", () => {
    const bytes = encodeSetVarRequest(abi, 27, 0x7023, 0xffff)
    const view = new DataView(bytes.buffer)

    expect(view.getUint32(0, true)).toBe(27)
    expect(view.getUint16(4, true)).toBe(0x7023)
    expect(view.getUint16(6, true)).toBe(0xffff)
    expect(bytes[86]).toBe(commands.setVar)
    expectNoFixtureMutations(bytes)
  })

  it("encodes the test-only battle-win command without fixture mutations", () => {
    const bytes = encodeWinBattleRequest(abi, 25)

    expect(new DataView(bytes.buffer).getUint32(0, true)).toBe(25)
    expect(bytes[86]).toBe(commands.winBattle)
    expectNoFixtureMutations(bytes)
  })

  it("decodes region-map observation values from the command result", () => {
    const bytes = new Uint8Array(abi.resultSize)
    const view = new DataView(bytes.buffer)
    view.setUint32(0, 23, true)
    view.setUint16(4, 6, true)
    view.setUint16(6, 25, true)
    view.setInt16(8, 15, true)
    view.setInt16(10, 11, true)
    bytes[14] = 3
    bytes[15] = 3

    expect(parseCommandResult(bytes)).toEqual({
      requestId: 23,
      mapGroup: 6,
      mapNum: 25,
      x: 15,
      y: 11,
      error: 0,
      status: 3,
      phase: 3,
    })
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

  it("decodes full dialogue, active map objects, and presentation state from the appended tail", () => {
    const bytes = new Uint8Array(abi.stateSize).fill(0xff)
    const view = new DataView(bytes.buffer)
    bytes.set([0xbb, 0xd5, 0xfe, 0xbc, 0xff], 452)
    bytes[964] = 1
    bytes[965] = 3
    bytes[966] = 2
    bytes[967] = 3
    view.setUint16(968, 1, true)
    view.setUint16(970, 7, true)
    view.setInt16(972, 12, true)
    view.setInt16(974, -3, true)
    bytes.set([5, 3, 4, 8, 2, 1, 1, 0], 976)

    const snapshot = parseStateSnapshot(bytes)
    expect(snapshot.fullDialogueText.slice(0, 6)).toEqual([0xbb, 0xd5, 0xfe, 0xbc, 0xff, 0xff])
    expect(snapshot).toMatchObject({
      choiceKind: 3,
      choiceCursor: 2,
      choiceOptionCount: 3,
      choiceResult: 1,
      displayedMonSpecies: 7,
      objectEvents: [
        {
          localId: 5,
          mapGroup: 3,
          mapNum: 4,
          // The wire values are already normalized map coordinates (MAP_OFFSET
          // was removed ROM-side), so parsing must preserve them verbatim.
          x: 12,
          y: -3,
          movementType: 8,
          movementDirection: 2,
          facingDirection: 1,
          visible: true,
          moving: false,
        },
      ],
    })
  })

  it("decodes battle message sequence and exact expanded text independently of field dialogue", () => {
    const bytes = new Uint8Array(abi.stateSize).fill(0xff)
    const view = new DataView(bytes.buffer)
    view.setUint32(120, 4, true)
    bytes.set([0xbb, 0xbc, 0xff], 452)
    view.setUint32(1164, 9, true)
    bytes.set([0xbb, 0xfe, 0xd5, 0xab, 0xff], 1168)

    const snapshot = parseStateSnapshot(bytes)
    expect(snapshot.dialogueSequence).toBe(4)
    expect(snapshot.battleDialogueSequence).toBe(9)
    expect(decodeFieldMessageText(snapshot.fullDialogueText)).toBe("AB")
    expect(decodeFieldMessageText(snapshot.battleDialogueText)).toBe("A\na!")
  })

  it("decodes the independent Pallet opening state from the snapshot tail", () => {
    const bytes = new Uint8Array(abi.stateSize)
    const view = new DataView(bytes.buffer)
    bytes[448] = 5
    bytes[449] = 2
    view.setUint16(450, 0x153, true)

    expect(parseStateSnapshot(bytes)).toMatchObject({
      palletOpeningPhase: 5,
      palletStarterSlot: 2,
      palletOpeningReceipts: 0x153,
    })
  })

  it("decodes semantic League Circuit state", () => {
    const bytes = new Uint8Array(abi.stateSize)
    bytes.set([4, 3, 1], 340)
    bytes.set([1, 0, 0], 343)
    bytes.set([2, 1, 0], 346)
    bytes[349] = 8
    bytes[350] = 55
    bytes[351] = 4
    bytes[352] = 1
    bytes[353] = 2
    bytes[354] = 40

    expect(parseStateSnapshot(bytes)).toMatchObject({
      regionalBadgeCounts: [4, 3, 1],
      leagueClears: [true, false, false],
      leagueStatuses: [2, 1, 0],
      globalBadgeCount: 8,
      trainerRating: 55,
      trainerCardState: 4,
      leagueRunActive: true,
      leagueRunRegion: 2,
      leagueRunRating: 40,
    })
  })

  it("keeps saved appearance separate from the pending candidate and confirmation", () => {
    const bytes = new Uint8Array(abi.stateSize)
    bytes.set([5, 6, 2, 1], 382)
    expect(parseStateSnapshot(bytes)).toMatchObject({
      playerAppearanceId: 5,
      appearanceCandidate: 6,
      appearanceConfirmed: 2,
      appearanceIntroStage: 1,
    })
  })

  it("decodes origin identity independently from location, choices, and recovery", () => {
    const bytes = new Uint8Array(abi.stateSize)
    const view = new DataView(bytes.buffer)
    bytes[355] = 2
    view.setUint16(356, 2, true)
    view.setUint16(358, 1, true)
    view.setUint16(360, 2, true)
    view.setUint16(362, 0, true)
    bytes.set([2, 6, 1, 1, 0, 1, 32, 3], 364)
    view.setInt16(374, 7, true)
    view.setInt16(376, 4, true)
    bytes[378] = 1
    bytes[379] = 3
    view.setUint16(380, 2, true)
    bytes.set([5, 6, 2, 1], 382)
    bytes.set([1, 5, 9, 7, 1, 10, 2, 3, 4, 5, 1, 0], 386)
    view.setUint32(400, 2992, true)
    view.setUint16(404, 0, true)
    view.setUint32(416, 8, true)
    expect(parseStateSnapshot(bytes)).toMatchObject({
      money: 2992,
      partyHp: [0, 0, 0, 0, 0, 0],
      partyStatus: [8, 0, 0, 0, 0, 0],
      trainerOnly: {
        active: true,
        initialCatchFactor: 5,
        catchFactor: 9,
        escapeFactor: 7,
        approach: 1,
        anger: 10,
        foodTurns: 2,
        rocks: 3,
        completedTurns: 4,
        runAttempts: 5,
        warned: true,
        outcome: 0,
      },
      playerAppearanceId: 5,
      appearanceCandidate: 6,
      appearanceConfirmed: 2,
      appearanceIntroStage: 1,
      originIntroStage: 2,
      startingOriginId: 2,
      johtoStarterChoice: 1,
      hoennStarterChoice: 2,
      maidenVoyageState: 0,
      originCurrentRegion: 2,
      originVisitedRegions: 6,
      originHoennInitialized: true,
      johtoStarterCommitted: true,
      johtoStarterReceived: false,
      hoennStarterReceived: true,
      lastHealMapGroup: 32,
      lastHealMapNum: 3,
      lastHealX: 7,
      lastHealY: 4,
      playerGender: 1,
      originEquipment: 3,
      littlerootTownState: 2,
    })
  })
})
