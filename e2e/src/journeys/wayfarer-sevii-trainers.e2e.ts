import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

const tommyPosition = {
  map: "sevii-one-island-kindle-road",
  x: 15,
  y: 26,
} as const

const kindleRoad = tommyPosition.map

const arrangeAtTommy = async (
  game: GameSession,
  party: Parameters<GameSession["arrange"]>[0]["party"],
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "up", position: tommyPosition },
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
    if (
      state.battle.ui === "text" ||
      state.dialogueOpen ||
      state.scriptActive ||
      state.controlsLocked
    ) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not reach its action menu: ${JSON.stringify(await game.state.read())}`,
  )
}

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.battle.active) return
    if (state.battle.active || state.dialogueOpen || state.scriptActive || state.controlsLocked) {
      await game.wait.frames(20)
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
  await finishFieldScript(game, description)
  await game.player.interact()
  await finishFieldScript(game, `${description} post-battle dialogue`)
  expect((await game.state.read()).dialogue.text).toContain(postBattleText)
}

const readTrainerOpponentId = async (game: GameSession): Promise<number> => {
  const symbols = await readSkyEmuSymbols(requireSymbolsPath())
  const client = (game as unknown as { running: RunningSkyEmu }).running.client
  const parameter = await client.readBytes(symbols.address("gTrainerBattleParameter") + 2, 2)
  return new DataView(parameter.buffer, parameter.byteOffset, 2).getUint16(0, true)
}

const finishBlackout = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 4_000; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active && state.map.name !== tommyPosition.map) return
    if (state.battle.active || state.dialogueOpen || state.scriptActive) {
      await game.wait.frames(20)
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `Tommy loss did not finish its normal blackout: ${JSON.stringify(await game.state.read())}`,
  )
}

const chargeVsSeekerOnKindleRoad = async (game: GameSession): Promise<void> => {
  // Map loads reset the in-bag counter, so charge on Sharon's nearby
  // north-south lane. Staying off their sight line also lets both response
  // animations finish before the player initiates the rematch.
  await game.player.warp(kindleRoad, 10, 55, "up")
  for (let step = 1; step <= 100; step++) {
    const direction = step % 2 === 1 ? "up" : "down"
    const y = direction === "up" ? 54 : 55
    await game.controls.press(direction, { holdFrames: 10, releaseFrames: 10 })
    await game.wait.until(
      (state) => state.ready && state.player.x === 10 && state.player.y === y,
      `Vs Seeker charge step ${step}`,
    )
  }
  await game.wait.frames(120)
}

const approachSharon = async (game: GameSession): Promise<void> => {
  const symbols = await readSkyEmuSymbols(requireSymbolsPath())
  const client = (game as unknown as { running: RunningSkyEmu }).running.client
  let trainerY: number | undefined
  for (let index = 0; index < 16; index++) {
    const object = await client.readBytes(symbols.address("gObjectEvents") + index * 0x24, 0x24)
    if ((object[0]! & 1) !== 0 && object[8] === 6) {
      trainerY =
        new DataView(object.buffer, object.byteOffset, object.byteLength).getInt16(0x12, true) - 7
      break
    }
  }
  if (trainerY === undefined) throw new Error("Sharon's active object was not found")

  const approachY = trainerY + 1
  for (;;) {
    const state = await game.state.read()
    if (state.player.y === approachY) break
    const direction = state.player.y > approachY ? "up" : "down"
    await game.controls.press(direction, { holdFrames: 10, releaseFrames: 10 })
  }
  // Enter Sharon's south-facing sight line from the adjacent lane.
  await game.controls.press("left", { holdFrames: 10, releaseFrames: 10 })
}

const waitForVsSeekerResponses = async (game: GameSession): Promise<void> => {
  const symbols = await readSkyEmuSymbols(requireSymbolsPath())
  const client = (game as unknown as { running: RunningSkyEmu }).running.client
  let idleSamples = 0
  for (let attempt = 0; attempt < 300; attempt++) {
    let responseTaskActive = false
    for (let index = 0; index < 16; index++) {
      const task = await client.readBytes(symbols.address("gTasks") + index * 0x28, 5)
      const func = new DataView(task.buffer, task.byteOffset, task.byteLength).getUint32(0, true)
      if (task[4] !== 0 && (func & ~1) === symbols.address("ScriptMovement_MoveObjects")) {
        responseTaskActive = true
        break
      }
    }

    let responseIconActive = false
    for (let index = 0; index < 65; index++) {
      const sprite = await client.readBytes(symbols.address("gSprites") + index * 0x44 + 0x1c, 4)
      const callback = new DataView(sprite.buffer, sprite.byteOffset, 4).getUint32(0, true)
      if ((callback & ~1) === symbols.address("SpriteCB_TrainerIcons")) {
        responseIconActive = true
        break
      }
    }

    let sharonMovementActive = false
    for (let index = 0; index < 16; index++) {
      const object = await client.readBytes(symbols.address("gObjectEvents") + index * 0x24, 9)
      if ((object[0]! & 1) !== 0 && object[8] === 6) {
        sharonMovementActive = (object[0]! & 0x42) !== 0
        break
      }
    }

    if (!responseTaskActive && !responseIconActive && !sharonMovementActive) idleSamples++
    else idleSamples = 0
    if (idleSamples >= 30) return
    await game.wait.frames(4)
  }
  throw new Error("Vs Seeker response animations did not finish")
}

const useVsSeekerFromBag = async (game: GameSession, firstUse: boolean): Promise<void> => {
  await game.controls.press("start", { releaseFrames: 30 })
  await game.wait.until((state) => state.ui.mode === "pause-menu", "open Vs Seeker pause menu")
  await game.wait.frames(30)

  // A fresh checkpoint opens the pause menu on POKéMON. Once BAG has been
  // selected, both the pause-menu cursor and Key Items pocket are retained.
  if (firstUse) {
    await game.controls.press("down", { releaseFrames: 30 })
  }
  await game.controls.press("a", { releaseFrames: 60 })
  await game.wait.frames(120)
  if (firstUse) {
    await game.controls.press("left", { releaseFrames: 30 })
    await game.wait.frames(90)
  }

  // Select the sole arranged Key Item, then USE from its context menu.
  await game.controls.press("a", { releaseFrames: 30 })
  await game.wait.frames(30)
  await game.controls.press("a", { releaseFrames: 60 })
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
    const battle = await game.state.read()
    expect(battle).toMatchObject({
      player: { x: 19, y: 80 },
      battle: { active: true },
    })
    expect(battle.battle.enemy!.level).toBeGreaterThan(0)
    await finishTrainerVictory(game, "Instead of using SURF", "Garrett victory")
  })

  it("denies the Crush Kin with one usable non-Egg, then starts their pair battle", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "right", position: { map: kindleRoad, x: 7, y: 68 } },
      party: [
        { species: "lapras", level: 100, moves: ["surf"] },
        { species: "pidgey", level: 100, moves: ["tackle"], egg: true },
      ],
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    await game.player.interact()
    await finishFieldScript(game, "Crush Kin party-size denial")
    expect((await game.state.read()).dialogue.text).toContain("do you want to battle")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
    await game.controls.press("a")
    await finishFieldScript(game, "Crush Kin party-size denial")

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "right", position: { map: kindleRoad, x: 7, y: 68 } },
      party: [
        { species: "lapras", level: 100, moves: ["surf"] },
        { species: "pidgey", level: 100, moves: ["tackle"] },
      ],
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    await waitForTrainerBattle(game, "Crush Kin pair encounter")
    const pairBattle = await game.state.read()
    expect(pairBattle).toMatchObject({ battle: { active: true } })
    expect(pairBattle.battle.enemy!.level).toBeGreaterThan(0)
    await finishTrainerVictory(game, "How could my combination", "Crush Kin victory")
  })

  it("keeps representative Three, Five, and Six Island defeats across map reloads", async () => {
    const encounters = [
      {
        name: "Violet",
        position: { map: "sevii-three-island-bond-bridge", x: 68, y: 9 },
        facing: "down",
        postBattleText: "If you keep going this way",
      },
      {
        name: "Laura",
        position: { map: "sevii-five-island-lost-cave-room4", x: 5, y: 4 },
        facing: "right",
        postBattleText: "Earlier, a lady went",
      },
      {
        name: "Garret",
        position: { map: "sevii-six-island-pattern-bush", x: 50, y: 6 },
        facing: "right",
        postBattleText: "Theres a girl near the BUSH",
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
      const battle = await game.state.read()
      expect(battle).toMatchObject({
        map: { name: encounter.position.map },
        battle: { active: true },
      })
      expect(battle.battle.enemy!.level).toBeGreaterThan(0)
      await finishTrainerVictory(game, encounter.postBattleText, `${encounter.name} victory`)

      await game.player.warp(
        encounter.position.map,
        encounter.position.x,
        encounter.position.y,
        encounter.facing,
      )
      await game.player.interact()
      await finishFieldScript(game, `${encounter.name} post-reload dialogue`)
      expect((await game.state.read()).dialogue.text).toContain(encounter.postBattleText)
      await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
    }
  })

  it("charges the Vs Seeker and advances through every authored Sharon party", async () => {
    const rematchGame = await GameSession.launch()
    try {
      await rematchGame.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "left", position: { map: kindleRoad, x: 10, y: 55 } },
        party: [{ species: "lapras", level: 100, moves: ["surf"] }],
        bag: { items: { vsSeeker: 1 } },
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })

      await rematchGame.controls.press("left", { holdFrames: 10, releaseFrames: 10 })
      await waitForTrainerBattle(rematchGame, "base Sharon battle", false)
      const base = await rematchGame.state.read()
      expect(base.battle.enemy!.level).toBeGreaterThan(0)
      const baseOpponentId = await readTrainerOpponentId(rematchGame)
      await finishTrainerVictory(rematchGame, "clear that youre skilled", "base Sharon battle")

      await chargeVsSeekerOnKindleRoad(rematchGame)
      await useVsSeekerFromBag(rematchGame, true)
      await waitForVsSeekerResponses(rematchGame)
      await approachSharon(rematchGame)
      await waitForTrainerBattle(rematchGame, "first Sharon rematch", false)
      const firstRematch = await rematchGame.state.read()
      expect(firstRematch.dialogue.text).toContain("help me out with my")
      expect(firstRematch.battle.enemy!.level).toBeGreaterThan(base.battle.enemy!.level)
      const firstRematchOpponentId = await readTrainerOpponentId(rematchGame)
      expect(firstRematchOpponentId).not.toBe(baseOpponentId)
      await finishTrainerVictory(rematchGame, "clear that youre skilled", "first Sharon rematch")

      await chargeVsSeekerOnKindleRoad(rematchGame)
      await useVsSeekerFromBag(rematchGame, false)
      await waitForVsSeekerResponses(rematchGame)
      await approachSharon(rematchGame)
      await waitForTrainerBattle(rematchGame, "final Sharon rematch", false)
      const finalRematch = await rematchGame.state.read()
      expect(finalRematch.dialogue.text).toContain("help me out with my")
      expect(finalRematch).toMatchObject({ battle: { active: true } })
      expect(finalRematch.battle.enemy!.level).toBeGreaterThan(0)
      expect(await readTrainerOpponentId(rematchGame)).not.toBe(firstRematchOpponentId)
      await finishTrainerVictory(rematchGame, "clear that youre skilled", "final Sharon rematch")
    } finally {
      await rematchGame.close()
    }
  })

  it("routes exterior Psychic Dario through an ordinary sight battle", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "right",
        position: { map: "sevii-seven-island-trainer-tower", x: 55, y: 26 },
      },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    await waitForTrainerBattle(game, "Dario exterior encounter")
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
    await finishFieldScript(game, "reloaded Tommy post-battle dialogue")
    expect((await game.state.read()).dialogue.text).toContain("Not only did I lose")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
  })

  it("uses a normal blackout and leaves Tommy available after a loss", async () => {
    await arrangeAtTommy(game, [{ species: "lapras", level: 100, moves: ["surf"] }])

    await waitForTrainerBattle(game, "first Fisherman Tommy attempt")
    await game.battle.lose()
    await finishBlackout(game)

    const recovered = await game.state.read()
    expect(recovered.map.name).not.toBe(tommyPosition.map)
    expect(recovered.party.every((mon) => !mon.fainted)).toBe(true)

    await game.player.warp(tommyPosition.map, tommyPosition.x, tommyPosition.y, "up")
    await waitForTrainerBattle(game, "Fisherman Tommy retry after blackout")
    await expect(game.state.read()).resolves.toMatchObject({
      battle: { active: true, enemy: { species: "goldeen" } },
    })
  })
})
