// Behavioral oracle transcribed from the standalone FireRed/LeafGreen map
// scripts under game/data/maps/*_Frlg. The harness deliberately normalizes
// every FRLG line/page/scroll control to a newline while retaining every word.

const pages = (...value: string[]): string => value.join("\n")

// The intro helper accepts the first canonical FRLG preset for each naming
// screen: RED for the player and GREEN for the rival.
export const frlgNames = { player: "RED", rival: "GREEN" } as const

export const frlgPalletDialogue = {
  bedroom: {
    nes: pages("RED played with the NES.", "…Okay!\nIt’s time to go!"),
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
      "MOM: RED!\nYou should take a quick rest.",
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
    playerHouseSign: "RED’s house",
    rivalHouseSign: "GREEN’s house",
    townSign: "PALLET TOWN\nShades of your journey await!",
    trainerTips: pages("TRAINER TIPS", "Press START to open the MENU!"),
  },
  lab: {
    arrival: [
      "GREEN: Gramps!\nI’m fed up with waiting!",
      pages(
        "OAK: GREEN?\nLet me think…",
        "Oh, that’s right, I told you to\ncome! Just wait!",
        "Here, RED.",
        "There are three POKéMON here.",
        "Haha!",
        "The POKéMON are held inside\nthese POKé BALLS.",
        "When I was young, I was a serious\nPOKéMON TRAINER.",
        "But now, in my old age, I have\nonly these three left.",
        "You can have one.\nGo on, choose!",
      ),
      "GREEN: Hey! Gramps! No fair!\nWhat about me?",
      "OAK: Be patient, GREEN.\nYou can have one, too!",
    ],
    leaveWithoutStarter: "OAK: Hey!\nDon’t go away yet!",
    rivalWaiting: pages(
      "GREEN: Heh, I don’t need to be\ngreedy like you. I’m mature!",
      "Go ahead and choose, RED!",
    ),
    rivalAfterStarter: "GREEN: My POKéMON looks a lot\ntougher than yours.",
    oakWaiting: pages(
      "OAK: Now, RED.",
      "Inside those three POKé BALLS are\nPOKéMON.",
      "Which one will you choose for\nyourself?",
    ),
    choices: {
      bulbasaur: pages(
        "I see! BULBASAUR is your choice.\nIt’s very easy to raise.",
        "So, RED, you want to go with\nthe GRASS POKéMON BULBASAUR?",
      ),
      squirtle: pages(
        "Hm! SQUIRTLE is your choice.\nIt’s one worth raising.",
        "So, RED, you’ve decided on the\nWATER POKéMON SQUIRTLE?",
      ),
      charmander: pages(
        "Ah! CHARMANDER is your choice.\nYou should raise it patiently.",
        "So, RED, you’re claiming the\nFIRE POKéMON CHARMANDER?",
      ),
    },
    energetic: "This POKéMON is really quite\nenergetic!",
    received: {
      bulbasaur: "RED received the BULBASAUR\nfrom PROF. OAK!",
      squirtle: "RED received the SQUIRTLE\nfrom PROF. OAK!",
      charmander: "RED received the CHARMANDER\nfrom PROF. OAK!",
    },
    nickname: {
      bulbasaur: "Do you want to give a nickname to\nthis BULBASAUR?",
      squirtle: "Do you want to give a nickname to\nthis SQUIRTLE?",
      charmander: "Do you want to give a nickname to\nthis CHARMANDER?",
    },
    rivalTake: "GREEN: I’ll take this one, then!",
    rivalReceived: {
      bulbasaur: "GREEN received the BULBASAUR\nfrom PROF. OAK!",
      squirtle: "GREEN received the SQUIRTLE\nfrom PROF. OAK!",
      charmander: "GREEN received the CHARMANDER\nfrom PROF. OAK!",
    },
    battleChallenge: pages(
      "GREEN: Wait, RED!\nLet’s check out our POKéMON!",
      "Come on, I’ll take you on!",
    ),
    rivalDefeat: "WHAT?\nUnbelievable!\nI picked the wrong POKéMON!",
    rivalVictory: "GREEN: Yeah!\nAm I great or what?",
    battleAftermath: pages(
      "GREEN: Okay! I’ll make my\nPOKéMON battle to toughen it up!",
      "RED! Gramps!\nSmell you later!",
    ),
    battleTutorial: {
      introduction: [
        pages(
          "OAK: Oh, for Pete’s sake…\nSo pushy, as always.",
          "RED.",
          "You’ve never had a POKéMON battle\nbefore, have you?",
          "A POKéMON battle is when TRAINERS\npit their POKéMON against each\nother.",
        ),
        "The TRAINER that makes the other\nTRAINER’s POKéMON faint by lowering\ntheir HP to “0,” wins.",
        pages(
          "But rather than talking about it,\nyou’ll learn more from experience.",
          "Try battling and see for yourself.",
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
        "But if you lose, RED, you end\nup paying prize money…",
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
      "OAK: RED, raise your young\nPOKéMON by making it battle.",
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
    potionAway: "RED put the POTION away in\nthe BAG’s ITEMS POCKET.",
    clerkRepeat: "Please come see us if you need\nPOKé BALLS for catching POKéMON.",
    ledges: pages(
      "See those ledges along the road?",
      "It’s a bit scary, but you can jump\nfrom them.",
      "You can get back to PALLET TOWN\nquicker that way.",
    ),
    sign: "ROUTE 1\nPALLET TOWN - VIRIDIAN CITY",
  },
  mart: {
    parcel: [
      "Hey!\nYou came from PALLET TOWN?",
      pages("You know PROF. OAK, right?", "His order came in.\nCan I get you to take it to him?"),
      "RED received OAK’S PARCEL\nfrom the POKéMON MART clerk.",
    ],
    repeat: "Okay, thanks! Please say hi to\nPROF. OAK for me, too.",
    woman: "This shop does good business in\nANTIDOTES, I’ve heard.",
    youngster: pages(
      "I’ve got to buy some POTIONS.",
      "You never know when your POKéMON\nwill need quick healing.",
    ),
  },
  parcelReturn: [
    pages(
      "OAK: Oh, RED!\nHow is my old POKéMON?",
      "Well, it seems to be growing more\nattached to you.",
      "You must be talented as a POKéMON\nTRAINER.",
      "What’s that?\nYou have something for me?",
    ),
    "RED delivered OAK’S PARCEL.",
    pages("Ah!", "It’s the custom POKé BALL!", "I had it on order.\nThank you!"),
    "GREEN: Gramps!",
    "GREEN: I almost forgot!\nWhat did you call me for?",
    "OAK: Oh, right!\nI have a request for you two.",
    pages(
      "On the desk there is my invention,\nthe POKéDEX!",
      "It automatically records data on\nPOKéMON you’ve seen or caught.",
      "It’s a high-tech encyclopedia!",
    ),
    "OAK: RED and GREEN.\nTake these with you.",
    "RED received the POKéDEX\nfrom PROF. OAK.",
    pages(
      "OAK: You can’t get detailed data\non POKéMON by just seeing them.",
      "You must catch them to obtain\ncomplete data.",
      "So, here are some tools for\ncatching wild POKéMON.",
    ),
    "RED received five POKé BALLS.",
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
    "GREEN: All right, Gramps!\nLeave it all to me!",
    pages(
      "RED, I hate to say it, but you\nwon’t be necessary for this.",
      "I know! I’ll borrow a TOWN MAP\nfrom my sis!",
      "I’ll tell her not to lend you one,\nRED! Hahaha!",
      "Don’t bother coming around to\nmy place after this!",
    ),
  ],
  daisy: {
    beforeBattle: pages("DAISY: Hi, RED!", "My brother, GREEN, is out at\nGrandpa's LAB."),
    afterBattle: pages(
      "DAISY: RED, I heard you had\na battle against GREEN.",
      "I wish I’d seen that!",
    ),
    mapOffer: pages(
      "Grandpa asked you to run an\nerrand?",
      "Gee, that’s lazy of him.\nHere, this will help you.",
    ),
    mapReceived: "RED received a TOWN MAP\nfrom DAISY.",
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

export const frlgStarterMatrix = [
  { slot: 0, player: "bulbasaur", rival: "charmander", ballX: 8 },
  { slot: 1, player: "squirtle", rival: "bulbasaur", ballX: 9 },
  { slot: 2, player: "charmander", rival: "squirtle", ballX: 10 },
] as const

export const frlgLabExitLanes = [5, 6, 7] as const
