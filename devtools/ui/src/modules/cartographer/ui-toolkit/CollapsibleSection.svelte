<script lang="ts">
  import type { Snippet } from "svelte"

  type Props = {
    title: string
    label?: string
    meta?: string
    count?: number | string
    open?: boolean
    children: Snippet
  }

  let { title, label = title, meta, count, open = false, children }: Props = $props()
</script>

<details class="min-w-0 border-b border-cartographer-border" {open} aria-label={label}>
  <summary
    class="flex cursor-pointer list-none items-center justify-between gap-3 px-4 py-3 marker:content-none hover:bg-cartographer-panel-raised focus-visible:outline-2 focus-visible:outline-offset-[-2px] focus-visible:outline-cartographer-signal"
  >
    <span class="min-w-0">
      <span class="block text-base font-semibold">{title}</span>
      {#if meta}
        <span
          class="mt-0.5 block break-all font-cartographer-mono text-[0.68rem] text-cartographer-muted"
          >{meta}</span
        >
      {/if}
    </span>
    <span class="flex shrink-0 items-center gap-3">
      {#if count !== undefined}
        <span class="font-cartographer-mono text-xs text-cartographer-amber">{count}</span>
      {/if}
      <span class="cartographer-disclosure-mark text-cartographer-muted" aria-hidden="true">+</span>
    </span>
  </summary>
  <div class="min-w-0 border-t border-cartographer-border">{@render children()}</div>
</details>

<style>
  .cartographer-disclosure-mark {
    font-family: var(--font-cartographer-mono);
    font-size: 1rem;
    line-height: 1;
  }

  details[open] .cartographer-disclosure-mark {
    color: var(--color-cartographer-signal-soft);
    transform: rotate(45deg);
  }
</style>
