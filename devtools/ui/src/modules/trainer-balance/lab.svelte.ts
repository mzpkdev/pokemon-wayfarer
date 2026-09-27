import catalogData from "./catalog.json"
import {
  ARC_CHECKPOINTS,
  ARC_IDS,
  MAX_EDITIONS,
  DEFAULT_CONTEXT,
  MAX_SEED,
  createExperiment,
  editionsAvailable,
  gymCapSummary,
  knownFlags,
  levelBase as baseLevel,
  playerRating,
  progressIndex,
  resolveTrainer,
  rosterGaps,
  serializeExperiment,
  validateExperiment,
  venueFeasibility,
  V1_REJECTION,
  V2_REJECTION,
  V3_REJECTION,
  worldCap,
} from "./engine.js"
import type {
  ArcId,
  Experiment,
  RosterContext,
  RosterFiller,
  TrainerRecord,
  TrainerSettings,
  WorldPoint,
} from "./types.js"

export const catalog = catalogData as TrainerRecord[]
const storageKey = "wayfarer-trainer-balance-v4"
/** Chart x positions: p = 0–24 in edition 1, then each completed edition at (24, 3). */
const CHART_PROGRESS = [...Array.from({ length: 25 }, (_, p) => p), 32, 40, 48]
const firstTrainer = catalog[0]
if (!firstTrainer) throw new Error("The trainer catalog is empty.")
const initialTrainerId = firstTrainer.id

export class BalanceLab {
  #_experiment = $state<Experiment>(createExperiment(catalog))
  badges = $state(0)
  leagueClears = $state(0)
  completedEditions = $state(0)
  query = $state("")
  region = $state("All regions")
  role = $state("All trainers")
  seed = $state(DEFAULT_CONTEXT.seed)
  flags = $state<string[]>([])
  selectedId = $state(initialTrainerId)
  editor = $state("")
  error = $state("")
  notice = $state("")

