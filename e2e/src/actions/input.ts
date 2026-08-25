import { type SkyEmuButton, type SkyEmuClient } from "../harness/skyemu"

export const advance = async (client: SkyEmuClient, frames: number): Promise<void> => {
  const result = await client.step(frames)
  if (result !== "ok") throw new Error(`SkyEmu failed to advance ${frames} frames: ${result}`)
}

export const press = async (client: SkyEmuClient, button: SkyEmuButton): Promise<void> => {
  const pressed = await client.input({ [button]: 1 })
  if (pressed !== "ok") throw new Error(`SkyEmu failed to press ${button}: ${pressed}`)
  await advance(client, 2)
  const released = await client.input({ [button]: 0 })
  if (released !== "ok") throw new Error(`SkyEmu failed to release ${button}: ${released}`)
  await advance(client, 2)
}
