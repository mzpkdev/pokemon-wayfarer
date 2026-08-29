<script lang="ts">
  import type { EncounterRosterMethodType, ResolvedEncounterPopulation } from "./encounters.js"
  import { mapImageUrl } from "./urls.js"

  type Props = {
    mapName: string
    method: EncounterRosterMethodType
    population: ResolvedEncounterPopulation
    trainerRating: number
    productName: string | null
    onClose: () => void
  }

  let { mapName, method, population, trainerRating, productName, onClose }: Props = $props()

  const methodLabels: Record<EncounterRosterMethodType, string> = {
    land_mons: "Land",
    water_mons: "Water",
  }

  const timeLabels = {
    morning: "Morning",
    day: "Day",
    evening: "Evening",
    night: "Night",
  } as const

  const levelLabel = (minimum: number, maximum: number): string => {
    return minimum === maximum ? `Lv. ${minimum}` : `Lv. ${minimum}-${maximum}`
  }

  const selectionWeightLabel = (weight: number | null): string => {
    return weight === null ? "Unavailable" : `${(weight * 100).toFixed(1).replace(/\.0$/, "")}%`
  }

  const runtimeUseLabel = (
    use: ResolvedEncounterPopulation["sources"][number]["activations"][number],
  ): string => {
    const resolution = use.resolution === "direct" ? "direct" : "Day fallback"
    return `${timeLabels[use.timeOfDay]}: ${resolution}`
  }
</script>

<div
  class="flex max-h-[calc(58vh-2rem)] w-[min(42rem,calc(100vw-2rem))] flex-col overflow-hidden border border-cartographer-border-strong bg-cartographer-panel shadow-cartographer-panel"
  role="dialog"
  aria-modal="false"
  aria-labelledby="encounter-roster-popup-title"
  aria-describedby="encounter-roster-popup-summary"
  tabindex="-1"
  onkeydown={(event) => {
    if (event.key === "Escape") {
      event.preventDefault()
      event.stopPropagation()
      onClose()
    }
  }}
