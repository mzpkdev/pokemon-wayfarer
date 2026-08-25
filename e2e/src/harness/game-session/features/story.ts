import { storyFlags, storyVars, type StoryFlag, type StoryVar } from "../catalog"
import { varsStart } from "../protocol"
import { type SessionRuntime } from "../runtime"

export type StoryApi = {
  flag: (name: StoryFlag) => Promise<boolean>
  var: (name: StoryVar) => Promise<number>
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
})
