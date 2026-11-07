import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  receiveBirchStarter,
} from "../playbooks/regional-opening"
import { rideAquaFromVermilionToSlateport } from "../playbooks/wayfarer-ports"

// Birch makes his weekly round once his opening is over: the lab on days 0, 1, 6, Route 101 on
// days 2, 3, and Route 103 on days 4, 5. The clock advances the Hoenn copy of his state. Until the
// opening is over, a rescued visitor's thanks is still waiting, or a Hoenn League win, he stays in
// his lab, and no order of play shows two Birches.

const labBirch = 2
const route101Birch = 5
const route103Birch = 11

type Place = "lab" | "route-101" | "route-103"

/** Warps to the map and reports which Birches stand in it. */
const birchesAt = async (game: GameSession, place: Place): Promise<number[]> => {
  const map = place === "lab" ? "birch-lab" : place
  const [x, y] = place === "lab" ? [6, 5] : place === "route-101" ? [5, 14] : [10, 4]
  await game.player.warp(map, x, y, "up")
  await game.wait.forReady()
  const birches = { lab: labBirch, "route-101": route101Birch, "route-103": route103Birch }
  const localId = birches[place]
  return (await game.state.read()).objects
    .filter((object) => object.localId === localId && object.visible)
    .map((object) => object.localId)
}

const expectBirchOnlyAt = async (game: GameSession, place: Place, day: number): Promise<void> => {
  await game.story.setVar("hoennBirchState", day)
  for (const where of ["lab", "route-101", "route-103"] as const) {
    const found = (await birchesAt(game, where)).length
    expect(found, `day ${day}, ${where}`).toBe(where === place ? 1 : 0)
  }
}

describe("Wayfarer Hoenn: Birch's weekly round", () => {
  it(
    "keeps a Hoenn-origin Birch in his lab until the Pokédex, then roams, and stays after a League win",
    { timeout: 600_000 },
    async () => {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint: "hoenn-before-rescue",
          player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
          determinism: { textSpeed: "instant" },
        })
        await receiveBirchStarter(game, 1)
        // Rescued, Pokédex not yet given: every day of the week finds him in the lab.
        for (const day of [0, 2, 4]) await expectBirchOnlyAt(game, "lab", day)

        await game.player.warp("route-103", 10, 4, "up")
        await game.player.interact()
        await advanceOpeningUntil(game, (state) => state.battle.active, "The rival did not battle")
        await game.battle.win()
        await finishOpeningDialogue(game)
        await game.player.warp("littleroot-town", 7, 17, "up")
        await game.player.move("up")
        await advanceOpeningUntil(game, (state) => state.origin.pokedex, "Birch gave no Pokédex")
        await finishOpeningDialogue(game)

        // His opening is over: he follows the clock.
        for (const [day, place] of [
          [0, "lab"],
          [1, "lab"],
          [2, "route-101"],
          [3, "route-101"],
          [4, "route-103"],
          [5, "route-103"],
          [6, "lab"],
        ] as const) {
          await expectBirchOnlyAt(game, place, day)
        }

        // A Hoenn League win brings him home for good, even for a visitor who never rescued him.
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: { facing: "up", position: { map: "birch-lab", x: 6, y: 8 } },
          story: {
            vars: { hoennBirchState: 2 },
            flags: { hoennGameClear: true, hoennHideBirchLabBirch: true },
          },
          determinism: { textSpeed: "instant" },
        })
        expect(await birchesAt(game, "lab")).toEqual([labBirch])
      } finally {
        await game.close()
      }
    },
  )

  it(
    "keeps a visitor's Birch in one place: no second Birch before the rescue, the lab for his thanks",
    { timeout: 600_000 },
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
          bag: { items: { ssTicket: 1 } },
          party: [{ species: "salamence", level: 60 }],
          determinism: { textSpeed: "instant" },
        })
        await rideAquaFromVermilionToSlateport(game)

        // Wally's tutorial is done and the week has come round to Route 101, but nobody has
        // rescued him: the rescue scene's Birch is the only one on the route.
        await game.story.setVar("hoennPetalburgGymState", 1)
        await game.story.setVar("hoennBirchState", 2)
        expect(await birchesAt(game, "route-101")).toEqual([])
        expect(await birchesAt(game, "lab")).toEqual([])

        await game.player.warp("route-101", 9, 19, "right")
        await game.wait.forReady()
        await game.player.move("right")
        await finishOpeningDialogue(game)
        await game.player.warp("route-101", 7, 15, "up")
        await game.player.interact()
        await advanceOpeningUntil(
          game,
          (state) => state.battle.ui === "action-menu",
          "The visitor's rescue battle did not start",
        )
        await game.battle.win()
        // The thanks plays in the lab with Birch there, on a day he would be on Route 101.
        await advanceOpeningUntil(
          game,
          (state) => state.dialogue.text.includes("You handled"),
          "Birch did not thank the visitor",
        )
        const lab = (await game.state.read()).objects
        expect(lab.some((object) => object.localId === labBirch && object.visible)).toBe(true)
        await advanceOpeningUntil(
          game,
          (state) => state.dialogue.text.startsWith("Look for POK"),
          "Birch did not rate the Pokédex",
        )
        await finishOpeningDialogue(game)

        // Thanked: the weekly round starts.
        await expectBirchOnlyAt(game, "route-101", 2)
        await expectBirchOnlyAt(game, "route-103", 4)
        await expectBirchOnlyAt(game, "lab", 6)

        // A thanks still waiting (a save made at the rescue) finds him in the lab on a Route 101 day.
        // The thanks starts as the lab loads, so the arrange never sees a free field: it stalls on
        // the open dialogue, which is the scene this case waits for.
        const arranged = game.arrange({
          checkpoint: "new-bark-after-intro",
          player: { facing: "up", position: { map: "birch-lab", x: 6, y: 5 } },
          story: {
            vars: { hoennBirchState: 2, hoennBirchLabState: 2 },
            flags: { hoennRescuedBirch: true },
          },
          determinism: { textSpeed: "instant" },
        })
        await arranged.catch(() => undefined)
        await game.wait.until((state) => state.dialogueOpen, "Birch's thanks")
        expect(
          (await game.state.read()).objects.some(
            (object) => object.localId === labBirch && object.visible,
          ),
        ).toBe(true)
      } finally {
        await game.close()
      }
    },
  )
})
