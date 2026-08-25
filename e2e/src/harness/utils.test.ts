import * as childProcess from "node:child_process"

import { describe, expect, it, vi } from "webanvil/test"

import { stopProcess } from "./utils"

describe("stopProcess", () => {
  it("returns when the child has already exited by signal", async () => {
    const kill = vi.fn()
    const child = {
      exitCode: null,
      signalCode: "SIGTERM",
      kill,
    } as unknown as childProcess.ChildProcess

    await stopProcess(child)

    expect(kill).not.toHaveBeenCalled()
  })
})
