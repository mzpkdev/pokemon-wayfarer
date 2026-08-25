import * as childProcess from "node:child_process"
import * as events from "node:events"
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

  it("terminates the detached process group", async () => {
    const child = Object.assign(new events.EventEmitter(), {
      exitCode: null,
      signalCode: null,
      pid: 4321,
      kill: vi.fn(),
    }) as unknown as childProcess.ChildProcess
    const kill = vi.spyOn(process, "kill").mockImplementation(() => {
      child.emit("exit", null, "SIGTERM")
      return true
    })

    await stopProcess(child)

    expect(kill).toHaveBeenCalledWith(-4321, "SIGTERM")
    expect(child.kill).not.toHaveBeenCalled()
    kill.mockRestore()
  })

  it("falls back to the direct child when no pid is available", async () => {
    const child = Object.assign(new events.EventEmitter(), {
      exitCode: null,
      signalCode: null,
      pid: undefined,
      kill: vi.fn(),
    }) as unknown as childProcess.ChildProcess

    await stopProcess(child)

    expect(child.kill).toHaveBeenCalledOnce()
  })
})
