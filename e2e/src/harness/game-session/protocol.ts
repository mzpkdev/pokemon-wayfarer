const abiVersion = 8
const expectedRequestSize = 424
const expectedResultSize = 16
const expectedStateSize = 344
const expectedRequestStatusOffset = 87
const expectedResultStatusOffset = 14

export const maxPatches = 8
export const maxParty = 6
export const maxMoves = 4
export const maxBagItems = 8
export const maxPcSlots = 8
export const maxPartyMenuActions = 8
export const maxFieldMessageTextLength = 32
export const totalPcBoxes = 14
export const pcBoxCapacity = 30
export const keepMap = 0xffff
export const keepCoordinate = -0x8000
export const varsStart = 0x4000

export const commandStatuses = { idle: 0, pending: 1, running: 2, success: 3, error: 4 } as const
export const arrangeStatuses = commandStatuses
export const commands = {
  arrange: 1,
  startWildBattle: 2,
  save: 3,
  observeRegionMap: 4,
  observeRegionMapSection: 5,
  winBattle: 6,
} as const
export const fullPocketMasks = { items: 1 << 0, keyItems: 1 << 1, tmHm: 1 << 2 } as const

export const gamePhases = ["boot", "overworld", "dialogue", "battle"] as const
export const arrangePhases = [
  "none",
  "validate",
  "new-game",
  "state",
  "warp",
  "field-ready",
] as const
export const commandErrors = [
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
  "species",
  "level",
  "item-quantity",
  "pc-slot-count",
  "pc-box",
  "pc-slot",
  "busy",
  "full-pocket-mask",
  "save",
] as const
export const arrangeErrors = commandErrors

export const dialogueMessages = [
  "none",
  "unknown",
  "field-move-used",
  "field-move-needs-hm",
  "field-move-no-eligible-mon",
  "want-to-use-surf",
  "player-used-surf",
] as const
export const uiModes = [
  "overworld",
  "pause-menu",
  "dialogue",
  "party-menu",
  "summary",
  "battle",
  "catch-swap",
  "storage",
] as const
export const catchSwapStates = ["none", "prompt", "choose-party", "resolved"] as const
export const storageUiStates = [
  "none",
  "initializing",
  "pc-menu",
  "ready",
  "mon-menu",
  "deposit-box",
  "withdrawing",
  "moving",
  "release-confirm",
  "release-check",
  "released",
  "release-blocked",
  "exit-confirm",
  "busy",
] as const
export const storageModes = ["none", "move", "deposit", "withdraw"] as const
export const battleUiStates = [
  "none",
  "starting",
  "action-menu",
  "bag",
  "bag-context",
  "caught-dex",
  "nickname",
  "catch-swap-prompt",
  "catch-swap-party",
  "other",
  "text",
] as const

export type SessionAbi = {
  requestSize: number
  resultSize: number
  stateSize: number
  requestStatusOffset: number
  resultStatusOffset: number
  flagsOffset: number
  varsOffset: number
}

export type CommandResult = {
  requestId: number
  mapGroup: number
  mapNum: number
  x: number
  y: number
  error: number
  status: number
  phase: number
}
export type ArrangeResult = CommandResult

export type MonFixtureWire = { species: number; moves: number[]; level: number; egg: boolean }
export type PartyMonFixtureWire = MonFixtureWire & { fainted: boolean }
export type PcSlotFixtureWire = { box: number; slot: number; mon: MonFixtureWire }

export type CommandRequest = {
  requestId: number
  command: number
  mapGroup: number
  mapNum: number
  x: number
  y: number
  rngSeed: number
  useRngSeed: boolean
  vars: { id: number; value: number }[]
  flags: { id: number; value: boolean }[]
  checkpoint: number
  facing: number
  textSpeed: number
  party: PartyMonFixtureWire[]
  bagItems: { item: number; quantity: number }[]
  pcSlots: PcSlotFixtureWire[]
  wildMon: MonFixtureWire
  currentBox: number
  hmsOverwrite: boolean
  fullPocketMask: number
}

export type ArrangeRequest = Omit<CommandRequest, "command" | "useRngSeed" | "wildMon"> & {
  wildMon?: MonFixtureWire
}
export type ObservedPcSlot = MonFixtureWire & { box: number; slot: number }

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
  bagItems: { item: number; quantity: number }[]
  pcSlots: ObservedPcSlot[]
  battleEnemySpecies: number
  battleEnemyMoves: number[]
  battleEnemyLevel: number
  battleActive: boolean
  caughtSpecies: number
  lastUsedItem: number
  catchSwapState: number
  catchSwapCursor: number
  catchSwapSelectedParty: number
  catchSwapBox: number
  catchSwapSlot: number
  storageUiState: number
  storageOpen: boolean
  storageReady: boolean
  storageCursorArea: number
  storageCursorPosition: number
  storageMovingMon: boolean
  storageCurrentBox: number
  battleUiState: number
  battleCursor: number
  battleBagPocket: number
  storageMode: number
  battleBagItem: number
}

