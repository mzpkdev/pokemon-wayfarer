import { GameSession, type ArrangeGame } from "../harness/game-session"

// Authoring flags clear during map loading. Enable after the fixture's warp,
// at the point where an authored scene would run its setflag command.
export const arrangeTrainerOnly = async (game: GameSession, options: ArrangeGame) => {
  await game.arrange(options)
  await game.story.setFlag("trainerOnlyEnabled", true)
}
