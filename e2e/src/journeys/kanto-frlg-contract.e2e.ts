import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  advanceOneExactDialogue,
  answerYesNo,
  expectExactDialogue,
  expectExactSequence,
  expectFidelityState,
  expectYesNo,
  interactForExactDialogue,
  visibleObject,
  walkExact,
} from "../playbooks/kanto-frlg-contract"
import {
  frlgLabExitLanes,
  frlgPalletDialogue as text,
  frlgStarterMatrix,
} from "../fixtures/kanto-frlg-contract-fixture"
import { playThroughNewGameIntro, type AppearanceStyle } from "../playbooks/new-game-intro"

const repeat = <T>(value: T, count: number): T[] => Array.from({ length: count }, () => value)

const reachFrlgLabChoiceWithoutWarp = async (
  game: GameSession,
  style: AppearanceStyle = 1,
): Promise<void> => {
  await playThroughNewGameIntro(game, "kanto", style)
  expectFidelityState(
    await game.state.read(),
    { map: "reds-house-2f", x: 5, y: 5, facing: "up", phase: 0, partySize: 0 },
    "FRLG bedroom handoff",
  )

  // The canonical run leaves the actual new-game bedroom on foot. These are
  // deliberate tile routes, not setup warps.
  await walkExact(game, ["right", "right", "right", "right", "up", "up", "up"], "bedroom stairs")
  expectFidelityState(
    await game.state.read(),
    { map: "reds-house-1f", x: 9, y: 3, facing: "down" },
    "FRLG downstairs arrival",
  )
  await walkExact(game, ["down"], "approach Mom")
  await game.player.move("left")
  await interactForExactDialogue(
    game,
    style % 2 === 1 ? text.home.momBoy : text.home.momGirl,
    "Mom before Oak",
  )
  await walkExact(game, ["down", "down", "down", "down"], "leave home")
  expectFidelityState(
    await game.state.read(),
    { map: "pallet-town", x: 6, y: 8, facing: "down" },
    "Pallet doorstep",
  )

  await walkExact(
    game,
    [
      ...repeat("right" as const, 3),
      "down",
      "right",
      "right",
      ...repeat("up" as const, 7),
      "right",
    ],
    "walk from home to Route 1 trigger",
  )
  await game.player.move("up")
  const waitSequence = await expectExactDialogue(
    game,
    text.town.oakWait,
    "Oak Route 1 interception",
  )
  await advanceOneExactDialogue(game, waitSequence, "Oak Route 1 interception")
  const warningSequence = await expectExactDialogue(game, text.town.oakWarning, "Oak grass warning")
  await advanceOneExactDialogue(game, warningSequence, "Oak grass warning")

  await game.wait.forMap("oak-lab")
  await expectExactSequence(game, text.lab.arrival, "Oak lab arrival")
  const staged = await game.state.read()
  expectFidelityState(
    staged,
    { map: "oak-lab", x: 6, y: 5, phase: 2, partySize: 0 },
    "starter staging",
  )
  const expectedObjects = [
    [4, 6, 3],
    [5, 8, 4],
    [6, 9, 4],
    [7, 10, 4],
    [8, 5, 4],
    [9, 4, 1],
    [10, 5, 1],
  ] as const
  for (const [localId, x, y] of expectedObjects) {
    const object = visibleObject(staged, localId, "FRLG lab staging")
    if (object.x !== x || object.y !== y)
      throw new Error(
        `FRLG lab staging: local object ${localId} expected at ${x}:${y}, observed ${object.x}:${object.y}`,
      )
  }
}

