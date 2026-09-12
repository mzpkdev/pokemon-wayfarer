import { moves, species, type FixtureSpecies, type Move } from "../catalog"
import { maxMoves, type MonFixtureWire, type PartyMonFixtureWire } from "../protocol"

export type MonFixture = {
  species: FixtureSpecies
  moves?: Move[]
  level?: number
  egg?: boolean
}

export type PartyMonFixture = MonFixture & {
  fainted?: boolean
}

export type ObservedPcSlotFixture = {
  box: number
  slot: number
  // The slot is both initialized to this value and retained in bounded state telemetry.
  mon: MonFixture | null
}

export const toWireMon = (mon: MonFixture): MonFixtureWire => ({
  species: species[mon.species],
  moves: Array.from({ length: maxMoves }, (_, index) => moves[mon.moves?.[index] ?? "none"]),
  level: mon.level ?? 20,
  egg: mon.egg ?? false,
})

export const toWirePartyMon = (mon: PartyMonFixture): PartyMonFixtureWire => ({
  ...toWireMon(mon),
  fainted: mon.fainted ?? false,
})

export const emptyWireMon = (): MonFixtureWire => ({
  species: species.none,
  moves: Array.from({ length: maxMoves }, () => moves.none),
  level: 0,
  egg: false,
})
