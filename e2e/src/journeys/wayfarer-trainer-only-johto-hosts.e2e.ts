import fs from "node:fs/promises"
import { describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap, type PartyMonFixture } from "../harness/game-session"
import { maps, storyFlags, storyVars } from "../harness/game-session/catalog"

const fixtureMaps = maps as Record<string, { mapGroup: number; mapNum: number }>
const fixtureVars = storyVars as Record<string, number>
const fixtureFlags = storyFlags as Record<string, number>
fixtureMaps["host-burned-tower-1f"] = { mapGroup: 24, mapNum: 16 }
fixtureVars.hostIlexFarfetchd = 0x406b
fixtureVars.hostEcruteakTheater = 0x4062
Object.assign(fixtureFlags, {
  hostReceivedCut: 0x1f5,
  hostIlexMasterHidden: 0x062,
  hostIlexApprenticeHidden: 0x05f,
  hostIlexFarfetchdHidden: 0x02a,
  hostBurnedNpcsHidden: 0x07c,
  hostBeastsHidden: 0x02e,
  hostBasementEusineHidden: 0x07b,
  hostBeastsReleased: 0x1d8,
})

const emptyParty: PartyMonFixture[] = []
const faintedParty: PartyMonFixture[] = [{ species: "rattata", fainted: true }]

const flag = (game: GameSession, name: string) => game.story.flag(name as never)
const variable = (game: GameSession, name: string) => game.story.var(name as never)

const finishHost = async (game: GameSession, complete: () => Promise<boolean>, name: string) => {
  const observed = new Set<string>()
  for (let attempt = 0; attempt < 700; attempt++) {
    const state = await game.state.read()
    expect(state.battle.active, `${name} must remain a nonbattle host scene`).toBe(false)
    if (state.dialogue.text) observed.add(state.dialogue.text)
    if (state.ready && !state.dialogueOpen && !state.scriptActive && (await complete()))
      return [...observed].join("\n")
    if (state.dialogueOpen || state.scriptActive) await game.controls.press("a")
    await game.wait.frames(12)
  }
  await fs.writeFile(`/tmp/trainer-only-host-${name}.png`, await game.screenshot())
  throw new Error(`${name} did not complete: ${JSON.stringify(await game.state.read())}`)
}

const finishFieldScript = async (game: GameSession, name: string) => {
  for (let attempt = 0; attempt < 400; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready && !state.dialogueOpen && !state.scriptActive) return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    await game.wait.frames(12)
  }
  await fs.writeFile(
    `/tmp/trainer-only-host-${name.replaceAll(" ", "-")}.png`,
    await game.screenshot(),
  )
  throw new Error(`${name} did not finish: ${JSON.stringify(await game.state.read())}`)
}

const reviveInTrainerOnlyBattle = async (game: GameSession) => {
  await game.battle.startWild({ species: "pidgey", level: 5 })
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") break
    if (state.battle.ui === "text") await game.controls.press("a")
    else await game.wait.frames(12)
  }
  expect((await game.state.read()).battle.ui).toBe("action-menu")
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (Math.floor(cursor / 2)) await game.controls.press("up")
  if (cursor % 2 === 0) await game.controls.press("right")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag", "Tower recovery Bag")
  for (let pocket = 0; pocket < 5; pocket++) {
    if ((await game.state.read()).battle.bag.item === "revive") break
    await game.controls.press("right")
    await game.wait.frames(60)
  }
  expect((await game.state.read()).battle.bag.item).toBe("revive")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag-context", "Tower recovery item context")
  await game.controls.press("a")
  await game.wait.until((state) => state.ui.mode === "party-menu", "Tower recovery real target")
  await game.wait.frames(60)
  await game.controls.press("a")
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready) return
    await game.controls.press("a")
    await game.wait.frames(60)
  }
  await finishFieldScript(game, "Tower real Revive")
}

