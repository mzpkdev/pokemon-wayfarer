import { advance, press } from "../actions/input"
import { type SkyEmuClient } from "../harness/skyemu"

export const startNewGame = async (client: SkyEmuClient): Promise<void> => {
  await advance(client, 3_600)
  await press(client, "Start")
  await advance(client, 240)
  await press(client, "A")

  await advance(client, 4_800)
  for (let interaction = 0; interaction < 32; interaction++) {
    await press(client, "A")
    await advance(client, 180)
  }
  for (let tab = 0; tab < 6; tab++) {
    await press(client, "R")
    await advance(client, 30)
  }
  await press(client, "A")
  await advance(client, 1_200)
  for (let interaction = 0; interaction < 24; interaction++) {
    await press(client, "A")
    await advance(client, 300)
  }
}
