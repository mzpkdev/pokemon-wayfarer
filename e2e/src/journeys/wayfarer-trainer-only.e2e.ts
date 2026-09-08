import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"
import {
  openPcStorage,
  depositPartyMon,
  closePcStorage,
  withdrawSlot,
} from "../playbooks/pc-storage"
import { catchWithMasterBallAndSwap } from "../playbooks/battle-catch-swap"
import { GameSession } from "../harness/game-session"
import { type PartyMonFixture } from "../harness/game-session/features/fixtures"
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
  item: "oranBerry" | "revive" | "masterBall" | "quickBall" | "timerBall",
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

const parties: { name: string; party: PartyMonFixture[] }[] = [
  { name: "empty", party: [] },
  { name: "fainted", party: [{ species: "rattata", fainted: true }] },
  { name: "Egg-only", party: [{ species: "rattata", egg: true }] },
  {
    name: "mixed unusable",
    party: [
      { species: "rattata", fainted: true },
      { species: "pidgey", egg: true },
    ],
  },
]

describe.sequential("Wayfarer trainer-only controller", () => {
  for (const fixture of parties) {
    it(`enters with a real ${fixture.name} party and cancels Bag without a turn`, async () => {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
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
        expect(cancelled.battle.trainerOnly).toEqual(initial.battle.trainerOnly)
        expect(cancelled.bag.items.masterBall).toBe(1)
        expect(cancelled.party).toEqual(initial.party)
      } finally {
        await game.close()
      }
    })
  }

  it("captures with exactly one owned ball and restores real party protection", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
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
      await game.arrange({
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
        await game.arrange({
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
        await game.arrange({
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
      await game.arrange({
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
          await game.arrange({
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

  it("deposits the last real party member, reloads empty, then withdraws protection", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "cherrygrove-pokemon-center", x: 11, y: 2 } },
        party: [{ species: "rattata" }],
        pc: { currentBox: 0, observedSlots: [{ box: 0, slot: 0, mon: null }] },
        determinism: { textSpeed: "instant" },
      })
      await openPcStorage(game, "deposit")
      await depositPartyMon(game, 0)
      expect((await game.state.read()).party).toHaveLength(0)
      expect((await game.storage.slot(0, 0)).mon?.species).toBe("rattata")
      await closePcStorage(game)
      await game.saveAndReload()
      expect((await game.state.read()).party).toHaveLength(0)
      await openPcStorage(game, "withdraw")
      await withdrawSlot(game, 0)
      await closePcStorage(game)
      expect((await game.state.read()).party).toMatchObject([
        { species: "rattata", fainted: false },
      ])
    } finally {
      await game.close()
    }
  })

  it("feeds an owned berry directly and consumes exactly one", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [],
        bag: { items: { oranBerry: 2 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "oranBerry")
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

  it("revives a real fainted member and ends before anger resolution", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [{ species: "rattata", fainted: true }],
        bag: { items: { revive: 1 } },
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "revive")
      await game.wait.until((state) => state.ui.mode === "party-menu", "real Revive target picker")
      // Party-menu allocations are live before their first rendered frame.
      // Capture the settled target screen, rather than its palette transition.
      await game.wait.frames(60)
      await fs.promises.writeFile("/tmp/trainer-only-revive-picker.png", await game.screenshot())
      await game.controls.press("a")
      for (let attempt = 0; attempt < 30; attempt++) {
        const state = await game.state.read()
        if (!state.battle.active && state.ready) break
        await game.controls.press("a")
        await game.wait.frames(60)
      }
      await advance(game, (state) => state.ready && !state.battle.active)
      const state = await game.state.read()
      expect(state.party).toMatchObject([{ species: "rattata", fainted: false }])
      expect(state.bag.items.revive).toBe(0)
      expect(state.battle.trainerOnly.active).toBe(false)
    } finally {
      await game.close()
    }
  })
  it("knocks out a real one-HP wild Pokémon with Rock and returns without a capture", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
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

  const recoveryScenarios = [
    {
      name: "empty party origin fallback",
      checkpoint: "new-bark-after-intro",
      source: "route-30",
      party: [],
      transform: false,
    },
    {
      name: "fainted party origin fallback",
      checkpoint: "new-bark-after-intro",
      source: "route-30",
      party: [{ species: "rattata", fainted: true }],
      transform: false,
    },
    {
      name: "Hoenn Lavaridge transform",
      checkpoint: "hoenn-before-rescue",
      source: "slateport-city",
      party: [],
      transform: true,
    },
    {
      name: "Johto ignores Hoenn transform",
      checkpoint: "hoenn-before-rescue",
      source: "olivine-city",
      party: [],
      transform: true,
    },
  ] as const
  for (const scenario of recoveryScenarios) {
    const party: PartyMonFixture[] = [...scenario.party]
    it(`retaliates after a warning: ${scenario.name}`, async () => {
      const game = await GameSession.launch()
      try {
        let retaliated = false
        for (let seed = 1; seed <= 12 && !retaliated; seed++) {
          await game.arrange({
            checkpoint: scenario.checkpoint,
            player: {
              position: {
                map: scenario.source,
                x: scenario.source === "route-30" ? 11 : 19,
                y: scenario.source === "route-30" ? 8 : 21,
              },
            },
            story: { flags: { hoennWhiteoutToLavaridge: scenario.transform } },
            party,
            determinism: { textSpeed: "instant", rngSeed: seed },
          })
          const beforeRecovery = await game.state.read()
          const moneyBefore = beforeRecovery.money
          await game.battle.startWild({ species: "pidgey", level: 100 })
          for (let rock = 0; rock < 3; rock++) {
            await menu(game)
            if (rock === 2) {
              expect((await game.state.read()).battle.trainerOnly).toMatchObject({
                anger: 80,
                warned: true,
              })
            }
            await select(game, 0)
            if (rock === 1) {
              // The second Rock crosses the threshold. Capture while the
              // warning's own timer still owns the message window, before the
              // next action-menu callback becomes available.
              await game.wait.frames(180)
              await fs.promises.writeFile(
                `/tmp/trainer-only-warning-${scenario.source}-${party.length}.png`,
                await game.screenshot(),
              )
            }
            await advance(
              game,
              (state) =>
                state.ready ||
                state.battle.trainerOnly.anger === 100 ||
                (state.battle.ui === "action-menu" &&
                  state.battle.trainerOnly.completedTurns === rock + 1),
            )
            const state = await game.state.read()
            if (!state.battle.active) break
            if (state.battle.trainerOnly.anger === 100) {
              retaliated = true
              await game.wait.frames(96)
              await fs.promises.writeFile(
                `/tmp/trainer-only-retaliation-${scenario.source}-${party.length}.png`,
                await game.screenshot(),
              )
              for (let confirmation = 0; confirmation < 80; confirmation++) {
                if ((await game.state.read()).ready) break
                await game.controls.press("a")
                await game.wait.frames(30)
              }
              await advance(game, (current) => current.ready && !current.battle.active)
              const recovered = await game.state.read()
              expect(recovered.money).toBe(moneyBefore)
              expect(recovered.map.name).toBe(
                scenario.source === "slateport-city"
                  ? "lavaridge-pokemon-center"
                  : scenario.source === "olivine-city"
                    ? "brendans-house-1f"
                    : beforeRecovery.origin.recovery.map,
              )
              expect(recovered.origin.id).toBe(beforeRecovery.origin.id)
              expect(recovered.party).toHaveLength(party.length)
              expect(recovered.party.every((mon) => !mon.fainted)).toBe(true)
              expect(recovered.battle.trainerOnly.active).toBe(false)
              break
            }
          }
        }
        expect(retaliated).toBe(true)
      } finally {
        await game.close()
      }
    }, 240_000)
  }

  it("sends a catch to PC with six fainted members and stays unprotected", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: Array.from({ length: 6 }, () => ({ species: "rattata" as const, fainted: true })),
        bag: { items: { masterBall: 1 } },
        pc: { currentBox: 0, observedSlots: [{ box: 0, slot: 0, mon: null }] },
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await useBagItem(game, "masterBall")
      await advance(
        game,
        (state) => state.battle.ui === "catch-swap-prompt" || (state.ready && !state.battle.active),
      )
      if ((await game.state.read()).battle.ui === "catch-swap-prompt")
        await game.controls.press("b")
      await advance(game, (state) => state.ready && !state.battle.active)
      const caught = await game.state.read()
      expect(caught.party).toHaveLength(6)
      expect(caught.party.every((mon) => mon.species === "rattata" && mon.fainted)).toBe(true)
      expect((await game.storage.slot(0, 0)).mon?.species).toBe("pidgey")
      expect(caught.bag.items.masterBall).toBe(0)
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await menu(game)
      expect((await game.state.read()).battle.trainerOnly.active).toBe(true)
    } finally {
      await game.close()
    }
  })

  it("swaps a chosen fainted party member through the real full-party capture picker", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [
          { species: "rattata", fainted: true },
          { species: "pidgey", fainted: true },
          { species: "geodude", fainted: true },
          { species: "onix", fainted: true },
          { species: "chikorita", fainted: true },
          { species: "zubat", fainted: true },
        ],
        bag: { items: { masterBall: 1 } },
        pc: { currentBox: 0, observedSlots: [{ box: 0, slot: 0, mon: null }] },
        determinism: { textSpeed: "instant" },
      })
      await game.battle.startWild({ species: "cyndaquil", level: 5 })
      await catchWithMasterBallAndSwap(game, { outgoingPartyIndex: 2 })

      const caught = await game.state.read()
      expect(caught.battle).toMatchObject({
        caughtSpecies: "cyndaquil",
        trainerOnly: { active: false },
        catchSwap: { state: "resolved", selectedParty: 2, box: 0, slot: 0 },
      })
      expect(caught.party).toHaveLength(6)
      expect(caught.party[2]).toMatchObject({ species: "cyndaquil", fainted: false, egg: false })
      expect((await game.storage.slot(0, 0)).mon?.species).toBe("geodude")
      expect(caught.bag.items.masterBall).toBe(0)

      await game.battle.startWild({ species: "rattata", level: 5 })
      await menu(game)
      expect((await game.state.read()).battle.trainerOnly.active).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("moves the last party member into a box without leaving a cursor-held Pokémon", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "cherrygrove-pokemon-center", x: 11, y: 2 } },
        party: [{ species: "rattata" }],
        pc: { currentBox: 0, observedSlots: [{ box: 0, slot: 0, mon: null }] },
        determinism: { textSpeed: "instant" },
      })
      await openPcStorage(game, "move")
      for (let row = 0; row < 5; row++) {
        await game.controls.press("down")
        await game.wait.frames(12)
      }
      await game.wait.until((state) => state.storage.cursor.area === 3, "PC party button")
      await game.controls.press("a")
      await game.wait.until(
        (state) => state.storage.ready && state.storage.cursor.area === 1,
        "PC party panel",
      )
      await game.controls.press("a")
      await game.wait.until((state) => state.storage.ui === "mon-menu", "last member Move menu")
      await game.wait.frames(30)
      await game.controls.press("a")
      await game.wait.until(
        (state) => state.storage.ready && state.storage.movingMon,
        "last member on cursor",
      )
      await game.controls.press("b")
      await game.wait.until(
        (state) => state.storage.ready && state.storage.cursor.area === 0,
        "box with held last member",
      )
      await game.controls.press("a")
      await game.wait.until((state) => state.storage.ui === "mon-menu", "place last member menu")
      await game.wait.frames(30)
      await game.controls.press("a")
      await game.wait.until(
        (state) => state.storage.ready && !state.storage.movingMon,
        "last member placed",
      )
      expect((await game.state.read()).party).toHaveLength(0)
      expect((await game.storage.slot(0, 0)).mon?.species).toBe("rattata")
      await closePcStorage(game)
      await game.saveAndReload()
      expect((await game.state.read()).party).toHaveLength(0)
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await menu(game)
      expect((await game.state.read()).battle.trainerOnly.active).toBe(true)
    } finally {
      await game.close()
    }
  })
  it("loses a real ordinary wild battle once and continues unhealed at the same field location", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [{ species: "rattata", level: 1, moves: ["tackle"] }],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const before = await game.state.read()
      await game.battle.startWild({ species: "pidgey", level: 100, moves: ["tackle"] })
      await menu(game)
      expect((await game.state.read()).battle.trainerOnly.active).toBe(false)
      await select(game, 0)
      await game.wait.until((state) => state.battle.ui === "move-menu", "ordinary move selection")
      await game.controls.press("a")
      await advance(game, (state) => state.ready && !state.battle.active)
      const lost = await game.state.read()
      expect(lost.map).toEqual(before.map)
      expect(lost.player).toEqual(before.player)
      expect(lost.partyVitals).toEqual([{ hp: 0, status: 0 }])
      expect(lost.money).toBe(before.money - Math.min(before.money, 8))
      expect(lost.battle.trainerOnly.active).toBe(false)
      await game.saveAndReload()
      expect((await game.state.read()).partyVitals).toEqual(lost.partyVitals)
      expect((await game.state.read()).money).toBe(lost.money)
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await menu(game)
      expect((await game.state.read()).battle.trainerOnly.active).toBe(true)
    } finally {
      await game.close()
    }
  })
})
