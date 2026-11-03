// The world map: the whole overworld in the game's own tiles. See README.md.
export { type PlaceInfo, WorldMap } from "./WorldMap"
export { type Caption, type StopName, type StopOverlay, WorldTour } from "./WorldTour"
export { LivingClock } from "./LivingClock"
export { pageMinutes } from "./clock"
export {
  clockText,
  gameClock,
  LIVING_RATE,
  livingOverlay,
  loadLiving,
  periodAt,
  pickShip,
  shipReach,
  shipsAt,
  trainAt,
  type Living,
  type LivingWorld,
  type Ship,
  type Train,
} from "./living"
export {
  FRAMES as WALK_FRAMES,
  loadNotables,
  NOTABLES_SRC,
  notablesAt,
  notablesOn,
  notablesOverlay,
  pickNotable,
  type Notable,
  type Notables,
} from "./notables"
