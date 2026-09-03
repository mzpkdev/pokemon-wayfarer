import { storyFlags, storyVars, type StoryFlag, type StoryVar } from "../catalog"
import { varsStart } from "../protocol"
import { type SessionRuntime } from "../runtime"

export type StoryApi = {
  flag: (name: StoryFlag) => Promise<boolean>
  var: (name: StoryVar) => Promise<number>
  setVar: (name: StoryVar, value: number) => Promise<void>
}

export const createStoryApi = (runtime: SessionRuntime): StoryApi => ({
  flag: async (name) => {
    const saveBlock = await runtime.readUint32(runtime.address("gSaveBlock1Ptr"))
    const id = storyFlags[name]
    const byte = await runtime.readBytes(
      saveBlock + runtime.abi.flagsOffset + Math.floor(id / 8),
      1,
    )
    return (byte[0]! & (1 << (id % 8))) !== 0
  },
  var: async (name) => {
    const saveBlock = await runtime.readUint32(runtime.address("gSaveBlock1Ptr"))
    const id = storyVars[name]
    return runtime.readUint16(saveBlock + runtime.abi.varsOffset + (id - varsStart) * 2)
  },
  setVar: async (name, value) => {
    if (!Number.isInteger(value) || value < 0 || value > 0xffff)
      throw new Error("Story variable values must be unsigned 16-bit integers")
    const saveBlock = await runtime.readUint32(runtime.address("gSaveBlock1Ptr"))
    const id = storyVars[name]
    await runtime.writeBytes(
      saveBlock + runtime.abi.varsOffset + (id - varsStart) * 2,
      new Uint8Array([value & 0xff, value >> 8]),
    )
  },
})
