import { describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"
import { advanceOpeningUntil } from "../playbooks/regional-opening"

const walkTo = async (game: GameSession, x: number, y: number): Promise<void> => {
  for (let attempt = 0; attempt < 100; attempt++) {
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y) return
    await game.player.move(
      state.player.x < x
        ? "right"
        : state.player.x > x
          ? "left"
          : state.player.y < y
            ? "down"
            : "up",
    )
    await game.wait.frames(16)
  }
  throw new Error(`Could not reach ${x},${y}: ${JSON.stringify(await game.state.read())}`)
}

const enterNorth = async (game: GameSession, destination: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 120; attempt++) {
    const state = await game.state.read()
    if (state.map.name === destination) {
      await advanceOpeningUntil(
        game,
        (current) => current.ready,
        `${destination} entry remained locked`,
      )
      return
    }
    if (state.dialogueOpen || state.scriptActive) await game.controls.press("a")
    else await game.player.move("up")
    await game.wait.frames(16)
  }
  throw new Error(`Could not enter ${destination}: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Hoenn-origin League admission without the maiden voyage", () => {
  // These fixtures supply badge counts and prior clears. They verify admission
  // and the first battle, not a played journey through all 24 badges or Leagues.
  for (const stage of [
    {
      region: "kanto",
      badges: { hoenn: 8 },
      clears: {},
      lobby: "indigo-league-lobby",
      room: "league-will",
      x: 31,
      y: 4,
    },
    {
      region: "johto",
      badges: { johto: 8, hoenn: 8 },
      clears: { kanto: true },
      lobby: "indigo-league-lobby",
      room: "league-will",
      x: 31,
      y: 4,
    },
    {
      region: "hoenn",
      badges: { kanto: 8, johto: 8, hoenn: 8 },
      clears: { kanto: true, johto: true },
      lobby: "hoenn-league-lobby",
      room: "league-sidney",
      x: 9,
      y: 3,
    },
  ] as const) {
    it(`admits the ${stage.region} stage and preserves its origin through save/reload`, async () => {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint: "hoenn-before-rescue",
          player: { facing: "up", position: { map: stage.lobby, x: stage.x, y: stage.y } },
          party: [{ species: "lapras", level: 100, moves: ["surf"] }],
          circuit: { badges: stage.badges, clears: stage.clears },
          determinism: { textSpeed: "instant" },
        })
        const before = await game.state.read()
        expect(before).toMatchObject({
          origin: { id: 2, maidenVoyageState: 0, johtoCommitted: false, hoennReceived: false },
          circuit: { leagues: { [stage.region]: "available" }, run: { active: false } },
        })
        expect(await game.inventory.contains("ssTicket")).toBe(false)
        if (stage.region === "hoenn") {
          await game.player.interact()
          await advanceOpeningUntil(
            game,
            (state) => state.ready,
            "Hoenn admission dialogue did not finish",
          )
          await enterNorth(game, "hoenn-league-hall5")
        } else {
          await walkTo(game, 32, 4)
        }
        await enterNorth(game, stage.room)
        const admitted = await game.state.read()
        expect(admitted).toMatchObject({
          map: { name: stage.room },
          origin: { id: 2, maidenVoyageState: 0, johtoCommitted: false, hoennReceived: false },
          circuit: {
            run: {
              active: true,
              region: stage.region,
              ratingAtEntry: before.circuit.trainerRating,
            },
          },
        })
        await game.saveAndReload()
        expect((await game.state.read()).origin).toEqual(admitted.origin)
        expect((await game.state.read()).circuit.run).toEqual(admitted.circuit.run)
        await walkTo(game, 6, stage.region === "hoenn" ? 6 : 7)
        await game.player.interact()
        await advanceOpeningUntil(
          game,
          (state) => state.battle.ui === "action-menu",
          "first League battle did not start",
        )
        expect(await game.state.read()).toMatchObject({
          origin: { id: 2, maidenVoyageState: 0, johtoCommitted: false, hoennReceived: false },
          circuit: {
            clears: { [stage.region]: false },
            run: {
              active: true,
              region: stage.region,
              ratingAtEntry: before.circuit.trainerRating,
            },
          },
          battle: { active: true },
        })
        expect(await game.inventory.contains("ssTicket")).toBe(false)
      } finally {
        await game.close()
      }
    }, 180_000)
  }
})
