import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

const tommyPosition = {
  map: "sevii-one-island-kindle-road",
  x: 14,
  y: 25,
} as const

const kindleRoad = tommyPosition.map

const arrangeAtTommy = async (
  game: GameSession,
  party: Parameters<GameSession["arrange"]>[0]["party"],
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "right", position: tommyPosition },
    party,
    determinism: { rngSeed: 1, textSpeed: "instant" },
  })
}

const waitForTrainerBattle = async (
  game: GameSession,
  description: string,
  interact = true,
): Promise<void> => {
  if (interact) await game.player.interact()
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not reach its action menu: ${JSON.stringify(await game.state.read())}`,
  )
}

const waitForDialogueText = async (
  game: GameSession,
  expectedText: string,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.dialogueOpen && state.dialogue.text.includes(expectedText)) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(`${description} was not shown: ${JSON.stringify(await game.state.read())}`)
}

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.battle.active) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not release the field: ${JSON.stringify(await game.state.read())}`,
  )
}

const finishTrainerVictory = async (
  game: GameSession,
  postBattleText: string,
  description: string,
): Promise<void> => {
  await game.battle.win()
  await waitForDialogueText(game, postBattleText, `${description} post-battle dialogue`)
  await game.controls.press("a")
  await finishFieldScript(game, description)
}

const finishBlackout = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 4_000; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active && state.map.name !== tommyPosition.map) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `Tommy loss did not finish its normal blackout: ${JSON.stringify(await game.state.read())}`,
  )
}

const chargeVsSeekerOnKindleRoad = async (game: GameSession): Promise<void> => {
  for (let cycle = 0; cycle < 50; cycle++) {
    await game.player.move("left")
    await game.wait.until(
      (state) => state.ready && state.player.x === 7 && state.player.y === 69,
      `Vs Seeker charge step ${cycle * 2 + 1}`,
    )
    await game.player.move("right")
    await game.wait.until(
      (state) => state.ready && state.player.x === 8 && state.player.y === 69,
      `Vs Seeker charge step ${cycle * 2 + 2}`,
    )
  }
}

const useVsSeekerFromBag = async (game: GameSession, firstUse: boolean): Promise<void> => {
  await game.controls.press("start")
  await game.wait.until((state) => state.ui.mode === "pause-menu", "open Vs Seeker pause menu")
  await game.wait.frames(30)

  // A fresh checkpoint opens the pause menu on POKéMON. Once BAG has been
  // selected, both the pause-menu cursor and Key Items pocket are retained.
  if (firstUse) await game.controls.press("down")
  await game.controls.press("a")
  await game.wait.frames(120)
  if (firstUse) {
    await game.controls.press("right")
    await game.wait.frames(90)
  }

  // Select the sole arranged Key Item, then USE from its context menu.
  await game.controls.press("a")
  await game.wait.frames(30)
  await game.controls.press("a")
  await game.wait.frames(180)
  await finishFieldScript(game, "Vs Seeker use")
}

