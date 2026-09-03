import { createIsolatedRom, type IsolatedRom } from "../skyemu/isolated-rom"
import { startSkyEmu, type RunningSkyEmu } from "../skyemu/server"
import { readSkyEmuSymbols } from "../skyemu/symbols"
import { requireRomPath, requireSymbolsPath } from "../skyemu/utils"
import { createArrangeApi, type ArrangeApi } from "./features/arrange"
import { createBattleApi, type BattleApi } from "./features/battle"
import { createControlsApi, type ControlsApi } from "./features/controls"
import { createDialogueApi, type DialogueApi } from "./features/dialogue"
import { createInventoryApi, type InventoryApi } from "./features/inventory"
import { createPlayerApi, type PlayerApi } from "./features/player"
import { createRegionMapApi, type RegionMapApi } from "./features/region-map"
import { createStateApi, type StateApi } from "./features/state"
import { createStorageApi, type StorageApi } from "./features/storage"
import { createStoryApi, type StoryApi } from "./features/story"
import { createWaitApi, type WaitApi } from "./features/wait"
import { createValidationApi, type ValidationApi } from "./features/validation"
import { createMailboxApi, type MailboxApi } from "./mailbox"
import { encodeSaveRequest, parseAbi } from "./protocol"
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
  readonly inventory: InventoryApi
  readonly player: PlayerApi
  readonly regionMap: RegionMapApi
  readonly state: StateApi
  readonly storage: StorageApi
  readonly story: StoryApi
  readonly wait: WaitApi
  readonly saveAndReload: () => Promise<void>

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
    this.inventory = createInventoryApi(runtime)
    this.player = createPlayerApi(runtime)
    this.regionMap = createRegionMapApi(runtime, mailbox)
    this.state = state
    this.storage = createStorageApi(state, wait)
    this.story = createStoryApi(runtime)
    this.wait = wait
    this.saveAndReload = async () => {
      await wait.forReady()
      await mailbox.execute((requestId) => encodeSaveRequest(runtime.abi, requestId), "save game")
      // Reset through the GBA itself so the normal boot/load path reads the
      // just-written flash. SkyEmu's load_rom endpoint reloads its host .sav,
      // which is not flushed atomically with emulated flash writes.
      const pressed = await running.client.input({ A: 1, B: 1, Select: 1, Start: 1 })
      if (pressed !== "ok") throw new Error(`SkyEmu failed to press the reset chord: ${pressed}`)
      await runtime.advance(2)
      const released = await running.client.input({ A: 0, B: 0, Select: 0, Start: 0 })
      if (released !== "ok")
        throw new Error(`SkyEmu failed to release the reset chord: ${released}`)

      const stateAddress = runtime.address("gE2ETestState")
      const saveStatusAddress = runtime.address("gSaveFileStatus")
      let hookFrame = 0
      let saveStatus = 0
      let saveLoaded = false
      for (let elapsed = 0; elapsed <= 600; elapsed += 2) {
        await runtime.advance(2)
        ;[hookFrame, saveStatus] = await Promise.all([
          runtime.readUint32(stateAddress),
          runtime.readUint16(saveStatusAddress),
        ])
        if (hookFrame > 0 && saveStatus === 1) {
          saveLoaded = true
          break
        }
      }
      if (!saveLoaded)
        throw new Error(
          `Saved ROM did not reload its test hook and flash data within 600 frames (hook=${hookFrame}, saveStatus=${saveStatus})`,
        )

      // A valid save puts Continue first on the main menu. Use isolated presses
      // to skip boot/title screens and select it, checking between presses so
      // no input leaks into the restored overworld.
      for (let attempt = 0; attempt < 40; attempt++) {
        await runtime.advance(30)
        const current = await state.read()
        if (current.ready) return
        await runtime.press("A")
      }
      throw new Error(
        `Saved ROM did not reach a ready overworld after Continue; ${JSON.stringify(await state.read())}`,
      )
    }
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