const emptyMon = (): MonFixtureWire => ({ species: 0, moves: [0, 0, 0, 0], level: 0, egg: false })
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

const encodeMon = (view: DataView, offset: number, mon: MonFixtureWire): void => {
  view.setUint16(offset, mon.species, true)
  for (let index = 0; index < maxMoves; index++)
    view.setUint16(offset + 2 + index * 2, mon.moves[index] ?? 0, true)
  view.setUint8(offset + 10, mon.level)
  view.setUint8(offset + 11, mon.egg ? 1 : 0)
}

export const encodeCommandRequest = (abi: SessionAbi, request: CommandRequest): Uint8Array => {
  const bytes = new Uint8Array(abi.requestSize)
  const view = new DataView(bytes.buffer)
  view.setUint32(0, request.requestId, true)
  view.setUint16(4, request.mapGroup, true)
  view.setUint16(6, request.mapNum, true)
  view.setInt16(8, request.x, true)
  view.setInt16(10, request.y, true)
  view.setUint32(12, request.rngSeed, true)
  for (const [index, patch] of request.vars.slice(0, maxPatches).entries()) {
    view.setUint16(16 + index * 4, patch.id, true)
    view.setUint16(18 + index * 4, patch.value, true)
  }
  for (const [index, patch] of request.flags.slice(0, maxPatches).entries()) {
    view.setUint16(48 + index * 4, patch.id, true)
    view.setUint8(50 + index * 4, patch.value ? 1 : 0)
  }
  view.setUint8(80, request.checkpoint)
  view.setUint8(81, request.facing)
  view.setUint8(82, request.vars.length)
  view.setUint8(83, request.flags.length)
  view.setUint8(84, request.textSpeed)
  view.setUint8(85, request.useRngSeed ? 1 : 0)
  view.setUint8(86, request.command)
  for (const [index, mon] of request.party.slice(0, maxParty).entries()) {
    const offset = 88 + index * 20
    encodeMon(view, offset, mon)
    view.setUint8(offset + 16, mon.fainted ? 1 : 0)
  }
  for (const [index, item] of request.bagItems.slice(0, maxBagItems).entries()) {
    const offset = 208 + index * 4
    view.setUint16(offset, item.item, true)
    view.setUint16(offset + 2, item.quantity, true)
  }
  for (const [index, pcSlot] of request.pcSlots.slice(0, maxPcSlots).entries()) {
    const offset = 240 + index * 20
    encodeMon(view, offset, pcSlot.mon)
    view.setUint8(offset + 16, pcSlot.box)
    view.setUint8(offset + 17, pcSlot.slot)
  }
  encodeMon(view, 400, request.wildMon)
  view.setUint8(416, request.party.length)
  view.setUint8(417, request.bagItems.length)
  view.setUint8(418, request.pcSlots.length)
  view.setUint8(419, request.currentBox)
  view.setUint8(420, request.hmsOverwrite ? 1 : 0)
  view.setUint8(421, request.fullPocketMask)
  return bytes
}

export const encodeArrangeRequest = (abi: SessionAbi, request: ArrangeRequest): Uint8Array =>
  encodeCommandRequest(abi, {
    ...request,
    command: commands.arrange,
    useRngSeed: true,
    wildMon: request.wildMon ?? emptyMon(),
  })

export const encodeStartWildBattleRequest = (
  abi: SessionAbi,
  requestId: number,
  wildMon: MonFixtureWire,
): Uint8Array =>
  encodeCommandRequest(abi, {
    requestId,
    command: commands.startWildBattle,
    mapGroup: keepMap,
    mapNum: keepMap,
    x: keepCoordinate,
    y: keepCoordinate,
    rngSeed: 0,
    useRngSeed: false,
    vars: [],
    flags: [],
    checkpoint: 0,
    facing: 0,
    textSpeed: 0,
    party: [],
    bagItems: [],
    pcSlots: [],
    wildMon,
    currentBox: 0,
    hmsOverwrite: false,
    fullPocketMask: 0,
  })

export const encodeSaveRequest = (abi: SessionAbi, requestId: number): Uint8Array =>
  encodeCommandRequest(abi, {
    requestId,
    command: commands.save,
    mapGroup: keepMap,
    mapNum: keepMap,
    x: keepCoordinate,
    y: keepCoordinate,
    rngSeed: 0,
    useRngSeed: false,
    vars: [],
    flags: [],
    checkpoint: 0,
    facing: 0,
    textSpeed: 0,
    party: [],
    bagItems: [],
    pcSlots: [],
    wildMon: emptyMon(),
    currentBox: 0,
    hmsOverwrite: false,
    fullPocketMask: 0,
  })

