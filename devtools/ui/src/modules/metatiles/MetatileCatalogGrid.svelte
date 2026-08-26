<script lang="ts">
  import type { CatalogMetatile, MetatileTileset } from "./catalog.js"
  import { metatileScopedLabel } from "./catalog.js"
  import { cn } from "../cartographer/lib/cn.js"
  import Button from "../cartographer/ui-toolkit/Button.svelte"
  import MetatilePreview from "./MetatilePreview.svelte"

  type Props = {
    includeUnused: boolean
    metatiles: readonly CatalogMetatile[]
    selectedId: number | null
    tileset: MetatileTileset
    usedMetatileCount: number
    onSelect?: (metatile: CatalogMetatile) => void
  }

  let { includeUnused, metatiles, selectedId, tileset, usedMetatileCount, onSelect }: Props =
    $props()
</script>

<section
  class="min-w-0 border border-cartographer-border bg-cartographer-panel shadow-cartographer-panel"
>
  <div
    class="flex items-center justify-between gap-3 border-b border-cartographer-border px-4 py-3"
  >
    <div class="min-w-0">
      <h2 class="m-0 break-words text-sm font-semibold text-cartographer-signal">
        {tileset.tilesetId}
      </h2>
      <p
        class="mb-0 mt-1 font-cartographer-mono text-[0.65rem] tracking-[0.04em] text-cartographer-muted"
      >
        {tileset.kind} · {metatiles.length} shown · {usedMetatileCount} used
      </p>
      {#if tileset.atlas.unresolvedTileReferences > 0}
        <p class="mb-0 mt-1 text-xs text-cartographer-rose-400">
          {tileset.atlas.unresolvedTileReferences} source tile references use a diagnostic checker.
        </p>
      {/if}
    </div>
    <span class="shrink-0 font-cartographer-mono text-[0.65rem] text-cartographer-muted"
      >{tileset.atlas.columns} columns</span
    >
  </div>
  {#if metatiles.length === 0}
    <p class="m-0 p-5 text-sm text-cartographer-muted">
      {includeUnused
        ? "No source metatiles match the current filter."
        : "This tileset has no metatiles used by maps in this render context."}
    </p>
  {:else}
    <div
      class="grid grid-cols-[repeat(auto-fill,minmax(5.5rem,1fr))] gap-px bg-cartographer-border p-px"
      aria-label={`${tileset.tilesetId} metatiles`}
    >
      {#each metatiles as metatile (metatile.sourceId)}
        <Button
          class={cn(
            "group grid min-w-0 place-items-center gap-1.5 border-transparent bg-cartographer-panel px-1.5 py-2 font-cartographer-mono text-[0.61rem] hover:bg-cartographer-panel-raised focus-visible:outline-cartographer-signal",
            selectedId === metatile.id && "border-cartographer-signal bg-cartographer-signal/10",
          )}
          aria-label={metatileScopedLabel(metatile)}
          aria-pressed={selectedId === metatile.id}
          variant="subtle"
          onclick={() => onSelect?.(metatile)}
        >
          <MetatilePreview {metatile} size={48} {tileset} />
          <span
            class="w-full truncate text-center text-cartographer-muted group-hover:text-cartographer-ink"
            >0x{metatile.id.toString(16).toUpperCase().padStart(3, "0")}</span
          >
        </Button>
      {/each}
    </div>
  {/if}
</section>
