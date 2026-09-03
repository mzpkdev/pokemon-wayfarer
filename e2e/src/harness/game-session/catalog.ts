import { type SkyEmuButton } from "../skyemu/client"

export const checkpoints = {
  "bedroom-before-clock": 1,
  "new-bark-after-intro": 2,
  "elm-lab-before-intro": 3,
} as const

export const maps = {
  "new-bark-town": { mapGroup: 0, mapNum: 0 },
  "cherrygrove-city": { mapGroup: 0, mapNum: 1 },
  "azalea-town": { mapGroup: 0, mapNum: 3 },
  "mahogany-town": { mapGroup: 0, mapNum: 9 },
  "route-30": { mapGroup: 0, mapNum: 12 },
  "route-32": { mapGroup: 0, mapNum: 14 },
  "route-36": { mapGroup: 0, mapNum: 18 },
  "route-40": { mapGroup: 0, mapNum: 22 },
  "route-33": { mapGroup: 0, mapNum: 15 },
  "route-41": { mapGroup: 0, mapNum: 23 },
  "route-44": { mapGroup: 0, mapNum: 26 },
  "saffron-city": { mapGroup: 0, mapNum: 38 },
  "route-5": { mapGroup: 0, mapNum: 45 },
  "route-6": { mapGroup: 0, mapNum: 46 },
  "route-7": { mapGroup: 0, mapNum: 47 },
  "route-8": { mapGroup: 0, mapNum: 48 },
  "route-13": { mapGroup: 0, mapNum: 53 },
  "route-16": { mapGroup: 0, mapNum: 56 },
  "route-17": { mapGroup: 0, mapNum: 57 },
  "route-18": { mapGroup: 0, mapNum: 58 },
  "route-21": { mapGroup: 0, mapNum: 61 },
  "pallet-town": { mapGroup: 0, mapNum: 31 },
  "vermilion-city": { mapGroup: 0, mapNum: 35 },
  "cinnabar-island": { mapGroup: 0, mapNum: 40 },
  "olivine-city": { mapGroup: 0, mapNum: 6 },
  "cianwood-city": { mapGroup: 0, mapNum: 7 },
  "elm-lab": { mapGroup: 1, mapNum: 0 },
  "players-house-1f": { mapGroup: 1, mapNum: 3 },
  "players-bedroom": { mapGroup: 1, mapNum: 4 },
  "cherrygrove-pokemon-center": { mapGroup: 2, mapNum: 0 },
  "olivine-house-3": { mapGroup: 7, mapNum: 6 },
  "olivine-port-inside": { mapGroup: 7, mapNum: 8 },
  "blackthorn-move-deleter": { mapGroup: 10, mapNum: 5 },
  "vermilion-port-inside": { mapGroup: 15, mapNum: 3 },
  "vermilion-fan-club": { mapGroup: 15, mapNum: 5 },
  "goldenrod-train-station": { mapGroup: 5, mapNum: 14 },
  "cerulean-gym": { mapGroup: 14, mapNum: 5 },
  "saffron-train-station": { mapGroup: 18, mapNum: 2 },
  "copycats-house-2f": { mapGroup: 18, mapNum: 8 },
  "hall-of-fame": { mapGroup: 21, mapNum: 6 },
  "reception-gate": { mapGroup: 22, mapNum: 22 },
  "mt-silver-pokemon-center": { mapGroup: 22, mapNum: 34 },
  "power-plant-entrance": { mapGroup: 23, mapNum: 20 },
  "power-plant-back-room": { mapGroup: 23, mapNum: 21 },
  "saffron-route-7-gate": { mapGroup: 23, mapNum: 14 },
  "saffron-route-8-gate": { mapGroup: 23, mapNum: 18 },
  "celadon-route-16-gate": { mapGroup: 23, mapNum: 23 },
  "fuchsia-route-18-gate": { mapGroup: 23, mapNum: 24 },
  "ilex-forest": { mapGroup: 24, mapNum: 13 },
  "ice-path-1f": { mapGroup: 24, mapNum: 25 },
  "mt-moon-cave": { mapGroup: 24, mapNum: 57 },
  "snowswept-cavern": { mapGroup: 28, mapNum: 0 },
  "new-sinjoh": { mapGroup: 28, mapNum: 4 },
  "route-32-pokemon-center": { mapGroup: 22, mapNum: 27 },
  "route-12-house": { mapGroup: 23, mapNum: 25 },
  "ss-aqua-1f": { mapGroup: 30, mapNum: 8 },
  "ss-aqua-b1f": { mapGroup: 30, mapNum: 9 },
  "ss-aqua-captains-room": { mapGroup: 30, mapNum: 10 },
  "ss-aqua-room-nw": { mapGroup: 30, mapNum: 12 },
  "ss-aqua-room-sse": { mapGroup: 30, mapNum: 16 },
  "test-map-1": { mapGroup: 30, mapNum: 27 },
} as const

