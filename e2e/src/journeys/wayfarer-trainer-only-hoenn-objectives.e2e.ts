import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap, type PartyMonFixture } from "../harness/game-session"
import { maps, storyFlags, storyVars } from "../harness/game-session/catalog"

// Source Emerald IDs are explicitly banked: C/HNS aliases must never stand in
// for the state read by the assembled Hoenn map scripts.
const flag = (name: string, source: number) => {
  const key = `objective-${name}`
  ;(storyFlags as Record<string, number>)[key] = 0x6000 | source
  return key
}
const variable = (name: string, source: number) => {
  const key = `objective-${name}`
  ;(storyVars as Record<string, number>)[key] = 0x7000 | (source - 0x4000)
  return key
}
const map = (name: string, mapGroup: number, mapNum: number): GameMap => {
  ;(maps as Record<string, { mapGroup: number; mapNum: number }>)[name] = { mapGroup, mapNum }
  return name as GameMap
}
const rusturf = variable("rusturf", 0x409a)
const rustboro = variable("rustboro", 0x405a)
const brineyHouse = variable("briney-house", 0x4090)
const weather = variable("weather-institute", 0x40b3)
const seafloor = variable("seafloor", 0x40a2)
const space = variable("space-center", 0x409f)
const mossdeep = variable("mossdeep", 0x405d)
const slateport = variable("slateport", 0x4058)
const harbor = variable("harbor", 0x40a0)
const recoveredGoods = flag("recovered-goods", 0x08f)
const chimneyVictory = flag("chimney-victory", 0x08b)
const groudonAwakened = flag("groudon-awakened", 0x06f)
const spaceVictory = flag("space-victory", 0x075)
const stevenInteraction = flag("steven-interaction", 0x0cd)
const spaceMap = map("objective-space-center-2f", 45, 10)
const jaggedMap = map("objective-jagged-pass", 55, 13)
const cableMap = map("objective-chimney-cable-station", 50, 1)

type Scenario = {
  name: string
  map: GameMap
  x: number
  y: number
  trigger?: "up" | "down" | "frame"
  text: string
  vars: Record<string, number>
  flags: Record<string, boolean>
}

const scenarios: Scenario[] = [
  {
    name: "Rusturf Peeko and Goods guard",
    map: "rusturf-tunnel",
    x: 13,
    y: 6,
    text: "Youre in over your head",
    vars: { [rusturf]: 2 },
    flags: {
      [recoveredGoods]: false,
      [flag("rusturf-grunt", 0x36e)]: false,
      [flag("peeko", 0x370)]: false,
    },
  },
  {
    name: "Mt. Chimney Maxie",
    map: "mt-chimney",
    x: 13,
    y: 7,
    text: "Stay out of our way",
    vars: {},
    flags: {
      [chimneyVictory]: false,
      [flag("met-archie-meteor-falls", 0x0cf)]: true,
      [flag("chimney-magma", 0x39f)]: false,
      [flag("chimney-ordinary-trainers", 0x36d)]: true,
    },
  },
  {
    name: "Magma Hideout before Groudon awakening",
    map: "magma-hideout-4f",
    x: 16,
    y: 22,
    text: "Stay out of our way",
    vars: { [slateport]: 0, [harbor]: 0 },
    flags: { [groudonAwakened]: false, [flag("magma-hideout-actors", 0x359)]: false },
  },
  {
    name: "Weather Institute Shelly",
    map: "route119-weather-institute-2f",
    x: 4,
    y: 7,
    text: "Youre in over your head",
    vars: { [weather]: 0 },
    flags: { [flag("weather-aqua", 0x37a)]: false },
  },
  {
    name: "Seafloor Archie before crisis staging",
    map: "seafloor-cavern-room9",
    x: 17,
    y: 43,
    trigger: "up",
    text: "Youre in over your head",
    vars: { [seafloor]: 0 },
    flags: { [flag("seafloor-archie", 0x33c)]: true },
  },
  {
    name: "Space Center three-grunt chain",
    map: spaceMap,
    x: 13,
    y: 2,
    trigger: "frame",
    text: "Stay out of our way",
    vars: { [mossdeep]: 2, [space]: 1 },
    flags: { [spaceVictory]: false, [flag("space-magma", 0x35e)]: false },
  },
  {
    name: "Space Center Steven partner offer",
    map: spaceMap,
    x: 1,
    y: 7,
    text: "We need a POK",
    vars: { [mossdeep]: 2, [space]: 2 },
    flags: {
      [spaceVictory]: false,
      [stevenInteraction]: false,
      [flag("space-steven", 0x35f)]: false,
      [flag("space-magma", 0x35e)]: false,
    },
  },
]

const parties: { name: string; party: PartyMonFixture[] }[] = [
  { name: "empty", party: [] },
  { name: "fainted", party: [{ species: "rattata", fainted: true }] },
  { name: "Egg-only", party: [{ species: "rattata", egg: true }] },
]

const step = async (
  game: GameSession,
  direction: "up" | "down" | "left" | "right",
  x: number,
  y: number,
) => {
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y) return
    await game.controls.press(direction)
    await game.wait.frames(12)
  }
  throw new Error(
    `Cannot walk ${direction} to ${x},${y}: ${JSON.stringify(await game.state.read())}`,
  )
}

