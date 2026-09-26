// Behavioral oracle transcribed from the standalone FireRed/LeafGreen map
// scripts under game/data/maps/*_Frlg; see
// .product/research/frlg-pallet-opening-inventory.md. The harness normalizes
// every FRLG line/page/scroll control to a newline while retaining every word.

const pages = (...value: string[]): string => value.join("\n")

// Wayfarer keeps the Kanto rival as BLUE, so FRLG {RIVAL} is transcribed
// literally; {PLAYER} is the name chosen in the intro.
export const kantoRivalName = "BLUE"

const frlgPalletTemplate = {
  bedroom: {
    nes: pages("{PLAYER} played with the NES.", "…Okay!\nIt’s time to go!"),
    pcBoot: "{PLAYER} booted up the PC.",
    help: pages(
      "It’s a posted notice…",
      "If you’re confused, ask for HELP!\nPress the L or R Button!",
    ),
  },
  home: {
    momBoy: pages(
      "MOM: …Right.\nAll boys leave home someday.\nIt said so on TV.",
      "Oh, yes. PROF. OAK, next door, was\nlooking for you.",
    ),
    momGirl: pages(
      "MOM: …Right.\nAll girls dream of traveling.\nIt said so on TV.",
      "Oh, yes. PROF. OAK, next door, was\nlooking for you.",
    ),
    tvBoy: pages(
      "There’s a movie on TV.\nFour boys are walking on railroad\ntracks.",
      "…I better go, too.",
    ),
    tvGirl: pages(
      "There’s a movie on TV.\nA girl with her hair in pigtails is\nwalking up a brick road.",
      "…I better go, too.",
    ),
    tvWrongSide: "Oops, wrong side…",
    heal: [
      "MOM: {PLAYER}!\nYou should take a quick rest.",
      "MOM: Oh, good! You and your\nPOKéMON are looking great.\nTake care now!",
    ],
  },
  town: {
    oakWait: "OAK: Hey! Wait!\nDon’t go out!",
    oakWarning: pages(
      "OAK: It’s unsafe!\nWild POKéMON live in tall grass!",
      "You need your own POKéMON for\nyour protection.",
      "I know!\nHere, come with me!",
    ),
    signLadyInitial: "Hmm…\nIs that right…",
    signLadyLook: "Oh!\nLook, look!",
    signLadyRead: "Read it, read it!",
    signLadyCopied: pages(
      "Look, look!",
      "I copied what it said on one of\nthose TRAINER TIPS signs!",
    ),
    signLadyCopiedTip: pages("TRAINER TIPS!", "Press START to open the MENU!"),
    signLadyDone: "Signs are useful, aren’t they?",
    raisingMons: pages("I’m raising POKéMON, too.", "When they get strong, they can\nprotect me."),
    technology: pages(
      "Technology is incredible!",
      "You can now store and recall items\nand POKéMON as data via PC.",
    ),
    oakLabSign: "OAK POKéMON RESEARCH LAB",
    playerHouseSign: "{PLAYER}’s house",
    rivalHouseSign: "{RIVAL}’s house",
    townSign: "PALLET TOWN\nShades of your journey await!",
    trainerTips: pages("TRAINER TIPS", "Press START to open the MENU!"),
  },
  lab: {
    arrival: [
      "{RIVAL}: Gramps!\nI’m fed up with waiting!",
      pages(
        "OAK: {RIVAL}?\nLet me think…",
        "Oh, that’s right, I told you to\ncome! Just wait!",
        "Here, {PLAYER}.",
        "There are three POKéMON here.",
        "Haha!",
        "The POKéMON are held inside\nthese POKé BALLS.",
        "When I was young, I was a serious\nPOKéMON TRAINER.",
        "But now, in my old age, I have\nonly these three left.",
        "You can have one.\nGo on, choose!",
      ),
      "{RIVAL}: Hey! Gramps! No fair!\nWhat about me?",
      "OAK: Be patient, {RIVAL}.\nYou can have one, too!",
    ],
    grampsIsntAround: "{RIVAL}: What, it’s only {PLAYER}?\nGramps isn’t around.",
    leaveWithoutStarter: "OAK: Hey!\nDon’t go away yet!",
    rivalWaiting: pages(
      "{RIVAL}: Heh, I don’t need to be\ngreedy like you. I’m mature!",
      "Go ahead and choose, {PLAYER}!",
    ),
    rivalAfterStarter: "{RIVAL}: My POKéMON looks a lot\ntougher than yours.",
    oakWaiting: pages(
      "OAK: Now, {PLAYER}.",
      "Inside those three POKé BALLS are\nPOKéMON.",
      "Which one will you choose for\nyourself?",
    ),
    choices: {
      bulbasaur: pages(
        "I see! BULBASAUR is your choice.\nIt’s very easy to raise.",
        "So, {PLAYER}, you want to go with\nthe GRASS POKéMON BULBASAUR?",
      ),
      squirtle: pages(
        "Hm! SQUIRTLE is your choice.\nIt’s one worth raising.",
        "So, {PLAYER}, you’ve decided on the\nWATER POKéMON SQUIRTLE?",
      ),
      charmander: pages(
        "Ah! CHARMANDER is your choice.\nYou should raise it patiently.",
        "So, {PLAYER}, you’re claiming the\nFIRE POKéMON CHARMANDER?",
      ),
    },
    energetic: "This POKéMON is really quite\nenergetic!",
    received: {
      bulbasaur: "{PLAYER} received the BULBASAUR\nfrom PROF. OAK!",
      squirtle: "{PLAYER} received the SQUIRTLE\nfrom PROF. OAK!",
      charmander: "{PLAYER} received the CHARMANDER\nfrom PROF. OAK!",
    },
    nickname: {
      bulbasaur: "Do you want to give a nickname to\nthis BULBASAUR?",
      squirtle: "Do you want to give a nickname to\nthis SQUIRTLE?",
      charmander: "Do you want to give a nickname to\nthis CHARMANDER?",
    },
    rivalTake: "{RIVAL}: I’ll take this one, then!",
    rivalReceived: {
      bulbasaur: "{RIVAL} received the BULBASAUR\nfrom PROF. OAK!",
      squirtle: "{RIVAL} received the SQUIRTLE\nfrom PROF. OAK!",
      charmander: "{RIVAL} received the CHARMANDER\nfrom PROF. OAK!",
    },
    battleChallenge: pages(
      "{RIVAL}: Wait, {PLAYER}!\nLet’s check out our POKéMON!",
      "Come on, I’ll take you on!",
    ),
    rivalDefeat: "WHAT?\nUnbelievable!\nI picked the wrong POKéMON!",
    rivalVictory: "{RIVAL}: Yeah!\nAm I great or what?",
    battleAftermath: pages(
      "{RIVAL}: Okay! I’ll make my\nPOKéMON battle to toughen it up!",
      "{PLAYER}! Gramps!\nSmell you later!",
    ),
    battleTutorial: {
      introduction: [
        pages(
          "OAK: Oh, for Pete’s sake…\nSo pushy, as always.",
          "{PLAYER}.",
          "You’ve never had a POKéMON battle\nbefore, have you?",
          "A POKéMON battle is when TRAINERS\npit their POKéMON against each\nother.",
        ),
        "The TRAINER that makes the other\nTRAINER’s POKéMON faint by lowering\ntheir HP to “0,” wins.",
        pages(
          "But rather than talking about it,\nyou’ll learn more from experience.",
          "Try battling and see for yourself.",
        ),
      ],
      party: [
        "OAK: It’s important to get to know\nyour POKéMON thoroughly.",
        pages(
          "This is a list of your POKéMON,\n{PLAYER}.",
          "Open this to check the skills\nand moves of your POKéMON.",
          "You also choose POKéMON here if\nyou want to use an item on one.",
        ),
      ],
      damage: "OAK: Inflicting damage on the foe\nis the key to any battle.",
      hp: "OAK: Keep your eyes on your\nPOKéMON’s HP.\nIt will faint if the HP drops to\n“0.”",
      stats: "OAK: Lowering the foe’s stats\nwill put you at an advantage.",
      noRunning: "OAK: No! There’s no running away\nfrom a TRAINER POKéMON battle!",
      win: pages(
        "OAK: Hm! Excellent!",
        "If you win, you earn prize money,\nand your POKéMON will grow!",
        "Battle other TRAINERS and make\nyour POKéMON strong!",
      ),
      loss: pages(
        "OAK: Hm…\nHow disappointing…",
        "If you win, you earn prize money,\nand your POKéMON grow.",
        "But if you lose, {PLAYER}, you end\nup paying prize money…",
        "However, since you had no warning\nthis time, I’ll pay for you.",
        "But things won’t be this way once\nyou step outside these doors.",
        "That’s why you must strengthen your\nPOKéMON by battling wild POKéMON.",
      ),
    },
    oakAfterStarter: pages(
      "OAK: If a wild POKéMON appears,\nyour POKéMON can battle it.",
      "With it at your side, you should be\nable to reach the next town.",
    ),
    oakAfterBattle: pages(
      "OAK: {PLAYER}, raise your young\nPOKéMON by making it battle.",
      "It has to battle for it to grow.",
    ),
    lastBall: "That’s PROF. OAK’s last POKéMON.",
    pokeBalls: "Those are POKé BALLS.\nThey contain POKéMON!",
    blankPokedex: "It’s like an encyclopedia, but the\npages are blank.",
    aide: "I study POKéMON as PROF. OAK’s\nAIDE.",
    authority: pages(
      "PROF. OAK may not look like much,\nbut he’s the authority on POKéMON.",
      "Many POKéMON TRAINERS hold him in\nhigh regard.",
    ),
    menuSign: "Press START to open the MENU!",
    saveSign: "The SAVE option is on the MENU.\nUse it regularly.",
    typeSign: "All POKéMON types have strong and\nweak points against others.",
    computer: pages(
      "There’s an e-mail message here.",
      "…",
      "Finally!\nThe ultimate TRAINERS of the\nPOKéMON LEAGUE are ready to\ntake on all comers!",
      "Bring your best POKéMON and see\nhow you rate as a TRAINER!",
      "POKéMON LEAGUE HQ\nINDIGO PLATEAU",
      "PROF. OAK, please visit us!\n…",
    ),
  },
  route1: {
    clerk: pages(
      "Hi!\nI work at a POKéMON MART.",
      "It’s part of a convenient chain\nselling all sorts of items.",
      "Please, visit us in VIRIDIAN CITY.",
      "I know, I’ll give you a sample.\nHere you go!",
    ),
    obtainedPotion: "Obtained the POTION!",
    potionAway: "{PLAYER} put the POTION away in\nthe BAG’s ITEMS POCKET.",
    clerkRepeat: "Please come see us if you need\nPOKé BALLS for catching POKéMON.",
    ledges: pages(
      "See those ledges along the road?",
      "It’s a bit scary, but you can jump\nfrom them.",
      "You can get back to PALLET TOWN\nquicker that way.",
    ),
    sign: "ROUTE 1\nPALLET TOWN - VIRIDIAN CITY",
  },
  viridian: {
    privateProperty: pages(
      "I absolutely forbid you from\ngoing through here!",
      "This is private property!",
    ),
    coffee: pages(
      "Oh, Grandpa!\nDon’t be so mean!",
      "I’m so sorry.\nHe hasn’t had his coffee yet.",
    ),
    carry: pages(
      "Those POKé BALLS at your waist!\nYou have POKéMON, don’t you?",
      "It’s great that you can carry and\nuse POKéMON anytime, anywhere.",
    ),
    caterpillarAsk: "You want to know about the two\nkinds of caterpillar POKéMON?",
    caterpillarYes: pages(
      "CATERPIE has no poison,\nbut WEEDLE does.",
      "Watch that your POKéMON aren’t\nstabbed by WEEDLE’s POISON STING.",
    ),
    citySign: "VIRIDIAN CITY \nThe Eternally Green Paradise",
    gymOldMan: "This POKéMON GYM is always closed.\nI wonder who the LEADER is?",
    gymSign: "VIRIDIAN CITY POKéMON GYM",
    gymLocked: "VIRIDIAN GYM’s doors are locked…",
  },
  mart: {
    parcel: [
      "Hey!\nYou came from PALLET TOWN?",
      pages("You know PROF. OAK, right?", "His order came in.\nCan I get you to take it to him?"),
      "{PLAYER} received OAK’S PARCEL\nfrom the POKéMON MART clerk.",
    ],
    // The shared receive-item path appends the engine's put-away line; this
    // repo's FRLG build shows the same line with the engine item name.
    parcelPutAway: "{PLAYER} put away the PARCEL\nin the KEY ITEMS POCKET.",
    repeat: "Okay, thanks! Please say hi to\nPROF. OAK for me, too.",
    woman: "This shop does good business in\nANTIDOTES, I’ve heard.",
    youngster: pages(
      "I’ve got to buy some POTIONS.",
      "You never know when your POKéMON\nwill need quick healing.",
    ),
  },
  parcelReturn: [
    pages(
      "OAK: Oh, {PLAYER}!\nHow is my old POKéMON?",
      "Well, it seems to be growing more\nattached to you.",
      "You must be talented as a POKéMON\nTRAINER.",
      "What’s that?\nYou have something for me?",
    ),
    "{PLAYER} delivered OAK’S PARCEL.",
    pages("Ah! ", "It’s the custom POKé BALL!", "I had it on order.\nThank you!"),
    "{RIVAL}: Gramps!",
    "{RIVAL}: I almost forgot!\nWhat did you call me for?",
    "OAK: Oh, right!\nI have a request for you two.",
    pages(
      "On the desk there is my invention,\nthe POKéDEX!",
      "It automatically records data on\nPOKéMON you’ve seen or caught.",
      "It’s a high-tech encyclopedia!",
    ),
    "OAK: {PLAYER} and {RIVAL}.\nTake these with you.",
    "{PLAYER} received the POKéDEX\nfrom PROF. OAK.",
    pages(
      "OAK: You can’t get detailed data\non POKéMON by just seeing them.",
      "You must catch them to obtain\ncomplete data.",
      "So, here are some tools for\ncatching wild POKéMON.",
    ),
    "{PLAYER} received five POKé BALLS.",
    pages(
      "When a wild POKéMON appears,\nit’s fair game.",
      "Just throw a POKé BALL at it and\ntry to catch it!",
      "This won’t always work, however.",
      "A healthy POKéMON can escape.\nYou have to be lucky!",
    ),
    pages(
      "To make a complete guide on all\nthe POKéMON in the world…",
      "That was my dream!",
      "But, I’m too old.\nI can’t get the job done.",
      "So, I want you two to fulfill my\ndream for me.",
      "Get moving, you two.",
      "This is a great undertaking in\nPOKéMON history!",
    ),
    "{RIVAL}: All right, Gramps!\nLeave it all to me!",
    pages(
      "{PLAYER}, I hate to say it, but you\nwon’t be necessary for this.",
      "I know! I’ll borrow a TOWN MAP\nfrom my sis!",
      "I’ll tell her not to lend you one,\n{PLAYER}! Hahaha!",
      "Don’t bother coming around to\nmy place after this!",
    ),
  ],
  daisy: {
    beforeBattle: pages("DAISY: Hi, {PLAYER}!", "My brother, {RIVAL}, is out at\nGrandpa’s LAB."),
    afterBattle: pages(
      "DAISY: {PLAYER}, I heard you had\na battle against {RIVAL}.",
      "I wish I’d seen that!",
    ),
    mapOffer: pages(
      "Grandpa asked you to run an\nerrand?",
      "Gee, that’s lazy of him.\nHere, this will help you.",
    ),
    mapReceived: "{PLAYER} received a TOWN MAP\nfrom DAISY.",
    mapPutAway: "{PLAYER} put away the TOWN MAP\nin the KEY ITEMS POCKET.",
    mapBagFull: "You don’t have space for this in\nyour BAG.",
    mapExplain:
      "You can use the TOWN MAP to find\nout where you are, or check the\nnames of places.",
    afterMap: pages(
      "DAISY: Just like people, POKéMON\nare living things.",
      "When they get tired, please give\nthem a rest.",
    ),
    displayedMap: "It’s a big map of the KANTO region.\nNow this would be useful!",
    bookshelf: "The shelves are crammed full of\nbooks on POKéMON.",
    picture: "“The lovely and sweet\nCLEFAIRY”",
  },
} as const

