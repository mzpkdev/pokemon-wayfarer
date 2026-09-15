import { afterEach, beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

describe("Wayfarer Trainer Tower run lifecycle", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
  })

  afterEach(async () => {
    await game.close()
  })

  it("heals and restores the entry snapshot while discarding an unsaved active run", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      party: [
        { species: "lapras", level: 55 },
        { species: "pidgey", level: 40 },
      ],
    })
    await game.saveAndReload()

    const savedVitals = (await game.state.read()).partyVitals
    expect(savedVitals).toHaveLength(2)
    expect(savedVitals.every(({ hp, status }) => hp > 1 && status === 0)).toBe(true)

    await game.trainerTower.start("mixed")
    await expect(game.state.read()).resolves.toMatchObject({
      trainerTower: {
        active: true,
        saveAllowed: false,
        bestTimes: [0, 0, 0, 0],
        pendingPrize: 0,
        completedMask: 0,
      },
    })

    await game.trainerTower.damageParty(1, 8)
    expect((await game.state.read()).partyVitals).toEqual([
      { hp: 1, status: 8 },
      { hp: 1, status: 8 },
    ])

    await game.resetAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      partyVitals: savedVitals,
      trainerTower: { active: false, saveAllowed: true },
    })

    await game.trainerTower.start("mixed")
    await game.trainerTower.damageParty(1, 8)
    await game.trainerTower.abandon()
    await expect(game.state.read()).resolves.toMatchObject({
      partyVitals: savedVitals,
      trainerTower: {
        active: false,
        saveAllowed: true,
        bestTimes: [0, 0, 0, 0],
        pendingPrize: 0,
        completedMask: 0,
      },
    })
  })
})
