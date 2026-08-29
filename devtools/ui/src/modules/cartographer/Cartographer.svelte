<script lang="ts">
  import { cubicOut } from "svelte/easing"
  import { fade } from "svelte/transition"
  import { onMount } from "svelte"

  import EncounterPanel from "./EncounterPanel.svelte"
  import MapDetails from "./MapDetails.svelte"
  import MapSearch from "./MapSearch.svelte"
  import MapViewport from "./MapViewport.svelte"
  import RegionPicker from "./RegionPicker.svelte"
  import {
    CatalogValidationError,
    loadCatalog,
    type CatalogMap,
    type CatalogWarp,
    type CatalogObject,
    type MapCatalog,
  } from "./catalog.js"
  import { visibleSurfaceMaps } from "./geography.js"
  import { resolveMapEncounters } from "./encounters.js"
  import type { ObjectSelection, WarpSelection } from "./types.js"
  import Slider from "./ui-toolkit/Slider.svelte"
  import {
    MAX_TRAINER_RATING,
    MIN_TRAINER_RATING,
    cartographerUrlWithState,
    parseCartographerUrlState,
    type CartographerViewState,
  } from "./urls.js"

  type LoadState =
    | { kind: "loading" }
    | { kind: "ready"; catalog: MapCatalog }
    | { kind: "error"; message: string; details: string[] }

  type CartographerTab = "map" | "encounters"

  const fadeIn = { delay: 75, duration: 125, easing: cubicOut }

  let loadState = $state<LoadState>({ kind: "loading" })
  let requestedRegion = $state<string | null>(null)
  let requestedMap = $state<string | null>(null)
  let initialView = $state<CartographerViewState | null>(null)
  let currentView = $state<CartographerViewState | null>(null)
  let searchQuery = $state("")
  let selectedWarp = $state<WarpSelection | null>(null)
  let selectedObject = $state<ObjectSelection | null>(null)
  let showExits = $state(false)
  let showObjects = $state(false)
  let showEncounterTrainers = $state(true)
  let focusToken = $state(0)
  let activeTab = $state<CartographerTab>("map")
  let trainerRating = $state(MIN_TRAINER_RATING)
  let requestedProduct = $state<string | null>(null)

  onMount(() => {
    const state = parseCartographerUrlState(window.location.href)
    requestedRegion = state.region
    requestedMap = state.selectedMap
    initialView = state.view
    trainerRating = state.trainerRating
    requestedProduct = state.product
    const controller = new AbortController()
    loadCatalog(controller.signal)
      .then((catalog) => {
        loadState = { kind: "ready", catalog }
      })
      .catch((error: unknown) => {
        if (controller.signal.aborted) return
        loadState = {
          kind: "error",
          message: error instanceof Error ? error.message : "The map catalog could not be read.",
          details: error instanceof CatalogValidationError ? [...error.details] : [],
        }
      })
    return () => controller.abort()
  })

  let catalog = $derived(loadState.kind === "ready" ? loadState.catalog : null)
  let selectedCandidate = $derived(
    catalog?.maps.find((map) => map.name === requestedMap || map.id === requestedMap) ?? null,
  )
  let activeRegion = $derived(
    catalog
      ? (selectedCandidate &&
          catalog.regions.find((region) => region.id === selectedCandidate.region)) ||
          catalog.regions.find((region) => region.id === requestedRegion) ||
          catalog.regions[0] ||
          null
      : null,
  )
  let maps = $derived(
    catalog && activeRegion ? catalog.maps.filter((map) => map.region === activeRegion.id) : [],
  )
  let selectedMap = $derived(
    selectedCandidate?.region === activeRegion?.id ? selectedCandidate : null,
  )
  let renderedMapNames = $derived(
    new Set(visibleSurfaceMaps(catalog?.maps ?? []).map((map) => map.name)),
  )
  let activeEncounters = $derived(
    selectedMap && catalog
      ? resolveMapEncounters(
          selectedMap,
          requestedProduct,
          catalog.wildEncounterProjection.products,
        )
      : null,
  )

  const replaceUrl = (): void => {
    if (!activeRegion) return
    const next = cartographerUrlWithState(window.location.href, {
      region: activeRegion.id,
      selectedMap: selectedMap?.name ?? null,
      view: currentView,
      trainerRating,
      product: activeEncounters?.product ?? requestedProduct,
    })
    const current = `${window.location.pathname}${window.location.search}${window.location.hash}`
    if (next !== current) window.history.replaceState(window.history.state, "", next)
  }

  const selectMap = (name: string, focus = false): void => {
    const map = catalog?.maps.find((candidate) => candidate.name === name) ?? null
    if (!map) return
    if (map.region !== activeRegion?.id) currentView = null
    requestedRegion = map.region
    requestedMap = map.name
    selectedWarp = null
    selectedObject = null
    if (focus && renderedMapNames.has(map.name)) focusToken += 1
    queueMicrotask(replaceUrl)
  }

  const selectRegion = (region: string): void => {
    requestedRegion = region
    requestedMap = null
    selectedWarp = null
    selectedObject = null
    currentView = null
    queueMicrotask(replaceUrl)
  }

  const selectWarp = (warp: CatalogWarp): void => {
    if (!selectedMap) return
    selectedWarp = { sourceMapName: selectedMap.name, warpId: warp.warpId }
    selectedObject = null
    showExits = true
  }

  const selectObject = (object: CatalogObject): void => {
    if (!selectedMap) return
    selectedObject = { sourceMapName: selectedMap.name, objectId: object.objectId }
    selectedWarp = null
    showObjects = true
  }

  const handleCameraChange = (view: CartographerViewState): void => {
    currentView = view
    replaceUrl()
  }

  const selectProduct = (product: string): void => {
    requestedProduct = product
    queueMicrotask(replaceUrl)
  }

  const selectTrainerRating = (rating: number): void => {
    trainerRating = rating
    queueMicrotask(replaceUrl)
  }
