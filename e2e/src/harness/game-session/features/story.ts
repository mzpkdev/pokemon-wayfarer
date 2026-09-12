import { storyFlags, storyVars, type StoryFlag, type StoryVar } from "../catalog"
import { varsStart } from "../protocol"
import { encodeObserveFlagRequest, encodeObserveVarRequest, encodeSetVarRequest } from "../protocol"
import { type MailboxApi } from "../mailbox"
import { type SessionRuntime } from "../runtime"

export type StoryApi = {
  flag: (name: StoryFlag) => Promise<boolean>
  setFlag: (name: StoryFlag, value: boolean) => Promise<void>
  var: (name: StoryVar) => Promise<number>
  setVar: (name: StoryVar, value: number) => Promise<void>
}

export const createStoryApi = (runtime: SessionRuntime, mailbox: MailboxApi): StoryApi => ({
  flag: async (name) => {
    const id = storyFlags[name]
    if (id < 0x6000) {
      const saveBlock = await runtime.readUint32(runtime.address("gSaveBlock1Ptr"))
      const byte = await runtime.readBytes(
        saveBlock + runtime.abi.flagsOffset + Math.floor(id / 8),
        1,
      )
      return (byte[0]! & (1 << (id % 8))) !== 0
    }
    const result = await mailbox.execute(
      (requestId) => encodeObserveFlagRequest(runtime.abi, requestId, id),
      `observe story flag ${name}`,
    )
    return result.x === 1
  },
  setFlag: async (name, value) => {
    const id = storyFlags[name]
    if (id >= 0x4000) throw new Error("setFlag supports only ordinary save-bank flags")
    const saveBlock = await runtime.readUint32(runtime.address("gSaveBlock1Ptr"))
    const address = saveBlock + runtime.abi.flagsOffset + Math.floor(id / 8)
    const bytes = await runtime.readBytes(address, 1)
    const mask = 1 << (id % 8)
    await runtime.writeBytes(address, new Uint8Array([value ? bytes[0]! | mask : bytes[0]! & ~mask]))
  },
  var: async (name) => {
    const id = storyVars[name]
    if ((id & 0xf000) === 0x7000) {
      const result = await mailbox.execute(
        (requestId) => encodeObserveVarRequest(runtime.abi, requestId, id),
        `observe story var ${name}`,
      )
      // Command results use signed coordinate fields, while script variables
      // are u16 values. Restore the original VarGet result.
      return result.x & 0xffff
    }
    const saveBlock = await runtime.readUint32(runtime.address("gSaveBlock1Ptr"))
    return runtime.readUint16(saveBlock + runtime.abi.varsOffset + (id - varsStart) * 2)
  },
  setVar: async (name, value) => {
    if (!Number.isInteger(value) || value < 0 || value > 0xffff)
      throw new Error("Story variable values must be unsigned 16-bit integers")
    const id = storyVars[name]
    if ((id & 0xf000) === 0x7000) {
      await mailbox.execute(
        (requestId) => encodeSetVarRequest(runtime.abi, requestId, id, value),
        `set story var ${name}`,
      )
      return
    }
    const saveBlock = await runtime.readUint32(runtime.address("gSaveBlock1Ptr"))
    await runtime.writeBytes(
      saveBlock + runtime.abi.varsOffset + (id - varsStart) * 2,
      new Uint8Array([value & 0xff, value >> 8]),
    )
  },
})
