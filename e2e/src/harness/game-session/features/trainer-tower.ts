import {
  encodeTrainerTowerAbandonRequest,
  encodeTrainerTowerDamagePartyRequest,
  encodeTrainerTowerStartRequest,
} from "../protocol"
import { type MailboxApi } from "../mailbox"
import { type SessionRuntime } from "../runtime"

export const trainerTowerChallengeTypes = {
  single: 0,
  double: 1,
  knockout: 2,
  mixed: 3,
} as const

export type TrainerTowerChallengeType = keyof typeof trainerTowerChallengeTypes

export type TrainerTowerApi = {
  start: (challengeType: TrainerTowerChallengeType) => Promise<void>
  damageParty: (hp: number, status?: number) => Promise<void>
  abandon: () => Promise<void>
}

export const createTrainerTowerApi = (
  runtime: SessionRuntime,
  mailbox: MailboxApi,
): TrainerTowerApi => ({
  start: async (challengeType) => {
    const result = await mailbox.execute(
      (requestId) =>
        encodeTrainerTowerStartRequest(
          runtime.abi,
          requestId,
          trainerTowerChallengeTypes[challengeType],
        ),
      `start Trainer Tower ${challengeType} challenge`,
    )
    if (result.x !== 1) throw new Error(`Trainer Tower rejected ${challengeType} challenge entry`)
    await runtime.advance(2)
  },
  damageParty: async (hp, status = 0) => {
    if (!Number.isInteger(hp) || hp < 1 || hp > 0xffff)
      throw new Error("Trainer Tower party HP must be an unsigned nonzero 16-bit integer")
    if (!Number.isInteger(status) || status < 0 || status > 0xffff)
      throw new Error("Trainer Tower party status must fit the test request's 16-bit field")
    await mailbox.execute(
      (requestId) => encodeTrainerTowerDamagePartyRequest(runtime.abi, requestId, hp, status),
      "damage Trainer Tower party",
    )
    await runtime.advance(2)
  },
  abandon: async () => {
    const result = await mailbox.execute(
      (requestId) => encodeTrainerTowerAbandonRequest(runtime.abi, requestId),
      "abandon Trainer Tower challenge",
    )
    if (result.x !== 1) throw new Error("Trainer Tower challenge abandonment failed")
    await runtime.advance(2)
  },
})
