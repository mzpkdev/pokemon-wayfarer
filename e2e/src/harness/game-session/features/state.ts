import {
  directions,
  hms,
  items,
  maps,
  moves,
  species,
  type Direction,
  type GameMap,
  type Hm,
  type Item,
  type Move,
  type Species,
} from "../catalog"
import {
  battleUiStates,
  catchSwapStates,
  dialogueMessages,
  gamePhases,
  parseStateSnapshot,
  storageUiStates,
  storageModes,
  uiModes,
} from "../protocol"
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
    items: Record<Item, number>
  }
  challenge: {
    hmsOverwrite: boolean
  }
  partyMenu: {
    open: boolean
    actions: number[]
  }
  pc: {
    currentBox: number
    slots: {
      box: number
      slot: number
      mon: {
        species: Species | "unknown"
        moves: (Move | "unknown")[]
        level: number
        egg: boolean
      } | null
    }[]
  }
  battle: {
    active: boolean
    ui: (typeof battleUiStates)[number]
    cursor: number | null
    enemy: {
      species: Species | "unknown"
      moves: (Move | "unknown")[]
      level: number
    } | null
    bag: {
      pocket: number | null
      item: Item | "unknown"
    }
    lastUsedItem: Item | "unknown"
    caughtSpecies: Species | "unknown"
    catchSwap: {
      state: (typeof catchSwapStates)[number]
      cursor: number | null
      selectedParty: number | null
      box: number | null
      slot: number | null
    }
  }
  storage: {
    open: boolean
    ready: boolean
    ui: (typeof storageUiStates)[number]
    mode: (typeof storageModes)[number]
    cursor: { area: number | null; position: number | null }
    movingMon: boolean
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
  `phase=${state.phase}, map=${state.map.name}(${state.map.mapGroup}:${state.map.mapNum}), position=${state.player.x}:${state.player.y}, facing=${state.player.facing}, ready=${state.ready}, controlsLocked=${state.controlsLocked}, scriptActive=${state.scriptActive}, dialogueOpen=${state.dialogueOpen}, battle=${state.battle.ui}, storage=${state.storage.ui}/${state.storage.mode}@${state.storage.cursor.area}:${state.storage.cursor.position}`

export const createStateApi = (runtime: SessionRuntime): StateApi => ({
  read: async () => {
    const snapshot = parseStateSnapshot(
      await runtime.readBytes(runtime.address("gE2ETestState"), runtime.abi.stateSize),
    )
    const bagQuantity = (item: number): number =>
      snapshot.bagItems.find((entry) => entry.item === item)?.quantity ?? 0
    const namedPcSlots = snapshot.pcSlots.map((slot) => ({
      box: slot.box,
      slot: slot.slot,
      mon:
        slot.species === species.none
          ? null
          : {
              species: nameByValue(species, slot.species),
              moves: slot.moves.map((move) => nameByValue(moves, move)),
              level: slot.level,
              egg: slot.egg,
            },
    }))
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
          Object.entries(hms).map(([name, item]) => [name, bagQuantity(item)]),
        ) as Record<Hm, number>,
        items: Object.fromEntries(
          Object.entries(items).map(([name, item]) => [name, bagQuantity(item)]),
        ) as Record<Item, number>,
      },
      challenge: {
        hmsOverwrite: snapshot.hmsOverwrite,
      },
      partyMenu: {
        open: snapshot.uiMode === 3,
        actions: snapshot.partyMenuActions,
      },
      pc: {
        currentBox: snapshot.storageCurrentBox,
        slots: namedPcSlots,
      },
      battle: {
        active: snapshot.battleActive,
        ui: battleUiStates[snapshot.battleUiState] ?? "other",
        cursor: snapshot.battleCursor === 0xff ? null : snapshot.battleCursor,
        enemy:
          snapshot.battleEnemySpecies === species.none
            ? null
            : {
                species: nameByValue(species, snapshot.battleEnemySpecies),
                moves: snapshot.battleEnemyMoves.map((move) => nameByValue(moves, move)),
                level: snapshot.battleEnemyLevel,
              },
        bag: {
          pocket: snapshot.battleBagPocket === 0xff ? null : snapshot.battleBagPocket,
          item: nameByValue(items, snapshot.battleBagItem),
        },
        lastUsedItem: nameByValue(items, snapshot.lastUsedItem),
        caughtSpecies: nameByValue(species, snapshot.caughtSpecies),
        catchSwap: {
          state: catchSwapStates[snapshot.catchSwapState] ?? "none",
          cursor: snapshot.catchSwapCursor === 0xff ? null : snapshot.catchSwapCursor,
          selectedParty:
            snapshot.catchSwapSelectedParty < 6 ? snapshot.catchSwapSelectedParty : null,
          box: snapshot.catchSwapBox < 14 ? snapshot.catchSwapBox : null,
          slot: snapshot.catchSwapSlot < 30 ? snapshot.catchSwapSlot : null,
        },
      },
      storage: {
        open: snapshot.storageOpen,
        ready: snapshot.storageReady,
        ui: storageUiStates[snapshot.storageUiState] ?? "busy",
        mode: storageModes[snapshot.storageMode] ?? "none",
        cursor: {
          area: snapshot.storageCursorArea === 0xff ? null : snapshot.storageCursorArea,
          position: snapshot.storageCursorPosition === 0xff ? null : snapshot.storageCursorPosition,
        },
        movingMon: snapshot.storageMovingMon,
      },
    }
  },
})