const chooseStarter = async (
  game: GameSession,
  starter: (typeof frlgStarterMatrix)[number],
  answer: "yes" | "no",
): Promise<void> => {
  // At FRLG's staging tile (6,5), each Poké Ball is directly above y=4.
  await walkExact(game, repeat("right" as const, starter.ballX - 6), `${starter.player} ball`)
  await game.player.move("up")
  await game.player.interact()
  await expectExactDialogue(
    game,
    text.lab.choices[starter.player],
    `${starter.player} confirmation`,
  )
  await expectYesNo(game, starter.player, `${starter.player} confirmation`)
  await answerYesNo(game, answer, `${starter.player} confirmation`)
  if (answer === "no") {
    await game.dialogue.waitForClosed()
    const declined = await game.state.read()
    if (declined.party.length !== 0 || declined.origin.pallet.phase !== 2)
      throw new Error(
        `${starter.player} NO mutated opening state: ${JSON.stringify(declined.origin.pallet)}`,
      )
    return
  }

  await expectExactSequence(
    game,
    [text.lab.energetic, text.lab.received[starter.player]],
    `${starter.player} receipt`,
  )
  await expectExactDialogue(
    game,
    text.lab.nickname[starter.player],
    `${starter.player} nickname prompt text`,
  )
  const nickname = await game.state.read()
  if (nickname.choice.kind !== "yes-no" || nickname.choice.cursor !== 0)
    throw new Error(`${starter.player} nickname prompt was not an exact default-YES choice`)
  await answerYesNo(game, "no", `${starter.player} nickname`)
  await expectExactSequence(
    game,
    [text.lab.rivalTake, text.lab.rivalReceived[starter.rival]],
    `${starter.player} rival counterpick`,
  )
  const received = await game.state.read()
  if (
    received.party.length !== 1 ||
    received.party[0]?.species !== starter.player ||
    received.origin.pallet.phase !== 3 ||
    received.origin.pallet.starterSlot !== starter.slot
  )
    throw new Error(
      `${starter.player} receipt/counterpick checkpoint mismatch: ${JSON.stringify({ party: received.party, opening: received.origin.pallet })}`,
    )
}

const interactWithObject = async (
  game: GameSession,
  localId: number,
  expected: string,
  context: string,
): Promise<void> => {
  const object = visibleObject(await game.state.read(), localId, context)
  await game.player.warp(
    (await game.state.read()).map.name as "pallet-town",
    object.x,
    object.y + 1,
    "up",
  )
  await interactForExactDialogue(game, expected, context)
}