</script>

{#if loadState.kind === "loading"}
  <section
    class="mx-auto mt-[18vh] max-w-md border border-cartographer-border bg-cartographer-panel p-6"
  >
    <p class="m-0 text-sm font-medium text-cartographer-signal">Loading catalog…</p>
    <div class="mt-4 h-px w-full overflow-hidden bg-cartographer-border">
      <div class="h-full w-2/5 bg-cartographer-signal"></div>
    </div>
  </section>
{:else if loadState.kind === "error"}
  <section
    class="mx-auto mt-[12vh] max-w-2xl border border-cartographer-diagnostic-border bg-cartographer-diagnostic-panel p-7 shadow-cartographer-error"
  >
    <p class="m-0 text-sm font-semibold text-cartographer-rose-400">Catalog unavailable</p>
    <h1 class="mb-3 mt-3 text-2xl font-semibold">Cartographer unavailable</h1>
    <p class="text-cartographer-muted">{loadState.message}</p>
    {#if loadState.details.length > 0}
      <ul>
        {#each loadState.details as detail}<li>{detail}</li>{/each}
      </ul>
    {/if}
  </section>
{:else if !catalog || !activeRegion}
  <section
    class="mx-auto mt-[12vh] max-w-2xl border border-cartographer-border bg-cartographer-panel p-8"
  >
    <p>The catalog has no regions.</p>
  </section>
{:else}
  <section class="mx-auto max-w-[1800px] p-[clamp(1rem,2.4vw,2.25rem)]">
    <header class="mb-5 border-b border-cartographer-border pb-4">
      <div>
        <p class="m-0 text-sm font-medium text-cartographer-signal">World maps</p>
        <h1 class="m-0 mt-1 text-[clamp(1.4rem,2.2vw,2.1rem)] font-semibold tracking-[-0.035em]">
          {activeRegion.label}
        </h1>
      </div>
      <div class="mt-4 flex flex-col gap-3 sm:flex-row sm:items-end sm:justify-between">
        <nav class="flex gap-1" aria-label="Cartographer views">
          <button
            class="border border-cartographer-border px-3 py-2 text-sm font-medium transition-colors hover:border-cartographer-signal focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-cartographer-signal"
            class:border-cartographer-signal={activeTab === "map"}
            class:bg-cartographer-signal={activeTab === "map"}
            class:text-cartographer-canvas={activeTab === "map"}
            aria-controls="cartographer-map"
            aria-pressed={activeTab === "map"}
            type="button"
            onclick={() => (activeTab = "map")}>World</button
          >
          <button
            class="border border-cartographer-border px-3 py-2 text-sm font-medium transition-colors hover:border-cartographer-signal focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-cartographer-signal"
            class:border-cartographer-signal={activeTab === "encounters"}
            class:bg-cartographer-signal={activeTab === "encounters"}
            class:text-cartographer-canvas={activeTab === "encounters"}
            aria-controls="cartographer-encounters"
            aria-pressed={activeTab === "encounters"}
            type="button"
            onclick={() => (activeTab = "encounters")}>Encounters</button
          >
        </nav>
        <p
          class="m-0 max-w-md font-cartographer-mono text-[0.68rem] leading-5 tracking-[0.04em] text-cartographer-muted sm:text-right"
        >
          {maps.length} source maps · only default-visible surface maps are shown
        </p>
      </div>
      {#if activeTab === "encounters"}
        <div
          class="mt-4 flex flex-wrap items-end gap-5 border border-cartographer-border bg-cartographer-panel-raised p-3"
          aria-label="Encounter scaling controls"
        >
          <Slider
            label="Trainer Rating"
            minimum={MIN_TRAINER_RATING}
            maximum={MAX_TRAINER_RATING}
            value={trainerRating}
            onValueChange={selectTrainerRating}
          />
          {#if activeEncounters && activeEncounters.availableProducts.length > 1}
            <label class="grid gap-2 text-xs font-medium text-cartographer-muted">
              Game version
              <select
                class="min-w-36 border border-cartographer-border bg-cartographer-field px-2.5 py-2 text-sm text-cartographer-ink focus-visible:border-cartographer-signal focus-visible:outline-2 focus-visible:outline-offset-1 focus-visible:outline-cartographer-signal"
                value={activeEncounters.product ?? ""}
                onchange={(event) => selectProduct(event.currentTarget.value)}
              >
                {#each activeEncounters.availableProducts as product (product.id)}
                  <option value={product.id}>{product.displayName}</option>
                {/each}
              </select>
            </label>
          {/if}
          <p class="m-0 text-xs leading-5 text-cartographer-muted">
            Preview of normal non-randomized ordinary encounters.
          </p>
        </div>
      {/if}
    </header>
    <div class="grid gap-4 xl:grid-cols-[15.5rem_minmax(0,1fr)_20rem]">
      <aside class="grid content-start gap-3">
        <RegionPicker
          regions={catalog.regions}
          activeRegionId={activeRegion.id}
          onSelectRegion={selectRegion}
        />
        <MapSearch maps={catalog.maps} bind:query={searchQuery} onSelectMap={selectMap} />
      </aside>
      {#if activeTab === "map"}
        <div id="cartographer-map" class="min-w-0" in:fade={fadeIn}>
          {#key activeRegion.id}
            <MapViewport
              {catalog}
              {maps}
              selectedMapName={selectedMap?.name}
              {selectedWarp}
              {selectedObject}
              {initialView}
              focusRequest={focusToken > 0 && selectedMap
                ? { mapName: selectedMap.name, token: focusToken }
                : null}
              {showExits}
              {showObjects}
              {showEncounterTrainers}
              onSelectMap={selectMap}
              onSelectWarp={(selection) => {
                selectMap(selection.sourceMapName)
                selectedWarp = selection
                selectedObject = null
                showExits = true
              }}
              onSelectObject={(selection) => {
                selectMap(selection.sourceMapName)
                selectedObject = selection
                selectedWarp = null
                showObjects = true
              }}
              onCameraChange={handleCameraChange}
              onToggleExits={(value) => (showExits = value)}
              onToggleObjects={(value) => (showObjects = value)}
              onToggleEncounterTrainers={(value) => (showEncounterTrainers = value)}
            />
          {/key}
        </div>
        <div class="min-w-0" in:fade={fadeIn}>
          <MapDetails
            maps={catalog.maps}
            {selectedMap}
            {selectedWarp}
            {selectedObject}
            {renderedMapNames}
            onSelectWarp={selectWarp}
            onSelectObject={selectObject}
            onFocusMap={(name) => selectMap(name, true)}
          />
        </div>
      {:else}
        <div id="cartographer-encounters" class="min-w-0" in:fade={fadeIn}>
          {#key activeRegion.id}
            <MapViewport
              {catalog}
              {maps}
              selectedMapName={selectedMap?.name}
              {initialView}
              focusRequest={focusToken > 0 && selectedMap
                ? { mapName: selectedMap.name, token: focusToken }
                : null}
              encounterMode
              {trainerRating}
              preferredProduct={activeEncounters?.product ?? requestedProduct}
              {showEncounterTrainers}
              onSelectMap={selectMap}
              onSelectObject={(selection) => {
                selectMap(selection.sourceMapName)
                selectedObject = selection
              }}
              onCameraChange={handleCameraChange}
              onToggleEncounterTrainers={(value) => (showEncounterTrainers = value)}
            />
          {/key}
        </div>
        <div class="min-w-0" in:fade={fadeIn}>
          <EncounterPanel
            {selectedMap}
            {selectedObject}
            encounters={activeEncounters}
            projection={catalog.wildEncounterProjection}
            {trainerRating}
            onSelectTrainer={(trainer) => {
              if (!selectedMap) return
              selectedObject = { sourceMapName: selectedMap.name, objectId: trainer.objectId }
            }}
          />
        </div>
      {/if}
    </div>
  </section>
{/if}
