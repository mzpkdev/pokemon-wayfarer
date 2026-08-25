import { describe, expect, it } from "webanvil/test"

import { parseSkyEmuSymbols } from "./symbols"

describe("SkyEmu symbols", () => {
  it("resolves global and unambiguous local symbols from objdump output", () => {
    const symbols = parseSkyEmuSymbols(
      [
        "02001614 g 00000240 gObjectEvents",
        "03001fc8 l 00000001 sGlobalScriptContextStatus",
        "030041c4 g 00000004 gSaveBlock1Ptr",
      ].join("\n"),
    )

    expect(symbols.address("gObjectEvents")).toBe(0x02001614)
    expect(symbols.address("gSaveBlock1Ptr")).toBe(0x030041c4)
    expect(symbols.address("sGlobalScriptContextStatus")).toBe(0x03001fc8)
  })

  it("rejects ambiguous local symbols", () => {
    const symbols = parseSkyEmuSymbols(
      ["08000000 l 00000001 duplicate", "08000004 l 00000001 duplicate"].join("\n"),
    )

    expect(() => symbols.address("duplicate")).toThrow(
      "SkyEmu symbol file defines ambiguous local symbol duplicate",
    )
  })
})
