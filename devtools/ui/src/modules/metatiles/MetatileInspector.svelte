<script lang="ts">
  import type { CatalogMetatile, MetatileTileset } from "./catalog.js"
  import { metatileScopedLabel } from "./catalog.js"
  import CollapsibleSection from "../cartographer/ui-toolkit/CollapsibleSection.svelte"
  import MetatilePreview from "./MetatilePreview.svelte"

  type Props = {
    metatile: CatalogMetatile | null
    tileset: MetatileTileset | null
  }

  let { metatile, tileset }: Props = $props()

  const hexadecimal = (value: number): string => `0x${value.toString(16).toUpperCase()}`

  const tileLabel = (layer: number, quadrant: number): string => {
    const positions = ["top left", "top right", "bottom left", "bottom right"]
    return `Layer ${layer + 1} · ${positions[quadrant] ?? `quadrant ${quadrant}`}`
  }
</script>

<aside
  class="cartographer-scrollbar min-w-0 overflow-y-auto border border-cartographer-border bg-cartographer-panel shadow-cartographer-panel xl:max-h-[calc(100vh-11rem)] xl:min-h-[34rem]"
  aria-live="polite"
  aria-label="Metatile inspector"
>
  {#if !metatile || !tileset}
    <div class="p-4">
      <p
        class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.15em] text-cartographer-signal"
      >
        Inspector
      </p>
      <h2 class="mb-3 mt-3 text-lg font-semibold">No metatile selected</h2>
      <p class="m-0 leading-6 text-cartographer-muted">
        Select a rendered metatile to inspect its scoped ID, source tiles, attributes, and map use.
      </p>
    </div>
  {:else}
    <div class="min-w-0 p-4">
      <p
        class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.15em] text-cartographer-signal"
      >
        Metatile inspector
      </p>
      <h2
        class="mb-0 mt-2 break-all font-cartographer-mono text-base font-semibold text-cartographer-signal-soft"
      >
        {metatileScopedLabel(metatile)}
      </h2>
      <div class="mt-4 flex items-start gap-4 border-y border-cartographer-border py-4">
        <MetatilePreview {metatile} size={112} {tileset} />
        <dl class="m-0 min-w-0 grid flex-1 gap-2 text-xs">
          <div class="grid gap-0.5">
            <dt class="font-cartographer-mono text-[0.65rem] text-cartographer-muted">Local ID</dt>
            <dd class="m-0 break-words font-cartographer-mono text-cartographer-ink">
              {metatile.id} · {hexadecimal(metatile.id)}
            </dd>
          </div>
          <div class="grid gap-0.5">
            <dt class="font-cartographer-mono text-[0.65rem] text-cartographer-muted">Tileset</dt>
            <dd class="m-0 break-all font-cartographer-mono text-cartographer-ink">
              {tileset.tilesetId}
            </dd>
          </div>
          <div class="grid gap-0.5">
            <dt class="font-cartographer-mono text-[0.65rem] text-cartographer-muted">Layer</dt>
            <dd class="m-0 text-cartographer-ink">{tileset.kind}</dd>
          </div>
        </dl>
      </div>
      <dl class="m-0 grid gap-2 border-b border-cartographer-border py-4 text-xs">
        <div class="flex min-w-0 items-start justify-between gap-3">
          <dt class="text-cartographer-muted">Raw attributes</dt>
          <dd class="m-0 break-all text-right font-cartographer-mono">
            {hexadecimal(metatile.attributes.raw)}
          </dd>
        </div>
        <div class="flex min-w-0 items-start justify-between gap-3">
          <dt class="text-cartographer-muted">Behavior</dt>
          <dd class="m-0 break-all text-right font-cartographer-mono">
            {metatile.attributes.behaviorName ?? hexadecimal(metatile.attributes.behavior)}
          </dd>
        </div>
        <div class="flex min-w-0 items-start justify-between gap-3">
          <dt class="text-cartographer-muted">Behavior ID</dt>
          <dd class="m-0 text-right font-cartographer-mono">
            {metatile.attributes.behavior} · {hexadecimal(metatile.attributes.behavior)}
          </dd>
        </div>
        <div class="flex min-w-0 items-start justify-between gap-3">
          <dt class="text-cartographer-muted">Layer type</dt>
          <dd class="m-0 text-right font-cartographer-mono">
            {metatile.attributes.layerType} · {hexadecimal(metatile.attributes.layerType)}
          </dd>
        </div>
      </dl>
    </div>
    <CollapsibleSection title="Source tiles" count={metatile.tiles.length} open>
      <ol class="m-0 grid min-w-0 list-none divide-y divide-cartographer-border p-0">
        {#each metatile.tiles.toSorted((left, right) => left.layer - right.layer || left.quadrant - right.quadrant) as tile (`${tile.layer}:${tile.quadrant}`)}
          <li class="grid min-w-0 gap-1 px-4 py-2.5 text-xs">
            <div class="flex min-w-0 items-center justify-between gap-3">
              <span class="min-w-0 break-words text-cartographer-ink"
                >{tileLabel(tile.layer, tile.quadrant)}</span
              >
              <span class="shrink-0 font-cartographer-mono text-cartographer-signal-soft"
                >{hexadecimal(tile.tileId)}</span
              >
            </div>
            <p
              class="m-0 break-words font-cartographer-mono text-[0.65rem] leading-5 text-cartographer-muted"
            >
              {tile.source} · source tile {hexadecimal(tile.sourceTileId)} · palette {tile.paletteId}{tile.horizontalFlip
                ? " · flip H"
                : ""}{tile.verticalFlip ? " · flip V" : ""}
            </p>
          </li>
        {/each}
      </ol>
    </CollapsibleSection>
    <CollapsibleSection title="Used by maps" count={metatile.usedBy.length}>
      {#if metatile.usedBy.length === 0}
        <p class="m-0 p-4 text-sm text-cartographer-muted">
          No rendered source layout uses this metatile.
        </p>
      {:else}
        <ul class="m-0 grid min-w-0 list-none divide-y divide-cartographer-border p-0">
          {#each metatile.usedBy as usage (`${usage.mapId}:${usage.layoutId}`)}
            <li class="grid min-w-0 gap-1 px-4 py-3 text-xs">
              <p class="m-0 break-words font-medium text-cartographer-ink">{usage.mapName}</p>
              <p
                class="m-0 break-all font-cartographer-mono text-[0.65rem] text-cartographer-muted"
              >
                {usage.mapId} · {usage.layoutId} · {usage.count} blocks
              </p>
              <p
                class="m-0 break-words font-cartographer-mono text-[0.65rem] text-cartographer-muted"
              >
                {usage.placements
                  .map(
                    (placement) =>
                      `collision ${hexadecimal(placement.collision)} · elevation ${placement.elevation} (${placement.count})`,
                  )
                  .join("; ")}
              </p>
            </li>
          {/each}
        </ul>
      {/if}
    </CollapsibleSection>
  {/if}
</aside>
