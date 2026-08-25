<script lang="ts">
  import Checkbox from "./ui-toolkit/Checkbox.svelte"

  type Props = {
    kinds: readonly { label: string; count: number; visible: boolean }[]
    onToggle?: (label: string, visible: boolean) => void
  }

  let { kinds, onToggle }: Props = $props()
</script>

<section
  class="border-t border-cartographer-border bg-cartographer-panel-raised px-4 py-3"
  aria-label="Object filters"
>
  <div class="flex items-baseline justify-between gap-4">
    <p
      class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.12em] text-cartographer-muted"
    >
      Object filters
    </p>
    <span class="font-cartographer-mono text-[0.68rem] text-cartographer-muted">
      {kinds.filter((kind) => kind.visible).length}/{kinds.length} kinds
    </span>
  </div>
  <div class="mt-2 grid grid-cols-2 gap-x-4 gap-y-2 sm:grid-cols-3 xl:grid-cols-5">
    {#each kinds as kind (kind.label)}
      <Checkbox
        checked={kind.visible}
        onCheckedChange={(visible) => onToggle?.(kind.label, visible)}
        >{kind.label} ({kind.count})</Checkbox
      >
    {/each}
  </div>
</section>
