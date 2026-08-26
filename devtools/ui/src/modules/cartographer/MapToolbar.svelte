<script lang="ts">
  import Button from "./ui-toolkit/Button.svelte"
  import Checkbox from "./ui-toolkit/Checkbox.svelte"

  type Props = {
    surfaceMapCount: number
    componentCount: number
    topologyDiagnosticCount: number
    atlasOverlapCount: number
    showTopologyConflicts: boolean
    showAtlasOverlaps: boolean
    showExits: boolean
    showObjects: boolean
    showEncounterTrainers?: boolean
    encounterMode?: boolean
    encounterMapCount?: number
    onToggleExits?: (value: boolean) => void
    onToggleObjects?: (value: boolean) => void
    onToggleEncounterTrainers?: (value: boolean) => void
    onToggleTopologyConflicts?: (value: boolean) => void
    onToggleAtlasOverlaps?: (value: boolean) => void
    onZoomOut?: () => void
    onZoomIn?: () => void
    onFit?: () => void
  }

  let {
    surfaceMapCount,
    componentCount,
    topologyDiagnosticCount,
    atlasOverlapCount,
    showTopologyConflicts,
    showAtlasOverlaps,
    showExits,
    showObjects,
    showEncounterTrainers = true,
    encounterMode = false,
    encounterMapCount = 0,
    onToggleExits,
    onToggleObjects,
    onToggleEncounterTrainers,
    onToggleTopologyConflicts,
    onToggleAtlasOverlaps,
    onZoomOut,
    onZoomIn,
    onFit,
  }: Props = $props()
</script>

<div
  class="flex flex-wrap items-center gap-x-4 gap-y-2 bg-cartographer-field px-4 py-3 text-sm text-cartographer-muted"
>
  <span class="text-xs">{surfaceMapCount} surfaces · {componentCount} groups</span>
  {#if encounterMode}
    <span class="font-cartographer-mono text-xs text-cartographer-signal-soft"
      >{encounterMapCount} encounter maps</span
    >
    <Checkbox checked={showEncounterTrainers} onCheckedChange={onToggleEncounterTrainers}
      >Trainer events</Checkbox
    >
  {:else}
    {#if topologyDiagnosticCount > 0}
      <Checkbox checked={showTopologyConflicts} onCheckedChange={onToggleTopologyConflicts}
        >Topology diagnostics ({topologyDiagnosticCount})</Checkbox
      >
    {/if}
    {#if atlasOverlapCount > 0}
      <Checkbox checked={showAtlasOverlaps} onCheckedChange={onToggleAtlasOverlaps}
        >Overlaps ({atlasOverlapCount})</Checkbox
      >
    {/if}
    <Checkbox checked={showExits} onCheckedChange={onToggleExits}>Exits</Checkbox>
    <Checkbox checked={showObjects} onCheckedChange={onToggleObjects}>Objects</Checkbox>
  {/if}
  <div class="ml-0 flex gap-1.5 sm:ml-auto" aria-label="Map controls">
    <Button class="px-2 py-1 text-xs" onclick={() => onZoomOut?.()}>−</Button>
    <Button class="px-2 py-1 text-xs" onclick={() => onZoomIn?.()}>+</Button>
    <Button class="text-xs" onclick={() => onFit?.()}>Fit view</Button>
  </div>
</div>
