<script lang="ts">
  import { onMount } from "svelte"

  import {
    MetatileCatalogValidationError,
    loadMetatileCatalog,
    loadMetatileContext,
    type CatalogMetatile,
    type MetatileCatalog,
    type MetatileCatalogContext,
    type MetatileRenderContext,
    type MetatileTileset,
  } from "./catalog.js"
  import MetatileCatalogGrid from "./MetatileCatalogGrid.svelte"
  import MetatileContextPicker from "./MetatileContextPicker.svelte"
  import MetatileInspector from "./MetatileInspector.svelte"
  import Button from "../cartographer/ui-toolkit/Button.svelte"
  import Checkbox from "../cartographer/ui-toolkit/Checkbox.svelte"

  type LoadState =
    | { kind: "loading" }
    | { kind: "ready"; catalog: MetatileCatalog }
    | { kind: "error"; message: string; details: string[] }

  type TilesetKind = "primary" | "secondary"

  type ContextLoadState =
    | { kind: "idle" }
    | { kind: "loading"; id: string }
    | { kind: "ready"; context: MetatileRenderContext }
    | { kind: "error"; id: string; message: string; details: string[] }

  let loadState = $state<LoadState>({ kind: "loading" })
  let activeContextId = $state<string | null>(null)
  let activeTilesetKind = $state<TilesetKind>("primary")
  let query = $state("")
  let includeUnused = $state(false)
  let selectedSourceId = $state<string | null>(null)
  let contextLoadState = $state<ContextLoadState>({ kind: "idle" })
  let renderedContext = $state<MetatileRenderContext | null>(null)
  let contextController: AbortController | null = null

  const filterMetatiles = (
    metatiles: readonly CatalogMetatile[],
    value: string,
  ): CatalogMetatile[] => {
    const search = value.trim().toLocaleLowerCase("en")
    if (!search) return [...metatiles]
    const hexadecimal = search.replace(/^0x/, "")
    return metatiles.filter((metatile) => {
      return (
        metatile.sourceId.toLocaleLowerCase("en").includes(search) ||
        metatile.displayName.toLocaleLowerCase("en").includes(search) ||
        metatile.id.toString(10).includes(search) ||
        metatile.id.toString(16).toLocaleLowerCase("en").includes(hexadecimal)
      )
    })
  }

  const usageCount = (metatile: CatalogMetatile): number => {
    return metatile.usedBy.reduce((total, usage) => total + usage.count, 0)
  }

  const sortMetatilesByUsage = (metatiles: readonly CatalogMetatile[]): CatalogMetatile[] => {
    return [...metatiles].toSorted((left, right) => {
      const difference = usageCount(right) - usageCount(left)
      return difference || left.id - right.id
    })
  }

  const requestContext = (entry: MetatileCatalogContext): void => {
    contextController?.abort()
    const controller = new AbortController()
    contextController = controller
    contextLoadState = { kind: "loading", id: entry.id }
    loadMetatileContext(entry, controller.signal)
      .then((context) => {
        if (controller.signal.aborted) return
        renderedContext = context
        activeTilesetKind = "primary"
        selectedSourceId = null
        query = ""
        contextLoadState = { kind: "ready", context }
      })
      .catch((error: unknown) => {
        if (controller.signal.aborted) return
        contextLoadState = {
          kind: "error",
          id: entry.id,
          message:
            error instanceof Error ? error.message : "The metatile context could not be read.",
          details: error instanceof MetatileCatalogValidationError ? [...error.details] : [],
        }
      })
  }

  onMount(() => {
    const controller = new AbortController()
    loadMetatileCatalog(controller.signal)
      .then((catalog) => {
        loadState = { kind: "ready", catalog }
        const initialContext =
          catalog.contexts.find(
            (context) =>
              context.primaryTileset === "gTileset_General" &&
              context.secondaryTileset === "gTileset_Cave",
          ) ?? catalog.contexts[0]
        activeContextId = initialContext?.id ?? null
        if (initialContext) requestContext(initialContext)
      })
      .catch((error: unknown) => {
        if (controller.signal.aborted) return
        loadState = {
          kind: "error",
          message:
            error instanceof Error ? error.message : "The metatile catalog could not be read.",
          details: error instanceof MetatileCatalogValidationError ? [...error.details] : [],
        }
      })
    return () => {
      controller.abort()
      contextController?.abort()
    }
  })

  let catalog = $derived(loadState.kind === "ready" ? loadState.catalog : null)
  let activeContextEntry = $derived(
    catalog?.contexts.find((context) => context.id === activeContextId) ??
      catalog?.contexts[0] ??
      null,
  )
  let activeContext = $derived(renderedContext)
  let renderedContextEntry = $derived(
    catalog?.contexts.find((context) => context.id === activeContext?.id) ?? null,
  )
  let activeTileset = $derived<MetatileTileset | null>(
    activeContext ? activeContext[activeTilesetKind] : null,
  )
  let usedMetatiles = $derived(
    (activeTileset?.metatiles ?? []).filter((metatile) => metatile.usedBy.length > 0),
  )
  let browsableMetatiles = $derived(
    sortMetatilesByUsage(includeUnused ? (activeTileset?.metatiles ?? []) : usedMetatiles),
  )
  let filteredMetatiles = $derived(filterMetatiles(browsableMetatiles, query))
  let selectedMetatile = $derived(
    activeTileset?.metatiles.find((metatile) => metatile.sourceId === selectedSourceId) ?? null,
  )
  const contextLabel = (context: MetatileCatalogContext): string => {
    return `${context.primaryTileset} + ${context.secondaryTileset}`
  }

  const selectContext = (id: string): void => {
    const nextContext = catalog?.contexts.find((context) => context.id === id)
    if (!nextContext || nextContext.id === activeContextId) return
    activeContextId = id
    requestContext(nextContext)
  }

  const selectTileset = (kind: TilesetKind): void => {
    activeTilesetKind = kind
    selectedSourceId = null
  }

  const setIncludeUnused = (value: boolean): void => {
    includeUnused = value
    if (!value && selectedMetatile?.usedBy.length === 0) selectedSourceId = null
  }
