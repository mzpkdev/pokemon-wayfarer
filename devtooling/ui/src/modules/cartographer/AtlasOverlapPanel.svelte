<script lang="ts">
  import type { AtlasLayoutOverlap } from "./geography.js"

  type Props = {
    overlaps: readonly AtlasLayoutOverlap[]
    visible: boolean
  }

  let { overlaps, visible }: Props = $props()
</script>

{#if visible && overlaps.length > 0}
  <section
    class="border-t border-cartographer-border bg-cartographer-panel-raised px-4 py-3"
    aria-label="Overlap details"
  >
    <p
      class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.12em] text-cartographer-layout-overlap"
    >
      Overlaps
    </p>
    <p class="mb-3 mt-1 text-xs leading-5 text-cartographer-muted">
      These rectangles overlap in the current atlas layout. They do not identify a source-data
      error.
    </p>
    <ul class="m-0 grid max-h-48 list-none gap-1.5 overflow-y-auto p-0">
      {#each overlaps as overlap (`${overlap.maps[0]}:${overlap.maps[1]}`)}
        <li class="border border-cartographer-border bg-cartographer-panel px-3 py-2 text-xs">
          <p class="m-0 font-cartographer-mono text-cartographer-signal-soft">
            {overlap.maps[0]} × {overlap.maps[1]}
          </p>
          <p class="mb-0 mt-1 text-cartographer-muted">
            Shared area: {overlap.area.width} × {overlap.area.height} metatiles at ({overlap.area
              .x}, {overlap.area.y}).
          </p>
        </li>
      {/each}
    </ul>
  </section>
{/if}