describe.sequential("Wayfarer deferred Silver independent host completion", () => {
  it("completes Ilex's native Cut handoff with an empty party and Silver still pending", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "right", position: { map: "ilex-forest", x: 25, y: 44 } },
        party: emptyParty,
        story: {
          // The Farfetch'd chase is already resolved; the quest's final gift
          // and completion writers must run through the native coordinate scene.
          vars: { hostIlexFarfetchd: 2, azaleaTownState: 6, starterMon: 0 } as never,
          flags: {
            johtoStarterChoiceCommitted: true,
            silverAzaleaComplete: false,
            hostReceivedCut: false,
            hostIlexMasterHidden: false,
            hostIlexApprenticeHidden: false,
            hostIlexFarfetchdHidden: false,
          } as never,
        },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const beforeParty = (await game.state.read()).party
      expect(await variable(game, "hostIlexFarfetchd")).toBe(2)
      expect(await flag(game, "hostReceivedCut")).toBe(false)
      await game.player.move("right")
      const dialogue = await finishHost(
        game,
        async () => (await variable(game, "hostIlexFarfetchd")) === 3,
        "ilex-empty",
      )
      expect(await flag(game, "hostReceivedCut")).toBe(true)
      expect(dialogue).toMatch(/obtained.*HM01|obtained.*Cut/is)
      expect(await variable(game, "azaleaTownState")).toBe(7)
      expect(await flag(game, "hostIlexMasterHidden")).toBe(true)
      expect(await flag(game, "hostIlexApprenticeHidden")).toBe(true)
      expect(await flag(game, "hostIlexFarfetchdHidden")).toBe(true)
      expect(await flag(game, "silverAzaleaComplete")).toBe(false)
      expect((await game.state.read()).party).toEqual(beforeParty)
    } finally {
      await game.close()
    }
  })

  it("completes native Tower discovery, revives protection, then probes deferred Silver completion with a forced win", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "host-burned-tower-1f" as GameMap, x: 15, y: 13 },
        },
        party: faintedParty,
        bag: { items: { revive: 1 } },
        story: {
          vars: { starterMon: 0, ecruteakCityState: 2, hostEcruteakTheater: 0 } as never,
          flags: {
            johtoStarterChoiceCommitted: true,
            silverBurnedTowerComplete: false,
            burnedTowerDiscovered: false,
            hostBurnedNpcsHidden: false,
            hostBeastsHidden: false,
            hostBasementEusineHidden: false,
            hostBeastsReleased: false,
          } as never,
        },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const beforeParty = (await game.state.read()).party
      expect(await flag(game, "burnedTowerDiscovered")).toBe(false)
      expect(await flag(game, "hostBeastsReleased")).toBe(false)
      await game.player.move("up")
      await finishHost(game, async () => await flag(game, "hostBeastsReleased"), "burned-fainted")
      expect((await game.state.read()).map).toMatchObject({ mapGroup: 24, mapNum: 17 })
      expect(await flag(game, "burnedTowerDiscovered")).toBe(true)
      expect(await variable(game, "ecruteakCityState")).toBe(4)
      expect(await flag(game, "hostBeastsHidden")).toBe(true)
      expect(await flag(game, "hostBasementEusineHidden")).toBe(true)
      expect(await flag(game, "silverBurnedTowerComplete")).toBe(false)
      expect((await game.state.read()).party).toEqual(beforeParty)

      // Keep this same save: real Revive targeting restores the fainted member
      // after native discovery; no fixture advances or replaces host progress.
      await reviveInTrainerOnlyBattle(game)
      expect((await game.state.read()).party[0]?.fainted).toBe(false)
      expect((await game.state.read()).bag.items.revive).toBe(0)
      await game.player.warp("host-burned-tower-1f" as GameMap, 18, 13, "up")
      await game.player.interact()
      for (let attempt = 0; attempt < 400; attempt++) {
        const state = await game.state.read()
        if (state.battle.active && state.battle.ui === "action-menu") break
        if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
          await game.controls.press("a")
        else await game.wait.frames(12)
      }
      const battle = await game.state.read()
      expect(battle.battle.active).toBe(true)
      expect(battle.battle.ui).toBe("action-menu")
      expect(battle.battle.trainerOnly.active).toBe(false)
      expect(await flag(game, "silverBurnedTowerComplete")).toBe(false)
      // The battle outcome is deliberately forced. This isolates the native
      // SilverAfterDiscovery continuation, not combat balance or earned victory.
      await game.battle.win()
      // Complete the outstanding ChooseAction controller after setting outcome.
      await game.controls.press("a")
      await finishFieldScript(game, "Tower Silver forced-win continuation")
      expect((await game.state.read()).map).toMatchObject({ mapGroup: 24, mapNum: 16 })
      expect(await flag(game, "silverBurnedTowerComplete")).toBe(true)
      expect(await flag(game, "burnedTowerDiscovered")).toBe(true)
      expect(await flag(game, "hostBeastsReleased")).toBe(true)
      expect(await variable(game, "ecruteakCityState")).toBe(4)
    } finally {
      await game.close()
    }
  })
})