describe.sequential("FireRed/LeafGreen Pallet opening contract", () => {
  it("follows the complete canonical opening without an administrative warp", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "kanto", 1)
      expectFidelityState(
        await game.state.read(),
        { map: "reds-house-2f", x: 5, y: 5, facing: "up", phase: 0, partySize: 0 },
        "canonical bedroom spawn",
      )

      // Bedroom interactions are part of the contract, before the same walking
      // route used by reachFirlgLabChoiceWithoutWarp.
      await walkExact(game, ["left", "up", "up"], "approach bedroom NES")
      await interactForExactDialogue(game, text.bedroom.nes, "bedroom NES")
      await walkExact(
        game,
        ["down", "down", "right", "right", "right", "right", "right", "up", "up", "up"],
        "bedroom to stairs",
      )
      await walkExact(game, ["down"], "approach Mom")
      await game.player.move("left")
      await interactForExactDialogue(game, text.home.momBoy, "Mom's original boy dialogue")
      await walkExact(game, ["down", "down", "down", "down"], "leave home")
      await walkExact(
        game,
        [
          ...repeat("right" as const, 3),
          "down",
          "right",
          "right",
          ...repeat("up" as const, 7),
          "right",
        ],
        "Pallet home to Route 1 trigger",
      )
      await game.player.move("up")
      await expectExactSequence(game, [text.town.oakWait, text.town.oakWarning], "Oak interception")
      await game.wait.forMap("oak-lab")
      await expectExactSequence(game, text.lab.arrival, "lab introduction")
      await chooseStarter(game, frlgStarterMatrix[0], "yes")

      // Center exit lane is the canonical one.
      await walkExact(game, ["left", "left", "down", "down", "down"], "lab center exit")
      const challenge = await expectExactDialogue(
        game,
        text.lab.battleChallenge,
        "first Blue challenge",
      )
      await advanceOneExactDialogue(game, challenge, "first Blue challenge")
      await game.wait.until((state) => state.battle.active, "first Blue battle", 1_200)
      const battle = await game.state.read()
      if (battle.battle.enemy?.species !== "charmander" || battle.battle.enemy.level !== 5)
        throw new Error(
          `Blue did not use the exact level-5 counter starter: ${JSON.stringify(battle.battle.enemy)}`,
        )
      await game.battle.win()
      await game.wait.until((state) => !state.battle.active, "first Blue battle end", 4_800)
      const battleResult = (await game.state.read()).battle.dialogue.text
      if (battleResult !== text.lab.rivalDefeat)
        throw new Error(
          `first Blue win text mismatch: expected ${JSON.stringify(text.lab.rivalDefeat)}, observed ${JSON.stringify(battleResult)}`,
        )
      await expectExactDialogue(game, text.lab.battleAftermath, "first Blue aftermath")
      await advanceOneExactDialogue(
        game,
        (await game.state.read()).dialogue.sequence,
        "first Blue aftermath",
      )
      const postBattle = await game.state.read()
      if (postBattle.origin.pallet.phase !== 4 || postBattle.partyVitals[0]?.hp === 0)
        throw new Error(
          `Blue battle did not heal and advance exactly once: ${JSON.stringify(postBattle)}`,
        )

      await walkExact(game, repeat("down" as const, 4), "leave Oak's lab")
      await game.wait.forMap("pallet-town")
      await walkExact(
        game,
        [...repeat("left" as const, 4), ...repeat("up" as const, 14), "up"],
        "Pallet to Route 1",
      )
      await game.wait.forMap("route-1")
      await walkExact(
        game,
        [
          ...repeat("up" as const, 7),
          ...repeat("left" as const, 4),
          ...repeat("up" as const, 4),
          "right",
          "up",
          ...repeat("right" as const, 3),
          ...repeat("up" as const, 6),
          ...repeat("right" as const, 6),
          ...repeat("up" as const, 10),
          "right",
          ...repeat("up" as const, 9),
          ...repeat("left" as const, 6),
          "up",
          "up",
          "left",
          "up",
        ],
        "Route 1 south-to-north",
      )
      await game.wait.forMap("viridian-city")
      await walkExact(
        game,
        [...repeat("up" as const, 21), ...repeat("right" as const, 14), "up"],
        "Viridian Mart approach",
      )
      await game.wait.forMap("viridian-mart")
      await expectExactSequence(game, text.mart.parcel, "Viridian Parcel one-shot")
      if ((await game.inventory.count("oaksParcel")) !== 1)
        throw new Error("Viridian clerk did not give exactly one Oak's Parcel")
      await game.saveAndReload()
      if ((await game.inventory.count("oaksParcel")) !== 1)
        throw new Error("Oak's Parcel did not survive reload exactly once")

      await game.player.interact()
      await expectExactDialogue(game, text.mart.repeat, "Viridian clerk repeat")
      await advanceOneExactDialogue(
        game,
        (await game.state.read()).dialogue.sequence,
        "Viridian clerk repeat",
      )
      if ((await game.inventory.count("oaksParcel")) !== 1)
        throw new Error("Viridian clerk duplicated Oak's Parcel")

      await walkExact(
        game,
        [
          ...repeat("down" as const, 15),
          ...repeat("left" as const, 14),
          ...repeat("down" as const, 10),
          "down",
        ],
        "return to Route 1",
      )
      await game.wait.forMap("route-1")
      await walkExact(
        game,
        [
          ...repeat("down" as const, 4),
          ...repeat("right" as const, 4),
          ...repeat("down" as const, 5),
          ...repeat("right" as const, 3),
          ...repeat("down" as const, 16),
          "left",
          "left",
          ...repeat("down" as const, 5),
          ...repeat("left" as const, 9),
          ...repeat("down" as const, 5),
          ...repeat("right" as const, 4),
          ...repeat("down" as const, 4),
          "down",
        ],
        "Route 1 north-to-south",
      )
      await game.wait.forMap("pallet-town")
      await walkExact(
        game,
        [...repeat("down" as const, 14), ...repeat("right" as const, 4), "up"],
        "return to Oak",
      )
      await game.wait.forMap("oak-lab")
      await walkExact(game, repeat("up" as const, 7), "approach Oak with Parcel")
      await game.player.interact()
      await expectExactSequence(game, text.parcelReturn, "Parcel, Pokédex, and five Balls")
      const complete = await game.state.read()
      if (
        complete.origin.pallet.phase !== 9 ||
        !complete.origin.pokedex ||
        (await game.inventory.count("oaksParcel")) !== 0 ||
        (await game.inventory.count("pokeBall")) !== 5
      )
        throw new Error(
          `terminal FRLG reward checkpoint mismatch: ${JSON.stringify(complete.origin)}`,
        )
      await game.saveAndReload()
      if (
        (await game.inventory.count("oaksParcel")) !== 0 ||
        (await game.inventory.count("pokeBall")) !== 5 ||
        !(await game.state.read()).origin.pokedex
      )
        throw new Error("terminal FRLG rewards were not exact-once across reload")

      // Isolated post-journey addendum: the no-warp canonical journey is
      // already complete above. Verify the opening's newly enabled Daisy
      // branch and its exact-once Town Map receipt without replaying 15 minutes.
      await game.player.warp("blues-house", 6, 4, "left")
      await game.player.interact()
      await expectExactSequence(
        game,
        [text.daisy.mapOffer, text.daisy.mapReceived],
        "Daisy Town Map receipt",
      )
      if ((await game.inventory.count("townMap")) !== 1)
        throw new Error("Daisy did not give exactly one Town Map")
      await game.player.interact()
      const mapExplanation = await expectExactDialogue(
        game,
        text.daisy.mapExplain,
        "Daisy Town Map explanation",
      )
      await advanceOneExactDialogue(game, mapExplanation, "Daisy Town Map explanation")
      await game.saveAndReload()
      await game.player.warp("blues-house", 6, 4, "left")
      await interactForExactDialogue(game, text.daisy.afterMap, "Daisy after Town Map reload")
      if ((await game.inventory.count("townMap")) !== 1)
        throw new Error("Daisy duplicated the Town Map after reload")
      await game.player.warp("oak-lab", 7, 2, "up")
      await interactForExactDialogue(game, text.lab.typeSign, "post-Pokédex lab type sign")
    } finally {
      await game.close()
    }
  }, 300_000)

  for (const style of [1, 2] as const) {
    it(`preserves Style ${style}'s gendered Mom and TV script`, async () => {
      const game = await GameSession.launch()
      try {
        await playThroughNewGameIntro(game, "kanto", style)
        await game.player.warp("reds-house-1f", 8, 4, "left")
        await interactForExactDialogue(
          game,
          style === 1 ? text.home.momBoy : text.home.momGirl,
          `Style ${style} Mom`,
        )
        await game.player.warp("reds-house-1f", 6, 2, "up")
        await interactForExactDialogue(
          game,
          style === 1 ? text.home.tvBoy : text.home.tvGirl,
          `Style ${style} TV`,
        )
        await game.player.warp("reds-house-1f", 5, 1, "right")
        await interactForExactDialogue(game, text.home.tvWrongSide, `Style ${style} TV wrong side`)
      } finally {
        await game.close()
      }
    }, 300_000)
  }

  it("preserves the bedroom NES and HELP notice independently", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "kanto", 1)
      await game.player.warp("reds-house-2f", 6, 6, "up")
      await interactForExactDialogue(game, text.bedroom.nes, "bedroom NES")
      await game.player.warp("reds-house-2f", 11, 2, "up")
      await interactForExactDialogue(game, text.bedroom.help, "bedroom HELP notice")
    } finally {
      await game.close()
    }
  }, 300_000)

  it("preserves opening-adjacent Pallet NPCs, signs, and Sign Lady text", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "kanto", 1)
      await game.player.warp("pallet-town", 12, 10, "up")
      await interactWithObject(game, 2, text.town.technology, "Pallet technology NPC")
      let lady = visibleObject(await game.state.read(), 1, "Pallet Sign Lady initial")
      await game.player.warp("pallet-town", lady.x, lady.y + 1, "up")
      await game.player.interact()
      await expectExactSequence(
        game,
        [text.town.signLadyInitial, text.town.signLadyLook],
        "Pallet Sign Lady initial choreography",
      )
      lady = visibleObject(await game.state.read(), 1, "Pallet Sign Lady read prompt")
      await game.player.warp("pallet-town", lady.x, lady.y + 1, "up")
      await interactForExactDialogue(game, text.town.signLadyRead, "Pallet Sign Lady read prompt")
      for (const interaction of [
        { x: 16, y: 17, facing: "up", expected: text.town.oakLabSign, label: "Oak Lab sign" },
        {
          x: 4,
          y: 8,
          facing: "up",
          expected: text.town.playerHouseSign,
          label: "player house sign",
        },
        {
          x: 13,
          y: 8,
          facing: "up",
          expected: text.town.rivalHouseSign,
          label: "rival house sign",
        },
        { x: 9, y: 12, facing: "up", expected: text.town.townSign, label: "Pallet town sign" },
        { x: 5, y: 15, facing: "up", expected: text.town.trainerTips, label: "Trainer Tips sign" },
      ] as const) {
        await game.player.warp("pallet-town", interaction.x, interaction.y, interaction.facing)
        await interactForExactDialogue(game, interaction.expected, interaction.label)
      }
      lady = visibleObject(await game.state.read(), 1, "Pallet Sign Lady after Trainer Tips")
      await game.player.warp("pallet-town", lady.x, lady.y + 1, "up")
      await interactForExactDialogue(
        game,
        text.town.signLadyDone,
        "Pallet Sign Lady after Trainer Tips",
      )
    } finally {
      await game.close()
    }
  }, 300_000)

  it("preserves Sign Lady's starter-ready copied-tip and finished branches", async () => {
    const game = await GameSession.launch()
    try {
      await reachFrlgLabChoiceWithoutWarp(game)
      await chooseStarter(game, frlgStarterMatrix[0], "yes")
      await game.player.warp("pallet-town", 3, 11, "up")
      let lady = visibleObject(await game.state.read(), 1, "starter-ready Sign Lady")
      await game.player.warp("pallet-town", lady.x, lady.y + 1, "up")
      await game.player.interact()
      await expectExactSequence(
        game,
        [text.town.signLadyCopied, text.town.signLadyCopiedTip],
        "Sign Lady copied Trainer Tips",
      )
      lady = visibleObject(await game.state.read(), 1, "Sign Lady just showed sign")
      await game.player.warp("pallet-town", lady.x, lady.y + 1, "up")
      await interactForExactDialogue(game, text.town.signLadyDone, "Sign Lady just showed sign")
      await game.player.warp("reds-house-1f", 9, 8, "down")
      await game.player.warp("pallet-town", 3, 11, "up")
      lady = visibleObject(await game.state.read(), 1, "Sign Lady finished")
      await game.player.warp("pallet-town", lady.x, lady.y + 1, "up")
      await interactForExactDialogue(game, text.town.raisingMons, "Sign Lady finished")
    } finally {
      await game.close()
    }
  }, 300_000)

  it("preserves Route 1's Potion, repeat, ledge advice, and sign exactly", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "kanto", 1)
      await game.player.warp("route-1", 8, 28, "left")
      await game.player.interact()
      await expectExactSequence(
        game,
        [text.route1.clerk, text.route1.obtainedPotion, text.route1.potionAway],
        "Route 1 sample Potion",
      )
      await game.saveAndReload()
      await game.player.warp("route-1", 8, 28, "left")
      await interactForExactDialogue(game, text.route1.clerkRepeat, "Route 1 clerk repeat")
      await game.player.warp("route-1", 19, 18, "up")
      await interactForExactDialogue(game, text.route1.ledges, "Route 1 ledge NPC")
      await game.player.warp("route-1", 9, 32, "up")
      await interactForExactDialogue(game, text.route1.sign, "Route 1 sign")
    } finally {
      await game.close()
    }
  }, 300_000)

  it("preserves both ambient Viridian Mart conversations during the Parcel visit", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "kanto", 1)
      await game.player.warp("viridian-mart", 6, 3, "up")
      await interactForExactDialogue(game, text.mart.youngster, "Viridian Mart youngster")
      await game.player.warp("viridian-mart", 9, 6, "up")
      await interactForExactDialogue(game, text.mart.woman, "Viridian Mart woman")
    } finally {
      await game.close()
    }
  }, 300_000)

  it("preserves unopened lab Poké Ball descriptions before Oak's scene", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "kanto", 1)
      await game.player.warp("oak-lab", 8, 5, "up")
      await interactForExactDialogue(game, text.lab.pokeBalls, "unopened Bulbasaur ball")
      await game.player.warp("oak-lab", 9, 5, "up")
      await interactForExactDialogue(game, text.lab.pokeBalls, "unopened Squirtle ball")
      await game.player.warp("oak-lab", 10, 5, "up")
      await interactForExactDialogue(game, text.lab.pokeBalls, "unopened Charmander ball")
    } finally {
      await game.close()
    }
  }, 300_000)

  it("preserves Daisy and her ambient house interactions before the lab battle", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "kanto", 1)
      await game.player.warp("blues-house", 6, 4, "left")
      await interactForExactDialogue(game, text.daisy.beforeBattle, "Daisy before Blue battle")
      await game.player.warp("blues-house", 6, 5, "up")
      await interactForExactDialogue(game, text.daisy.displayedMap, "Daisy's displayed Town Map")
      await game.player.warp("blues-house", 12, 2, "up")
      await interactForExactDialogue(game, text.daisy.bookshelf, "Daisy bookshelf")
      await game.player.warp("blues-house", 9, 2, "up")
      await interactForExactDialogue(game, text.daisy.picture, "Daisy Clefairy picture")
    } finally {
      await game.close()
    }
  }, 300_000)

  it("preserves Oak's lab ambient opening interactions", async () => {
    const game = await GameSession.launch()
    try {
      await reachFrlgLabChoiceWithoutWarp(game)
      for (const interaction of [
        { x: 5, y: 5, facing: "up", expected: text.lab.rivalWaiting, label: "Blue waiting" },
        { x: 6, y: 4, facing: "up", expected: text.lab.oakWaiting, label: "Oak waiting" },
        { x: 4, y: 2, facing: "up", expected: text.lab.blankPokedex, label: "blank Pokédex" },
        { x: 3, y: 12, facing: "up", expected: text.lab.aide, label: "left aide" },
        { x: 11, y: 11, facing: "up", expected: text.lab.aide, label: "right aide" },
        { x: 2, y: 11, facing: "up", expected: text.lab.authority, label: "female aide" },
        { x: 2, y: 2, facing: "up", expected: text.lab.computer, label: "lab PC e-mail" },
        { x: 6, y: 2, facing: "up", expected: text.lab.menuSign, label: "lab menu sign" },
        { x: 7, y: 2, facing: "up", expected: text.lab.saveSign, label: "lab save sign" },
      ] as const) {
        await game.player.warp("oak-lab", interaction.x, interaction.y, interaction.facing)
        await interactForExactDialogue(game, interaction.expected, interaction.label)
      }
      await game.player.warp("oak-lab", 6, 7, "down")
      await game.player.move("down")
      const refusal = await expectExactDialogue(
        game,
        text.lab.leaveWithoutStarter,
        "Oak blocks leaving without a starter",
      )
      await advanceOneExactDialogue(game, refusal, "Oak blocks leaving without a starter")
      const stillChoosing = await game.state.read()
      if (stillChoosing.party.length !== 0 || stillChoosing.origin.pallet.phase !== 2)
        throw new Error("starterless lab-exit refusal mutated the opening")
    } finally {
      await game.close()
    }
  }, 300_000)

  for (const starter of frlgStarterMatrix) {
    it(`${starter.player} NO is inert and YES gives it while Blue takes ${starter.rival}`, async () => {
      const game = await GameSession.launch()
      try {
        await reachFrlgLabChoiceWithoutWarp(game)
        await chooseStarter(game, starter, "no")
        await game.player.warp("oak-lab", 6, 5, "up")
        await chooseStarter(game, starter, "yes")
        await game.player.warp("oak-lab", 6, 4, "up")
        await interactForExactDialogue(
          game,
          text.lab.oakAfterStarter,
          `${starter.player} Oak guidance`,
        )
        await game.player.warp("oak-lab", 5, 5, "up")
        await interactForExactDialogue(
          game,
          text.lab.rivalAfterStarter,
          `${starter.player} rival boast`,
        )
        const remainingBallX = starter.slot === 0 ? 9 : starter.slot === 1 ? 10 : 8
        await game.player.warp("oak-lab", remainingBallX, 5, "up")
        await interactForExactDialogue(
          game,
          text.lab.lastBall,
          `${starter.player} leaves Oak's last Poké Ball`,
        )
      } finally {
        await game.close()
      }
    }, 300_000)
  }

  for (const lane of frlgLabExitLanes) {
    for (const outcome of ["win", "lose"] as const) {
      it(`runs Blue's ${outcome} branch from FRLG exit lane x=${lane}`, async () => {
        const game = await GameSession.launch()
        try {
          await reachFrlgLabChoiceWithoutWarp(game)
          await chooseStarter(game, frlgStarterMatrix[0], "yes")
          await game.player.warp("oak-lab", lane, 7, "down")
          await game.player.move("down")
          const challenge = await expectExactDialogue(
            game,
            text.lab.battleChallenge,
            `Blue ${outcome} lane ${lane}`,
          )
          await advanceOneExactDialogue(game, challenge, `Blue ${outcome} lane ${lane}`)
          await game.wait.until(
            (state) => state.battle.active,
            `Blue ${outcome} lane ${lane}`,
            1_200,
          )
          await game.battle[outcome]()
          await game.wait.until(
            (state) => !state.battle.active,
            `Blue ${outcome} lane ${lane} end`,
            4_800,
          )
          const expectedBattleResult =
            outcome === "win" ? text.lab.rivalDefeat : text.lab.rivalVictory
          const observedBattleResult = (await game.state.read()).battle.dialogue.text
          if (observedBattleResult !== expectedBattleResult)
            throw new Error(
              `Blue ${outcome} lane ${lane} battle text mismatch: expected ${JSON.stringify(expectedBattleResult)}, observed ${JSON.stringify(observedBattleResult)}`,
            )
          await expectExactDialogue(
            game,
            text.lab.battleAftermath,
            `Blue ${outcome} aftermath lane ${lane}`,
          )
          const resolved = await game.state.read()
          if (resolved.origin.pallet.phase !== 4)
            throw new Error(`Blue ${outcome} lane ${lane} did not resolve phase exactly to 4`)
          await advanceOneExactDialogue(
            game,
            resolved.dialogue.sequence,
            `Blue ${outcome} aftermath before Oak follow-up`,
          )
          await game.player.warp("oak-lab", 6, 4, "up")
          await interactForExactDialogue(
            game,
            text.lab.oakAfterBattle,
            `Oak after Blue ${outcome} lane ${lane}`,
          )
          if (lane === 5 && outcome === "win") {
            await game.player.warp("blues-house", 6, 4, "left")
            await interactForExactDialogue(game, text.daisy.afterBattle, "Daisy after Blue battle")
          }
          if (lane === 7 && outcome === "lose") {
            await game.player.warp("reds-house-1f", 9, 4, "left")
            await game.player.interact()
            await expectExactSequence(game, text.home.heal, "Mom post-battle heal")
            const healed = await game.state.read()
            if (healed.partyVitals.some(({ hp }) => hp === 0))
              throw new Error("Mom's post-battle rest left a fainted party member")
          }
        } finally {
          await game.close()
        }
      }, 300_000)
    }
  }

  it("locks source-only tutorial and Town Map bag-full branches", () => {
    // Forced battle outcomes do not play every optional Oak tutorial branch,
    // and the public session API cannot fill an in-progress Key Items pocket.
    // Keep these exact source contracts beside the adjacent runtime paths.
    expect(text.lab.battleTutorial.introduction).toHaveLength(3)
    expect(text.lab.battleTutorial.damage).toBe(
      "OAK: Inflicting damage on the foe\nis the key to any battle.",
    )
    expect(text.lab.battleTutorial.hp).toBe(
      "OAK: Keep your eyes on your\nPOKéMON’s HP.\nIt will faint if the HP drops to\n“0.”",
    )
    expect(text.lab.battleTutorial.stats).toBe(
      "OAK: Lowering the foe’s stats\nwill put you at an advantage.",
    )
    expect(text.lab.battleTutorial.noRunning).toBe(
      "OAK: No! There’s no running away\nfrom a TRAINER POKéMON battle!",
    )
    expect(text.daisy.mapBagFull).toBe("You don’t have space for this in\nyour BAG.")
  })
})
