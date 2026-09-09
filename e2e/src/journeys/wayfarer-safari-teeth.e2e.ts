import * as fs from "node:fs"
import { beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction, type GameMap } from "../harness/game-session"

const settle = async (game: GameSession, map: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 120; attempt++) {
    await game.wait.frames(20)
    const state = await game.state.read()
    if (state.ready && state.map.name === map) return
    await game.controls.press("a")
  }
  throw new Error(`Safari interaction did not settle: ${JSON.stringify(await game.state.read())}`)
}

const interact = async (game: GameSession, map: GameMap): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await settle(game, map)
}

const assertOtherProgress = async (game: GameSession, research = 3): Promise<void> => {
  expect(await game.story.var("baobaResearch")).toBe(research)
  expect(await game.story.var("kantoSafariProgress")).toBe(1)
  expect(await game.story.flag("receivedExistingStrength")).toBe(false)
}

const walk = async (game: GameSession, route: string): Promise<void> => {
  const directions: Record<string, Direction> = { N: "up", S: "down", E: "right", W: "left" }
  const offsets: Record<Direction, [number, number]> = {
    up: [0, -1],
    down: [0, 1],
    right: [1, 0],
    left: [-1, 0],
  }
  for (const segment of route.split(" ")) {
    const direction = directions[segment[0]!]!
    for (let step = 0; step < Number(segment.slice(1)); step++) {
      const before = await game.state.read()
      const [dx, dy] = offsets[direction]
      const target = { x: before.player.x + dx, y: before.player.y + dy }
      // Roaming Safari actors can briefly occupy an otherwise open path.
      for (let attempt = 0; attempt < 40; attempt++) {
        await game.controls.press(direction)
        await game.wait.frames(20)
        const state = await game.state.read()
        if (state.player.x === target.x && state.player.y === target.y) break
        expect(state.battle.active).toBe(false)
      }
      expect((await game.state.read()).player).toMatchObject(target)
    }
  }
}

const enterSafari = async (game: GameSession): Promise<void> => {
  await game.controls.press("up")
  await game.dialogue.waitForOpen()
  await settle(game, "fuchsia-safari-beach")
  expect(await game.story.flag("inKantoSafari")).toBe(true)
  expect(await game.story.var("safariSession")).toBe(2)
  await walk(game, "N1")
  expect((await game.state.read()).player).toMatchObject({ x: 20, y: 38 })
}

const exitSafari = async (game: GameSession): Promise<void> => {
  await game.player.warp("fuchsia-safari-beach", 20, 38, "down")
  await game.controls.press("down")
  await game.wait.frames(20)
  await game.controls.press("down")
  await settle(game, "fuchsia-safari-entrance")
  expect(await game.story.flag("inKantoSafari")).toBe(false)
  expect(await game.story.var("safariSession")).toBe(1)
  expect((await game.state.read()).player).toMatchObject({ x: 5, y: 7 })
  await walk(game, "N1")
}

const routes = {
  surf: "E1 N5 E1 N2 W1 N1 W2 N6 W15",
  teeth: "E1 N5 E1 N3 E5 N1 E2 S2 E3 N1 E3 N4 E2 N2 E2",
}

