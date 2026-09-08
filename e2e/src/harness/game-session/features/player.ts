import { buttons, directions, maps, type GameMap, type Direction } from "../catalog"
import { encodeWarpRequest } from "../protocol"
import { type MailboxApi } from "../mailbox"
import { type SessionRuntime } from "../runtime"

export type PlayerApi = {
  warp: (map: GameMap, x: number, y: number, facing?: Direction) => Promise<void>
  interact: () => Promise<void>
  move: (direction: Direction) => Promise<void>
}

export const createPlayerApi = (runtime: SessionRuntime, mailbox: MailboxApi): PlayerApi => ({
  warp: async (map, x, y, facing = "down") => {
    if (!Number.isInteger(x) || !Number.isInteger(y) || x < 0 || y < 0 || x > 0x7fff || y > 0x7fff)
      throw new Error("Warp coordinates must be nonnegative signed 16-bit integers")
    await mailbox.execute(
      (requestId) =>
        encodeWarpRequest(
          runtime.abi,
          requestId,
          maps[map].mapGroup,
          maps[map].mapNum,
          x,
          y,
          directions[facing],
        ),
      `warp to ${map}`,
    )
  },
  interact: () => runtime.press("A"),
  move: (direction) => runtime.press(buttons[direction], 3, 1),
})
