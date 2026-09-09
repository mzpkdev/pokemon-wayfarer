<script lang="ts">
  import type { MapCatalog } from "./catalog.js"

  type Props = {
    builds: MapCatalog["builds"]
    activeBuildId: string
    onSelectBuild?: (buildId: string) => void
  }

  let { builds, activeBuildId, onSelectBuild }: Props = $props()
</script>

<section
  class="border border-cartographer-border bg-cartographer-panel p-3 shadow-cartographer-panel"
  aria-label="Game build"
>
  <label class="grid gap-2 text-xs font-medium text-cartographer-muted">
    Game build
    <select
      class="border border-cartographer-border bg-cartographer-field px-2.5 py-2 text-sm text-cartographer-ink focus-visible:border-cartographer-signal focus-visible:outline-2 focus-visible:outline-offset-1 focus-visible:outline-cartographer-signal"
      value={activeBuildId}
      onchange={(event) => onSelectBuild?.(event.currentTarget.value)}
    >
      {#each builds as build (build.id)}
        <option value={build.id}>{build.label} ({build.mapCount} maps)</option>
      {/each}
    </select>
  </label>
</section>