export const storyVars = {
  newBarkTownLabState: 0x4074,
  newBarkTownState: 0x4075,
  azaleaTownState: 0x4053,
  cherrygroveCityState: 0x405a,
  fanClubClefairy: 0x4065,
  kantoRocketStoryState: 0x406c,
  leagueState: 0x4070,
  numBadges: 0x4076,
  pewterCityState: 0x4079,
  saffronCityState: 0x40bd,
  ssAquaState: 0x408b,
  starterMon: 0x4023,
  train: 0x408e,
  vermilionCityState: 0x408f,
  violetCityState: 0x4091,
} as const

export const storyFlags = {
  hideSilverInNewBark: 0x04a,
  hideElmLabAide: 0x04b,
  adventureStarted: 0x1d4,
  momVisited: 0x265,
  standardRodRoute32Contributed: 0x304,
  standardRodOlivineContributed: 0x305,
  standardRodRoute12Contributed: 0x306,
  magnetTrainRestorationStarted: 0x307,
  hideSudowoodo: 0x036,
  hideVermilionSnorlax: 0x03a,
  hideSilverCherrygrove: 0x051,
  hideSproutTowerSilver: 0x058,
  hideAzaleaSilver: 0x05c,
  hideIndigoPlateauSilver: 0x0a6,
  hideMtMoonSilver: 0x0aa,
  hideCopycatClefairyDoll: 0x0d2,
  hideFanClubClefairyDoll: 0x0d3,
  hiddenMachinePart: 0x1d3,
  returnedMachinePart: 0x1dd,
  receivedTogepiEgg: 0x1ff,
  kantoRadioGot: 0x261,
  defeatedVioletGym: 0x227,
  cyclingRoad: 0x896,
  isChampion: 0x89f,
  visitedNewBarkTown: 0x8f0,
  visitedKanto: 0x8ff,
  visitedVermilionCity: 0x904,
  visitedSaffronCity: 0x907,
  badge1: 0x880,
  badge2: 0x881,
  badge3: 0x882,
  badge4: 0x883,
  badge5: 0x884,
  badge6: 0x885,
  badge7: 0x886,
  badge8: 0x887,
  badge9: 0x888,
} as const

export const species = {
  none: 0,
  pidgey: 16,
  rattata: 19,
  zubat: 41,
  poliwag: 60,
  geodude: 74,
  onix: 95,
  krabby: 98,
  goldeen: 118,
  magikarp: 129,
  gyarados: 130,
  lapras: 131,
  chikorita: 152,
  cyndaquil: 155,
  totodile: 158,
  sentret: 161,
  furret: 162,
  sudowoodo: 185,
  wooper: 194,
  quagsire: 195,
  kyogre: 382,
} as const

export const moves = {
  none: 0,
  cut: 15,
  fly: 19,
  tackle: 33,
  surf: 57,
  strength: 70,
  waterfall: 127,
  flash: 148,
  rockSmash: 249,
  whirlpool: 250,
  dive: 291,
} as const

export const hms = {
  cut: 682,
  fly: 683,
  surf: 684,
  strength: 685,
  flash: 686,
  rockSmash: 687,
  waterfall: 688,
  whirlpool: 689,
} as const

export const items = {
  masterBall: 4,
  miracleSeed: 429,
  metalCoat: 441,
  tmThunder: 606,
  bicycle: 706,
  oldRod: 709,
  goodRod: 710,
  superRod: 711,
  ssTicket: 727,
  lostItem: 882,
  machinePart: 883,
  pass: 885,
  squirtBottle: 892,
} as const

export const partyMenuActions = {
  cut: 34,
  flash: 35,
  rockSmash: 36,
  strength: 37,
  surf: 38,
  fly: 39,
  dive: 40,
  waterfall: 41,
  whirlpool: 42,
} as const

export const directions = {
  down: 1,
  up: 2,
  left: 3,
  right: 4,
} as const

export const buttons = {
  a: "A",
  b: "B",
  down: "Down",
  left: "Left",
  r: "R",
  right: "Right",
  start: "Start",
  up: "Up",
} as const satisfies Record<string, SkyEmuButton>

export const textSpeeds = {
  fast: 2,
  instant: 3,
} as const

export type Checkpoint = keyof typeof checkpoints
export type GameMap = keyof typeof maps
export type Direction = keyof typeof directions
export type StoryVar = keyof typeof storyVars
export type StoryFlag = keyof typeof storyFlags
export type Button = keyof typeof buttons
export type TextSpeed = keyof typeof textSpeeds
export type Species = keyof typeof species
export type FixtureSpecies = Exclude<Species, "none">
export type Move = keyof typeof moves
export type Hm = keyof typeof hms
export type Item = keyof typeof items
export type PartyMenuAction = keyof typeof partyMenuActions
