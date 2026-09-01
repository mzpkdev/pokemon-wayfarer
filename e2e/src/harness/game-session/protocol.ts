const abiVersion = 6
const expectedRequestSize = 196
const expectedResultSize = 16
const expectedStateSize = 160
const expectedRequestStatusOffset = 87
const expectedResultStatusOffset = 14

export const maxPatches = 8
export const maxParty = 6
export const maxMoves = 4
export const maxBagItems = 8
export const maxPartyMenuActions = 8
export const maxFieldMessageTextLength = 32
export const keepMap = 0xffff
export const keepCoordinate = -0x8000
export const varsStart = 0x4000

export const arrangeStatuses = {
  pending: 1,
  success: 3,
  error: 4,
} as const

export const arrangeCommand = 1

export const gamePhases = ["boot", "overworld", "dialogue", "battle"] as const
export const arrangePhases = [
  "none",
  "validate",
  "new-game",
  "state",
  "warp",
  "field-ready",
] as const
export const arrangeErrors = [
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
  "party-count",
  "party",
  "move",
  "bag-item-count",
  "bag-item",
] as const

export const dialogueMessages = [
  "none",
  "unknown",
  "field-move-used",
  "field-move-needs-hm",
  "field-move-no-eligible-mon",
  "want-to-use-surf",
  "player-used-surf",
] as const

export const uiModes = ["overworld", "pause-menu", "dialogue", "party-menu", "summary"] as const

export type SessionAbi = {
  requestSize: number
  resultSize: number
  stateSize: number
  requestStatusOffset: number
  resultStatusOffset: number
  flagsOffset: number
  varsOffset: number
}

export type ArrangeResult = {
  requestId: number
  error: number
  status: number
  phase: number
}

export type ArrangeRequest = {
  requestId: number
  mapGroup: number
  mapNum: number
  x: number
  y: number
  rngSeed: number
  vars: { id: number; value: number }[]
  flags: { id: number; value: boolean }[]
  checkpoint: number
  facing: number
  textSpeed: number
  party: { species: number; moves: number[]; egg: boolean; fainted: boolean }[]
  bagItems: { item: number; quantity: number }[]
  hmsOverwrite: boolean
}

export type StateSnapshot = {
  frame: number
  mapGroup: number
  mapNum: number
  x: number
  y: number
  phase: number
  ready: boolean
  controlsLocked: boolean
  scriptActive: boolean
  dialogueOpen: boolean
  facing: number
  avatarFlags: number
  avatarSurfing: boolean
  surfBlobCount: number
  surfEffectActive: boolean
  fieldMoveMove: number
  fieldMoveUser: number
  fieldMoveResult: number
  fieldMoveUserSpecies: number
  fieldMoveUnlocked: boolean
  partyCount: number
  hmsOverwrite: boolean
  uiMode: number
  partyMenuActions: number[]
  dialogueMessage: number
  dialogueSequence: number
  dialogueText: number[]
  partySpecies: number[]
  partyMoves: number[][]
  partyEggMask: number
  partyFaintedMask: number
  bagItemCounts: number[]
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

export const parseAbi = (bytes: Uint8Array): SessionAbi => {
  const version = uint16(bytes, 0)
  const abi: SessionAbi = {
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

export const encodeArrangeRequest = (abi: SessionAbi, request: ArrangeRequest): Uint8Array => {
  const bytes = new Uint8Array(abi.requestSize)
  const view = new DataView(bytes.buffer)

  view.setUint32(0, request.requestId, true)
  view.setUint16(4, request.mapGroup, true)
  view.setUint16(6, request.mapNum, true)
  view.setInt16(8, request.x, true)
  view.setInt16(10, request.y, true)
  view.setUint32(12, request.rngSeed, true)
  for (const [index, patch] of request.vars.entries()) {
    view.setUint16(16 + index * 4, patch.id, true)
    view.setUint16(18 + index * 4, patch.value, true)
  }
  for (const [index, patch] of request.flags.entries()) {
    view.setUint16(48 + index * 4, patch.id, true)
    view.setUint8(50 + index * 4, patch.value ? 1 : 0)
  }
  view.setUint8(80, request.checkpoint)
  view.setUint8(81, request.facing)
  view.setUint8(82, request.vars.length)
  view.setUint8(83, request.flags.length)
  view.setUint8(84, request.textSpeed)
  view.setUint8(85, 1)
  view.setUint8(86, arrangeCommand)
  for (const [index, mon] of request.party.entries()) {
    const offset = 88 + index * 12
    view.setUint16(offset, mon.species, true)
    for (const [moveIndex, move] of mon.moves.entries()) {
      view.setUint16(offset + 2 + moveIndex * 2, move, true)
    }
    view.setUint8(offset + 10, mon.egg ? 1 : 0)
    view.setUint8(offset + 11, mon.fainted ? 1 : 0)
  }
  for (const [index, item] of request.bagItems.entries()) {
    const offset = 160 + index * 4
    view.setUint16(offset, item.item, true)
    view.setUint16(offset + 2, item.quantity, true)
  }
  view.setUint8(192, request.party.length)
  view.setUint8(193, request.bagItems.length)
  view.setUint8(194, request.hmsOverwrite ? 1 : 0)

  return bytes
}

export const parseArrangeResult = (bytes: Uint8Array): ArrangeResult => ({
  requestId: uint32(bytes, 0),
  error: uint16(bytes, 12),
  status: bytes[14]!,
  phase: bytes[15]!,
})

export const parseStateSnapshot = (bytes: Uint8Array): StateSnapshot => {
  const partySpecies = Array.from({ length: maxParty }, (_, index) => uint16(bytes, 18 + index * 2))
  const partyMoves = Array.from({ length: maxParty }, (_, partyIndex) =>
    Array.from({ length: maxMoves }, (_, moveIndex) =>
      uint16(bytes, 30 + (partyIndex * maxMoves + moveIndex) * 2),
    ),
  )

  return {
    frame: uint32(bytes, 0),
    mapGroup: uint16(bytes, 4),
    mapNum: uint16(bytes, 6),
    x: int16(bytes, 8),
    y: int16(bytes, 10),
    avatarFlags: uint16(bytes, 12),
    fieldMoveMove: uint16(bytes, 14),
    fieldMoveUserSpecies: uint16(bytes, 16),
    fieldMoveUnlocked: bytes[156] === 1,
    partySpecies,
    partyMoves,
    bagItemCounts: Array.from({ length: maxBagItems }, (_, index) => uint16(bytes, 78 + index * 2)),
    phase: bytes[94]!,
    ready: bytes[95] === 1,
    controlsLocked: bytes[96] === 1,
    scriptActive: bytes[97] === 1,
    dialogueOpen: bytes[98] === 1,
    facing: bytes[99]!,
    avatarSurfing: bytes[100] !== 0,
    surfBlobCount: bytes[101]!,
    surfEffectActive: bytes[102] === 1,
    fieldMoveUser: bytes[103]!,
    fieldMoveResult: bytes[104]!,
    partyCount: bytes[105]!,
    hmsOverwrite: bytes[106] === 1,
    uiMode: bytes[107]!,
    partyMenuActions: Array.from(bytes.slice(112, 112 + maxPartyMenuActions)).filter(
      (action) => action !== 0xff,
    ),
    dialogueMessage: bytes[109]!,
    partyEggMask: bytes[110]!,
    partyFaintedMask: bytes[111]!,
    dialogueSequence: uint32(bytes, 120),
    dialogueText: Array.from(bytes.slice(124, 124 + maxFieldMessageTextLength)),
  }
}