</script>

{#if loadState.kind === "loading"}
  <section
    class="mx-auto mt-[18vh] max-w-md border border-cartographer-border bg-cartographer-panel p-6"
  >
    <p class="m-0 text-sm font-medium text-cartographer-signal">Loading metatile catalog…</p>
    <div class="mt-4 h-px w-full overflow-hidden bg-cartographer-border">
      <div class="h-full w-2/5 bg-cartographer-signal"></div>
    </div>
  </section>
{:else if loadState.kind === "error"}
  <section
    class="mx-auto mt-[12vh] max-w-2xl border border-cartographer-diagnostic-border bg-cartographer-diagnostic-panel p-7 shadow-cartographer-error"
  >
    <p class="m-0 text-sm font-semibold text-cartographer-rose-400">Catalog unavailable</p>
    <h1 class="mb-3 mt-3 text-2xl font-semibold">Metatiles unavailable</h1>
    <p class="text-cartographer-muted">{loadState.message}</p>
    {#if loadState.details.length > 0}
      <ul>
        {#each loadState.details as detail}<li>{detail}</li>{/each}
      </ul>
    {/if}
  </section>
{:else if !catalog || !activeContextEntry}
  <section
    class="mx-auto mt-[12vh] max-w-2xl border border-cartographer-border bg-cartographer-panel p-8"
  >
    <p class="m-0 text-sm text-cartographer-muted">
      The generated catalog has no render contexts to browse.
    </p>
  </section>
{:else if (contextLoadState.kind === "loading" || contextLoadState.kind === "idle") && !activeContext}
  <section
    class="mx-auto mt-[18vh] max-w-md border border-cartographer-border bg-cartographer-panel p-6"
  >
    <p class="m-0 text-sm font-medium text-cartographer-signal">Loading render context…</p>
    <p class="mb-0 mt-2 text-sm text-cartographer-muted">
      {contextLabel(activeContextEntry)}
    </p>
  </section>
{:else if contextLoadState.kind === "error" && !activeContext}
  <section
    class="mx-auto mt-[12vh] max-w-2xl border border-cartographer-diagnostic-border bg-cartographer-diagnostic-panel p-7 shadow-cartographer-error"
  >
    <p class="m-0 text-sm font-semibold text-cartographer-rose-400">Context unavailable</p>
    <h1 class="mb-3 mt-3 text-2xl font-semibold">Metatiles unavailable</h1>
    <p class="text-cartographer-muted">{contextLoadState.message}</p>
    {#if contextLoadState.details.length > 0}
      <ul>
        {#each contextLoadState.details as detail}<li>{detail}</li>{/each}
      </ul>
    {/if}
  </section>
{:else if !activeContext || !activeTileset || !renderedContextEntry}
  <section
    class="mx-auto mt-[12vh] max-w-2xl border border-cartographer-border bg-cartographer-panel p-8"
  >
    <p class="m-0 text-sm text-cartographer-muted">
      The selected render context did not provide both source tilesets.
    </p>
  </section>
{:else}
  <section class="mx-auto max-w-[1800px] p-[clamp(1rem,2.4vw,2.25rem)]">
    <header class="mb-5 border-b border-cartographer-border pb-4">
      <p class="m-0 text-sm font-medium text-cartographer-signal">Source tilesets</p>
      <h1 class="m-0 mt-1 text-[clamp(1.4rem,2.2vw,2.1rem)] font-semibold tracking-[-0.035em]">
        Metatiles
      </h1>
      <div class="mt-4 flex flex-col gap-3 sm:flex-row sm:items-end sm:justify-between">
        <p class="m-0 max-w-3xl text-sm leading-6 text-cartographer-muted">
          Rendered in the selected primary and secondary tileset context. Source IDs stay scoped to
          their tileset; colors are not assumed to be universal.
        </p>
        <p
          class="m-0 shrink-0 font-cartographer-mono text-[0.68rem] leading-5 tracking-[0.04em] text-cartographer-muted sm:text-right"
        >
          {catalog.contexts.length} contexts · {catalog.pixelsPerMetatile} px per metatile
        </p>
      </div>
    </header>
    <div class="grid gap-4 xl:grid-cols-[15.5rem_minmax(0,1fr)_20rem]">
      <aside class="grid content-start gap-3">
        <MetatileContextPicker
          contexts={catalog.contexts}
          activeContextId={activeContextEntry.id}
          loadingContextId={contextLoadState.kind === "loading" ? contextLoadState.id : null}
          onSelectContext={selectContext}
        />
        <section
          class="border border-cartographer-border bg-cartographer-panel p-3 shadow-cartographer-panel"
          aria-label="Metatile search"
        >
          <h2
            class="mb-3 border-b border-cartographer-border pb-2 text-sm font-semibold text-cartographer-signal"
          >
            Find a metatile
          </h2>
          <label
            class="mb-2 block text-xs font-medium text-cartographer-muted"
            for="metatile-search">Scoped or local ID</label
          >
          <input
            id="metatile-search"
            class="w-full border border-cartographer-border bg-cartographer-field px-2.5 py-2 text-sm text-cartographer-ink placeholder:text-cartographer-muted-soft focus-visible:border-cartographer-signal focus-visible:outline-2 focus-visible:outline-offset-1 focus-visible:outline-cartographer-signal"
            bind:value={query}
            placeholder="e.g. 0x012 or 18"
            type="search"
          />
          <p
            class="mb-0 mt-2 font-cartographer-mono text-[0.65rem] leading-5 text-cartographer-muted"
          >
            {filteredMetatiles.length} shown · {usedMetatiles.length} used in this context
          </p>
          <div class="mt-3 border-t border-cartographer-border pt-3">
            <Checkbox checked={includeUnused} onCheckedChange={setIncludeUnused}
              >Include unused source metatiles</Checkbox
            >
            <p class="mb-0 mt-1 text-xs leading-5 text-cartographer-muted">
              Shows declared rows that no map in this render context places.
            </p>
          </div>
        </section>
      </aside>
      <main class="min-w-0" aria-label="Metatile browser">
        <div
          class="mb-3 flex flex-col gap-3 border border-cartographer-border bg-cartographer-panel p-3 sm:flex-row sm:items-center sm:justify-between"
          aria-busy={contextLoadState.kind === "loading"}
        >
          <div class="min-w-0">
            <p
              class="m-0 font-cartographer-mono text-[0.65rem] tracking-[0.05em] text-cartographer-muted"
            >
              Render context
            </p>
            <p class="mb-0 mt-1 break-words text-sm font-medium text-cartographer-ink">
              {contextLabel(renderedContextEntry)}
            </p>
            {#if contextLoadState.kind === "loading"}
              <span class="sr-only" role="status">Loading {contextLabel(activeContextEntry)}…</span>
            {/if}
          </div>
          <nav class="flex shrink-0 gap-1" aria-label="Tileset layer">
            <Button
              variant={activeTilesetKind === "primary" ? "selected" : "outline"}
              aria-pressed={activeTilesetKind === "primary"}
              onclick={() => selectTileset("primary")}>Primary</Button
            >
            <Button
              variant={activeTilesetKind === "secondary" ? "selected" : "outline"}
              aria-pressed={activeTilesetKind === "secondary"}
              onclick={() => selectTileset("secondary")}>Secondary</Button
            >
          </nav>
        </div>
        <MetatileCatalogGrid
          {includeUnused}
          metatiles={filteredMetatiles}
          selectedId={selectedMetatile?.id ?? null}
          tileset={activeTileset}
          usedMetatileCount={usedMetatiles.length}
          onSelect={(metatile) => (selectedSourceId = metatile.sourceId)}
        />
      </main>
      <MetatileInspector metatile={selectedMetatile} tileset={activeTileset} />
    </div>
  </section>
{/if}
