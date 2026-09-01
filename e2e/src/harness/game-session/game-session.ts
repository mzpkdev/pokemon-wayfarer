import { createIsolatedRom, type IsolatedRom } from "../skyemu/isolated-rom"
import { startSkyEmu, type RunningSkyEmu } from "../skyemu/server"
import { readSkyEmuSymbols } from "../skyemu/symbols"
import { requireRomPath, requireSymbolsPath } from "../skyemu/utils"
import { createArrangeApi, type ArrangeApi } from "./features/arrange"
import { createBattleApi, type BattleApi } from "./features/battle"
import { createControlsApi, type ControlsApi } from "./features/controls"
import { createDialogueApi, type DialogueApi } from "./features/dialogue"
import { createPlayerApi, type PlayerApi } from "./features/player"
import { createStateApi, type StateApi } from "./features/state"
import { createStorageApi, type StorageApi } from "./features/storage"
import { createStoryApi, type StoryApi } from "./features/story"
import { createWaitApi, type WaitApi } from "./features/wait"
import { createValidationApi, type ValidationApi } from "./features/validation"
import { createMailboxApi, type MailboxApi } from "./mailbox"
import { parseAbi } from "./protocol"
import { createSessionRuntime, type SessionRuntime } from "./runtime"

const protocolTestInternals = new WeakMap<
  GameSession,
  { runtime: SessionRuntime; mailbox: MailboxApi }
>()

export class GameSession {
  readonly arrange: ArrangeApi["arrange"]
  readonly checkpoint: ArrangeApi["checkpoint"]
  readonly battle: BattleApi
  readonly controls: ControlsApi
  readonly dialogue: DialogueApi
  readonly player: PlayerApi
  readonly state: StateApi
  readonly storage: StorageApi
  readonly story: StoryApi
  readonly wait: WaitApi

  private constructor(
    runtime: SessionRuntime,
    private readonly running: RunningSkyEmu,
    private readonly rom: IsolatedRom,
  ) {
    const state = createStateApi(runtime)
    const wait = createWaitApi(runtime, state)
    const mailbox = createMailboxApi(runtime, state)
    const arrange = createArrangeApi(runtime, mailbox)

    this.arrange = arrange.arrange
    this.checkpoint = arrange.checkpoint
    this.battle = createBattleApi(runtime, mailbox)
    this.controls = createControlsApi(runtime)
    this.dialogue = createDialogueApi(state, wait)
    this.player = createPlayerApi(runtime)
    this.state = state
    this.storage = createStorageApi(state, wait)
    this.story = createStoryApi(runtime)
    this.wait = wait
    protocolTestInternals.set(this, { runtime, mailbox })
  }

  static launch = async (): Promise<GameSession> => {
    const rom = await createIsolatedRom(requireRomPath())
    try {
      const symbols = await readSkyEmuSymbols(requireSymbolsPath())
      const running = await startSkyEmu(rom.path)
      try {
        const abi = parseAbi(await running.client.readBytes(symbols.address("gE2ETestAbi"), 16))
        const runtime = createSessionRuntime(running.client, symbols, abi)
        const stateAddress = runtime.address("gE2ETestState")
        let hookFrame = 0
        for (let elapsed = 0; elapsed <= 600; elapsed += 2) {
          hookFrame = await runtime.readUint32(stateAddress)
          if (hookFrame > 0) break
          await runtime.advance(2)
        }
        if (hookFrame === 0) throw new Error("Test ROM hook did not start within 600 frames")
        return new GameSession(runtime, running, rom)
      } catch (error) {
        await running.stop()
        throw error
      }
    } catch (error) {
      await rom.cleanup()
      throw error
    }
  }

  readonly close = async (): Promise<void> => {
    await this.running.stop()
    await this.rom.cleanup()
  }
}

export type ProtocolTestSession = {
  game: GameSession
  protocol: ValidationApi
}

export const launchProtocolTestSession = async (): Promise<ProtocolTestSession> => {
  const game = await GameSession.launch()
  const internals = protocolTestInternals.get(game)
  if (!internals) {
    await game.close()
    throw new Error("Protocol-test internals were not initialized")
  }
  return {
    game,
    protocol: createValidationApi(internals.runtime, internals.mailbox),
  }
}
