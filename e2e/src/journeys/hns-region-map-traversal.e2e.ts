import { beforeAll, describe, expect, it } from "webanvil/test"

import {
  GameSession,
  partyMenuActions,
  type Button,
  type RegionMapLayout,
  type RegionMapPoint,
} from "../harness/game-session"
import { openFieldPartyMenuActions, selectFieldPartyAction } from "../playbooks/field-party-menu"

const mapSections = {
  palletTown: 20,
  viridianCity: 21,
  pewterCity: 22,
  ceruleanCity: 23,
  lavenderTown: 24,
  vermilionCity: 25,
  celadonCity: 26,
  fuchsiaCity: 27,
  cinnabarIsland: 28,
  indigoPlateau: 29,
  saffronCity: 30,
  route4: 34,
  route10: 40,
  newBarkTown: 63,
  cherrygroveCity: 64,
  violetCity: 65,
  azaleaTown: 66,
  goldenrodCity: 67,
  ecruteakCity: 68,
  olivineCity: 69,
  cianwoodCity: 70,
  mahoganyTown: 71,
  blackthornCity: 72,
  route26: 73,
  lakeOfRage: 103,
  mtSilver: 107,
  safariZoneGate: 117,
} as const

type ExpectedLocation = {
  mapSection: number
  johto?: [x: number, y: number, width: number, height: number]
  combined: [x: number, y: number, width: number, height: number]
}

const expectedLocations: ExpectedLocation[] = [
  { mapSection: mapSections.newBarkTown, johto: [19, 10, 1, 1], combined: [13, 11, 1, 1] },
  { mapSection: mapSections.cherrygroveCity, johto: [14, 10, 1, 1], combined: [9, 11, 1, 1] },
  { mapSection: mapSections.violetCity, johto: [12, 4, 1, 1], combined: [7, 5, 1, 1] },
  { mapSection: mapSections.azaleaTown, johto: [10, 13, 1, 1], combined: [6, 12, 1, 1] },
  { mapSection: mapSections.goldenrodCity, johto: [8, 8, 1, 2], combined: [5, 7, 1, 2] },
  { mapSection: mapSections.ecruteakCity, johto: [10, 2, 1, 1], combined: [6, 3, 1, 1] },
  { mapSection: mapSections.olivineCity, johto: [6, 4, 1, 1], combined: [3, 6, 1, 1] },
  { mapSection: mapSections.cianwoodCity, johto: [4, 11, 1, 1], combined: [1, 9, 1, 1] },
  { mapSection: mapSections.mahoganyTown, johto: [15, 2, 1, 1], combined: [9, 3, 1, 1] },
  { mapSection: mapSections.blackthornCity, johto: [18, 2, 1, 1], combined: [12, 3, 1, 1] },
  { mapSection: mapSections.route26, johto: [24, 6, 1, 5], combined: [16, 8, 1, 4] },
  { mapSection: mapSections.lakeOfRage, johto: [15, 0, 1, 1], combined: [9, 1, 1, 1] },
  { mapSection: mapSections.mtSilver, johto: [20, 5, 1, 1], combined: [14, 7, 1, 1] },
  { mapSection: mapSections.safariZoneGate, johto: [2, 9, 1, 1], combined: [0, 7, 1, 1] },
  { mapSection: mapSections.indigoPlateau, johto: [24, 3, 1, 2], combined: [16, 2, 1, 2] },
  { mapSection: mapSections.palletTown, combined: [19, 11, 1, 1] },
  { mapSection: mapSections.viridianCity, combined: [19, 7, 1, 1] },
  { mapSection: mapSections.pewterCity, combined: [19, 2, 1, 1] },
  { mapSection: mapSections.ceruleanCity, combined: [24, 2, 1, 1] },
  { mapSection: mapSections.lavenderTown, combined: [27, 5, 1, 1] },
  { mapSection: mapSections.vermilionCity, combined: [24, 7, 1, 1] },
  { mapSection: mapSections.celadonCity, combined: [22, 5, 1, 1] },
  { mapSection: mapSections.fuchsiaCity, combined: [24, 11, 1, 1] },
  { mapSection: mapSections.cinnabarIsland, combined: [19, 13, 1, 1] },
  { mapSection: mapSections.saffronCity, combined: [24, 5, 1, 1] },
  { mapSection: mapSections.route4, combined: [21, 2, 1, 1] },
  { mapSection: mapSections.route10, combined: [27, 3, 1, 1] },
]

const asLocation = ([x, y, width, height]: [number, number, number, number]) => ({
  x,
  y,
  width,
  height,
})