describe.sequential("Wayfarer Fuchsia Safari and Gold Teeth", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("gives Surf without Teeth, badges, or advancing Safari research", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-safari-beach", x: 4, y: 24 }, facing: "up" },
      story: { vars: { baobaResearch: 3, kantoSafariProgress: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await fs.promises.writeFile("/tmp/wayfarer-safari-surf.png", await game.screenshot())
    expect(await game.story.flag("fuchsiaSurfReceived")).toBe(false)
    await interact(game, "fuchsia-safari-beach")
    // Snapshot quantities watch fixture items; newly earned rewards need a live Bag read.
    expect(await game.inventory.contains("surf")).toBe(true)
    expect(await game.story.flag("fuchsiaSurfReceived")).toBe(true)
    expect(await game.story.flag("fuchsiaGoldTeethClaimed")).toBe(false)
    expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(false)
    await assertOtherProgress(game)
    await interact(game, "fuchsia-safari-beach")
    expect(await game.inventory.contains("surf")).toBe(true)
    await game.saveAndReload()
    await game.controls.press("up")
    await interact(game, "fuchsia-safari-beach")
    expect(await game.inventory.contains("surf")).toBe(true)
  })

  it("returns Teeth for Strength without Surf and preserves the result across reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-safari-beach", x: 39, y: 24 }, facing: "up" },
      story: { vars: { baobaResearch: 3, kantoSafariProgress: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await fs.promises.writeFile("/tmp/wayfarer-safari-teeth.png", await game.screenshot())
    await interact(game, "fuchsia-safari-beach")
    expect(await game.inventory.contains("goldTeeth")).toBe(true)
    expect(await game.story.flag("fuchsiaGoldTeethClaimed")).toBe(true)
    expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(false)
    await game.saveAndReload()
    await game.player.warp("fuchsia-wardens-house", 8, 3, "up")
    await fs.promises.writeFile("/tmp/wayfarer-safari-baoba.png", await game.screenshot())
    await interact(game, "fuchsia-wardens-house")
    expect(await game.inventory.contains("goldTeeth")).toBe(false)
    expect(await game.inventory.contains("strength")).toBe(true)
    expect(await game.inventory.contains("surf")).toBe(false)
    expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(true)
    await assertOtherProgress(game)
    await interact(game, "fuchsia-wardens-house")
    expect(await game.inventory.contains("strength")).toBe(true)
    await game.saveAndReload()
    await game.controls.press("up")
    await interact(game, "fuchsia-wardens-house")
    expect(await game.inventory.contains("strength")).toBe(true)
    await game.player.warp("fuchsia-safari-beach", 39, 24, "up")
    await game.player.interact()
    await game.wait.frames(30)
    expect((await game.state.read()).ready).toBe(true)
    expect(await game.inventory.contains("goldTeeth")).toBe(false)
  })

  it("reconciles pre-owned Teeth at the pickup without requiring Key Items space", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-safari-beach", x: 39, y: 24 }, facing: "up" },
      bag: { items: { goldTeeth: 1 }, fullPockets: ["keyItems"] },
      determinism: { textSpeed: "instant" },
    })
    expect(await game.story.flag("fuchsiaGoldTeethClaimed")).toBe(false)
    await interact(game, "fuchsia-safari-beach")
    expect(await game.inventory.contains("goldTeeth")).toBe(true)
    expect(await game.story.flag("fuchsiaGoldTeethClaimed")).toBe(true)
    expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(false)
    await game.saveAndReload()
    await game.player.interact()
    await game.wait.frames(30)
    expect((await game.state.read()).ready).toBe(true)
  })

  it("keeps the Teeth pickup available with a full Key Items pocket", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-safari-beach", x: 39, y: 24 }, facing: "up" },
      bag: { fullPockets: ["keyItems"] },
      determinism: { textSpeed: "instant" },
    })
    for (let attempt = 0; attempt < 2; attempt++) {
      await game.controls.press("up")
      await interact(game, "fuchsia-safari-beach")
      expect(await game.inventory.contains("goldTeeth")).toBe(false)
      expect(await game.story.flag("fuchsiaGoldTeethClaimed")).toBe(false)
      await game.saveAndReload()
    }
  })

  it("keeps Surf pending with a full HM pocket", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-safari-beach", x: 4, y: 24 }, facing: "up" },
      bag: { fullPockets: ["tmHm"] },
      determinism: { textSpeed: "instant" },
    })
    for (let attempt = 0; attempt < 2; attempt++) {
      await game.controls.press("up")
      await interact(game, "fuchsia-safari-beach")
      expect(await game.inventory.contains("surf")).toBe(false)
      expect(await game.story.flag("fuchsiaSurfReceived")).toBe(false)
      await game.saveAndReload()
    }
  })

  it("retains the Teeth and pending Strength delivery when the HM pocket is full", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-wardens-house", x: 8, y: 3 }, facing: "up" },
      bag: { items: { goldTeeth: 1 }, fullPockets: ["tmHm"] },
      determinism: { textSpeed: "instant" },
    })
    for (let attempt = 0; attempt < 2; attempt++) {
      await game.controls.press("up")
      await interact(game, "fuchsia-wardens-house")
      expect(await game.inventory.contains("goldTeeth")).toBe(true)
      expect(await game.inventory.contains("strength")).toBe(false)
      expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(false)
      expect(await game.story.flag("fuchsiaGoldTeethClaimed")).toBe(false)
      await game.saveAndReload()
    }
  })

  it("reconciles pre-owned Surf only at the local attendant", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-safari-beach", x: 4, y: 24 }, facing: "up" },
      bag: { hms: { surf: 1 }, fullPockets: ["tmHm"] },
      determinism: { textSpeed: "instant" },
    })
    expect(await game.story.flag("fuchsiaSurfReceived")).toBe(false)
    await interact(game, "fuchsia-safari-beach")
    expect(await game.story.flag("fuchsiaSurfReceived")).toBe(true)
    expect((await game.state.read()).bag.hms.surf).toBe(1)
    expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(false)
  })

  it("requires Teeth even when Strength is already owned", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-wardens-house", x: 8, y: 3 }, facing: "up" },
      bag: { hms: { strength: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await interact(game, "fuchsia-wardens-house")
    expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(false)
    expect(await game.story.flag("fuchsiaGoldTeethClaimed")).toBe(false)
    expect((await game.state.read()).bag.hms.strength).toBe(1)
  })

  it("accepts pre-owned Teeth with pre-owned Strength without duplicating either", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fuchsia-wardens-house", x: 8, y: 3 }, facing: "up" },
      bag: { hms: { strength: 1 }, items: { goldTeeth: 1 }, fullPockets: ["tmHm"] },
      determinism: { textSpeed: "instant" },
    })
    expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(false)
    await interact(game, "fuchsia-wardens-house")
    expect(await game.inventory.contains("goldTeeth")).toBe(false)
    expect((await game.state.read()).bag.hms.strength).toBe(1)
    expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(true)
    expect(await game.story.flag("fuchsiaGoldTeethClaimed")).toBe(true)
    await game.saveAndReload()
    await game.player.warp("fuchsia-safari-beach", 39, 24, "up")
    await game.player.interact()
    await game.wait.frames(30)
    expect((await game.state.read()).ready).toBe(true)
    expect(await game.inventory.contains("goldTeeth")).toBe(false)
  })
  for (const first of ["surf", "teeth"] as const) {
    it(`walks to both rewards in paid Safari sessions, ${first} first`, async () => {
      const research = first === "surf" ? 0 : 3
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          position:
            first === "surf"
              ? { map: "fuchsia-city", x: 26, y: 6 }
              : { map: "fuchsia-safari-entrance", x: 5, y: 6 },
          facing: "up",
        },
        party: [{ species: "lapras", level: 100 }],
        bag: { items: first === "surf" ? {} : { pokeblockCase: 1 } },
        story: {
          vars: {
            baobaResearch: research,
            kantoSafariProgress: first === "surf" ? 0 : 1,
            repelStepCount: 250,
          },
        },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      if (first === "surf") {
        expect(await game.story.var("kantoSafariProgress")).toBe(0)
        expect(await game.inventory.contains("pokeblockCase")).toBe(false)
        await game.controls.press("up")
        await game.dialogue.waitForOpen()
        await settle(game, "fuchsia-safari-entrance")
        expect(await game.story.var("kantoSafariProgress")).toBe(1)
        expect(await game.story.var("baobaResearch")).toBe(0)
        expect(await game.inventory.contains("pokeblockCase")).toBe(true)
        const player = (await game.state.read()).player
        expect(player.x).toBe(5)
        if (player.y > 6) await walk(game, `N${player.y - 6}`)
      }
      const order = first === "surf" ? (["surf", "teeth"] as const) : (["teeth", "surf"] as const)
      for (const objective of order) {
        await enterSafari(game)
        await walk(game, routes[objective])
        await game.controls.press("up")
        await interact(game, "fuchsia-safari-beach")
        expect(await game.story.flag("inKantoSafari")).toBe(true)
        expect(await game.story.var("safariSession")).toBe(2)
        expect(await game.inventory.contains(objective === "surf" ? "surf" : "goldTeeth")).toBe(
          true,
        )
        await assertOtherProgress(game, research)
        await exitSafari(game)
        if (objective === "teeth") {
          await game.player.warp("fuchsia-wardens-house", 8, 3, "up")
          await interact(game, "fuchsia-wardens-house")
          expect(await game.inventory.contains("strength")).toBe(true)
          expect(await game.inventory.contains("goldTeeth")).toBe(false)
          await game.player.warp("fuchsia-safari-entrance", 5, 6, "up")
        }
      }
      expect(await game.story.flag("fuchsiaSurfReceived")).toBe(true)
      expect(await game.story.flag("fuchsiaStrengthReceived")).toBe(true)
      await assertOtherProgress(game, research)
    })
  }
})
