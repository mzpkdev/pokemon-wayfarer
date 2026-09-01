import { type SkyEmuButton } from "../skyemu/client"

export const checkpoints = {
  "bedroom-before-clock": 1,
  "new-bark-after-intro": 2,
  "elm-lab-before-intro": 3,
} as const

export const maps = {
  "new-bark-town": { mapGroup: 0, mapNum: 0 },
  "azalea-town": { mapGroup: 0, mapNum: 3 },
  "route-33": { mapGroup: 0, mapNum: 15 },
  "route-41": { mapGroup: 0, mapNum: 23 },
  "elm-lab": { mapGroup: 1, mapNum: 0 },
  "players-house-1f": { mapGroup: 1, mapNum: 3 },
  "players-bedroom": { mapGroup: 1, mapNum: 4 },
  "blackthorn-move-deleter": { mapGroup: 10, mapNum: 5 },
  "test-map-1": { mapGroup: 30, mapNum: 27 },
} as const

export const storyVars = {
  newBarkTownLabState: 0x4074,
  newBarkTownState: 0x4075,
} as const

export const storyFlags = {
  hideSilverInNewBark: 0x04a,
  hideElmLabAide: 0x04b,
  adventureStarted: 0x1d4,
  momVisited: 0x265,
  badge1: 0x880,
  badge2: 0x881,
  badge3: 0x882,
  badge4: 0x883,
  badge5: 0x884,
  badge6: 0x885,
  badge7: 0x886,
  badge8: 0x887,
} as const

export const species = {
  none: 0,
  pidgey: 16,
  rattata: 19,
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
  wooper: 194,
  quagsire: 195,
} as const

export const moves = {
  none: 0,
  cut: 15,
  fly: 19,
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
export type Move = keyof typeof moves
export type Hm = keyof typeof hms
export type PartyMenuAction = keyof typeof partyMenuActions
