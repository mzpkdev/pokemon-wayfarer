import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

import { afterAll, describe, expect, it } from "vitest"

import { cropSpriteFrame } from "../renderer/png"
import { catalogObjects, objectSourceTables } from "./objects"
import { mapScriptBodies } from "./scripts"
import type { ObjectEvent } from "./types"

const sourceRoot = path.resolve(import.meta.dirname, "../../../../..", "game")
const outputRoot = fs.mkdtempSync(path.join(os.tmpdir(), "cartographer-objects-"))

const event = (graphicsId: string, overrides: Partial<ObjectEvent> = {}): ObjectEvent => ({
  graphics_id: graphicsId,
  x: 4,
  y: 9,
  elevation: 3,
  movement_type: "MOVEMENT_TYPE_NONE",
  movement_range_x: 0,
  movement_range_y: 0,
  trainer_type: "TRAINER_TYPE_NONE",
  trainer_sight_or_berry_tree_id: "0",
  script: "EventScript_None",
  flag: "FLAG_NONE",
  ...overrides,
})

const tables = objectSourceTables(sourceRoot)

afterAll(() => {
  fs.rmSync(outputRoot, { recursive: true, force: true })
})

describe("source-backed object graphics", () => {
  it("resolves NPCs, aliases, species expressions, static objects, and field effects", () => {
    const objects = catalogObjects(
      sourceRoot,
      outputRoot,
      [
        event("OBJ_EVENT_GFX_LITTLE_BOY"),
        event("OBJ_EVENT_GFX_ITEM_BALL"),
        event("OBJ_EVENT_GFX_SPECIES(PIKACHU)"),
        event("OBJ_EVENT_GFX_BREAKABLE_ROCK"),
        event("OBJ_EVENT_GFX_LIGHT_SPRITE"),
      ],
      tables,
    )

    expect(objects.map((object) => object.diagnostic)).toEqual([null, null, null, null, null])
    expect(objects.map((object) => object.kind.id)).toEqual([
      "npc",
      "item",
      "pokemon",
      "obstacle",
      "light",
    ])
    expect(objects.map((object) => object.sprite?.widthPixels)).toEqual([16, 16, 32, 16, 32])
    expect(objects.map((object) => object.sprite?.heightPixels)).toEqual([16, 32, 32, 16, 32])
    expect(objects[0]?.sprite?.source).toMatch(
      /graphics\/object_events\/pics\/people\/little_boy\.png$/,
    )
    expect(objects[1]?.sprite?.source).toMatch(
      /graphics\/object_events\/pics\/misc\/ball_poke\.png$/,
    )
    expect(objects[2]?.sprite?.source).toMatch(/graphics\/pokemon\/pikachu\/overworld\.png$/)
    expect(objects[3]?.sprite?.source).toMatch(
      /graphics\/object_events\/pics\/misc\/breakable_rock\.png$/,
    )
    expect(objects[4]?.sprite?.source).toMatch(/graphics\/object_events\/pics\/misc\/light\.png$/)

    for (const object of objects) {
      expect(object.sprite).not.toBeNull()
      expect(fs.existsSync(path.join(outputRoot, object.sprite!.path))).toBe(true)
    }
  })

  it("renders a source-backed static berry-tree preview and shiny species expression", () => {
    const objects = catalogObjects(
      sourceRoot,
      outputRoot,
      [
        event("OBJ_EVENT_GFX_BERRY_TREE", {
          movement_type: "MOVEMENT_TYPE_BERRY_TREE_GROWTH",
          trainer_sight_or_berry_tree_id: "BERRY_TREE_KELPSY_2",
        }),
        event("OBJ_EVENT_GFX_SPECIES_SHINY(PIKACHU)"),
      ],
      tables,
    )

    expect(objects.map((object) => object.diagnostic)).toEqual([null, null])
    expect(objects.map((object) => object.kind.id)).toEqual(["berry-tree", "pokemon"])
    expect(objects.map((object) => object.isShiny)).toEqual([false, true])
    expect(objects[0]?.sprite?.source).toMatch(
      /graphics\/object_events\/pics\/berry_trees\/kelpsy\.png$/,
    )
    expect(objects[0]?.sprite).toMatchObject({ widthPixels: 16, heightPixels: 32 })
    expect(objects[1]?.sprite?.source).toMatch(/graphics\/pokemon\/pikachu\/overworld\.png$/)
    expect(objects[1]?.sprite?.sha256).not.toBe(
      catalogObjects(sourceRoot, outputRoot, [event("OBJ_EVENT_GFX_SPECIES(PIKACHU)")], tables)[0]
        ?.sprite?.sha256,
    )
  })

  it("renders Route20_hns's legacy shiny Magikarp expression", () => {
    const route20 = JSON.parse(
      fs.readFileSync(path.join(sourceRoot, "data/maps/Route20_hns/map.json"), "utf8"),
    ) as { object_events: ObjectEvent[] }
    const magikarp = route20.object_events.find(
      (object) => object.graphics_id === "OBJ_EVENT_MON+SPECIES_MAGIKARP+OBJ_EVENT_MON_SHINY",
    )
    expect(magikarp).toBeDefined()

    const [object] = catalogObjects(sourceRoot, outputRoot, [magikarp!], tables)

    expect(object).toMatchObject({ kind: { id: "pokemon" }, isShiny: true, diagnostic: null })
    expect(object?.sprite?.source).toMatch(/graphics\/pokemon\/magikarp\/overworld\.png$/)
  })

  it("uses the source new-game berry assignment ahead of a tree ID's name", () => {
    const [tree] = catalogObjects(
      sourceRoot,
      outputRoot,
      [
        event("OBJ_EVENT_GFX_BERRY_TREE", {
          movement_type: "MOVEMENT_TYPE_BERRY_TREE_GROWTH",
          trainer_sight_or_berry_tree_id: "BERRY_TREE_ROUTE_123_POMEG_1",
        }),
      ],
      tables,
    )

    expect(tree?.diagnostic).toBeNull()
    expect(tree?.sprite?.source).toMatch(/graphics\/object_events\/pics\/berry_trees\/pomeg\.png$/)
  })

  it("classifies event roles and recognizes map-local item and battle scripts", () => {
    const scripts = mapScriptBodies(sourceRoot, "Route15_hns")
    const objects = catalogObjects(
      sourceRoot,
      outputRoot,
      [
        event("OBJ_EVENT_GFX_ITEM_BALL", { script: "Route15_EventScript_PPup" }),
        event("OBJ_EVENT_GFX_LITTLE_BOY", {
          script: "Route15_EventScript_Tommy",
          trainer_type: "TRAINER_TYPE_NORMAL",
        }),
      ],
      tables,
      scripts,
    )

    expect(objects.map((object) => object.kind)).toEqual([
      {
        id: "item",
        label: "Item",
        evidence: "graphics",
        action: "Gives PP Up",
      },
      {
        id: "trainer",
        label: "Trainer",
        evidence: "trainer-type",
        action: "Battles Tommy",
      },
    ])
  })

  it("crops indexed source PNGs to RGBA while retaining index-zero transparency", () => {
    const cropped = cropSpriteFrame(
      path.join(sourceRoot, "graphics/object_events/pics/people/little_boy.png"),
      { index: 1, width: 16, height: 16 },
    )

    expect(cropped).toMatchObject({ width: 16, height: 16 })
    const alpha = cropped.pixels.filter((_, index) => index % 4 === 3)
    expect(alpha).toContain(0)
    expect(alpha).toContain(255)
  })

  it("keeps unresolved graphics explicit instead of guessing a source asset", () => {
    const [object] = catalogObjects(
      sourceRoot,
      outputRoot,
      [event("OBJ_EVENT_GFX_NOT_A_REAL_GRAPHIC")],
      tables,
    )

    expect(object).toMatchObject({
      graphicsId: "OBJ_EVENT_GFX_NOT_A_REAL_GRAPHIC",
      sprite: null,
      diagnostic: {
        code: "unknown_graphics_id",
        message: expect.stringContaining("no graphics-info pointer"),
      },
    })
  })
})