const finish = async (game: GameSession) => {
  for (let attempt = 0; attempt < 180; attempt++) {
    const state = await game.state.read()
    expect(state.battle.active).toBe(false)
    if (state.ready && !state.scriptActive && !state.dialogueOpen) return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`Refusal did not release control: ${JSON.stringify(await game.state.read())}`)
}

const fightThroughBattle = async (game: GameSession) => {
  let entered = false
  let sawFaint = false
  for (let attempt = 0; attempt < 1800; attempt++) {
    const state = await game.state.read()
    entered ||= state.battle.active
    if (entered) sawFaint ||= state.partyVitals.some((vital) => vital.hp === 0)
    if (entered && !state.battle.active) return sawFaint
    if (state.battle.ui === "action-menu") {
      const cursor = state.battle.cursor ?? 0
      if (cursor % 2) await game.controls.press("left")
      if (Math.floor(cursor / 2)) await game.controls.press("up")
      await game.controls.press("a")
      await game.wait.until((next) => next.battle.ui === "move-menu", "actual move selection")
      const moveCursor = (await game.state.read()).battle.cursor ?? 0
      if (moveCursor % 2) await game.controls.press("left")
      if (Math.floor(moveCursor / 2)) await game.controls.press("up")
      await game.controls.press("a")
    } else if (
      state.dialogueOpen ||
      state.scriptActive ||
      state.battle.ui === "text" ||
      state.battle.ui === "move-menu" ||
      state.battle.ui === "other"
    ) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `Actual Fight inputs did not complete battle: ${JSON.stringify(await game.state.read())}`,
  )
}

