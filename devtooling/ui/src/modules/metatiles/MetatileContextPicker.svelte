<script lang="ts">
  import type { MetatileCatalogContext } from "./catalog.js"
  import Button from "../cartographer/ui-toolkit/Button.svelte"

  type Props = {
    activeContextId: string
    contexts: readonly MetatileCatalogContext[]
    loadingContextId?: string | null
    onSelectContext?: (id: string) => void
  }

  let { activeContextId, contexts, loadingContextId = null, onSelectContext }: Props = $props()
  let query = $state("")

  const labelFor = (context: MetatileCatalogContext): string => {
    return `${context.primaryTileset} + ${context.secondaryTileset}`
  }

  let visibleContexts = $derived(
    contexts
      .filter((context) =>
        labelFor(context).toLocaleLowerCase("en").includes(query.toLocaleLowerCase("en")),
      )
      .toSorted((left, right) => {
        const usageDifference = right.usedMetatileCount - left.usedMetatileCount
        if (usageDifference) return usageDifference
        const placementDifference = right.placementCount - left.placementCount
        if (placementDifference) return placementDifference
        return labelFor(left).localeCompare(labelFor(right))
      }),
  )
</script>

<nav
  class="cartographer-scrollbar grid max-h-[24rem] content-start gap-1.5 overflow-y-auto border border-cartographer-border bg-cartographer-panel p-3 shadow-cartographer-panel"
  aria-label="Metatile render contexts"
>
  <div class="mb-2 flex items-center justify-between border-b border-cartographer-border pb-2">
    <h2 class="m-0 text-sm font-semibold text-cartographer-signal">Render contexts</h2>
    <span class="text-xs text-cartographer-muted">{contexts.length}</span>
  </div>
  <label class="sr-only" for="metatile-context-search">Find a render context</label>
  <input
    id="metatile-context-search"
    class="mb-1 w-full border border-cartographer-border bg-cartographer-field px-2 py-1.5 text-xs text-cartographer-ink placeholder:text-cartographer-muted-soft focus-visible:border-cartographer-signal focus-visible:outline-2 focus-visible:outline-offset-1 focus-visible:outline-cartographer-signal"
    bind:value={query}
    placeholder="Find a tileset"
    type="search"
  />
  {#each visibleContexts as context (context.id)}
    <Button
      class="grid w-full min-w-0 gap-1 px-2.5 py-2 text-left text-sm"
      variant={context.id === activeContextId ? "selected" : "subtle"}
      aria-busy={context.id === loadingContextId ? "true" : undefined}
      aria-current={context.id === activeContextId ? "true" : undefined}
      onclick={() => onSelectContext?.(context.id)}
    >
      <span class="break-words font-medium">{labelFor(context)}</span>
      <small class="break-words font-cartographer-mono text-[0.65rem] text-cartographer-muted"
        >{context.mapCount} maps · {context.usedMetatileCount} used</small
      >
    </Button>
  {/each}
  {#if visibleContexts.length === 0}
    <p class="m-0 px-2 py-3 text-xs text-cartographer-muted">No render contexts match that ID.</p>
  {/if}
</nav>
