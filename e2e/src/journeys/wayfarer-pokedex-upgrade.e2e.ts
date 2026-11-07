import { describe, expect, it } from "webanvil/test"

import { type Direction, GameSession, type GameMap } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  receiveBirchStarter,
} from "../playbooks/regional-opening"
import { playThroughNewGameIntro } from "../playbooks/new-game-intro"
import { rideAquaFromVermilionToSlateport } from "../playbooks/wayfarer-ports"

// The National upgrade comes from the first professor a traveller meets in the other land: Birch
// after the Route 101 rescue for a Kanto or Johto origin, Elm or Oak on the first lab talk for a
// Hoenn origin. Each land's own professor still upgrades a player who never leaves home, and a
// professor who finds the upgrade held says so and does not upgrade again.

const UPGRADED = "upgraded"

const messageKey = (state: GameState): string =>
  `${state.dialogue.sequence}|${state.dialogue.fullText}`

/**
 * Reads a talk or scene through with A until the field is free and `done` holds. Returns everything
 * said after `before`, each message once. The open-dialogue bit drops between pages, so every new
 * message counts, by its sequence number and text.
 */
const readThrough = async (
  game: GameSession,
  description: string,
  done: () => Promise<boolean> = async () => true,
  before = "",
): Promise<string> => {
  let said = ""
  let last = before
  for (let attempt = 0; attempt < 200; attempt++) {
    await game.wait.frames(30)
    const state = await game.state.read()
    expect(state.battle.active).toBe(false)
    const message = messageKey(state)
    if (message !== last) {
      last = message
      said += ` ${state.dialogue.fullText}`
    }
    // The ready bit can report true between two pages, so the scene's own state decides.
    if (state.ready && !state.scriptActive && !state.controlsLocked && !state.dialogueOpen)
      if (await done()) return said
    await game.controls.press("a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

/** Talks to whoever stands ahead of the tile and reads the talk through. */
const talkAt = async (
  game: GameSession,
  map: GameMap,
  x: number,
  y: number,
  facing: Direction,
  description: string,
): Promise<string> => {
  await game.player.warp(map, x, y, facing)
  await game.wait.forReady()
  const before = messageKey(await game.state.read())
  await game.player.interact()
  await game.wait.until(
    (state) => state.scriptActive || messageKey(state) !== before,
    description,
  )
  return readThrough(game, description, async () => true, before)
}

const talkToBirch = (game: GameSession) => talkAt(game, "birch-lab", 6, 5, "up", "Birch's talk")
const talkToElm = (game: GameSession) => talkAt(game, "elm-lab", 6, 4, "up", "Elm's talk")
const talkToOak = (game: GameSession) => talkAt(game, "oak-lab", 13, 12, "up", "Oak's talk")

/** Birch's rating: the home list's counts, then, with the upgrade, the National ones. */
const ratingCounts = (said: string): { seen: number; caught: number }[] =>
  [...said.matchAll(/seen (\d+) POK.MON,\s+and you['’]ve caught (\d+)/g)].map((match) => ({
    seen: Number(match[1]),
    caught: Number(match[2]),
  }))

/** A Master Ball catch of a wild Pokémon, read through to the field. */
const catchWild = async (game: GameSession, species: "treecko"): Promise<void> => {
  await game.battle.startWild({ species, level: 2, moves: ["tackle"] })
  await advanceOpeningUntil(game, (state) => state.battle.ui === "action-menu", "wild battle menu")
  await game.controls.press("right")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag", "battle Bag", 3_600)
  for (let pocket = 0; pocket < 5; pocket++) {
    if ((await game.state.read()).battle.bag.item === "masterBall") break
    await game.controls.press("right")
    await game.wait.frames(20)
  }
  expect((await game.state.read()).battle.bag.item).toBe("masterBall")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag-context", "Master Ball menu", 1_200)
  await game.controls.press("a")
  for (let attempt = 0; attempt < 200; attempt++) {
    await game.wait.frames(20)
    const state = await game.state.read()
    if (!state.battle.active && state.ready) break
    await game.controls.press(state.battle.ui === "nickname" ? "b" : "a")
  }
  expect((await game.state.read()).party.map((mon) => mon.species)).toContain(species)
}

/** The Route 103 win and Birch's Pokédex, for a player who started in Hoenn. */
const receiveBirchPokedex = async (game: GameSession): Promise<void> => {
  await game.player.warp("route-103", 10, 4, "up")
  await game.player.interact()
  await advanceOpeningUntil(game, (state) => state.battle.active, "The rival did not battle")
  await game.battle.win()
  await finishOpeningDialogue(game)
  await game.player.warp("littleroot-town", 7, 17, "up")
  await game.player.move("up")
  await advanceOpeningUntil(game, (state) => state.origin.pokedex, "Birch gave no Pokédex")
  await finishOpeningDialogue(game)
}

const arrangeHoennOrigin = async (game: GameSession): Promise<void> => {
  await game.arrange({
    checkpoint: "hoenn-before-rescue",
    player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
    story: { flags: { disableEncounters: true, disableTrainers: true } },
    determinism: { textSpeed: "instant" },
  })
  await receiveBirchStarter(game, 1)
}

/** Birch's Hall of Fame scene, from stepping out of the rival's house into Littleroot to his lab. */
const playHallOfFameScene = async (game: GameSession): Promise<string> => {
  await game.player.warp("mays-house-1f", 2, 8, "down")
  await game.wait.forReady()
  await game.story.setVar("hoennDexUpgradeState", 1)
  await game.player.move("down")
  await game.wait.until(
    (state) => state.map.name === "littleroot-town" && state.dialogueOpen,
    "Birch's Hall of Fame scene",
  )
  const said = await readThrough(
    game,
    "Birch's Hall of Fame scene did not finish",
    async () => (await game.story.var("hoennDexUpgradeState")) === 2,
  )
  expect((await game.state.read()).map.name).toBe("birch-lab")
  return said
}

describe("Wayfarer Pokédex: the professors' National upgrade", () => {
  it(
    "Birch upgrades a Johto traveller after the rescue, showing the Hoenn Pokémon they caught",
    { timeout: 420_000 },
    async () => {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: { facing: "down", position: { map: "vermilion-port-inside", x: 8, y: 9 } },
          story: {
            vars: { ssAquaState: 8 },
            flags: { disableEncounters: true, disableTrainers: true },
          },
          bag: { items: { ssTicket: 1, masterBall: 1 } },
          party: [{ species: "gyarados", level: 60 }],
          determinism: { textSpeed: "instant" },
        })
        await rideAquaFromVermilionToSlateport(game)
        await catchWild(game, "treecko")
        await expect(game.story.flag("nationalDex")).resolves.toBe(false)

        await game.player.warp("route-101", 9, 19, "right")
        await game.wait.forReady()
        await game.player.move("right")
        await finishOpeningDialogue(game)
        await game.player.warp("route-101", 7, 15, "up")
        await game.player.interact()
        await advanceOpeningUntil(
          game,
          (state) => state.battle.ui === "action-menu",
          "The rescue battle did not start",
        )
        await game.battle.win()
        await advanceOpeningUntil(
          game,
          (state) => state.dialogue.text.includes("You handled"),
          "Birch did not thank the traveller",
        )
        const thanks = await readThrough(game, "Birch's thanks did not finish")
        expect(thanks).toContain("come a long way")
        expect(thanks).toContain(UPGRADED)
        await expect(game.story.flag("nationalDex")).resolves.toBe(true)

        // The rating follows: Treecko is no Kanto-Johto Pokémon, so only National mode counts it.
        const [home, national] = ratingCounts(thanks)
        expect(home).toBeDefined()
        expect(national).toBeDefined()
        expect(national!.caught).toBe(home!.caught + 1)

        // A later talk rates the Pokédex and upgrades nothing.
        const later = await talkToBirch(game)
        expect(later).not.toContain(UPGRADED)
        expect(ratingCounts(later)).toEqual([home, national])
      } finally {
        await game.close()
      }
    },
  )

  it("Birch upgrades a traveller whose save is past the rescue at the next talk", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "birch-lab", x: 6, y: 5 } },
        story: {
          vars: { hoennBirchLabState: 5 },
          flags: { hoennRescuedBirch: true, pokedex: true, disableTrainers: true },
        },
        party: [{ species: "gyarados", level: 60 }],
        determinism: { textSpeed: "instant" },
      })
      const first = await talkToBirch(game)
      expect(first).toContain(UPGRADED)
      expect(first.indexOf(UPGRADED)).toBeLessThan(first.indexOf("Hmhm"))
      expect(ratingCounts(first)).toHaveLength(2)
      await expect(game.story.flag("nationalDex")).resolves.toBe(true)

      const second = await talkToBirch(game)
      expect(second).not.toContain(UPGRADED)
      expect(ratingCounts(second)).toHaveLength(2)
    } finally {
      await game.close()
    }
  })

  it(
    "Elm upgrades a Hoenn traveller once they have a Pokédex, Oak notes it, and Birch's Hall of Fame scene still plays",
    { timeout: 480_000 },
    async () => {
      const game = await GameSession.launch()
      try {
        await arrangeHoennOrigin(game)

        // No Pokédex yet: Elm only greets.
        const greeting = await talkToElm(game)
        expect(greeting).toContain("traveling with")
        expect(greeting).not.toContain(UPGRADED)
        await expect(game.story.flag("nationalDex")).resolves.toBe(false)

        // With Birch's Pokédex, the next talk ends with the upgrade.
        await receiveBirchPokedex(game)
        await expect(game.story.flag("nationalDex")).resolves.toBe(false)
        const upgrade = await talkToElm(game)
        expect(upgrade).toContain("from HOENN")
        expect(upgrade).toContain(UPGRADED)
        expect(upgrade.indexOf("traveling with")).toBeLessThan(upgrade.indexOf(UPGRADED))
        await expect(game.story.flag("nationalDex")).resolves.toBe(true)

        // Oak comes second: his welcome notes the upgrade and gives nothing.
        const welcome = await talkToOak(game)
        expect(welcome).toContain("way to KANTO")
        expect(welcome).toContain("already")
        expect(welcome).not.toContain(UPGRADED)
        await expect(game.story.var("palletLabState")).resolves.toBe(1)

        // Second visits upgrade nothing.
        const elmAgain = await talkToElm(game)
        expect(elmAgain).not.toContain(UPGRADED)
        expect(elmAgain).not.toContain("from HOENN")
        expect(await talkToOak(game)).not.toContain(UPGRADED)

        // Birch's Hall of Fame scene finds the upgrade held: he says so, still arms Scott's call,
        // and the Johto-starter chain goes on.
        await game.story.setVar("hoennScottFrontierCallSteps", 7)
        const scene = await playHallOfFameScene(game)
        expect(scene).toMatch(/let['’]s go to my LAB/)
        expect(scene).not.toContain("something to show you")
        expect(scene).toContain("already, I see")
        expect(scene).toContain("But listen")
        expect(scene).not.toContain(UPGRADED)
        expect(scene).not.toContain("MAY:")
        expect(scene).not.toContain("BRENDAN:")
        await expect(game.story.flag("hoennScottFrontierCall")).resolves.toBe(true)
        await expect(game.story.var("hoennScottFrontierCallSteps")).resolves.toBe(0)
        await game.player.warp("littleroot-town", 7, 17, "down")
        await game.wait.forReady()
        await expect(game.story.var("hoennDexUpgradeState")).resolves.toBe(3)
      } finally {
        await game.close()
      }
    },
  )

  it(
    "Birch upgrades a Hoenn player who never left after the Hall of Fame, once",
    { timeout: 420_000 },
    async () => {
      const game = await GameSession.launch()
      try {
        await arrangeHoennOrigin(game)
        await receiveBirchPokedex(game)
        const scene = await playHallOfFameScene(game)
        expect(scene).toContain("something to show you")
        expect(scene).toContain(UPGRADED)
        expect(scene).toContain("But listen")
        await expect(game.story.flag("nationalDex")).resolves.toBe(true)
        await expect(game.story.flag("hoennScottFrontierCall")).resolves.toBe(true)

        const later = await talkToBirch(game)
        expect(later).not.toContain(UPGRADED)
      } finally {
        await game.close()
      }
    },
  )

  it("Oak upgrades a Johto player at eight Kanto badges, once", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "oak-lab", x: 13, y: 12 } },
        story: { flags: { disableTrainers: true } },
        circuit: { badges: { kanto: 8 } },
        party: [{ species: "gyarados", level: 60 }],
        determinism: { textSpeed: "instant" },
      })
      // At home in Kanto-Johto, the welcome is no traveller's upgrade.
      const welcome = await talkToOak(game)
      expect(welcome).toContain("way to KANTO")
      expect(welcome).not.toContain(UPGRADED)
      await expect(game.story.flag("nationalDex")).resolves.toBe(false)
      await expect(game.story.var("palletLabState")).resolves.toBe(2)

      expect(await talkToOak(game)).toContain("How is your POK")
      await expect(game.story.flag("nationalDex")).resolves.toBe(true)
      expect(await talkToOak(game)).toContain("How is your POK")
      await expect(game.story.flag("nationalDex")).resolves.toBe(true)
    } finally {
      await game.close()
    }
  })

  it(
    "Oak upgrades a Pallet player at the Olivine port, and only notes a held upgrade",
    { timeout: 420_000 },
    async () => {
      const game = await GameSession.launch()
      try {
        await playThroughNewGameIntro(game, "kanto")
        const meetOak = async (): Promise<string> => {
          await game.story.setFlag("hideOlivinePortOak", false)
          return talkAt(game, "olivine-port-inside", 8, 15, "down", "Oak at the port")
        }

        const first = await meetOak()
        expect(first).toContain("attitude of a CHAMPION")
        expect(first).toContain(UPGRADED)
        expect(first).toContain("radio show")
        await expect(game.story.flag("nationalDex")).resolves.toBe(true)
        await expect(game.story.flag("hideOlivinePortOak")).resolves.toBe(true)

        const again = await meetOak()
        expect(again).toContain("already, I see")
        expect(again).not.toContain(UPGRADED)
        expect(again).toContain("radio show")
        await expect(game.story.flag("hideOlivinePortOak")).resolves.toBe(true)
      } finally {
        await game.close()
      }
    },
  )
})
