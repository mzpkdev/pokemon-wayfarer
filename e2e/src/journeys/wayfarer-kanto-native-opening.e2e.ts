import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  beginPalletOpening,
  checkViridianAfterOpening,
  deliverPalletParcel,
  palletPhases,
  receivePalletParcel,
  receivePalletStarter,
  resolvePalletBlueBattle,
} from "../playbooks/kanto-opening"
import { type AppearanceStyle, appearanceStyles } from "../playbooks/new-game-intro"

for (const style of [1, 2, 3, 4] as const satisfies readonly AppearanceStyle[]) {
  describe.sequential(`Wayfarer Kanto opening, Style ${style}`, () => {
    it("plays Oak, Blue, the Parcel errand, and terminal rewards from a fresh intro", async () => {
      const game = await GameSession.launch()
      try {
        await beginPalletOpening(game, style)
        const laterBlueBefore = await game.story.flag("viridianBlueIntroduced")
        const gymBefore = await game.story.flag("defeatedViridianGym")
        const labBefore = await game.story.var("palletLabState")
        const viridianBefore = await game.story.var("viridianCityState")
        await receivePalletStarter(game)
        let state = await game.state.read()
        expect(state.origin.pallet).toMatchObject({
          phase: palletPhases.starterReceived,
          starterSlot: 0,
        })
        expect(state.party).toHaveLength(1)
        expect(state.party[0]?.species).toBe("bulbasaur")
        expect(state.origin).toMatchObject({
          johtoCommitted: false,
          johtoReceived: false,
          hoennReceived: false,
          pokedex: false,
        })

        const outcome = style === 3 ? "lose" : "win"
        await resolvePalletBlueBattle(game, outcome)
        state = await game.state.read()
        expect(state.origin.pallet.phase).toBe(palletPhases.battleResolved)
        expect(state.origin.recovery.map).toBe("reds-house-1f")
        if (outcome === "lose") expect(["reds-house-1f", "oak-lab"]).toContain(state.map.name)
        await game.saveAndReload()
        expect((await game.state.read()).origin.pallet).toEqual(state.origin.pallet)

        await receivePalletParcel(game)
        state = await game.state.read()
        expect(state.origin.pallet.phase).toBe(palletPhases.parcelReceived)
        expect(await game.inventory.count("oaksParcel")).toBe(1)
        expect(state.origin.pokedex).toBe(false)
        await game.saveAndReload()
        expect(await game.inventory.count("oaksParcel")).toBe(1)
        expect((await game.state.read()).origin.pallet).toEqual(state.origin.pallet)

        await deliverPalletParcel(game)
        state = await game.state.read()
        expect(state.map.name).toBe("oak-lab")
        expect(state.origin.pallet).toMatchObject({ phase: palletPhases.complete, starterSlot: 0 })
        expect(state.origin.pallet.receipts & 0x1ff).toBe(0x1ff)
        expect(await game.inventory.count("oaksParcel")).toBe(0)
        expect(await game.inventory.count("pokeBall")).toBe(5)
        expect(await game.inventory.count("townMap")).toBe(0)
        expect(state.origin.pokedex).toBe(true)
        expect(state.appearance.id).toBe(appearanceStyles[style].id)
        expect(state.origin.gender).toBe(appearanceStyles[style].gender)
        expect(state.origin).toMatchObject({
          id: 3,
          johtoCommitted: false,
          johtoReceived: false,
          hoennReceived: false,
        })
        expect(await game.story.flag("viridianBlueIntroduced")).toBe(laterBlueBefore)
        expect(await game.story.flag("defeatedViridianGym")).toBe(gymBefore)
        expect(await game.story.var("palletLabState")).toBe(labBefore)
        expect(await game.story.var("viridianCityState")).toBe(viridianBefore)
        await game.saveAndReload()
        const saved = await game.state.read()
        expect(saved.origin.pallet).toEqual(state.origin.pallet)
        expect(await game.inventory.count("oaksParcel")).toBe(0)
        expect(await game.inventory.count("pokeBall")).toBe(5)
        expect(await game.inventory.count("townMap")).toBe(0)
        expect(saved.origin.pokedex).toBe(true)

        await checkViridianAfterOpening(game)
        expect((await game.state.read()).origin.pallet).toEqual(saved.origin.pallet)
        await game.player.warp("oak-lab", 13, 12, "up")
        expect((await game.state.read()).map.name).toBe("oak-lab")
      } catch (error) {
        await fs.promises.writeFile(
          "/tmp/wayfarer-kanto-opening-failure.png",
          await game.screenshot(),
        )
        throw error
      } finally {
        await game.close()
      }
    }, 300_000)
  })
}