  point = $derived({
    badges: this.badges,
    leagueClears: this.leagueClears,
    completedEditions: this.completedEditions,
  })
  editionsEnabled = $derived(editionsAvailable(this.point))
  progress = $derived(progressIndex(this.point))
  playerTR = $derived(playerRating(this.point))
  cap = $derived(worldCap(this.point))
  levelBase = $derived(baseLevel(this.point, this.#_experiment.headroom))
  arcs = $derived(this.#_experiment.arcs)
  roleWindows = $derived(this.#_experiment.roleWindows)
  headroom = $derived(this.#_experiment.headroom)
  sizeTable = $derived(this.#_experiment.sizeTable)
  jitter = $derived(this.#_experiment.jitter)
  modifiers = $derived(this.#_experiment.modifiers)
  context = $derived<RosterContext>({ seed: this.seed, flags: this.flags })
  /** Flags the experiment mentions, plus any set flag no longer mentioned. */
  knownFlags = $derived([...new Set([...knownFlags(this.#_experiment), ...this.flags])].toSorted())
  allRows = $derived(
    catalog.map((trainer) =>
      resolveTrainer(trainer, this.#_experiment, this.point, undefined, this.context),
    ),
  )
  rows = $derived(
    this.allRows.filter(({ trainer }) => {
      const query = this.query.trim().toLowerCase()
      return (
        (this.region === "All regions" || trainer.region === this.region) &&
        (this.role === "All trainers" ||
          (this.role === "Gym Leaders"
            ? trainer.role === "Gym Leader"
            : trainer.role === this.role)) &&
        (!query ||
          `${trainer.name} ${trainer.region} ${trainer.homeLeagues.join(" ")}`
            .toLowerCase()
            .includes(query))
      )
    }),
  )
  selected = $derived.by(() => {
    const row =
      this.allRows.find((entry) => entry.trainer.id === this.selectedId) ?? this.allRows[0]
    if (!row) throw new Error("The trainer catalog is empty.")
    return row
  })
  settings = $derived(this.#_settings(this.selected.trainer.id))
  feasibility = $derived(venueFeasibility(catalog, this.#_experiment, this.progress, this.flags))
  gaps = $derived(rosterGaps(catalog, this.#_experiment, this.flags))
  gymSummary = $derived(gymCapSummary(catalog, this.#_experiment, this.point))
  /** p 0–24 use the selected clears; p 32/40/48 are post-game editions at (24, 3). */
  curve = $derived(
    CHART_PROGRESS.map((progress) => {
      const point =
        progress <= 24
          ? { badges: progress, leagueClears: this.leagueClears }
          : { badges: 24, leagueClears: 3, completedEditions: (progress - 24) / 8 }
      const aces = Object.fromEntries(
        this.settings.allowedArcs.map((arc) => [
          arc,
          resolveTrainer(this.selected.trainer, this.#_experiment, point, arc, this.context)
            .strengthLevel,
        ]),
      ) as Partial<Record<ArcId, number>>
      return { progress, aces, cap: worldCap(point) }
    }),
  )

  constructor() {
    this.#_syncEditors()
  }

  #_settings(id: string): TrainerSettings {
    const settings = this.#_experiment.trainers[id]
    if (!settings) throw new Error(`Unknown trainer: ${id}`)
    return settings
  }

  arcsFor = (id: string): ArcId[] => this.#_settings(id).allowedArcs
  fillersFor = (id: string): RosterFiller[] => this.#_settings(id).roster.fillers

  #_syncEditors = (): void => {
    this.editor = JSON.stringify(this.#_experiment.trainers[this.selectedId], null, 2)
  }

  #_persist = (): void => {
    try {
      localStorage.setItem(storageKey, this.exportText())
    } catch {
      this.notice = "Browser storage is unavailable. Export the experiment to keep your changes."
    }
  }

  #_clone = (): Experiment => JSON.parse(serializeExperiment(this.#_experiment)) as Experiment

  #_accept = (candidate: unknown, message: string): void => {
    this.#_experiment = validateExperiment(candidate, catalog)
    this.error = ""
    this.notice = message
    this.#_syncEditors()
    this.#_persist()
  }

  #_point = (value: unknown): Required<WorldPoint> => {
    if (!value || typeof value !== "object") throw new Error("Missing world progression.")
    const point = value as WorldPoint
    if (
      !Number.isInteger(point.badges) ||
      point.badges < 0 ||
      point.badges > 24 ||
      !Number.isInteger(point.leagueClears) ||
      point.leagueClears < 0 ||
      point.leagueClears > 3 ||
      !Number.isInteger(point.completedEditions) ||
      (point.completedEditions ?? 0) < 0 ||
      (point.completedEditions ?? 0) > MAX_EDITIONS
    )
      throw new Error(
        `Use 0–24 badges, 0–3 first league clears and 0–${MAX_EDITIONS} completed editions.`,
      )
    if (point.completedEditions && (point.badges !== 24 || point.leagueClears !== 3))
      throw new Error("Completed editions need 24 badges and 3 first league clears.")
    return point as Required<WorldPoint>
  }

  /** A completed edition implies 24 badges and 3 clears; leaving that point resets it. */
  #_syncEditions = (): void => {
    if (this.badges !== 24 || this.leagueClears !== 3) this.completedEditions = 0
  }

  load = (): void => {
    try {
      const saved = localStorage.getItem(storageKey)
      if (saved) this.importText(saved, "Restored your local experiment.")
    } catch {
      this.notice = "Browser storage is unavailable. You can still import and export experiments."
    }
  }

  select = (id: string): void => {
    if (!catalog.some((trainer) => trainer.id === id)) return
    this.selectedId = id
    this.error = ""
    this.#_syncEditors()
    this.#_persist()
  }

  setBadges = (badges: number): void => {
    this.badges = Number.isFinite(badges) ? Math.min(24, Math.max(0, Math.round(badges))) : 0
    this.#_syncEditions()
    this.#_persist()
  }

  setClears = (clears: number): void => {
    this.leagueClears = Number.isFinite(clears) ? Math.min(3, Math.max(0, Math.round(clears))) : 0
    this.#_syncEditions()
    this.#_persist()
  }

  /** Clamped to 0–3 (3 = "3 or more"); only non-zero at 24 badges and 3 clears. */
  setEditions = (editions: number): void => {
    this.completedEditions =
      this.editionsEnabled && Number.isFinite(editions)
        ? Math.min(MAX_EDITIONS, Math.max(0, Math.round(editions)))
        : 0
    this.#_persist()
  }

  setArc = (id: string, arc: string): void => {
    try {
      const next = this.#_clone()
      const settings = next.trainers[id]
      if (!settings) throw new Error("Unknown trainer.")
      settings.arc = arc as ArcId
      const name = catalog.find((trainer) => trainer.id === id)?.name ?? id
      this.#_accept(next, `${name} now follows the ${arc} arc.`)
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not change the arc."
    }
  }

  applyTrainer = (form: HTMLFormElement): void => {
    try {
      const data = new FormData(form)
      const next = this.#_clone()
      const settings = next.trainers[this.selectedId]
      if (!settings) throw new Error("Unknown selected trainer.")
      settings.bias = Number(data.get("bias"))
      settings.allowedArcs = ARC_IDS.filter((arc) => data.get(`allow-${arc}`) === "on")
      const [first] = settings.allowedArcs
      if (!first) throw new Error("Allow at least one growth arc.")
      if (!settings.allowedArcs.includes(settings.arc)) settings.arc = first
      this.#_accept(next, `Updated ${this.selected.trainer.name}’s standing.`)
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not apply the standing."
    }
  }

  applyTeam = (): void => {
    try {
      const next = this.#_clone()
      next.trainers[this.selectedId] = JSON.parse(this.editor)
      this.#_accept(next, `Updated ${this.selected.trainer.name}’s settings.`)
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not apply the settings."
    }
  }

  /** Swaps an ace with its neighbour; priority decides which aces fill the allowance. */
  moveAce = (index: number, direction: -1 | 1): void => {
    try {
      const next = this.#_clone()
      const aces = next.trainers[this.selectedId]?.roster.aces
      const ace = aces?.[index]
      const other = aces?.[index + direction]
      if (!aces || !ace || !other) return
      aces[index] = other
      aces[index + direction] = ace
      this.#_accept(next, `Moved ${ace.id} to ace priority ${index + direction + 1}.`)
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not reorder the aces."
    }
  }

  /** Reads evolve levels, base scores, offsets, signature moves and flags from the roster form. */
  applyRoster = (form: HTMLFormElement): void => {
    try {
      const data = new FormData(form)
      const next = this.#_clone()
      const roster = next.trainers[this.selectedId]?.roster
      if (!roster) throw new Error("Unknown selected trainer.")
      const text = (name: string) => String(data.get(name) ?? "").trim()
      const number = (name: string) => {
        const value = text(name)
        return value === "" ? Number.NaN : Number(value)
      }
      for (const ace of roster.aces)
        ace.line.forEach((stage, index) => {
          if (index) stage.level = number(`ace-${ace.id}-level-${index}`)
        })
      for (const filler of roster.fillers) {
        filler.line.forEach((stage, index) => {
          if (index) stage.level = number(`filler-${filler.id}-level-${index}`)
        })
        filler.baseScore = number(`filler-${filler.id}-score`)
        filler.levelOffset = number(`filler-${filler.id}-offset`)
        filler.signatureMove = text(`filler-${filler.id}-signature`) || null
        filler.requiresFlag = text(`filler-${filler.id}-flag`) || null
      }
      roster.prototype = data.get("roster-prototype") === "on"
      this.#_accept(next, `Updated ${this.selected.trainer.name}’s roster.`)
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not apply the roster."
    }
  }

  setSeed = (value: string): void => {
    const seed = Number(value.trim())
    if (!value.trim() || !Number.isInteger(seed) || seed < 0 || seed > MAX_SEED) {
      this.error = `Use a whole-number seed from 0 to ${MAX_SEED}.`
      return
    }
    this.seed = seed
    this.error = ""
    this.#_persist()
  }

  toggleFlag = (flag: string, on: boolean): void => {
    this.flags = on
      ? [...new Set([...this.flags, flag])].toSorted()
      : this.flags.filter((entry) => entry !== flag)
    this.#_persist()
  }

  applyRosterRules = (form: HTMLFormElement): void => {
    try {
      const data = new FormData(form)
      const next = this.#_clone()
      next.sizeTable = next.sizeTable.map((step, index) => ({
        minLevel: index === 0 ? 1 : Number(data.get(`size-level-${index}`)),
        size: Number(data.get(`size-${index}`)),
      }))
      next.jitter = Number(data.get("jitter"))
      this.#_accept(next, "Updated the team size table and filler jitter.")
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not apply the size table."
    }
  }

  addModifier = (form: HTMLFormElement): void => {
    try {
      const data = new FormData(form)
      const next = this.#_clone()
      const [trainer, filler] = String(data.get("modifier-target") ?? "").split("/")
      const modifier = {
        flag: String(data.get("modifier-flag") ?? "").trim(),
        trainer: trainer ?? "",
        filler: filler ?? "",
        delta: Number(data.get("modifier-delta")),
      }
      next.modifiers.push(modifier)
      this.#_accept(
        next,
        `Added ${modifier.flag} → ${modifier.trainer}/${modifier.filler} ${modifier.delta > 0 ? "+" : ""}${modifier.delta}.`,
      )
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not add the modifier."
    }
  }

  removeModifier = (index: number): void => {
    const next = this.#_clone()
    next.modifiers.splice(index, 1)
    this.#_accept(next, "Removed the modifier.")
  }

  applyWorld = (form: HTMLFormElement): void => {
    try {
      const data = new FormData(form)
      const next = this.#_clone()
      for (const arc of ARC_IDS)
        next.arcs[arc] = ARC_CHECKPOINTS.map((progress) =>
          progress === 0 ? 0 : Number(data.get(`arc-${arc}-${progress}`)),
        ) as Experiment["arcs"][ArcId]
      next.roleWindows = {
        contenderMax: Number(data.get("contender-max")),
        headlinerMin: Number(data.get("headliner-min")),
      }
      next.headroom = Number(data.get("headroom"))
      this.#_accept(
        next,
        "Updated the growth arcs, role windows and level headroom. The world cap is unchanged.",
      )
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not apply the arcs."
    }
  }

  reset = (): void => {
    this.#_accept(createExperiment(catalog), "Restored the experimental defaults for all trainers.")
  }

  resetTrainer = (): void => {
    const next = this.#_clone()
    const defaults = createExperiment(catalog).trainers[this.selectedId]
    if (!defaults) throw new Error("Unknown selected trainer.")
    next.trainers[this.selectedId] = defaults
    this.#_accept(
      next,
      `Restored ${this.selected.trainer.name}’s current defaults. Other trainers, arcs and role windows are unchanged.`,
    )
  }

  exportText = (): string =>
    JSON.stringify(
      {
        tool: "wayfarer-trainer-balance",
        version: 4,
        point: {
          badges: this.badges,
          leagueClears: this.leagueClears,
          completedEditions: this.completedEditions,
        },
        seed: this.seed,
        flags: this.flags,
        selectedTrainer: this.selectedId,
        experiment: JSON.parse(serializeExperiment(this.#_experiment)),
      },
      null,
      2,
    )

  importText = (text: string, message = "Imported experiment."): void => {
    try {
      const data = JSON.parse(text)
      if (data?.tool === "wayfarer-trainer-balance" && data.version === 1)
        throw new Error(V1_REJECTION)
      if (data?.tool === "wayfarer-trainer-balance" && data.version === 2)
        throw new Error(V2_REJECTION)
      if (data?.tool === "wayfarer-trainer-balance" && data.version === 3)
        throw new Error(V3_REJECTION)
      if (data?.tool !== "wayfarer-trainer-balance" || data.version !== 4) {
        throw new Error("This is not a version 4 Wayfarer balance experiment.")
      }
      const experiment = validateExperiment(data.experiment, catalog)
      const point = this.#_point(data.point)
      if (!Number.isInteger(data.seed) || data.seed < 0 || data.seed > MAX_SEED)
        throw new Error(`The seed must be a whole number from 0 to ${MAX_SEED}.`)
      if (
        !Array.isArray(data.flags) ||
        data.flags.some((flag: unknown) => typeof flag !== "string" || !flag.trim())
      )
        throw new Error("Flags must be a list of flag names.")
      if (!catalog.some((trainer) => trainer.id === data.selectedTrainer)) {
        throw new Error("Unknown selected trainer.")
      }
      this.#_experiment = experiment
      this.badges = point.badges
      this.leagueClears = point.leagueClears
      this.completedEditions = point.completedEditions
      this.seed = data.seed
      this.flags = [...new Set(data.flags as string[])].toSorted()
      this.selectedId = data.selectedTrainer
      this.error = ""
      this.notice = message
      this.#_syncEditors()
      this.#_persist()
    } catch (error) {
      this.error = error instanceof Error ? error.message : "Could not read the experiment."
    }
  }
}
