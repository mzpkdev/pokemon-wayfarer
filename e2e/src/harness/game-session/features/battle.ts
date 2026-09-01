import { encodeStartWildBattleRequest, maxMoves } from "../protocol"
import { type MailboxApi } from "../mailbox"
import { type SessionRuntime } from "../runtime"
import { toWireMon, type MonFixture } from "./fixtures"

export type WildBattleFixture = Omit<MonFixture, "egg" | "level"> & {
  level: number
  egg?: never
}

export type BattleApi = {
  startWild: (wild: WildBattleFixture) => Promise<void>
}

export const createBattleApi = (runtime: SessionRuntime, mailbox: MailboxApi): BattleApi => ({
  startWild: async (wild) => {
    if (wild.level < 1 || wild.level > 100)
      throw new Error("Wild Pokémon level must be between 1 and 100")
    if ((wild.moves?.length ?? 0) > maxMoves)
      throw new Error(`Test ROM supports at most ${maxMoves} moves per wild Pokémon`)
    await mailbox.execute(
      (requestId) => encodeStartWildBattleRequest(runtime.abi, requestId, toWireMon(wild)),
      "start wild battle",
    )
  },
})
