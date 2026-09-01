import {
  arrangePhases,
  commandErrors,
  commandStatuses,
  parseCommandResult,
  type CommandResult,
} from "./protocol"
import { type SessionRuntime } from "./runtime"
import { describeState, type StateApi } from "./features/state"

export class TestRomCommandError extends Error {
  constructor(
    readonly commandError: (typeof commandErrors)[number] | `unknown-${number}`,
    readonly phase: (typeof arrangePhases)[number] | `unknown-${number}`,
  ) {
    super(`Test ROM command failed during ${phase}: ${commandError}`)
  }
}

export type MailboxApi = {
  execute: (
    encode: (requestId: number) => Uint8Array,
    description: string,
    maxFrames?: number,
  ) => Promise<CommandResult>
}

export const createMailboxApi = (runtime: SessionRuntime, state: StateApi): MailboxApi => {
  let nextRequestId = 0
  let commandQueue = Promise.resolve()

  return {
    execute: (encode, description, maxFrames = 3_600) => {
      const run = commandQueue.then(async () => {
        const requestId = ++nextRequestId
        const requestAddress = runtime.address("gE2ETestRequest")
        await runtime.writeBytes(requestAddress, encode(requestId))
        await runtime.writeBytes(
          requestAddress + runtime.abi.requestStatusOffset,
          Uint8Array.of(commandStatuses.pending),
        )

        const resultAddress = runtime.address("gE2ETestResult")
        let result = parseCommandResult(
          await runtime.readBytes(resultAddress, runtime.abi.resultSize),
        )
        for (let elapsed = 0; elapsed <= maxFrames; elapsed += 2) {
          await runtime.advance(2)
          result = parseCommandResult(
            await runtime.readBytes(resultAddress, runtime.abi.resultSize),
          )
          if (result.status !== commandStatuses.success && result.status !== commandStatuses.error)
            continue
          // A rejected concurrent request temporarily owns the single result
          // slot. The active request keeps running and will publish its own
          // result again, so each caller waits for its matching request ID.
          if (result.requestId !== requestId) continue
          if (result.status === commandStatuses.error) {
            const phase = arrangePhases[result.phase] ?? `unknown-${result.phase}`
            const error = commandErrors[result.error] ?? `unknown-${result.error}`
            throw new TestRomCommandError(error, phase)
          }
          return result
        }

        throw new Error(
          `${description} timed out in ${maxFrames} frames; ${describeState(await state.read())}`,
        )
      })
      commandQueue = run.then(
        () => undefined,
        () => undefined,
      )
      return run
    },
  }
}
