<script lang="ts">
  import type {
    CatalogMap,
    CatalogObject,
    CatalogWildEncounterMethod,
    CatalogWildEncounterProjection,
  } from "./catalog.js"
  import {
    fishingProfiles,
    placeLabel,
    placeLevelAt,
    resolveMethodSlots,
    rodLabel,
    type ResolvedMapEncounters,
  } from "./encounters.js"
  import EncounterSlotsTable from "./EncounterSlotsTable.svelte"
  import TrainerEvents from "./TrainerEvents.svelte"
  import type { ObjectSelection } from "./types.js"
  import CollapsibleSection from "./ui-toolkit/CollapsibleSection.svelte"

  type Props = {
    selectedMap: CatalogMap | null
    selectedObject: ObjectSelection | null
    encounters: ResolvedMapEncounters | null
    projection: CatalogWildEncounterProjection
    trainerRating: number
    onSelectTrainer?: (trainer: CatalogObject) => void
  }

  let {
    selectedMap,
    selectedObject,
    encounters,
    projection,
    trainerRating,
    onSelectTrainer,
  }: Props = $props()

  const methodLabels: Record<CatalogWildEncounterMethod["type"], string> = {
    land_mons: "Land",
    water_mons: "Water",
    rock_smash_mons: "Rock Smash",
    fishing_mons: "Fishing",
  }
  const methodTypes = Object.keys(methodLabels) as CatalogWildEncounterMethod["type"][]

  const timeOfDayLabels = {
    morning: "Morning",
    day: "Day",
    evening: "Evening",
    night: "Night",
  } as const

  const resolutionLabels = {
    direct: "Table",
    unavailable: "No table",
  } as const
</script>

<section
  class="min-w-0 border border-cartographer-border bg-cartographer-panel shadow-cartographer-panel"
