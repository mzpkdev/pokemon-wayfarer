<script lang="ts">
  import type { MapCatalog } from "./catalog.js"
  import Button from "./ui-toolkit/Button.svelte"

  type Props = {
    regions: MapCatalog["regions"]
    activeRegionId: string
    onSelectRegion?: (regionId: string) => void
  }

  let { regions, activeRegionId, onSelectRegion }: Props = $props()
</script>

<nav
  class="flex flex-wrap items-center gap-1.5 border border-cartographer-border bg-cartographer-panel p-3 shadow-cartographer-panel md:grid md:gap-1.5"
  aria-label="Regions"
>
  <div
    class="mb-2 flex w-full items-center justify-between border-b border-cartographer-border pb-2 md:mb-3"
  >
    <h2 class="m-0 text-sm font-semibold text-cartographer-signal">Regions</h2>
    <span class="text-xs text-cartographer-muted">{regions.length}</span>
  </div>
  {#each regions as region}
    <Button
      class="flex w-auto items-center justify-between gap-2 px-2.5 py-2 text-left text-sm md:w-full"
      variant={region.id === activeRegionId ? "selected" : "subtle"}
      onclick={() => onSelectRegion?.(region.id)}
    >
      <span>{region.label}</span>
      <small class="text-xs opacity-75">{region.mapCount} maps</small>
    </Button>
  {/each}
</nav>