type Expand<T> = T extends string ? string : { readonly [K in keyof T]: Expand<T[K]> }

const expand = <T>(value: T, player: string): Expand<T> =>
  (typeof value === "string"
    ? value.replaceAll("{PLAYER}", player).replaceAll("{RIVAL}", kantoRivalName)
    : Array.isArray(value)
      ? value.map((item) => expand(item, player))
      : Object.fromEntries(
          Object.entries(value as object).map(([key, item]) => [key, expand(item, player)]),
        )) as Expand<T>

export const frlgPalletDialogue = (player: string) => expand(frlgPalletTemplate, player)
export type FrlgPalletDialogue = ReturnType<typeof frlgPalletDialogue>

// `slot` is the Wayfarer local slot (KANTO_STARTER_SLOT_*). HNS lab geometry:
// each ball sits on row 12 and is taken from row 13; the
// only exit gap is (12..14,16), guarded from row 15.
export const frlgStarterMatrix = [
  { slot: 0, player: "bulbasaur", rival: "charmander", ballX: 15 },
  { slot: 2, player: "squirtle", rival: "bulbasaur", ballX: 16 },
  { slot: 1, player: "charmander", rival: "squirtle", ballX: 17 },
] as const

export const labExitLanes = [12, 13, 14] as const

const speciesNames = { bulbasaur: "BULBASAUR", squirtle: "SQUIRTLE", charmander: "CHARMANDER" }

/** Engine battle lines around Oak's tutorial for the first Blue battle. */
export const firstBattleLines = (starter: (typeof frlgStarterMatrix)[number], player: string) => {
  const text = frlgPalletDialogue(player).lab
  const rivalMon = speciesNames[starter.rival]
  return {
    intro: [
      `You are challenged by RIVAL ${kantoRivalName}!`,
      `RIVAL ${kantoRivalName} sent out ${rivalMon}!`,
      `Go! ${speciesNames[starter.player]}!`,
      ...text.battleTutorial.introduction,
    ],
    win: [
      `You defeated RIVAL ${kantoRivalName}!`,
      text.rivalDefeat,
      "You got ¥80 for winning!",
      text.battleTutorial.win,
    ],
    lose: [
      `${kantoRivalName}: ${rivalMon}, come back!`,
      text.rivalVictory,
      text.battleTutorial.loss,
    ],
  }
}
