import { directions, type Direction } from "../catalog"
import { gamePhases, parseStateSnapshot } from "../protocol"
import { type SessionRuntime } from "../runtime"

export type GameState = {
  frame: number
  phase: (typeof gamePhases)[number]
  ready: boolean
  map: {
    mapGroup: number
    mapNum: number
  }
  player: {
    x: number
    y: number
    facing: Direction | "unknown"
  }
  controlsLocked: boolean
  scriptActive: boolean
  dialogueOpen: boolean
}

export type StateApi = {
  read: () => Promise<GameState>
}

const facingName = (facing: number): Direction | "unknown" =>
  (Object.entries(directions).find(([, value]) => value === facing)?.[0] as
    | Direction
    | undefined) ?? "unknown"

export const describeState = (state: GameState): string =>
  `phase=${state.phase}, map=${state.map.mapGroup}:${state.map.mapNum}, position=${state.player.x}:${state.player.y}, facing=${state.player.facing}, ready=${state.ready}, controlsLocked=${state.controlsLocked}, scriptActive=${state.scriptActive}, dialogueOpen=${state.dialogueOpen}`

export const createStateApi = (runtime: SessionRuntime): StateApi => ({
  read: async () => {
    const snapshot = parseStateSnapshot(
      await runtime.readBytes(runtime.address("gE2ETestState"), runtime.abi.stateSize),
    )
    return {
      frame: snapshot.frame,
      map: { mapGroup: snapshot.mapGroup, mapNum: snapshot.mapNum },
      player: {
        x: snapshot.x,
        y: snapshot.y,
        facing: facingName(snapshot.facing),
      },
      phase: gamePhases[snapshot.phase] ?? "boot",
      ready: snapshot.ready,
      controlsLocked: snapshot.controlsLocked,
      scriptActive: snapshot.scriptActive,
      dialogueOpen: snapshot.dialogueOpen,
    }
  },
})
