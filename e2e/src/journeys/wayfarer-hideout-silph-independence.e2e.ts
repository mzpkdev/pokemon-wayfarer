import { beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"
import { settleSilph, talkSilph, triggerSilphGiovanni, waitSilphBattle } from "../playbooks/silph"

const settle = async (game: GameSession, map: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active && state.map.name === map) return
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`Integration interaction did not settle: ${JSON.stringify(await game.state.read())}`)
}

const startTalkBattle = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`Integration battle did not start: ${JSON.stringify(await game.state.read())}`)
}

const finishHideout = async (game: GameSession): Promise<void> => {
  for (const x of [16, 19]) {
    await game.player.warp("celadon-hideout-b4f", x, 15, "up")
    await startTalkBattle(game)
    await game.battle.win()
    await settle(game, "celadon-hideout-b4f")
  }
  expect(await game.story.flag("celadonHideoutLeftGuardDefeated")).toBe(true)
  expect(await game.story.flag("celadonHideoutRightGuardDefeated")).toBe(true)

  await game.player.warp("celadon-hideout-b4f", 19, 5, "up")
  await startTalkBattle(game)
  await game.battle.win()
  await settle(game, "celadon-hideout-b4f")
  expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(true)
  expect(await game.inventory.contains("silphScope")).toBe(false)
  await game.player.warp("celadon-hideout-b4f", 20, 6, "up")
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await settle(game, "celadon-hideout-b4f")
  expect(await game.inventory.contains("silphScope")).toBe(true)
  expect(await game.story.flag("celadonHideoutScopeReceived")).toBe(true)
  expect(await game.story.flag("celadonHideoutGiovanniTrainerDefeated")).toBe(true)
}

const finishSilph = async (game: GameSession): Promise<void> => {
  await game.player.warp("silph-11f", 5, 16, "up")
  await triggerSilphGiovanni(game)
  await waitSilphBattle(game)
  await game.battle.win()
  await settleSilph(game)
  expect(await game.story.flag("silphLiberated")).toBe(true)
  expect(await game.story.flag("silphMasterBallPending")).toBe(true)
  expect(await game.story.flag("silphGiovanniDefeated")).toBe(true)
  expect(await game.inventory.count("masterBall")).toBe(0)
  await game.player.warp("silph-11f", 9, 10, "up")
  await talkSilph(game)
  expect(await game.inventory.count("masterBall")).toBe(1)
  expect(await game.story.flag("silphMasterBallReceived")).toBe(true)
  expect(await game.story.flag("silphMasterBallPending")).toBe(false)
}

const finishMtMoon = async (game: GameSession): Promise<void> => {
  await game.player.warp("mt-moon-cave", 25, 16, "up")
  await startTalkBattle(game)
  await game.battle.win()
  await game.wait.frames(180)
  await settle(game, "mt-moon-cave")
  expect(await game.story.flag("mtMoonMiguelDefeated")).toBe(true)
  await game.player.warp("mt-moon-cave", 24, 16, "up")
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.controls.press("a")
  await game.wait.frames(60)
  await game.wait.frames(20)
  await game.controls.press("a")
  await settle(game, "mt-moon-cave")
  expect(await game.inventory.contains("helixFossil")).toBe(true)
  expect(await game.story.flag("receivedMtMoonFossil")).toBe(true)
}

const assertPersisted = async (game: GameSession, hideoutDone: boolean, silphDone: boolean): Promise<void> => {
  expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(hideoutDone)
  expect(await game.story.flag("celadonHideoutGiovanniTrainerDefeated")).toBe(hideoutDone)
  expect(await game.story.flag("celadonHideoutScopeReceived")).toBe(hideoutDone)
  expect(await game.inventory.contains("silphScope")).toBe(hideoutDone)
  expect(await game.story.flag("silphLiberated")).toBe(silphDone)
  expect(await game.story.flag("silphGiovanniDefeated")).toBe(silphDone)
  expect(await game.story.flag("silphMasterBallReceived")).toBe(silphDone)
  expect(await game.inventory.count("masterBall")).toBe(silphDone ? 1 : 0)
  expect(await game.story.flag("mtMoonMiguelDefeated")).toBe(true)
  expect(await game.story.flag("receivedMtMoonFossil")).toBe(true)
  expect(await game.inventory.contains("helixFossil")).toBe(true)
}

describe.sequential("Wayfarer Hideout and Silph independent progression", () => {
  let game: GameSession
  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  for (const first of ["hideout", "silph"] as const) {
    it(`completes ${first} first, interleaves Mt. Moon, and preserves both rewards after reload`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "celadon-hideout-b4f", x: 16, y: 15 }, facing: "up" },
        party: [{ species: "lapras", level: 100 }],
        // The 11F door is travel setup; both Giovanni wins and both rewards remain live.
        story: { flags: { silph11FDoor: true } },
        determinism: { textSpeed: "instant" },
      })
      expect(await game.inventory.contains("silphScope")).toBe(false)
      expect(await game.story.flag("silphLiberated")).toBe(false)
      if (first === "hideout") await finishHideout(game)
      else await finishSilph(game)
      await finishMtMoon(game)
      await game.saveAndReload()
      await assertPersisted(game, first === "hideout", first === "silph")
      if (first === "hideout") await finishSilph(game)
      else await finishHideout(game)
      await game.saveAndReload()
      await assertPersisted(game, true, true)
    })
  }
})
