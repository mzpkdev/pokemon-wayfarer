import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"

const advance = async (game: GameSession, predicate: (state: GameState) => boolean) => {
  for (let frame = 0; frame < 7200; frame += 15) {
    const state = await game.state.read()
    if (predicate(state)) return
    if (state.dialogueOpen || state.battle.ui === "text" || state.battle.ui === "caught-dex")
      await game.controls.press("a")
    else if (state.battle.ui === "nickname" || state.battle.ui === "catch-swap-prompt")
      await game.controls.press("b")
    else await game.wait.frames(15)
  }
  throw new Error(
    `Trainer-only medicine transition timed out: ${JSON.stringify(await game.state.read())}`,
  )
}

const menu = (game: GameSession) => advance(game, (state) => state.battle.ui === "action-menu")

const selectAction = async (game: GameSession, target: number) => {
  await menu(game)
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (cursor % 2 !== target % 2) await game.controls.press("right")
  if (Math.floor(cursor / 2) !== Math.floor(target / 2)) await game.controls.press("down")
  await game.controls.press("a")
}

const useBagItem = async (game: GameSession, item: "antidote" | "ether") => {
  await selectAction(game, 1)
  await game.wait.until((state) => state.battle.ui === "bag", "trainer-only Bag")
  for (let pocket = 0; pocket < 5; pocket++) {
    if ((await game.state.read()).battle.bag.item === item) break
    await game.controls.press("right")
    await game.wait.frames(60)
  }
  expect((await game.state.read()).battle.bag.item).toBe(item)
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag-context", "medicine item context")
  await game.controls.press("a")
}

const selectFirstPartyTargetAndDismiss = async (game: GameSession, moves: number) => {
  await game.wait.until((state) => state.ui.mode === "party-menu", "real medicine target picker")
  await game.controls.press("a")
  for (let input = 0; input < moves; input++) {
    await game.wait.frames(30)
    if ((await game.state.read()).ui.mode === "party-menu") await game.controls.press("a")
  }
}

describe.sequential("Wayfarer trainer-only medicine", () => {
  it("restores only the selected exhausted party move with Ether, then commits exactly one turn", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [{ species: "rattata", fainted: true, moves: ["tackle", "surf"], pp: [3, 2] }],
        bag: { items: { ether: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      expect((await game.state.read()).partyPp).toEqual([[3, 2, 0, 0]])
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "ether")
      await selectFirstPartyTargetAndDismiss(game, 4)
      await advance(
        game,
        (state) =>
          !state.battle.active ||
          (state.battle.ui === "action-menu" && state.battle.trainerOnly.completedTurns === 1),
      )
      const state = await game.state.read()
      expect(state.partyPp).toEqual([[13, 2, 0, 0]])
      expect(state.bag.items.ether).toBe(0)
      if (state.battle.active)
        expect(state.battle.trainerOnly).toMatchObject({ active: true, completedTurns: 1 })
    } finally {
      await game.close()
    }
  })

  it("cancels a recovery target picker without consuming the item or a turn", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [{ species: "rattata", fainted: true, moves: ["tackle"] }],
        bag: { items: { ether: 1 } },
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "ether")
      await game.wait.until((state) => state.ui.mode === "party-menu", "Ether target picker")
      await game.controls.press("b")
      await game.wait.frames(60)
      const state = await game.state.read()
      expect(state.bag.items.ether).toBe(1)
      expect(state.battle.trainerOnly).toMatchObject({ active: true, completedTurns: 0, anger: 0 })
    } finally {
      await game.close()
    }
  })

  it("does not consume Ether or a turn when the selected actual move already has full PP", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [{ species: "rattata", fainted: true, moves: ["tackle"] }],
        bag: { items: { ether: 1 } },
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "ether")
      await selectFirstPartyTargetAndDismiss(game, 1)
      await game.wait.frames(60)
      const state = await game.state.read()
      expect(state.ui.mode).toBe("party-menu")
      expect(state.bag.items.ether).toBe(1)
      expect(state.battle.trainerOnly).toMatchObject({ active: true, completedTurns: 0, anger: 0 })
    } finally {
      await game.close()
    }
  })

  it("keeps an inapplicable status cure on the Bag after choosing a real fainted party target", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [{ species: "rattata", fainted: true, moves: ["tackle"] }],
        bag: { items: { antidote: 1 } },
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "antidote")
      await selectFirstPartyTargetAndDismiss(game, 1)
      await game.wait.frames(60)
      const state = await game.state.read()
      expect(state.ui.mode).toBe("party-menu")
      expect(state.partyVitals).toEqual([{ hp: 0, status: 0 }])
      expect(state.bag.items.antidote).toBe(1)
      expect(state.battle.trainerOnly).toMatchObject({ active: true, completedTurns: 0, anger: 0 })
    } finally {
      await game.close()
    }
  })

  it("refuses recovery medicine for a literal empty party before opening a target picker", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [],
        bag: { items: { antidote: 1 } },
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "antidote")
      await game.wait.frames(60)
      const state = await game.state.read()
      expect(state.ui.mode).not.toBe("party-menu")
      expect(state.bag.items.antidote).toBe(1)
      expect(state.battle.trainerOnly).toMatchObject({ active: true, completedTurns: 0, anger: 0 })
    } finally {
      await game.close()
    }
  })
})
