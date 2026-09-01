import {
  directions,
  hms,
  maps,
  moves,
  species,
  type Direction,
  type GameMap,
  type Hm,
  type Move,
  type Species,
} from "../catalog"
import { dialogueMessages, gamePhases, parseStateSnapshot, uiModes } from "../protocol"
import { type SessionRuntime } from "../runtime"

export type GameState = {
  frame: number
  phase: (typeof gamePhases)[number]
  ready: boolean
  map: {
    name: GameMap | "unknown"
    mapGroup: number
    mapNum: number
  }
  player: {
    x: number
    y: number
    facing: Direction | "unknown"
    avatarFlags: number
    surfing: boolean
    surfBlobCount: number
    surfEffectActive: boolean
  }
  avatar: {
    flags: number
    surfing: boolean
    surfBlobCount: number
    surfEffectActive: boolean
  }
  controlsLocked: boolean
  scriptActive: boolean
  dialogueOpen: boolean
  dialogue: {
    message: (typeof dialogueMessages)[number]
    sequence: number
    text: string
  }
  ui: {
    mode: (typeof uiModes)[number]
  }
  fieldMove: {
    move: Move | "unknown"
    user: number | null
    result: "found" | "missing-item" | "no-eligible-mon" | "selected" | "unknown"
    userSpecies: Species | "unknown"
    unlocked: boolean
  }
  party: {
    species: Species | "unknown"
    moves: (Move | "unknown")[]
    egg: boolean
    fainted: boolean
  }[]
  bag: {
    hms: Record<Hm, number>
  }
  challenge: {
    hmsOverwrite: boolean
  }
  partyMenu: {
    open: boolean
    actions: number[]
  }
}

export type StateApi = {
  read: () => Promise<GameState>
}

const facingName = (facing: number): Direction | "unknown" =>
  (Object.entries(directions).find(([, value]) => value === facing)?.[0] as
    | Direction
    | undefined) ?? "unknown"

const mapName = (mapGroup: number, mapNum: number): GameMap | "unknown" =>
  (Object.entries(maps).find(
    ([, map]) => map.mapGroup === mapGroup && map.mapNum === mapNum,
  )?.[0] as GameMap | undefined) ?? "unknown"

const nameByValue = <Name extends string>(
  values: Record<Name, number>,
  value: number,
): Name | "unknown" =>
  (Object.entries(values).find(([, candidate]) => candidate === value)?.[0] as Name | undefined) ??
  "unknown"

const fieldMoveResults = ["found", "missing-item", "no-eligible-mon", "selected"] as const

const fieldMoveResult = (result: number): GameState["fieldMove"]["result"] =>
  fieldMoveResults[result] ?? "unknown"

const decodeFieldMessageText = (bytes: number[]): string => {
  let text = ""

  for (const value of bytes) {
    if (value === 0xff) break
    if (value === 0x00) text += " "
    else if (value >= 0xa1 && value <= 0xaa) text += String(value - 0xa1)
    else if (value === 0xab) text += "!"
    else if (value === 0xac) text += "?"
    else if (value === 0xad) text += "."
    else if (value === 0xae) text += "-"
    else if (value === 0xb8) text += ","
    else if (value === 0xba) text += "/"
    else if (value >= 0xbb && value <= 0xd4)
      text += String.fromCharCode("A".charCodeAt(0) + value - 0xbb)
    else if (value >= 0xd5 && value <= 0xee)
      text += String.fromCharCode("a".charCodeAt(0) + value - 0xd5)
    else if (value === 0xfe) text += "\n"
  }

  return text
}

export const describeState = (state: GameState): string =>
  `phase=${state.phase}, map=${state.map.name}(${state.map.mapGroup}:${state.map.mapNum}), position=${state.player.x}:${state.player.y}, facing=${state.player.facing}, ready=${state.ready}, controlsLocked=${state.controlsLocked}, scriptActive=${state.scriptActive}, dialogueOpen=${state.dialogueOpen}`

export const createStateApi = (runtime: SessionRuntime): StateApi => ({
  read: async () => {
    const snapshot = parseStateSnapshot(
      await runtime.readBytes(runtime.address("gE2ETestState"), runtime.abi.stateSize),
    )
    return {
      frame: snapshot.frame,
      map: {
        name: mapName(snapshot.mapGroup, snapshot.mapNum),
        mapGroup: snapshot.mapGroup,
        mapNum: snapshot.mapNum,
      },
      player: {
        x: snapshot.x,
        y: snapshot.y,
        facing: facingName(snapshot.facing),
        avatarFlags: snapshot.avatarFlags,
        surfing: snapshot.avatarSurfing,
        surfBlobCount: snapshot.surfBlobCount,
        surfEffectActive: snapshot.surfEffectActive,
      },
      avatar: {
        flags: snapshot.avatarFlags,
        surfing: snapshot.avatarSurfing,
        surfBlobCount: snapshot.surfBlobCount,
        surfEffectActive: snapshot.surfEffectActive,
      },
      phase: gamePhases[snapshot.phase] ?? "boot",
      ready: snapshot.ready,
      controlsLocked: snapshot.controlsLocked,
      scriptActive: snapshot.scriptActive,
      dialogueOpen: snapshot.dialogueOpen,
      dialogue: {
        message: dialogueMessages[snapshot.dialogueMessage] ?? "unknown",
        sequence: snapshot.dialogueSequence,
        text: decodeFieldMessageText(snapshot.dialogueText),
      },
      ui: {
        mode: uiModes[snapshot.uiMode] ?? "overworld",
      },
      fieldMove: {
        move: nameByValue(moves, snapshot.fieldMoveMove),
        user: snapshot.fieldMoveUser < 6 ? snapshot.fieldMoveUser : null,
        result: fieldMoveResult(snapshot.fieldMoveResult),
        userSpecies: nameByValue(species, snapshot.fieldMoveUserSpecies),
        unlocked: snapshot.fieldMoveUnlocked,
      },
      party: snapshot.partySpecies.slice(0, snapshot.partyCount).map((partySpecies, index) => ({
        species: nameByValue(species, partySpecies),
        moves: snapshot.partyMoves[index]!.map((move) => nameByValue(moves, move)),
        egg: (snapshot.partyEggMask & (1 << index)) !== 0,
        fainted: (snapshot.partyFaintedMask & (1 << index)) !== 0,
      })),
      bag: {
        hms: Object.fromEntries(
          Object.keys(hms).map((name, index) => [name, snapshot.bagItemCounts[index]!]),
        ) as Record<Hm, number>,
      },
      challenge: {
        hmsOverwrite: snapshot.hmsOverwrite,
      },
      partyMenu: {
        open: snapshot.uiMode === 3,
        actions: snapshot.partyMenuActions,
      },
    }
  },
})