>
  <header class="flex items-start justify-between gap-4 border-b border-cartographer-border p-4">
    <div class="min-w-0">
      <p
        class="m-0 font-cartographer-mono text-[0.65rem] font-bold uppercase tracking-[0.14em] text-cartographer-signal"
      >
        Projected encounter population
      </p>
      <h2 id="encounter-roster-popup-title" class="mb-0 mt-1 break-words text-lg font-semibold">
        {mapName} · {methodLabels[method]}
      </h2>
      <p
        id="encounter-roster-popup-summary"
        class="mb-0 mt-1 font-cartographer-mono text-[0.68rem] text-cartographer-muted"
      >
        Trainer Rating {trainerRating}{productName ? ` · ${productName}` : ""}
      </p>
    </div>
    <button
      type="button"
      class="grid size-8 shrink-0 place-items-center border border-cartographer-border bg-cartographer-panel-raised font-cartographer-mono text-base text-cartographer-muted hover:border-cartographer-border-strong hover:text-cartographer-ink focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-cartographer-signal"
      aria-label="Close encounter roster"
      onclick={onClose}
    >
      ×
    </button>
  </header>

  <div class="cartographer-scrollbar min-h-0 flex-1 overflow-y-auto p-4">
    {#if population.sources.length === 0}
      <p class="m-0 text-sm leading-6 text-cartographer-muted">
        No projected {methodLabels[method].toLowerCase()} population is available for this game version
        at Trainer Rating {trainerRating}.
      </p>
    {:else}
      <div class="grid gap-4">
        {#if population.unavailableTimes.length > 0}
          <p
            class="m-0 border border-cartographer-border px-3 py-2 text-xs leading-5 text-cartographer-muted"
          >
            No {methodLabels[method].toLowerCase()} source is available at
            {population.unavailableTimes.map((time) => timeLabels[time]).join(", ")}.
          </p>
        {/if}
        {#each population.sources as source (source.key)}
          <section
            class="min-w-0 border border-cartographer-border"
            aria-label={`Source encounter set ${source.set.baseLabel}`}
          >
            <header
              class="border-b border-cartographer-border bg-cartographer-panel-raised px-3 py-2"
            >
              <h3 class="m-0 break-all font-cartographer-mono text-xs font-semibold">
                {source.set.baseLabel}
              </h3>
              <p class="mb-0 mt-1 text-xs text-cartographer-muted">
                Source time: {source.set.runtimeTime}
              </p>
              {#if source.activations.length > 0}
                <ul class="mb-0 mt-2 flex list-none flex-wrap gap-1 p-0" aria-label="Runtime use">
                  {#each source.activations as use (`${use.timeOfDay}-${use.resolution}`)}
                    <li
                      class="border border-cartographer-border px-2 py-1 font-cartographer-mono text-[0.62rem] text-cartographer-muted"
                    >
                      {runtimeUseLabel(use)}
                    </li>
                  {/each}
                </ul>
              {:else}
                <p class="mb-0 mt-2 text-xs text-cartographer-muted">
                  No runtime activation metadata is recorded. The authored source time is shown.
                </p>
              {/if}
            </header>

            {#if source.lockedSlotCount > 0}
              <p
                class="m-0 border-b border-cartographer-border px-3 py-2 text-xs leading-5 text-cartographer-muted"
              >
                {source.lockedSlotCount === source.slots.length
                  ? "This source group is locked"
                  : `${source.lockedSlotCount} authored ${source.lockedSlotCount === 1 ? "slot is" : "slots are"} locked`}
                at Trainer Rating {trainerRating}. Locked slots stay visible but are removed before
                selection weights are renormalized.
              </p>
            {/if}

            <ol class="m-0 grid list-none divide-y divide-cartographer-border p-0">
              {#each source.slots as slot (`${slot.source.slotIndex}-${slot.source.speciesId}`)}
                <li
                  class:opacity-65={!slot.eligible}
                  class="grid gap-2 px-3 py-3 sm:grid-cols-[1fr_auto]"
                  aria-label={`Authored slot ${slot.source.slotIndex + 1} ${slot.source.speciesLabel ?? slot.source.speciesId}`}
                >
                  <div class="min-w-0">
                    <div class="flex min-w-0 items-center gap-2">
                      {#if slot.source.sprite}
                        <img
                          class="size-8 shrink-0 object-contain [image-rendering:pixelated]"
                          src={mapImageUrl(slot.source.sprite.path)}
                          alt=""
                          width={slot.source.sprite.widthPixels}
                          height={slot.source.sprite.heightPixels}
                        />
                      {/if}
                      <div class="min-w-0">
                        <p class="m-0 break-words text-sm font-medium">
                          Slot {slot.source.slotIndex + 1}: {slot.source.speciesLabel ??
                            slot.source.speciesId}
                        </p>
                        <p
                          class="mb-0 mt-0.5 font-cartographer-mono text-[0.64rem] text-cartographer-muted"
                        >
                          Authored {levelLabel(slot.source.minLevel, slot.source.maxLevel)} · source weight
                          {slot.source.slotRate}
                        </p>
                      </div>
                    </div>

                    {#if slot.outcomes.length > 0}
                      <ul
                        class="mb-0 mt-2 grid list-none gap-1 border-l border-cartographer-border pl-3"
                      >
                        {#each slot.outcomes as outcome (`${outcome.speciesId}-${outcome.authoredMinimumLevel}-${outcome.authoredMaximumLevel}`)}
                          <li class="flex min-w-0 items-center gap-2">
                            {#if outcome.sprite}
                              <img
                                class="size-7 shrink-0 object-contain [image-rendering:pixelated]"
                                src={mapImageUrl(outcome.sprite.path)}
                                alt=""
                                width={outcome.sprite.widthPixels}
                                height={outcome.sprite.heightPixels}
                              />
                            {/if}
                            <span class="min-w-0 text-sm">
                              <span class="font-medium">{outcome.speciesLabel}</span>
                              <span
                                class="block font-cartographer-mono text-[0.64rem] text-cartographer-muted"
                              >
                                Projected {levelLabel(
                                  outcome.projectedMinimumLevel,
                                  outcome.projectedMaximumLevel,
                                )}
                                {#if !outcome.eligible}
                                  · requires ordinary wild level {outcome.minimumOrdinaryWildLevel}
                                {/if}
                              </span>
                            </span>
                          </li>
                        {/each}
                      </ul>
                    {:else}
                      <p class="mb-0 mt-2 text-xs text-cartographer-muted">
                        Projection unavailable for this authored slot.
                      </p>
                    {/if}
                  </div>
                  <p
                    class="m-0 font-cartographer-mono text-xs sm:text-right"
                    class:text-cartographer-muted={!slot.eligible}
                  >
                    <span class="block text-[0.6rem] uppercase tracking-[0.08em]">Selection</span>
                    {selectionWeightLabel(slot.selectionWeight)}
                  </p>
                </li>
              {/each}
            </ol>
          </section>
        {/each}
      </div>
    {/if}
  </div>
</div>