describe.sequential("HNS combined Johto/Kanto region map", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  const openFlyMap = async (layout: RegionMapLayout): Promise<void> => {
    await openFieldPartyMenuActions(game)
    await selectFieldPartyAction(game, partyMenuActions.fly)
    for (let elapsed = 0; elapsed < 600; elapsed += 4) {
      await game.wait.frames(4)
      try {
        const active = await game.regionMap.active()
        if (active.mapSection === mapSections.newBarkTown && active.layout === layout) {
          await game.wait.frames(60)
          return
        }
      } catch {
        // The map allocation and sRegionMap assignment happen on separate frames.
      }
    }
    throw new Error("Fly region map did not initialize within 600 frames")
  }

  const moveCursor = async (direction: Button, target: RegionMapPoint): Promise<void> => {
    await game.controls.press(direction)
    for (let elapsed = 0; elapsed < 90; elapsed += 2) {
      const cursor = (await game.regionMap.active()).cursor
      if (cursor.x === target.x && cursor.y === target.y) return
      await game.wait.frames(2)
    }
    throw new Error(`Region-map cursor did not reach ${target.x}:${target.y}`)
  }

  const expectActiveGridEntry = async (
    mapSection: number,
    [x, y, width, height]: [number, number, number, number],
  ): Promise<void> => {
    for (let offsetY = 0; offsetY < height; offsetY++) {
      for (let offsetX = 0; offsetX < width; offsetX++) {
        await expect(
          game.regionMap.sectionAt({ x: x + offsetX + 1, y: y + offsetY + 2 }),
        ).resolves.toBe(mapSection)
      }
    }
  }

  it("uses Johto layout, entries, player marker, and Pokedex coordinates before Kanto", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      story: { flags: { visitedKanto: false, visitedNewBarkTown: true } },
      party: [{ species: "pidgey", moves: ["fly"] }],
    })

    for (const expected of expectedLocations.filter((location) => location.johto)) {
      await expect(game.regionMap.entry("johto", expected.mapSection)).resolves.toEqual(
        asLocation(expected.johto!),
      )
      await expectActiveGridEntry(expected.mapSection, expected.johto!)
    }

    await expect(game.regionMap.observePokedex()).resolves.toEqual({
      mapSection: mapSections.newBarkTown,
      cursor: { x: 20, y: 12 },
      playerMarker: { x: 20, y: 12 },
      layout: "johto",
    })

    await openFlyMap("johto")
    await expect(game.regionMap.active()).resolves.toEqual({
      mapSection: mapSections.newBarkTown,
      mapSectionType: 2,
      cursor: { x: 20, y: 12 },
      playerMarker: { x: 20, y: 12 },
      layout: "johto",
    })
    await game.controls.press("b")
  })

  it("uses combined entries and Pokedex coordinates after Kanto is unlocked", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      story: { flags: { visitedKanto: true, visitedNewBarkTown: true } },
      party: [{ species: "pidgey", moves: ["fly"] }],
    })

    for (const expected of expectedLocations) {
      await expect(game.regionMap.entry("combined", expected.mapSection)).resolves.toEqual(
        asLocation(expected.combined),
      )
      await expectActiveGridEntry(expected.mapSection, expected.combined)
    }

    await expect(game.regionMap.observePokedex()).resolves.toEqual({
      mapSection: mapSections.newBarkTown,
      cursor: { x: 14, y: 13 },
      playerMarker: { x: 14, y: 13 },
      layout: "combined",
    })

    await openFlyMap("combined")
    await expect(game.regionMap.active()).resolves.toEqual({
      mapSection: mapSections.newBarkTown,
      mapSectionType: 2,
      cursor: { x: 14, y: 13 },
      playerMarker: { x: 14, y: 13 },
      layout: "combined",
    })
    await game.controls.press("b")
  })

  it("rejects an unvisited Kanto settlement and routes Fly to visited Vermilion", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      story: {
        flags: {
          visitedKanto: true,
          visitedNewBarkTown: true,
          visitedSaffronCity: false,
          visitedVermilionCity: true,
        },
      },
      party: [{ species: "pidgey", moves: ["fly"] }],
    })
    await openFlyMap("combined")

    for (const x of [15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25])
      await moveCursor("right", { x, y: 13 })
    for (const y of [12, 11, 10, 9, 8, 7]) await moveCursor("up", { x: 25, y })
    await expect(game.regionMap.active()).resolves.toMatchObject({
      mapSection: mapSections.saffronCity,
      mapSectionType: 3,
    })

    await game.controls.press("a")
    await game.wait.frames(30)
    await expect(game.regionMap.active()).resolves.toMatchObject({
      mapSection: mapSections.saffronCity,
      mapSectionType: 3,
    })

    for (const y of [8, 9]) await moveCursor("down", { x: 25, y })
    await expect(game.regionMap.active()).resolves.toMatchObject({
      mapSection: mapSections.vermilionCity,
      mapSectionType: 2,
    })
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.ready && state.map.name === "vermilion-city",
      "Fly to Vermilion City",
      3_600,
    )
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "vermilion-city" },
      player: { x: 22, y: 5 },
      ready: true,
    })
  })
})
