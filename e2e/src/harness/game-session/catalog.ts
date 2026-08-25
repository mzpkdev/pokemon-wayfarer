import { type SkyEmuButton } from "../skyemu/client"

export const checkpoints = {
  "bedroom-before-clock": 1,
  "new-bark-after-intro": 2,
  "elm-lab-before-intro": 3,
} as const

export const maps = {
  "new-bark-town": { mapGroup: 0, mapNum: 0 },
  "elm-lab": { mapGroup: 1, mapNum: 0 },
  "players-house-1f": { mapGroup: 1, mapNum: 3 },
  "players-bedroom": { mapGroup: 1, mapNum: 4 },
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
