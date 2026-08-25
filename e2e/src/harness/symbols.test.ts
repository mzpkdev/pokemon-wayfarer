import { describe, expect, it } from "webanvil/test"

import { parseSkyEmuSymbols } from "./symbols"

describe("SkyEmu symbols", () => {
  it("resolves global symbols from objdump output", () => {
    const symbols = parseSkyEmuSymbols(
      [
        "02001614 g 00000240 gObjectEvents",
        "03001fc8 l 00000001 sGlobalScriptContextStatus",
        "030041c4 g 00000004 gSaveBlock1Ptr",
      ].join("\n"),
    )

    expect(symbols.address("gObjectEvents")).toBe(0x02001614)
    expect(symbols.address("gSaveBlock1Ptr")).toBe(0x030041c4)
    expect(() => symbols.address("sGlobalScriptContextStatus")).toThrow(
      "SkyEmu symbol file does not define sGlobalScriptContextStatus",
    )
  })
})
