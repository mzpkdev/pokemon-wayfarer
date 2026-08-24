import { describe, expect, it } from "webanvil/test"

import { SkyEmuClient } from "./skyemu.js"

describe("SkyEmu HTTP client", () => {
  it("uses the local HTTP endpoint", () => {
    expect(new SkyEmuClient(4321).baseUrl).toBe("http://127.0.0.1:4321")
  })
})
