import * as fs from "node:fs/promises"

import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  meetBirchAsVisitor,
  receiveBirchStarter,
  receiveElmStarter,
  walkRoute101AsVisitor,
} from "../playbooks/regional-opening"
import { rideAquaFromVermilionToSlateport } from "../playbooks/wayfarer-ports"

describe.sequential("Wayfarer Hoenn native and visitor campaign handoffs", () => {
  it("continues the native rescue through Route 103, shared Dex, and Mom's running shoes", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveBirchStarter(game, 1)
      expect((await game.state.read()).origin).toMatchObject({
        pokedex: false,
        runningShoes: false,
      })
      expect(await game.story.flag("sharedPokegear")).toBe(true)
      expect(await game.story.flag("sharedMatchCall")).toBe(true)
      await game.player.warp("route-103", 10, 4, "up")
      await game.player.interact()
      await advanceOpeningUntil(
        game,
        (state) => state.battle.active,
        "Route 103 rival did not battle",
      )
      expect((await game.state.read()).battle.enemy?.species).toBe("mudkip")
      await game.battle.win()
      await finishOpeningDialogue(game)
      await game.player.warp("littleroot-town", 7, 17, "up")
      await game.player.move("up")
      await advanceOpeningUntil(
        game,
        (state) => state.origin.pokedex,
        "Birch did not grant shared Dex",
      )
      await finishOpeningDialogue(game)
      expect((await game.state.read()).origin.runningShoes).toBe(false)
      await game.player.warp("littleroot-town", 10, 3, "up")
      await game.player.move("up")
      await advanceOpeningUntil(
        game,
        (state) => state.origin.runningShoes,
        "Mom did not grant shared shoes",
      )
      await finishOpeningDialogue(game)
      await game.saveAndReload()
      expect(await game.state.read()).toMatchObject({
        origin: {
          id: 2,
          hoennChoice: 1,
          hoennReceived: true,
          johtoCommitted: false,
          pokedex: true,
          runningShoes: true,
        },
        party: [{ species: "torchic" }],
        circuit: { badges: { total: 0 }, clears: { indigo: false, masters: false, hoenn: false } },
      })
      await game.controls.press("start")
      await game.wait.frames(60)
      const startMenuImage = await game.screenshot()
      await fs.writeFile("/tmp/wayfarer-hoenn-native-start-menu.png", startMenuImage)
      await game.controls.press("a")
      await game.wait.frames(360)
      for (let entry = 0; entry < 3; entry++) await game.controls.press("down")
      await game.wait.frames(90)
      const pokedexImage = await game.screenshot()
      await fs.writeFile("/tmp/wayfarer-hoenn-native-pokedex.png", pokedexImage)
      expect(pokedexImage).not.toEqual(startMenuImage)
      await advanceOpeningUntil(
        game,
        (state) => state.ready,
        "Pokédex did not return to field",
        "b",
      )
      await game.controls.press("start")
      await game.wait.frames(60)
      for (let option = 0; option < 3; option++) await game.controls.press("down")
      await game.controls.press("a")
      await game.wait.frames(180)
      await fs.writeFile("/tmp/wayfarer-hoenn-native-pokegear.png", await game.screenshot())
      await advanceOpeningUntil(
        game,
        (state) => state.ready,
        "Pokégear did not return to field",
        "b",
      )
      await game.player.warp("birch-lab", 6, 5, "up")
      await game.player.interact()
      await finishOpeningDialogue(game)
      expect((await game.state.read()).party).toHaveLength(1)
      expect((await game.state.read()).origin.pokedex).toBe(true)
    } finally {
      await game.close()
    }
  })

  it("meets Birch in his lab with the Johto party and leaves with no Hoenn starter", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "elm-lab-before-intro",
        story: { vars: { ssAquaState: 8 } },
        bag: { items: { ssTicket: 1 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveElmStarter(game, 1)
      await game.player.warp("vermilion-port-inside", 8, 9, "down")
      // The S.S. Aqua is Vermilion's ship by day: one leg to Slateport.
      await rideAquaFromVermilionToSlateport(game)
      expect((await game.state.read()).map.name).toBe("slateport-city-harbor")
      // The rescue belongs to a Hoenn-origin player: a visitor walks Route 101 with no scene.
      await walkRoute101AsVisitor(game)
      await expect(game.story.flag("hoennRescuedBirch")).resolves.toBe(true)
      // A visitor's opening is thanks and a rating in the lab: no starter, no gift, no rival.
      await meetBirchAsVisitor(game)
      expect((await game.state.read()).origin).toMatchObject({
        id: 1,
        johtoChoice: 1,
        hoennChoice: 65535,
        hoennReceived: false,
      })
      expect((await game.state.read()).party.map((mon) => mon.species)).toEqual(["cyndaquil"])
      await game.saveAndReload()
      expect((await game.state.read()).party).toHaveLength(1)
      await game.player.warp("route-103", 10, 4, "up")
      await game.player.interact()
      await game.wait.frames(90)
      expect((await game.state.read()).ready).toBe(true)
      expect((await game.state.read()).battle.active).toBe(false)
    } finally {
      await game.close()
    }
  })
})
