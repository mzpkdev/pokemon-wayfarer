import { arrangeTrainerOnly } from "../playbooks/trainer-only-scenario"
import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"
import { GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"

const B_OUTCOME_RAN = 4

const advance = async (game: GameSession, predicate: (state: GameState) => boolean) => {
  for (let frame = 0; frame < 7200; frame += 15) {
    const state = await game.state.read()
    if (predicate(state)) return
    if (state.battle.ui === "text" || state.battle.ui === "caught-dex")
      await game.controls.press("a")
    else if (state.battle.ui === "nickname" || state.battle.ui === "catch-swap-prompt")
      await game.controls.press("b")
    else await game.wait.frames(15)
  }
  throw new Error(`Trainer-only transition timed out: ${JSON.stringify(await game.state.read())}`)
}

const menu = (game: GameSession) => advance(game, (state) => state.battle.ui === "action-menu")
const select = async (game: GameSession, target: number) => {
  await menu(game)
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (cursor % 2 !== target % 2) await game.controls.press("right")
  if (Math.floor(cursor / 2) !== Math.floor(target / 2)) await game.controls.press("down")
  await game.controls.press("a")
}

const useBagItem = async (
  game: GameSession,
  item: "oranBerry" | "masterBall" | "quickBall" | "timerBall",
) => {
  await select(game, 1)
  await game.wait.until((state) => state.battle.ui === "bag", "trainer-only Bag")
  for (let pocket = 0; pocket < 5; pocket++) {
    if ((await game.state.read()).battle.bag.item === item) break
    await game.controls.press("right")
    await game.wait.frames(60)
  }
  expect((await game.state.read()).battle.bag.item).toBe(item)
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag-context", "item context")
  await game.controls.press("a")
}

const parties = [
  { name: "empty", party: [] },
]

describe.sequential("Wayfarer trainer-only controller", () => {
  for (const [index, fixture] of parties.entries()) {
    it(`enters with a real ${fixture.name} party and cancels Bag without a turn`, async () => {
      const game = await GameSession.launch()
      try {
        await arrangeTrainerOnly(game, {
          checkpoint: "new-bark-after-intro",
          player: { appearanceStyle: (index + 1) as 1 | 2 | 3 | 4 },
          party: fixture.party,
          bag: { items: { masterBall: 1 } },
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        await game.battle.startWild({ species: "pidgey", level: 5 })
        await menu(game)
        const initial = await game.state.read()
        await fs.promises.writeFile(
          `/tmp/trainer-only-${fixture.name.replaceAll(" ", "-")}-menu.png`,
          await game.screenshot(),
        )
        expect(initial.appearance.id).toBe([1, 2, 5, 6][index])
        expect(initial.party).toHaveLength(fixture.party.length)
        expect(initial.battle.trainerOnly).toMatchObject({
          active: true,
          completedTurns: 0,
          anger: 0,
          approach: 0,
        })
        await select(game, 1)
        await game.wait.until((state) => state.battle.ui === "bag", "trainer-only Bag")
        await game.controls.press("b")
        await menu(game)
        const cancelled = await game.state.read()
        await fs.promises.writeFile(
          `/tmp/trainer-only-${fixture.name.replaceAll(" ", "-")}-bag-return.png`,
          await game.screenshot(),
        )
        expect(cancelled.appearance.id).toBe(initial.appearance.id)
        expect(cancelled.battle.trainerOnly).toEqual(initial.battle.trainerOnly)
        expect(cancelled.bag.items.masterBall).toBe(1)
        expect(cancelled.party).toEqual(initial.party)
      } finally {
        await game.close()
      }
    })
  }

  it("captures with exactly one owned ball and restores ordinary battle routing", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeTrainerOnly(game, {
        checkpoint: "new-bark-after-intro",
        party: [],
        bag: { items: { masterBall: 1 } },
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await select(game, 1)
      await game.wait.until((state) => state.battle.ui === "bag", "trainer-only Bag")
      for (let pocket = 0; pocket < 5; pocket++) {
        if ((await game.state.read()).battle.bag.item === "masterBall") break
        await game.controls.press("right")
        await game.wait.frames(60)
      }
      expect((await game.state.read()).battle.bag.item).toBe("masterBall")
      await game.controls.press("a")
      await game.wait.until((state) => state.battle.ui === "bag-context", "owned ball menu")
      await game.controls.press("a")
      await advance(game, (state) => !state.battle.active && state.ready)
      const caught = await game.state.read()
      expect(caught.party).toMatchObject([{ species: "pidgey", fainted: false, egg: false }])
      expect(caught.bag.items.masterBall).toBe(0)
      expect(caught.battle.trainerOnly.active).toBe(false)
      await game.battle.startWild({ species: "rattata", level: 5 })
      await menu(game)
      expect((await game.state.read()).battle.trainerOnly.active).toBe(false)
    } finally {
      await game.close()
    }
  })
  it("commits Go Near once and returns through the real field callback if the wild flees", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeTrainerOnly(game, {
        checkpoint: "new-bark-after-intro",
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await select(game, 2)
      await advance(
        game,
        (state) =>
          state.ready ||
          (state.battle.ui === "action-menu" && state.battle.trainerOnly.completedTurns === 1),
      )
      const state = await game.state.read()
      expect(state.party).toHaveLength(0)
      if (state.battle.active) {
        expect(state.battle.trainerOnly).toMatchObject({
          approach: 1,
          escapeFactor: 7,
          completedTurns: 1,
          anger: 3,
        })
        expect(state.battle.trainerOnly.catchFactor).toBe(
          Math.min(20, state.battle.trainerOnly.initialCatchFactor + 4),
        )
      } else {
        expect(state.controlsLocked).toBe(false)
        expect(state.battle.trainerOnly.active).toBe(false)
      }
    } finally {
      await game.close()
    }
  })

  it("fails a Run as a committed turn", async () => {
    let observedFailure = false
    for (let seed = 1; seed <= 12 && !observedFailure; seed++) {
      const game = await GameSession.launch()
      try {
        await arrangeTrainerOnly(game, {
          checkpoint: "new-bark-after-intro",
          party: [],
          determinism: { textSpeed: "instant", rngSeed: seed },
        })
        await game.battle.startWild({ species: "kyogre", level: 100 })
        await select(game, 3)
        await advance(
          game,
          (state) =>
            state.ready ||
            (state.battle.ui === "action-menu" && state.battle.trainerOnly.completedTurns === 1),
        )
        const state = await game.state.read()
        if (state.battle.active) {
          expect(state.battle.trainerOnly).toMatchObject({
            completedTurns: 1,
            runAttempts: 1,
            anger: 15,
          })
          observedFailure = true
        }
      } finally {
        await game.close()
      }
    }
    expect(observedFailure).toBe(true)
  })

  it("succeeds at Run without inventing a player battler", async () => {
    let escaped = false
    for (let seed = 1; seed <= 12 && !escaped; seed++) {
      const game = await GameSession.launch()
      try {
        await arrangeTrainerOnly(game, {
          checkpoint: "new-bark-after-intro",
          party: [],
          determinism: { textSpeed: "instant", rngSeed: seed },
        })
        const before = await game.state.read()
        await game.battle.startWild({ species: "pidgey", level: 1 })
        await select(game, 3)
        await game.wait.until(
          (state) => !state.battle.active || state.battle.trainerOnly.runAttempts > 0,
          "Run outcome before a failed attempt can become a wild flee",
        )
        const resolved = await game.state.read()
        if (!resolved.battle.active && resolved.battle.trainerOnly.outcome === B_OUTCOME_RAN) {
          await advance(game, (state) => state.ready && !state.battle.active)
          const state = await game.state.read()
          expect(state.battle.trainerOnly.outcome).toBe(B_OUTCOME_RAN)
          expect(state.party).toHaveLength(0)
          expect(state.map).toEqual(before.map)
          expect(state.player).toEqual(before.player)
          expect(await game.story.flag("trainerOnlyEnabled")).toBe(true)
          await game.battle.startWild({ species: "pidgey", level: 1 })
          await menu(game)
          expect((await game.state.read()).battle.trainerOnly.active).toBe(true)
          escaped = true
        }
      } finally {
        await game.close()
      }
    }
    expect(escaped).toBe(true)
  })

  it("keeps globally disabled Run uncommitted", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeTrainerOnly(game, {
        checkpoint: "new-bark-after-intro",
        story: { flags: { noWildRunning: true } },
        party: [],
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await select(game, 3)
      await menu(game)
      expect((await game.state.read()).battle.trainerOnly).toMatchObject({
        completedTurns: 0,
        runAttempts: 0,
        anger: 0,
      })
    } finally {
      await game.close()
    }
  })

  for (const ball of ["quickBall", "timerBall"] as const) {
    it(`counts a failed ${ball} only after its committed action`, async () => {
      let observedFailure = false
      for (let seed = 1; seed <= 12 && !observedFailure; seed++) {
        const game = await GameSession.launch()
        try {
          await arrangeTrainerOnly(game, {
            checkpoint: "new-bark-after-intro",
            party: [],
            bag: { items: { [ball]: 1 } },
            determinism: { textSpeed: "instant", rngSeed: seed },
          })
          await game.battle.startWild({ species: "kyogre", level: 100 })
          if (ball === "timerBall") {
            await select(game, 2)
            await advance(
              game,
              (state) =>
                state.ready ||
                (state.battle.ui === "action-menu" &&
                  state.battle.trainerOnly.completedTurns === 1),
            )
            if (!(await game.state.read()).battle.active) continue
          }
          await useBagItem(game, ball)
          const priorTurns = ball === "quickBall" ? 0 : 1
          await advance(
            game,
            (state) =>
              state.ready ||
              (state.battle.ui === "action-menu" &&
                state.battle.trainerOnly.completedTurns === priorTurns + 1),
          )
          const state = await game.state.read()
          if (state.battle.active) {
            expect(state.battle.lastUsedItem).toBe(ball)
            expect(state.bag.items[ball]).toBe(0)
            expect(state.battle.caughtSpecies).toBe("none")
            expect(state.battle.trainerOnly.completedTurns).toBe(priorTurns + 1)
            observedFailure = true
          }
        } finally {
          await game.close()
        }
      }
      expect(observedFailure).toBe(true)
    })
  }

  it("feeds an owned berry directly and consumes exactly one", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeTrainerOnly(game, {
        checkpoint: "new-bark-after-intro",
        party: [],
        bag: { items: { oranBerry: 2 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "oranBerry")
      // The native Pokéblock animation holds the food effect until
      // the throw, projectile, and target's eating reaction have played.
      // Preserve evenly spaced frames from that animation interval for
      // visual inspection without adding a test-only animation decoder.
      let foodApplied = false
      for (let sample = 0; sample < 90; sample++) {
        const animating = await game.state.read()
        expect(animating.battle.trainerOnly.completedTurns).toBe(0)
        if (animating.battle.trainerOnly.foodTurns > 0) {
          foodApplied = true
          break
        }
        await fs.promises.writeFile(
          `/tmp/trainer-only-berry-feed-${String(sample).padStart(2, "0")}.png`,
          await game.screenshot(),
        )
        await game.wait.frames(4)
      }
      expect(foodApplied).toBe(true)
      await advance(
        game,
        (state) =>
          state.ready ||
          (state.battle.ui === "action-menu" && state.battle.trainerOnly.completedTurns === 1),
      )
      const state = await game.state.read()
      expect(state.bag.items.oranBerry).toBe(1)
      expect(state.party).toHaveLength(0)
      if (state.battle.active)
        expect(state.battle.trainerOnly).toMatchObject({
          anger: 3,
          foodTurns: 2,
          completedTurns: 1,
        })
    } finally {
      await game.close()
    }
  })

  it("knocks out a real one-HP wild Pokémon with Rock and returns without a capture", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeTrainerOnly(game, {
        checkpoint: "new-bark-after-intro",
        party: [],
        determinism: { textSpeed: "instant" },
      })
      const before = await game.state.read()
      await game.battle.startWild({ species: "shedinja", level: 5 })
      await select(game, 0)
      await game.wait.frames(30)
      await fs.promises.writeFile("/tmp/trainer-only-rock-impact.png", await game.screenshot())
      await advance(game, (state) => state.ready && !state.battle.active)
      const after = await game.state.read()
      expect(after.map).toEqual(before.map)
      expect(after.player).toEqual(before.player)
      expect(after.party).toHaveLength(0)
      expect(after.battle.caughtSpecies).toBe("none")
      expect(after.circuit.trainerRating).toBe(before.circuit.trainerRating)
      expect(after.battle.trainerOnly.active).toBe(false)
    } finally {
      await game.close()
    }
  })

})
