import {
  checkpoints,
  directions,
  maps,
  hms,
  items,
  leagueRegions,
  storyFlags,
  storyVars,
  textSpeeds,
  type Checkpoint,
  type Direction,
  type GameMap,
  type Hm,
  type Item,
  type LeagueRegion,
  type StoryFlag,
  type StoryVar,
  type TextSpeed,
} from "../catalog"
import {
  encodeArrangeRequest,
  keepCoordinate,
  keepMap,
  maxBagItems,
  maxMoves,
  maxParty,
  maxPcSlots,
  maxPatches,
  fullPocketMasks,
  pcBoxCapacity,
  totalPcBoxes,
} from "../protocol"
import { type MailboxApi } from "../mailbox"
import { type SessionRuntime } from "../runtime"
import {
  emptyWireMon,
  toWireMon,
  toWirePartyMon,
  type PartyMonFixture,
  type ObservedPcSlotFixture,
} from "./fixtures"

export type PlayerPosition = {
  map?: GameMap
  x: number
  y: number
}

export type ArrangeGame = {
  checkpoint: Checkpoint
  player?: {
    appearanceStyle?: 1 | 2 | 3 | 4
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
  party?: PartyMonFixture[]
  bag?: {
    hms?: Partial<Record<Hm, number>>
    items?: Partial<Record<Item, number>>
    fullPockets?: (keyof typeof fullPocketMasks)[]
  }
  pc?: {
    currentBox?: number
    observedSlots?: ObservedPcSlotFixture[]
  }
  challenge?: {
    hmsOverwrite?: boolean
  }
  circuit?: {
    badges?: Partial<Record<LeagueRegion, number>>
    clears?: Partial<Record<LeagueRegion, boolean>>
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

export const createArrangeApi = (runtime: SessionRuntime, mailbox: MailboxApi): ArrangeApi => {
  const arrange = async (options: ArrangeGame): Promise<void> => {
    const appearanceIds = { 1: 1, 2: 2, 3: 5, 4: 6 } as const
    const appearanceStyle = options.player?.appearanceStyle
    if (appearanceStyle !== undefined && !Object.hasOwn(appearanceIds, appearanceStyle))
      throw new Error("Appearance style must be 1, 2, 3, or 4")
    const appearanceId = appearanceStyle === undefined ? 0 : appearanceIds[appearanceStyle]
    const vars = entries(options.story?.vars)
    const flags = entries(options.story?.flags)
    const party = options.party ?? []
    const hmBagItems = entries(options.bag?.hms)
      .filter(([, quantity]) => quantity > 0)
      .map(([name, quantity]) => ({ item: hms[name], quantity }))
    const genericBagItems = entries(options.bag?.items)
      .filter(([, quantity]) => quantity > 0)
      .map(([name, quantity]) => ({ item: items[name], quantity }))
    const bagItems = [...hmBagItems, ...genericBagItems]
    const fullPockets = options.bag?.fullPockets ?? []
    if (new Set(fullPockets).size !== fullPockets.length)
      throw new Error("Each full Bag pocket fixture may be requested only once")
    const fullPocketMask = fullPockets.reduce((mask, pocket) => mask | fullPocketMasks[pocket], 0)
    const observedPcSlots = options.pc?.observedSlots ?? []
    const regionalBadgeCounts = leagueRegions.map(
      (region) => options.circuit?.badges?.[region] ?? 0,
    )
    const leagueClears = leagueRegions.map((region) => options.circuit?.clears?.[region] ?? false)
    const currentBox = options.pc?.currentBox ?? 0
    if (vars.length > maxPatches) {
      throw new Error(`Test ROM supports at most ${maxPatches} var overrides`)
    }
    if (flags.length > maxPatches) {
      throw new Error(`Test ROM supports at most ${maxPatches} flag overrides`)
    }
    if (party.length > maxParty) {
      throw new Error(`Test ROM supports at most ${maxParty} party Pokémon`)
    }
    if (bagItems.length > maxBagItems) {
      throw new Error(`Test ROM supports at most ${maxBagItems} Bag item fixtures`)
    }
    if (new Set(bagItems.map((entry) => entry.item)).size !== bagItems.length)
      throw new Error("Bag fixtures must not request the same item through hms and items")
    if (currentBox < 0 || currentBox >= totalPcBoxes)
      throw new Error(`Current PC box ${currentBox} is outside 0..${totalPcBoxes - 1}`)
    if (observedPcSlots.length > maxPcSlots)
      throw new Error(`Test ROM supports at most ${maxPcSlots} observed PC slots`)
    for (const [index, count] of regionalBadgeCounts.entries()) {
      if (!Number.isInteger(count) || count < 0 || count > 8)
        throw new Error(`${leagueRegions[index]} badge count ${count} is outside 0..8`)
    }
    for (const mon of party) {
      if ((mon.moves?.length ?? 0) > maxMoves) {
        throw new Error(`Test ROM supports at most ${maxMoves} moves per party Pokémon`)
      }
    }
    for (const requested of observedPcSlots) {
      if (requested.box < 0 || requested.box >= totalPcBoxes)
        throw new Error(`PC box ${requested.box} is outside 0..${totalPcBoxes - 1}`)
      if (requested.slot < 0 || requested.slot >= pcBoxCapacity)
        throw new Error(`PC slot ${requested.slot} is outside 0..${pcBoxCapacity - 1}`)
      if ((requested.mon?.moves?.length ?? 0) > maxMoves)
        throw new Error(`Test ROM supports at most ${maxMoves} moves per boxed Pokémon`)
    }
    if (
      new Set(observedPcSlots.map(({ box, slot }) => `${box}:${slot}`)).size !==
      observedPcSlots.length
    )
      throw new Error("Each requested PC box slot must be unique")

    const position = options.player?.position
    const map = position?.map ? maps[position.map] : undefined
    await mailbox.execute(
      (requestId) =>
        encodeArrangeRequest(runtime.abi, {
          requestId,
          mapGroup: map?.mapGroup ?? keepMap,
          mapNum: map?.mapNum ?? keepMap,
          x: position?.x ?? keepCoordinate,
          y: position?.y ?? keepCoordinate,
          rngSeed: options.determinism?.rngSeed ?? 1,
          vars: vars.map(([name, value]) => ({ id: storyVars[name], value })),
          flags: flags.map(([name, value]) => ({ id: storyFlags[name], value })),
          checkpoint: checkpoints[options.checkpoint],
          appearanceId,
          facing: directions[options.player?.facing ?? "up"],
          textSpeed: textSpeeds[options.determinism?.textSpeed ?? "instant"],
          party: party.map(toWirePartyMon),
          bagItems,
          pcSlots: observedPcSlots.map((requested) => ({
            box: requested.box,
            slot: requested.slot,
            mon: requested.mon ? toWireMon(requested.mon) : emptyWireMon(),
          })),
          currentBox,
          hmsOverwrite: options.challenge?.hmsOverwrite ?? false,
          fullPocketMask,
          regionalBadgeCounts,
          leagueClears,
          applyLeagueCircuit: options.circuit !== undefined,
        }),
      "arrange game",
    )
  }

  return {
    arrange,
    checkpoint: {
      load: (checkpoint, overrides = {}) => arrange({ checkpoint, ...overrides }),
    },
  }
}
