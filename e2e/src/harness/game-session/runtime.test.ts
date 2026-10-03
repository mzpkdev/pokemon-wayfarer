import { describe, expect, it } from "webanvil/test"

import { type SkyEmuClient } from "../skyemu/client"
import { type SkyEmuSymbols } from "../skyemu/symbols"
import { type SessionAbi } from "./protocol"
import { createSessionRuntime } from "./runtime"

describe("game-session runtime", () => {
  it("never reads memory while another caller's step is running", async () => {
    const events: string[] = []
    let stepping = false
    const client = {
      step: async (frames: number) => {
        stepping = true
        events.push(`step ${frames}`)
        await new Promise((resolve) => setTimeout(resolve, 5))
        stepping = false
        return "ok"
      },
      readBytes: async () => {
        events.push(stepping ? "read during step" : "read")
        return new Uint8Array(4)
      },
    } as unknown as SkyEmuClient
    const runtime = createSessionRuntime(client, {} as SkyEmuSymbols, {} as SessionAbi)

    // A pending arrange keeps stepping while a test polls state.
    await Promise.all([runtime.advance(2), runtime.readBytes(0, 4), runtime.advance(2)])

    expect(events).toEqual(["step 2", "read", "step 2"])
  })
})