describe.sequential("Wayfarer Sevii ordinary Trainers", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("starts Garrett's Kindle Road battle from his sight line", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: kindleRoad, x: 19, y: 81 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    await game.player.move("up")
    await waitForTrainerBattle(game, "Garrett's Kindle Road sight encounter", false)
    await expect(game.state.read()).resolves.toMatchObject({
      player: { x: 19, y: 80 },
      battle: { active: true, enemy: { species: "shellder" } },
    })
    await finishTrainerVictory(game, "Instead of using SURF", "Garrett victory")
  })

  it("denies the Crush Kin with one usable non-Egg, then starts their pair battle", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: kindleRoad, x: 8, y: 69 } },
      party: [
        { species: "lapras", level: 100, moves: ["surf"] },
        { species: "pidgey", level: 100, moves: ["tackle"], egg: true },
      ],
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    await game.player.interact()
    await waitForDialogueText(game, "Bring two or more", "Crush Kin party-size denial")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
    await game.controls.press("a")
    await finishFieldScript(game, "Crush Kin party-size denial")

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: kindleRoad, x: 8, y: 69 } },
      party: [
        { species: "lapras", level: 100, moves: ["surf"] },
        { species: "pidgey", level: 100, moves: ["tackle"] },
      ],
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    await waitForTrainerBattle(game, "Crush Kin pair encounter")
    await expect(game.state.read()).resolves.toMatchObject({
      battle: { active: true, enemy: { species: "machoke" } },
    })
    await finishTrainerVictory(game, "How could my combination", "Crush Kin victory")
  })

  it("keeps representative Three, Five, and Six Island defeats across map reloads", async () => {
    const encounters = [
      {
        name: "Violet",
        position: { map: "sevii-three-island-bond-bridge", x: 67, y: 10 },
        facing: "right",
        lead: "bulbasaur",
        postBattleText: "eventually reach BERRY FOREST",
      },
      {
        name: "Laura",
        position: { map: "sevii-five-island-lost-cave-room4", x: 6, y: 5 },
        facing: "up",
        lead: "natu",
        postBattleText: "Earlier, a lady went",
      },
      {
        name: "Garret",
        position: { map: "sevii-six-island-pattern-bush", x: 51, y: 7 },
        facing: "up",
        lead: "heracross",
        postBattleText: "measures HERACROSS",
      },
    ] as const

    for (const encounter of encounters) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: encounter.facing, position: encounter.position },
        party: [{ species: "lapras", level: 100, moves: ["surf"] }],
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })

      await waitForTrainerBattle(game, `${encounter.name} encounter`)
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: encounter.position.map },
        battle: { active: true, enemy: { species: encounter.lead } },
      })
      await finishTrainerVictory(game, encounter.postBattleText, `${encounter.name} victory`)

      await game.player.warp(
        encounter.position.map,
        encounter.position.x,
        encounter.position.y,
        encounter.facing,
      )
      await game.player.interact()
      await waitForDialogueText(
        game,
        encounter.postBattleText,
        `${encounter.name} post-reload dialogue`,
      )
      await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
      await game.controls.press("a")
      await finishFieldScript(game, `${encounter.name} post-reload dialogue`)
    }
  })

  it("charges the Vs Seeker and advances through every authored Crush Kin party", async () => {
    const rematchGame = await GameSession.launch()
    try {
      await rematchGame.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: kindleRoad, x: 8, y: 69 } },
        party: [
          { species: "lapras", level: 100, moves: ["surf"] },
          { species: "pidgey", level: 100, moves: ["tackle"] },
        ],
        bag: { items: { vsSeeker: 1 } },
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })

      await waitForTrainerBattle(rematchGame, "base Crush Kin battle")
      const base = await rematchGame.state.read()
      expect(base.battle.enemy).toMatchObject({ species: "machoke" })
      await finishTrainerVictory(rematchGame, "How could my combination", "base Crush Kin battle")

      await chargeVsSeekerOnKindleRoad(rematchGame)
      await useVsSeekerFromBag(rematchGame, true)
      await rematchGame.player.interact()
      await waitForDialogueText(rematchGame, "We'll prove it", "first Crush Kin rematch intro")
      await waitForTrainerBattle(rematchGame, "first Crush Kin rematch", false)
      const firstRematch = await rematchGame.state.read()
      expect(firstRematch.battle.enemy).toMatchObject({ species: "machoke" })
      expect(firstRematch.battle.enemy!.level).toBeGreaterThan(base.battle.enemy!.level)
      await finishTrainerVictory(rematchGame, "How could my combination", "first Crush Kin rematch")

      await chargeVsSeekerOnKindleRoad(rematchGame)
      await useVsSeekerFromBag(rematchGame, false)
      await rematchGame.player.interact()
      await waitForDialogueText(rematchGame, "We'll prove it", "final Crush Kin rematch intro")
      await waitForTrainerBattle(rematchGame, "final Crush Kin rematch", false)
      await expect(rematchGame.state.read()).resolves.toMatchObject({
        battle: { active: true, enemy: { species: "machamp" } },
      })
      await finishTrainerVictory(rematchGame, "How could my combination", "final Crush Kin rematch")
    } finally {
      await rematchGame.close()
    }
  })

  it("routes exterior Psychic Dario through an ordinary sight battle", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "sevii-seven-island-trainer-tower", x: 56, y: 24 },
      },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    await game.player.move("down")
    await waitForTrainerBattle(game, "Dario sight encounter", false)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "sevii-seven-island-trainer-tower" },
      battle: { active: true, enemy: { species: "girafarig" } },
    })
    await finishTrainerVictory(game, "In your future", "Dario ordinary victory")
  })

  it("keeps Fisherman Tommy defeated and talkable after saving and reloading", async () => {
    await arrangeAtTommy(game, [{ species: "lapras", level: 100, moves: ["surf"] }])

    await waitForTrainerBattle(game, "Fisherman Tommy talk encounter")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: tommyPosition.map },
      battle: { active: true, enemy: { species: "goldeen" } },
    })
    await finishTrainerVictory(game, "Not only did I lose", "Tommy victory")

    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: tommyPosition.map },
      battle: { active: false },
    })

    await game.player.interact()
    await waitForDialogueText(game, "Not only did I lose", "reloaded Tommy post-battle dialogue")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
  })

  it("uses a normal blackout and leaves Tommy available after a loss", async () => {
    await arrangeAtTommy(game, [{ species: "rattata", level: 1, moves: ["tackle"] }])

    await waitForTrainerBattle(game, "first Fisherman Tommy attempt")
    await game.battle.lose()
    await finishBlackout(game)

    const recovered = await game.state.read()
    expect(recovered.map.name).not.toBe(tommyPosition.map)
    expect(recovered.party.every((mon) => !mon.fainted)).toBe(true)

    await game.player.warp(tommyPosition.map, tommyPosition.x, tommyPosition.y, "right")
    await waitForTrainerBattle(game, "Fisherman Tommy retry after blackout")
    await expect(game.state.read()).resolves.toMatchObject({
      battle: { active: true, enemy: { species: "goldeen" } },
    })
  })
})