describe.sequential("Wayfarer trainer-only Hoenn local objectives", () => {
  for (const scenario of scenarios) {
    const variant = parties[0]!
    it(`${scenario.name}: ${variant.name} refuses before staging and keeps the objective pending`, async () => {
      const game = await GameSession.launch()
      try {
        const vars = { ...scenario.vars }
        if (scenario.trigger === "frame") vars[space] = 0
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: {
            position: { map: scenario.map, x: scenario.x, y: scenario.y },
            facing: scenario.name.includes("Steven") ? "down" : "up",
          },
          party: variant.party,
          story: { vars: vars as never, flags: scenario.flags as never },
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        if (scenario.trigger === "frame") await game.story.setVar(space as never, 1)
        else if (scenario.trigger === "up" || scenario.trigger === "down")
          await step(
            game,
            scenario.trigger,
            scenario.x,
            scenario.y + (scenario.trigger === "up" ? -1 : 1),
          )
        else await game.player.interact()
        let text = ""
        for (let attempt = 0; attempt < 180; attempt++) {
          const state = await game.state.read()
          expect(state.battle.active).toBe(false)
          text = state.dialogue.text
          if (text.includes(scenario.text)) break
          await game.wait.frames(12)
        }
        if (!text.includes(scenario.text)) {
          await fs.promises.writeFile(
            `/tmp/trainer-only-objective-failure-${scenario.name.replace(/[^a-z0-9]+/gi, "-")}.png`,
            await game.screenshot(),
          )
          throw new Error(
            `${scenario.name}: expected ${scenario.text}; state ${JSON.stringify(await game.state.read())}; vars ${JSON.stringify(await Promise.all(Object.keys(scenario.vars).map(async (name) => [name, await game.story.var(name as never)])))}`,
          )
        }
        if (variant.name === "empty") {
          await game.wait.frames(30)
          await fs.promises.writeFile(
            `/tmp/trainer-only-objective-${scenario.name.replace(/[^a-z0-9]+/gi, "-")}.png`,
            await game.screenshot(),
          )
        }
        await finish(game)
        for (const [name, value] of Object.entries(scenario.vars))
          expect(await game.story.var(name as never), `${scenario.name}: ${name}`).toBe(value)
        for (const [name, value] of Object.entries(scenario.flags))
          expect(await game.story.flag(name as never), `${scenario.name}: ${name}`).toBe(value)
        expect((await game.state.read()).money).toBe(3000)
        if (scenario.trigger === "up" || scenario.trigger === "down") {
          await step(game, scenario.trigger === "up" ? "down" : "up", scenario.x, scenario.y)
          await game.wait.forReady()
        }
      } finally {
        await game.close()
      }
    })
  }

  for (const { y, variant } of [
    { y: 4, variant: parties[1]! },
    { y: 5, variant: parties[2]! },
  ]) {
    it(`Rusturf west entrance row ${y}: ${variant.name} refuses before moving Grunt or Peeko`, async () => {
      const game = await GameSession.launch()
      const scenario = scenarios[0]!
      try {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: { position: { map: scenario.map, x: 8, y }, facing: "right" },
          party: variant.party,
          story: { vars: scenario.vars as never, flags: scenario.flags as never },
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        await step(game, "right", 9, y)
        await game.wait.until(
          (state) => state.dialogue.text.includes(scenario.text),
          "Rusturf west entrance refusal before staging",
        )
        await finish(game)
        expect(await game.story.var(rusturf as never)).toBe(2)
        expect(await game.story.flag(recoveredGoods as never)).toBe(false)
        await step(game, "left", 8, y)
        await game.wait.forReady()
        expect((await game.state.read()).money).toBe(3000)
      } finally {
        await game.close()
      }
    })
  }

  it("Rusturf west approach retains native Grunt and Peeko staging with a usable party", async () => {
    const game = await GameSession.launch()
    const scenario = scenarios[0]!
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: scenario.map, x: 8, y: 4 }, facing: "right" },
        party: [{ species: "rattata", level: 5 }],
        story: { vars: scenario.vars as never, flags: scenario.flags as never },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await step(game, "right", 9, 4)
      await game.wait.until((state) => state.dialogueOpen, "native Rusturf backup dialogue")
      await finish(game)
      expect(await game.story.var(rusturf as never)).toBe(3)
      expect(await game.story.flag(recoveredGoods as never)).toBe(false)
      await step(game, "left", 8, 4)
    } finally {
      await game.close()
    }
  })

  for (const scenario of scenarios.slice(0, 5)) {
    it(`${scenario.name}: actual defeat returns locally without completing or rewarding the objective`, async () => {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: { position: { map: scenario.map, x: scenario.x, y: scenario.y }, facing: "up" },
          party: [{ species: "rattata", level: 1, moves: ["tackle"] }],
          story: { vars: scenario.vars as never, flags: scenario.flags as never },
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        const before = await game.state.read()
        if (scenario.trigger === "up") await step(game, "up", scenario.x, scenario.y - 1)
        else await game.player.interact()
        expect(await fightThroughBattle(game)).toBe(true)
        await finish(game)
        const after = await game.state.read()
        expect(after.map).toEqual(before.map)
        expect(after.partyVitals[0]?.hp).toBe(0)
        expect(after.money).toBe(before.money - Math.min(before.money, 8))
        for (const [name, value] of Object.entries(scenario.vars))
          expect(await game.story.var(name as never), name).toBe(value)
        for (const [name, value] of Object.entries(scenario.flags))
          expect(await game.story.flag(name as never), name).toBe(value)
        // A second approach is a fresh refusal, without another loss charge.
        if (scenario.trigger === "up") {
          await step(game, "down", scenario.x, scenario.y)
          await step(game, "up", scenario.x, scenario.y - 1)
        } else {
          // Maxie's native scene leaves the player facing away from him.
          if ((await game.state.read()).player.facing !== "up") {
            await game.controls.press("up")
            await game.wait.frames(12)
          }
          await game.player.interact()
        }
        await game.wait.until(
          (state) => state.dialogue.text.includes(scenario.text),
          "post-loss objective refusal",
        )
        await finish(game)
        expect((await game.state.read()).money).toBe(after.money)
      } finally {
        await game.close()
      }
    })
  }

  it("a usable party still earns Rusturf's real rescue completion through a battle victory", async () => {
    const game = await GameSession.launch()
    const scenario = scenarios[0]!
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: scenario.map, x: scenario.x, y: scenario.y }, facing: "up" },
        party: [{ species: "rattata", level: 100, moves: ["tackle"] }],
        story: {
          vars: { ...scenario.vars, [rustboro]: 0, [brineyHouse]: 0 } as never,
          flags: scenario.flags as never,
        },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.player.interact()
      expect(await fightThroughBattle(game)).toBe(false)
      await finish(game)
      expect(await game.story.flag(recoveredGoods as never)).toBe(true)
      expect(await game.story.var(rusturf as never)).toBe(2)
      expect(await game.story.var(rustboro as never)).toBe(4)
      expect(await game.story.var(brineyHouse as never)).toBe(1)
    } finally {
      await game.close()
    }
  })

  it("keeps the Chimney cable-car and Jagged Pass exits open before Maxie's victory", async () => {
    const game = await GameSession.launch()
    try {
      for (const destination of [
        { x: 20, y: 40, target: jaggedMap, returnDirection: "up" as const },
        { x: 17, y: 35, target: cableMap, returnDirection: "down" as const },
      ]) {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: {
            position: { map: "mt-chimney", x: destination.x, y: destination.y },
            facing: "down",
          },
          party: [],
          story: { flags: { [chimneyVictory]: false } as never },
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        for (let attempt = 0; attempt < 30; attempt++) {
          if ((await game.state.read()).map.name === destination.target) break
          await game.controls.press("down")
          await game.wait.frames(20)
        }
        await game.wait.forReady()
        expect((await game.state.read()).map.name).toBe(destination.target)
        expect(await game.story.flag(chimneyVictory as never)).toBe(false)
        for (let attempt = 0; attempt < 30; attempt++) {
          if ((await game.state.read()).map.name === "mt-chimney") break
          await game.controls.press(destination.returnDirection)
          await game.wait.frames(20)
        }
        await game.wait.forReady()
        expect((await game.state.read()).map.name).toBe("mt-chimney")
        expect(await game.story.flag(chimneyVictory as never)).toBe(false)
      }
    } finally {
      await game.close()
    }
  })
})
