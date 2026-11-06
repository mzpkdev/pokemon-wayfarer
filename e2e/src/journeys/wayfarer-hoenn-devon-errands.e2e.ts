import { beforeAll, describe, expect, it } from "webanvil/test"

import { type GameSession as Session, GameSession } from "../harness/game-session"
import { stepOpening } from "../playbooks/regional-opening"

// The Devon errands and the Slateport harbor, in the orders the spec singles out: the theft with no
// badge, deliveries only from a player who holds the item, the museum door, the Parts handed over
// after the harbor scene, and a ferry passenger who lands while that scene is waiting.

const harborAttendant = 1
const harborStern = 4
const streetStern = 11
const queueGrunts = [5, 18]

const sees = async (game: Session, localId: number): Promise<boolean> =>
  (await game.state.read()).objects.some((object) => object.localId === localId && object.visible)

const expectFree = async (game: Session): Promise<void> => {
  await game.wait.frames(60)
  expect(await game.state.read()).toMatchObject({
    ready: true,
    controlsLocked: false,
    scriptActive: false,
    dialogueOpen: false,
  })
}

/** Reads a script through with A until `done`, then until the field is free again. Fails on any battle. */
const readThrough = async (
  game: Session,
  done: () => Promise<boolean>,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 120; attempt++) {
    await game.wait.frames(45)
    const state = await game.state.read()
    expect(state.battle.active).toBe(false)
    if (state.ready && !state.scriptActive && !state.controlsLocked && (await done())) return
    await game.controls.press("a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

const talk = async (game: Session, description: string): Promise<void> => {
  await game.player.interact()
  await game.wait.frames(30)
  await readThrough(game, async () => true, description)
}

describe.sequential("Wayfarer Hoenn Devon errands", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  const arrange = (
    position: { map: Parameters<GameSession["player"]["warp"]>[0]; x: number; y: number },
    facing: "up" | "down" | "left" | "right",
    fixture: Pick<Parameters<GameSession["arrange"]>[0], "bag"> & {
      vars?: NonNullable<NonNullable<Parameters<GameSession["arrange"]>[0]["story"]>["vars"]>
      flags?: NonNullable<NonNullable<Parameters<GameSession["arrange"]>[0]["story"]>["flags"]>
    } = {},
  ) =>
    game.arrange({
      checkpoint: "hoenn-before-rescue",
      player: { facing, position },
      story: {
        vars: fixture.vars ?? {},
        flags: { disableEncounters: true, disableTrainers: true, ...fixture.flags },
      },
      bag: fixture.bag ?? {},
      party: [{ species: "salamence", level: 60 }],
      determinism: { textSpeed: "instant" },
    })

  it("starts the Rustboro theft from the street with no badge, once", { timeout: 120_000 }, async () => {
    await arrange({ map: "rustboro-city", x: 22, y: 22 }, "right")
    await expect(game.story.var("hoennRustboroCityState")).resolves.toBe(0)

    await stepOpening(game, "right")
    await readThrough(
      game,
      async () => (await game.story.var("hoennRustboroCityState")) === 2,
      "the street scene did not play",
    )
    await expect(game.story.flag("hoennDevonGoodsStolen")).resolves.toBe(true)
    await expect(game.story.var("hoennRusturfTunnelState")).resolves.toBe(2)

    // Crossing again plays nothing.
    await stepOpening(game, "left")
    await stepOpening(game, "right")
    await expectFree(game)
    await expect(game.story.var("hoennRustboroCityState")).resolves.toBe(2)
  })

  it("keeps a late theft from rewinding an opened tunnel", { timeout: 120_000 }, async () => {
    await arrange({ map: "rustboro-city", x: 22, y: 22 }, "right", {
      vars: { hoennRusturfTunnelState: 6 },
    })
    await stepOpening(game, "right")
    await readThrough(
      game,
      async () => (await game.story.var("hoennRustboroCityState")) === 2,
      "the street scene did not play",
    )
    await expect(game.story.var("hoennRusturfTunnelState")).resolves.toBe(6)
  })

  it("has Steven take the Letter only from a player who holds it and has room", { timeout: 180_000 }, async () => {
    await arrange({ map: "granite-cave-stevens-room", x: 7, y: 9 }, "up")
    await talk(game, "Steven did not finish his first-meeting line")
    await expect(game.story.flag("hoennDeliveredStevenLetter")).resolves.toBe(false)
    await expect(game.inventory.contains("tmSteelWing")).resolves.toBe(false)

    await arrange({ map: "granite-cave-stevens-room", x: 7, y: 9 }, "up", {
      bag: { items: { letter: 1 }, fullPockets: ["tmHm"] },
    })
    await talk(game, "Steven did not turn the full Bag away")
    await expect(game.story.flag("hoennDeliveredStevenLetter")).resolves.toBe(false)
    await expect(game.inventory.contains("letter")).resolves.toBe(true)
    expect(await sees(game, 1)).toBe(true)

    await arrange({ map: "granite-cave-stevens-room", x: 7, y: 9 }, "up", {
      bag: { items: { letter: 1 } },
    })
    await game.player.interact()
    await readThrough(
      game,
      () => game.story.flag("hoennDeliveredStevenLetter"),
      "Steven did not take the Letter",
    )
    await expect(game.inventory.contains("letter")).resolves.toBe(false)
    await expect(game.inventory.count("tmSteelWing")).resolves.toBe(1)
  })

  it("has Stern take nothing from empty hands at the museum", { timeout: 120_000 }, async () => {
    await arrange({ map: "slateport-oceanic-museum-2f", x: 14, y: 6 }, "left")
    await talk(game, "Stern did not finish his first-meeting line")
    await expect(game.story.flag("hoennDeliveredDevonGoods")).resolves.toBe(false)
    await expect(game.story.var("hoennSlateportOutsideMuseumState")).resolves.toBe(0)
    expect(await sees(game, 1)).toBe(true)
  })

  it("opens the museum door from Dock on every branch", { timeout: 180_000 }, async () => {
    for (const hoennMindBadge of [false, true]) {
      await arrange({ map: "slateport-sterns-shipyard-1f", x: 5, y: 6 }, "up", {
        flags: { hoennMindBadge },
      })
      await game.player.interact()
      await game.wait.frames(30)
      await readThrough(game, () => game.story.flag("hoennHideSlateportTeamAqua"), "Dock's talk")

      await game.player.warp("slateport-city", 30, 29, "up")
      await game.wait.forReady()
      for (const grunt of queueGrunts) expect(await sees(game, grunt)).toBe(false)
      await stepOpening(game, "up")
      await stepOpening(game, "up")
      await stepOpening(game, "up")
      await game.wait.forMap("slateport-oceanic-museum-1f")
    }
  })

  it("takes the Parts at the harbor after its scene, with no confrontation", { timeout: 180_000 }, async () => {
    await arrange({ map: "slateport-city-harbor", x: 6, y: 14 }, "up", {
      vars: { hoennSlateportHarborState: 2, hoennSlateportCityState: 2, hoennScottState: 2 },
      flags: { hoennHideHarborCaptainStern: false, hoennHideSlateportTeamAqua: true },
      bag: { items: { devonParts: 1 } },
    })
    await game.player.interact()
    await readThrough(
      game,
      () => game.story.flag("hoennDeliveredDevonGoods"),
      "Stern did not take the Parts",
    )
    await expect(game.inventory.contains("devonParts")).resolves.toBe(false)
    await expect(game.story.var("hoennSlateportHarborState")).resolves.toBe(2)
    await expect(game.story.var("hoennScottState")).resolves.toBe(3)
    await expect(game.story.var("hoennSlateportOutsideMuseumState")).resolves.toBe(2)

    await game.saveAndReload()
    await expect(game.story.flag("hoennDeliveredDevonGoods")).resolves.toBe(true)

    // Stern stands in one place: the museum no longer shows him.
    await game.player.warp("slateport-oceanic-museum-2f", 14, 6, "left")
    await game.wait.forReady()
    expect(await sees(game, 1)).toBe(false)
  })

  it("lets a ferry passenger land ahead of Stern and still plays the harbor scene", { timeout: 300_000 }, async () => {
    // Magma Hideout is done and the Mind Badge came first: the street Stern waits at the harbor door.
    await arrange({ map: "slateport-city-harbor", x: 9, y: 11 }, "left", {
      vars: { hoennSlateportHarborState: 1, hoennSlateportCityState: 1 },
      flags: { hoennHideSlateportCaptainStern: false, hoennMindBadge: true },
      bag: { items: { devonParts: 1 } },
    })
    await game.player.warp("slateport-city-harbor", 9, 11, "left")
    await game.wait.forReady()
    expect(await sees(game, harborAttendant)).toBe(false)

    // The scene's tiles do nothing while its cast is still in town.
    await stepOpening(game, "left")
    await expectFree(game)
    await expect(game.story.var("hoennSlateportHarborState")).resolves.toBe(1)

    // Out by the door, around Stern, and into his street scene.
    await game.player.warp("slateport-city-harbor", 11, 14, "down")
    await game.wait.forReady()
    await stepOpening(game, "down")
    await game.wait.forMap("slateport-city")
    await expectFree(game)
    expect(await sees(game, streetStern)).toBe(true)
    // The door lets out on Stern's own tile, and the player walks off it.
    await stepOpening(game, "down")
    await game.player.move("up")
    await game.wait.frames(24)
    await game.player.interact()
    await game.wait.frames(30)
    await readThrough(
      game,
      async () => (await game.story.var("hoennSlateportCityState")) === 2,
      "the street scene did not play",
    )
    await game.wait.forMap("slateport-city-harbor")
    expect(await sees(game, harborStern)).toBe(true)

    // The Parts wait for the scene.
    await game.player.warp("slateport-city-harbor", 9, 12, "left")
    await game.wait.forReady()
    await stepOpening(game, "left")
    await readThrough(
      game,
      async () => (await game.story.var("hoennSlateportHarborState")) === 2,
      "the harbor scene did not play",
    )
    await expect(game.inventory.contains("devonParts")).resolves.toBe(true)
    await expect(game.story.flag("hoennHideSlateportTeamAqua")).resolves.toBe(true)

    // The attendant returns with the scene, on the player's first step, and the Mind Badge had nothing to do with it.
    await stepOpening(game, "down")
    await game.wait.frames(30)
    expect(await sees(game, harborAttendant)).toBe(true)

    // The Parts the scene left in the Bag now go to Stern, with no confrontation.
    await game.player.warp("slateport-city-harbor", 6, 14, "up")
    await game.wait.forReady()
    await game.player.interact()
    await readThrough(
      game,
      () => game.story.flag("hoennDeliveredDevonGoods"),
      "Stern did not take the Parts",
    )
    await expect(game.story.var("hoennSlateportHarborState")).resolves.toBe(2)
  })

  it("pays each delivery once in either order, across a save and travel", { timeout: 300_000 }, async () => {
    const giveTheLetter = async () => {
      await game.player.warp("granite-cave-stevens-room", 7, 9, "up")
      await game.wait.forReady()
      await game.player.interact()
      await readThrough(game, () => game.story.flag("hoennDeliveredStevenLetter"), "Steven's delivery")
    }
    const giveTheParts = async () => {
      await game.player.warp("slateport-oceanic-museum-2f", 14, 6, "left")
      await game.wait.forReady()
      await game.player.interact()
      let winning = false
      for (let attempt = 0; attempt < 200; attempt++) {
        await game.wait.frames(45)
        const state = await game.state.read()
        if (state.ready && !state.scriptActive && !state.controlsLocked && !state.battle.active) {
          if (await game.story.flag("hoennDeliveredDevonGoods")) return
        }
        if (!state.battle.active) winning = false
        if (state.battle.ui === "action-menu" && !winning) {
          winning = true
          await game.battle.win()
        } else await game.controls.press("a")
      }
      throw new Error(`Stern's delivery: ${JSON.stringify(await game.state.read())}`)
    }

    for (const deliveries of [
      [giveTheLetter, giveTheParts],
      [giveTheParts, giveTheLetter],
    ]) {
      await arrange({ map: "slateport-city-harbor", x: 9, y: 11 }, "left", {
        bag: { items: { letter: 1, devonParts: 1 } },
      })
      for (const deliver of deliveries) {
        await deliver()
        await game.saveAndReload()
      }
      await expect(game.inventory.contains("letter")).resolves.toBe(false)
      await expect(game.inventory.contains("devonParts")).resolves.toBe(false)
      await expect(game.inventory.count("tmSteelWing")).resolves.toBe(1)
      await expect(game.story.var("hoennSlateportOutsideMuseumState")).resolves.toBe(1)
    }
  })
})
