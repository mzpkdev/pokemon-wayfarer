import { type GameSession } from "../harness/game-session"

/**
 * Starts a regular S.S. Aqua departure from Wayfarer's Vermilion attendant.
 *
 * The first menu entry is the isolated Sevii ferry. Regular Aqua routes stay
 * behind the explicit "Other destinations" entry so a test cannot silently
 * select a Sevii island while exercising the Hoenn link.
 */
export const beginWayfarerRegularAquaDeparture = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.wait.frames(30)
  await game.controls.press("down")
  await game.wait.frames(12)
  await game.controls.press("a")
}