>
  {#if !selectedMap}
    <div class="p-6">
      <p
        class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.15em] text-cartographer-signal"
      >
        Wayfarer wild encounters
      </p>
      <h2 class="mb-3 mt-3 text-xl font-semibold">Select a map</h2>
      <p class="m-0 leading-6 text-cartographer-muted">
        Choose a rendered exterior map from the region index or search to inspect the encounter sets
        in the Wayfarer wild tables.
      </p>
    </div>
  {:else}
    {@const trainers = selectedMap.objects.filter((object) => object.kind.id === "trainer")}
    <header class="border-b border-cartographer-border p-5 sm:flex sm:items-end sm:justify-between">
      <div>
        <p
          class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.15em] text-cartographer-signal"
        >
          Wayfarer wild encounters
        </p>
        <h2 class="mb-0 mt-2 text-2xl font-semibold tracking-[-0.025em]">{selectedMap.name}</h2>
      </div>
      <p class="mb-0 mt-2 font-cartographer-mono text-xs text-cartographer-muted sm:text-right">
        {encounters?.sets.length ?? 0}
        {(encounters?.sets.length ?? 0) === 1 ? "table" : "tables"}
      </p>
    </header>

    <TrainerEvents mapName={selectedMap.name} {trainers} {selectedObject} {onSelectTrainer} />

    <p class="m-0 border-b border-cartographer-border px-5 py-3 text-sm text-cartographer-muted">
      Normal non-randomized ordinary encounters · levels at Trainer Rating {trainerRating}
      {#if encounters?.product}
        · {encounters.availableProducts.find((product) => product.id === encounters?.product)
          ?.displayName ?? encounters.product}{/if}
    </p>

    {#if !encounters || encounters.sets.length === 0}
      <div class="p-6">
        <h3 class="m-0 text-base font-semibold">No wild encounter tables</h3>
        <p class="mb-0 mt-2 leading-6 text-cartographer-muted">
          This exterior map has no table in the Wayfarer wild encounter data.
        </p>
      </div>
    {:else}
      <div class="grid min-w-0 gap-5 p-5">
        {#if encounters.runtimeTimes.length > 0}
          <CollapsibleSection
            title="Runtime encounter times"
            count={encounters.runtimeTimes.length}
          >
            <p class="m-0 px-4 py-3 text-sm text-cartographer-muted">
              Every map holds a day and a night table per method. Morning and Day use the day table;
              Evening and Night use the night table.
            </p>
            <ul class="m-0 grid list-none divide-y divide-cartographer-border p-0">
              {#each encounters.runtimeTimes as time (`${time.product}-${time.timeOfDay}`)}
                <li class="grid gap-2 px-4 py-3">
                  <div>
                    <p class="m-0 text-sm font-medium">{timeOfDayLabels[time.timeOfDay]}</p>
                  </div>
                  <ul class="m-0 grid list-none gap-1 p-0">
                    {#each time.methods as method (method.type)}
                      <li class="flex flex-wrap items-baseline gap-x-2 gap-y-1 text-sm">
                        <span class="font-medium">{methodLabels[method.type]}</span>
                        <span class="text-cartographer-muted"
                          >{resolutionLabels[method.resolution]}</span
                        >
                        {#each method.sets as set (set.baseLabel)}
                          <code class="font-cartographer-mono text-xs text-cartographer-signal-soft"
                            >{set.baseLabel}</code
                          >
                        {/each}
                      </li>
                    {/each}
                  </ul>
                </li>
              {/each}
            </ul>
          </CollapsibleSection>
        {/if}

        {#each encounters.sets as encounterSet, setIndex (`${encounterSet.product}-${encounterSet.baseLabel}-${setIndex}`)}
          {@const listedMethods = new Set(encounterSet.methods.map((method) => method.type))}
          {@const missingMethods = methodTypes.filter((type) => !listedMethods.has(type))}
          <CollapsibleSection
            title={`${encounterSet.place.name} · ${encounterSet.runtimeTime} table`}
            meta={encounterSet.baseLabel}
            label={`Encounter table ${encounterSet.baseLabel}`}
          >
            <p class="m-0 px-4 pt-4 text-sm">
              {placeLabel(encounterSet.place)} · place level
              <strong>{placeLevelAt(projection, encounterSet, trainerRating)}</strong>
              at Trainer Rating {trainerRating} (wild levels roll ±2 around it)
            </p>
            <p
              class="m-0 break-all p-4 pt-2 font-cartographer-mono text-[0.68rem] text-cartographer-muted"
            >
              {encounterSet.source.path}{encounterSet.source.pointer}
            </p>

            {#if missingMethods.length > 0}
              <p
                class="m-0 border-b border-cartographer-border px-4 py-2 text-sm text-cartographer-muted"
              >
                Not in this table: {missingMethods.join(", ")}
              </p>
            {/if}

            <div class="grid divide-y divide-cartographer-border">
              {#each encounterSet.methods as method (method.type)}
                {@const resolvedSlots = resolveMethodSlots(
                  projection,
                  encounterSet,
                  method,
                  trainerRating,
                )}
                <CollapsibleSection
                  title={methodLabels[method.type]}
                  meta={`Encounter rate ${method.encounterRate}`}
                  label={`${methodLabels[method.type]} encounter method`}
                >
                  <p
                    class="mb-3 mt-0 px-4 pt-3 font-cartographer-mono text-[0.68rem] text-cartographer-muted"
                  >
                    Selection weight is the slot's share of this method's table weights.
                  </p>
                  {#if resolvedSlots.length === 0 && method.type !== "fishing_mons"}
                    <p class="m-0 px-4 pb-4 text-sm text-cartographer-muted">
                      This method has no slots.
                    </p>
                  {:else if method.type === "fishing_mons"}
                    {@const profiles = fishingProfiles(method)}
                    <div class="grid min-w-0 gap-4 px-4 pb-4">
                      {#each profiles as profile (profile.profileKey)}
                        <section
                          class="min-w-0 border border-cartographer-border"
                          aria-label={`${rodLabel(profile.fishingRod)} fishing`}
                        >
                          <header
                            class="flex items-baseline justify-between gap-3 border-b border-cartographer-border bg-cartographer-panel-raised px-3 py-2"
                          >
                            <h4 class="m-0 text-sm font-semibold">
                              {rodLabel(profile.fishingRod)}
                            </h4>
                            <code
                              class="font-cartographer-mono text-[0.68rem] text-cartographer-muted"
                              >{profile.fishingRod}</code
                            >
                          </header>
                          <EncounterSlotsTable
                            slots={resolveMethodSlots(
                              projection,
                              encounterSet,
                              method,
                              trainerRating,
                              profile.fishingRod,
                            )}
                            {trainerRating}
                          />
                        </section>
                      {/each}
                    </div>
                  {:else}
                    <div class="px-4 pb-4">
                      <EncounterSlotsTable slots={resolvedSlots} {trainerRating} />
                    </div>
                  {/if}
                </CollapsibleSection>
              {/each}
            </div>
          </CollapsibleSection>
        {/each}
      </div>
    {/if}
  {/if}
</section>
