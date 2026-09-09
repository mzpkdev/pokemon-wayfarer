import { beforeEach, describe, expect, it } from "webanvil/test"
import { GameSession } from "../harness/game-session"
import { silphDoors, silphPanels, silphPickups } from "../playbooks/silph-fixtures"
import { arrangeSilph, moveSilph, silphFloorFlags, talkSilph } from "../playbooks/silph"

const floors = [...new Set(silphDoors.map((door) => door.floor))]

describe.sequential("Wayfarer Silph doors, panels and pickups", () => {
  let game: GameSession
  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  for (const floor of floors) {
    it(`opens and persists every individual door on ${floor}`, async () => {
      for (const door of silphDoors.filter((entry) => entry.floor === floor)) {
        await arrangeSilph(game, {
          player: { position: { map: floor, ...door.approach }, facing: door.approach.facing },
          story: { flags: { silphLiberated: true, silphCardKeyReceived: true } },
        })
        expect(await game.story.flag(door.flag)).toBe(false)
        await game.controls.press(door.approach.facing)
        await game.wait.frames(20)
        expect((await game.state.read()).player).toMatchObject({ x: door.approach.x, y: door.approach.y })
        await talkSilph(game)
        expect(await game.story.flag(door.flag)).toBe(true)
        for (const other of silphDoors.filter((entry) => entry.floor === floor && entry.flag !== door.flag))
          expect(await game.story.flag(other.flag)).toBe(false)
        await game.saveAndReload()
        expect(await game.story.flag(door.flag)).toBe(true)
        await moveSilph(game, door.approach.facing, door.target.x, door.target.y)
      }
    })

    it(`preserves the authored warp panels on ${floor}`, async () => {
      const view = silphDoors.find((door) => door.floor === floor)!
      await arrangeSilph(game, {
        player: { position: { map: floor, ...view.approach }, facing: view.approach.facing },
        party: [],
      })
      for (const panel of silphPanels.filter((entry) => entry.floor === floor)) {
        await arrangeSilph(game, {
          player: { position: { map: floor, ...panel.approach }, facing: panel.approach.facing },
          story: { flags: { silphLiberated: true, ...silphFloorFlags(floor) } },
        })
        for (let attempt = 0; attempt < 8; attempt++) {
          const state = await game.state.read()
          if (state.map.name === panel.destination && state.player.x === panel.target.x && state.player.y === panel.target.y) break
          if (state.ready) await game.controls.press(panel.approach.facing)
          await game.wait.frames(40)
        }
        await game.wait.forMap(panel.destination)
        expect((await game.state.read()).player).toMatchObject(panel.target)
      }
    })

    it(`delivers every visible and hidden pickup on ${floor} once`, async () => {
      for (const pickup of silphPickups.filter((entry) => entry.floor === floor)) {
        await arrangeSilph(game, {
          player: { position: { map: floor, ...pickup.approach }, facing: pickup.approach.facing },
          story: { flags: { silphLiberated: true, ...silphFloorFlags(floor) } },
        })
        expect(await game.story.flag(pickup.flag)).toBe(false)
        await talkSilph(game)
        expect(await game.inventory.count(pickup.item)).toBe(1)
        if (pickup.item === "silphCardKey") expect(await game.inventory.count("goldenrodCardKey")).toBe(0)
        expect(await game.story.flag(pickup.flag)).toBe(true)
        await game.saveAndReload()
        expect(await game.inventory.count(pickup.item)).toBe(1)
        expect(await game.story.flag(pickup.flag)).toBe(true)
      }
    })
  }

  it("does not let Goldenrod's Card Key open a Silph door", async () => {
    const door = silphDoors[0]!
    await arrangeSilph(game, {
      player: { position: { map: door.floor, ...door.approach }, facing: door.approach.facing },
      bag: { items: { goldenrodCardKey: 1 } },
    })
    await talkSilph(game)
    expect(await game.story.flag(door.flag)).toBe(false)
    expect(await game.story.flag("silphCardKeyReceived")).toBe(false)
    expect(await game.inventory.count("goldenrodCardKey")).toBe(1)
  })

  for (const [item, pocket] of [["silphCardKey", "keyItems"], ["tmTorment", "tmHm"], ["nugget", "items"]] as const) {
    it(`keeps ${item} pending across a full ${pocket} pocket and reload`, async () => {
      const pickup = silphPickups.find((entry) => entry.item === item)!
      await arrangeSilph(game, {
        player: { position: { map: pickup.floor, ...pickup.approach }, facing: pickup.approach.facing },
        story: { flags: { silphLiberated: true, ...silphFloorFlags(pickup.floor) } },
        bag: { fullPockets: [pocket] },
      })
      await talkSilph(game)
      expect(await game.inventory.count(item)).toBe(0)
      expect(await game.story.flag(pickup.flag)).toBe(false)
      await game.saveAndReload()
      await talkSilph(game)
      expect(await game.story.flag(pickup.flag)).toBe(false)
      expect(await game.inventory.count(item)).toBe(0)
      await game.inventory.freeFixtureSlot(pocket)
      await talkSilph(game)
      expect(await game.inventory.count(item)).toBe(1)
      expect(await game.story.flag(pickup.flag)).toBe(true)
      await game.saveAndReload()
      expect(await game.inventory.count(item)).toBe(1)
    })
  }
})
