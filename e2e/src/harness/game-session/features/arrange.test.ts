import { describe, expect, it } from "webanvil/test"

import { type MailboxApi } from "../mailbox"
import { type SessionRuntime } from "../runtime"
import { createArrangeApi, type ArrangeGame } from "./arrange"

const captureArrange = () => {
  const requests: Uint8Array[] = []
  const runtime = { abi: { requestSize: 432 } } as SessionRuntime
  const mailbox = {
    execute: async (encode: (id: number) => Uint8Array) => {
      requests.push(encode(1))
      return { requestId: 1, mapGroup: 0, mapNum: 0, x: 0, y: 0, error: 0, status: 3, phase: 0 }
    },
  } as MailboxApi
  return { ...createArrangeApi(runtime, mailbox), requests }
}

describe("appearance arrangement fixtures", () => {
  it("maps display styles to stable wire identities and preserves omitted defaults", async () => {
    const fixture = captureArrange()
    for (const [style, id] of [
      [1, 1],
      [2, 2],
      [3, 5],
      [4, 6],
    ] as const) {
      await fixture.arrange({
        checkpoint: "new-bark-after-intro",
        player: { appearanceStyle: style },
      })
      expect(fixture.requests.at(-1)![429]).toBe(id)
    }
    await fixture.arrange({ checkpoint: "new-bark-after-intro" })
    expect(fixture.requests.at(-1)![429]).toBe(0)
  })

  it("rejects invalid styles before submitting a mailbox request", async () => {
    const fixture = captureArrange()
    for (const style of [0, 5, 6, 1.5, Number.NaN]) {
      await expect(
        fixture.arrange({
          checkpoint: "new-bark-after-intro",
          player: { appearanceStyle: style },
        } as ArrangeGame),
      ).rejects.toThrow("Appearance style must")
    }
    expect(fixture.requests).toHaveLength(0)
  })
})
