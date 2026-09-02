import { directions, items, moves, species, textSpeeds } from "../catalog"
import { TestRomCommandError, type MailboxApi } from "../mailbox"
import {
  commands,
  encodeCommandRequest,
  keepCoordinate,
  keepMap,
  maxMoves,
  type CommandRequest,
} from "../protocol"
import { type SessionRuntime } from "../runtime"

export type InvalidFixtureCase =
  | "current-box"
  | "pc-box"
  | "pc-slot"
  | "species"
  | "item-quantity"
  | "item-quantity-high"

export type ValidationApi = {
  rejectedFixtureError: (invalid: InvalidFixtureCase) => Promise<string>
}

const emptyMon = () => ({
  species: species.none,
  moves: Array(maxMoves).fill(moves.none),
  level: 0,
  egg: false,
})

export const createValidationApi = (
  runtime: SessionRuntime,
  mailbox: MailboxApi,
): ValidationApi => ({
  rejectedFixtureError: async (invalid) => {
    const request = (requestId: number): CommandRequest => {
      const base: CommandRequest = {
        requestId,
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
        facing: directions.up,
        textSpeed: textSpeeds.instant,
        party: [
          {
            species: species.lapras,
            moves: Array(maxMoves).fill(moves.none),
            level: 5,
            egg: false,
            fainted: false,
          },
        ],
        bagItems: [],
        pcSlots: [],
        wildMon: emptyMon(),
        currentBox: 0,
        hmsOverwrite: false,
        fullPocketMask: 0,
      }
      switch (invalid) {
        case "current-box":
          base.currentBox = 14
          break
        case "pc-box":
          base.pcSlots = [{ box: 14, slot: 0, mon: emptyMon() }]
          break
        case "pc-slot":
          base.pcSlots = [{ box: 0, slot: 30, mon: emptyMon() }]
          break
        case "species":
          base.party[0]!.species = 0xffff
          break
        case "item-quantity":
          base.bagItems = [{ item: items.masterBall, quantity: 0 }]
          break
        case "item-quantity-high":
          base.bagItems = [{ item: items.masterBall, quantity: 1_000 }]
          break
      }
      return base
    }

    try {
      await mailbox.execute(
        (requestId) => encodeCommandRequest(runtime.abi, request(requestId)),
        `reject invalid ${invalid} fixture`,
      )
    } catch (error) {
      if (error instanceof TestRomCommandError) return error.commandError
      throw error
    }
    throw new Error(`Test ROM accepted invalid ${invalid} fixture`)
  },
})
