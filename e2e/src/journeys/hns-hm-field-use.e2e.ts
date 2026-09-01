import { beforeAll, context, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { partyMenuActions } from "../harness/game-session/catalog"
import { openFieldPartyMenuActions, selectFieldPartyAction } from "../playbooks/field-party-menu"

describe.sequential("HNS HM field use", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  context("at the Route 41 shore", () => {
    it("surfs automatically from land and ignores a second activation while surfing", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "route-41", x: 9, y: 21 },
        },
        party: [{ species: "lapras" }],
        bag: { hms: { surf: 1 } },
        determinism: { textSpeed: "instant" },
      })

      await game.player.interact()
      await game.dialogue.waitForOpen()
      await expect(game.state.read()).resolves.toMatchObject({
        dialogue: { message: "want-to-use-surf" },
        fieldMove: { move: "surf", user: 0, userSpecies: "lapras", result: "found" },
      })

      await game.wait.until(
        (state) => state.dialogue.message === "want-to-use-surf" && !state.dialogueOpen,
        "Surf prompt",
      )
      await game.wait.frames(12)
      await game.controls.press("a")
      await game.wait.until(
        (state) => state.dialogue.message === "player-used-surf",
        "Surf confirmation",
      )
      await game.dialogue.waitForClosed()
      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.until((state) => state.player.surfing, "start surfing", 3_600)
      await game.wait.forReady()

      const beforeRepeat = await game.state.read()
      await game.player.interact()
      await game.wait.frames(120)

      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        dialogueOpen: false,
        player: {
          surfing: true,
          surfBlobCount: beforeRepeat.player.surfBlobCount,
          surfEffectActive: false,
        },
      })
    })

    it("keeps resolver feedback available from land when Surf's HM is missing", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "route-41", x: 9, y: 21 },
        },
        party: [{ species: "lapras" }],
        determinism: { textSpeed: "instant" },
      })

      await game.player.interact()
      await game.dialogue.waitForOpen()

      await expect(game.state.read()).resolves.toMatchObject({
        dialogue: { message: "field-move-needs-hm" },
        fieldMove: { move: "surf", result: "missing-item" },
        player: { surfing: false, surfBlobCount: 0 },
      })

      await game.wait.frames(12)
      await game.controls.press("a")
      await game.wait.forReady()
    })
  })

  context("in the field party menu", () => {
    it("reserves terrain HMs for contextual interactions while retaining Fly and Flash", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "route-41", x: 9, y: 21 } },
        party: [
          {
            species: "lapras",
            moves: ["cut", "surf", "fly", "flash"],
          },
        ],
      })

      await openFieldPartyMenuActions(game)

      await expect(game.state.read()).resolves.toMatchObject({
        partyMenu: {
          open: true,
          actions: expect.arrayContaining([partyMenuActions.fly, partyMenuActions.flash]),
        },
      })
      await expect(game.state.read()).resolves.toMatchObject({
        partyMenu: {
          actions: expect.not.arrayContaining([partyMenuActions.cut, partyMenuActions.surf]),
        },
      })

      await selectFieldPartyAction(game, partyMenuActions.fly)
      await game.wait.until(
        (state) => !state.partyMenu.open && state.phase === "boot",
        "open Fly map",
      )
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "route-41" },
        partyMenu: { open: false },
        ready: false,
        fieldMove: {
          move: "fly",
          user: 0,
          userSpecies: "lapras",
          result: "selected",
          unlocked: true,
        },
      })
    })

    it("selects Flash explicitly and returns its unavailable-here result to the party menu", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "route-41", x: 9, y: 21 } },
        party: [{ species: "lapras", moves: ["flash"] }],
      })

      await openFieldPartyMenuActions(game)
      await expect(game.state.read()).resolves.toMatchObject({
        partyMenu: { actions: expect.arrayContaining([partyMenuActions.flash]) },
      })

      await selectFieldPartyAction(game, partyMenuActions.flash)
      await game.wait.frames(120)
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "route-41" },
        partyMenu: { open: true },
        player: { surfing: false },
        fieldMove: {
          move: "flash",
          user: 0,
          userSpecies: "lapras",
          result: "selected",
          unlocked: true,
        },
      })
      await game.controls.press("a")
    })

    it("keeps HNS's non-HM Dive selectable but badge-gated", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "route-41", x: 9, y: 21 } },
        story: { flags: { badge7: false } },
        party: [{ species: "lapras", moves: ["dive"] }],
      })

      await openFieldPartyMenuActions(game)

      await expect(game.state.read()).resolves.toMatchObject({
        partyMenu: {
          open: true,
          actions: expect.arrayContaining([partyMenuActions.dive]),
        },
      })

      await selectFieldPartyAction(game, partyMenuActions.dive)
      await game.wait.frames(120)
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "route-41" },
        partyMenu: { open: true },
        player: { surfing: false },
        fieldMove: {
          move: "dive",
          user: 0,
          userSpecies: "lapras",
          result: "selected",
          unlocked: false,
        },
      })
      await game.controls.press("a")
    })
  })

  context("at a contextual Cut obstacle", () => {
    it("chooses an unlearned compatible user and clears the obstacle without a badge", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "test-map-1", x: 6, y: 4 },
        },
        party: [{ species: "rattata" }],
        bag: { hms: { cut: 1 } },
        determinism: { textSpeed: "instant" },
      })

      await game.player.interact()
      await game.dialogue.waitForOpen()
      await expect(game.state.read()).resolves.toMatchObject({
        dialogue: { message: "field-move-used" },
        fieldMove: { move: "cut", user: 0, userSpecies: "rattata", result: "found" },
      })

      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.forReady()
      await game.player.move("up")
      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        player: { x: 6, y: 3 },
      })
    })
  })

  context("at a Route 41 whirlpool", () => {
    it("names the automatically resolved user and moves through the whirlpool", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "down",
          position: { map: "route-41", x: 27, y: 65 },
        },
        party: [{ species: "lapras", moves: ["whirlpool"] }],
        bag: { hms: { whirlpool: 1 } },
        determinism: { textSpeed: "instant" },
      })

      await game.player.interact()
      await game.dialogue.waitForOpen()
      await expect(game.state.read()).resolves.toMatchObject({
        dialogue: { message: "field-move-used", text: "LAPRAS used WHIRLPOOL!" },
        fieldMove: { move: "whirlpool", user: 0, userSpecies: "lapras", result: "found" },
      })

      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.until(
        (state) => state.ready && state.player.x === 27 && state.player.y === 68,
        "pass through Route 41's whirlpool",
      )
    })

    it("reports missing-HM and no-eligible-user resolver failures", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "down",
          position: { map: "route-41", x: 27, y: 65 },
        },
        party: [{ species: "lapras" }],
        determinism: { textSpeed: "instant" },
      })

      await game.player.interact()
      await game.dialogue.waitForOpen()
      await expect(game.state.read()).resolves.toMatchObject({
        dialogue: { message: "field-move-needs-hm" },
        fieldMove: { move: "whirlpool", result: "missing-item" },
      })

      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.forReady()

      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "down",
          position: { map: "route-41", x: 27, y: 65 },
        },
        party: [{ species: "lapras", egg: true }],
        bag: { hms: { whirlpool: 1 } },
        determinism: { textSpeed: "instant" },
      })

      await game.player.interact()
      await game.dialogue.waitForOpen()
      await expect(game.state.read()).resolves.toMatchObject({
        dialogue: { message: "field-move-no-eligible-mon" },
        fieldMove: { move: "whirlpool", result: "no-eligible-mon" },
      })

      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.forReady()
    })
  })

  context("on the first Route 33 arrival", () => {
    it("retains HM Surf while travelling into Azalea", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "left",
          position: { map: "route-33", x: 1, y: 23 },
        },
        bag: { hms: { surf: 1 } },
      })

      await game.player.move("left")
      await game.wait.forReady()
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "route-33" },
        player: { x: 0, y: 23 },
        bag: { hms: { surf: 1 } },
      })

      for (let step = 0; step < 8; step++) {
        await game.player.move("left")
        await game.wait.forReady()
        if ((await game.state.read()).map.name === "azalea-town") break
      }
      await game.wait.forMap("azalea-town")
      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        bag: { hms: { surf: 1 } },
      })
    })
  })
})