export const encodeObserveRegionMapRequest = (abi: SessionAbi, requestId: number): Uint8Array =>
  encodeCommandRequest(abi, {
    requestId,
    command: commands.observeRegionMap,
    mapGroup: keepMap,
    mapNum: keepMap,
    x: keepCoordinate,
    y: keepCoordinate,
    rngSeed: 0,
    useRngSeed: false,
    vars: [],
    flags: [],
    checkpoint: 0,
    facing: 0,
    textSpeed: 0,
    party: [],
    bagItems: [],
    pcSlots: [],
    wildMon: emptyMon(),
    currentBox: 0,
    hmsOverwrite: false,
    fullPocketMask: 0,
  })

export const encodeObserveRegionMapSectionRequest = (
  abi: SessionAbi,
  requestId: number,
  x: number,
  y: number,
): Uint8Array =>
  encodeCommandRequest(abi, {
    requestId,
    command: commands.observeRegionMapSection,
    mapGroup: keepMap,
    mapNum: keepMap,
    x,
    y,
    rngSeed: 0,
    useRngSeed: false,
    vars: [],
    flags: [],
    checkpoint: 0,
    facing: 0,
    textSpeed: 0,
    party: [],
    bagItems: [],
    pcSlots: [],
    wildMon: emptyMon(),
    currentBox: 0,
    hmsOverwrite: false,
    fullPocketMask: 0,
  })

export const encodeWinBattleRequest = (abi: SessionAbi, requestId: number): Uint8Array =>
  encodeCommandRequest(abi, {
    requestId,
    command: commands.winBattle,
    mapGroup: keepMap,
    mapNum: keepMap,
    x: keepCoordinate,
    y: keepCoordinate,
    rngSeed: 0,
    useRngSeed: false,
    vars: [],
    flags: [],
    checkpoint: 0,
    facing: 0,
    textSpeed: 0,
    party: [],
    bagItems: [],
    pcSlots: [],
    wildMon: emptyMon(),
    currentBox: 0,
    hmsOverwrite: false,
    fullPocketMask: 0,
  })

export const parseCommandResult = (bytes: Uint8Array): CommandResult => ({
  requestId: uint32(bytes, 0),
  mapGroup: uint16(bytes, 4),
  mapNum: uint16(bytes, 6),
  x: int16(bytes, 8),
  y: int16(bytes, 10),
  error: uint16(bytes, 12),
  status: bytes[14]!,
  phase: bytes[15]!,
})
export const parseArrangeResult = parseCommandResult

const parseObservedPcSlot = (bytes: Uint8Array, offset: number): ObservedPcSlot => ({
  species: uint16(bytes, offset),
  moves: Array.from({ length: maxMoves }, (_, index) => uint16(bytes, offset + 2 + index * 2)),
  level: bytes[offset + 10]!,
  egg: bytes[offset + 11] === 1,
  box: bytes[offset + 12]!,
  slot: bytes[offset + 13]!,
})

export const parseStateSnapshot = (bytes: Uint8Array): StateSnapshot => {
  const partySpecies = Array.from({ length: maxParty }, (_, index) => uint16(bytes, 18 + index * 2))
  const partyMoves = Array.from({ length: maxParty }, (_, partyIndex) =>
    Array.from({ length: maxMoves }, (_, moveIndex) =>
      uint16(bytes, 30 + (partyIndex * maxMoves + moveIndex) * 2),
    ),
  )
  const pcSlotCount = Math.min(bytes[332]!, maxPcSlots)
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
    bagItems: Array.from({ length: maxBagItems }, (_, index) => ({
      item: uint16(bytes, 158 + index * 2),
      quantity: uint16(bytes, 78 + index * 2),
    })),
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
    pcSlots: Array.from({ length: pcSlotCount }, (_, index) =>
      parseObservedPcSlot(bytes, 176 + index * 16),
    ),
    battleEnemySpecies: uint16(bytes, 304),
    battleEnemyMoves: Array.from({ length: maxMoves }, (_, index) =>
      uint16(bytes, 306 + index * 2),
    ),
    caughtSpecies: uint16(bytes, 314),
    lastUsedItem: uint16(bytes, 316),
    battleEnemyLevel: bytes[318]!,
    battleActive: bytes[319] === 1,
    catchSwapState: bytes[320]!,
    catchSwapCursor: bytes[321]!,
    catchSwapSelectedParty: bytes[322]!,
    catchSwapBox: bytes[323]!,
    catchSwapSlot: bytes[324]!,
    storageUiState: bytes[325]!,
    storageOpen: bytes[326] === 1,
    storageReady: bytes[327] === 1,
    storageCursorArea: bytes[328]!,
    storageCursorPosition: bytes[329]!,
    storageMovingMon: bytes[330] === 1,
    storageCurrentBox: bytes[331]!,
    battleUiState: bytes[333]!,
    battleCursor: bytes[334]!,
    battleBagPocket: bytes[335]!,
    storageMode: bytes[336]!,
    battleBagItem: uint16(bytes, 338),
  }
}
