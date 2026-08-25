import * as childProcess from "node:child_process"
import * as stream from "node:stream"

import { describe, expect, it, vi } from "webanvil/test"

import { captureOutputTail, stopProcess } from "./utils"

describe("captureOutputTail", () => {
  it("retains bounded output from every stream", async () => {
    const stdout = new stream.PassThrough()
    const stderr = new stream.PassThrough()
    const output = captureOutputTail([stdout, stderr], 8)

    stdout.write("12345")
    stderr.end("67890")
    stdout.end()
    await output.closed

    expect(output.read()).toBe("34567890")
  })
})

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
