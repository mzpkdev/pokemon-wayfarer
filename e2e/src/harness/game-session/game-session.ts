import { createIsolatedRom, type IsolatedRom } from "../isolated-rom"
import { startSkyEmu, type RunningSkyEmu } from "../skyemu-server"
import { readSkyEmuSymbols } from "../symbols"
import { requireRomPath, requireSymbolsPath } from "../utils"
import { createArrangeApi, type ArrangeApi } from "./features/arrange"
import { createDialogueApi, type DialogueApi } from "./features/dialogue"
import { createPlayerApi, type PlayerApi } from "./features/player"
import { createStateApi, type StateApi } from "./features/state"
import { createStoryApi, type StoryApi } from "./features/story"
import { createWaitApi, type WaitApi } from "./features/wait"
import { parseAbi } from "./protocol"
import { createSessionRuntime, type SessionRuntime } from "./runtime"

export class GameSession {
  readonly arrange: ArrangeApi["arrange"]
  readonly checkpoint: ArrangeApi["checkpoint"]
  readonly dialogue: DialogueApi
  readonly player: PlayerApi
  readonly state: StateApi
  readonly story: StoryApi
  readonly wait: WaitApi

  private constructor(
    runtime: SessionRuntime,
    private readonly running: RunningSkyEmu,
    private readonly rom: IsolatedRom,
  ) {
    const state = createStateApi(runtime)
    const wait = createWaitApi(runtime, state)
    const arrange = createArrangeApi(runtime, state)

    this.arrange = arrange.arrange
    this.checkpoint = arrange.checkpoint
    this.dialogue = createDialogueApi(state, wait)
    this.player = createPlayerApi(runtime)
    this.state = state
    this.story = createStoryApi(runtime)
    this.wait = wait
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
