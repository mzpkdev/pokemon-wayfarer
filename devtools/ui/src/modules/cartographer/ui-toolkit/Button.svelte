<script lang="ts">
  import type { Snippet } from "svelte"
  import type { HTMLButtonAttributes } from "svelte/elements"

  import { cn } from "../lib/cn.js"

  type Props = HTMLButtonAttributes & {
    children?: Snippet
    class?: string
    variant?: "outline" | "selected" | "solid" | "subtle"
  }

  let {
    children,
    class: className,
    type = "button",
    variant = "outline",
    ...rest
  }: Props = $props()

  const variants = {
    outline:
      "border border-cartographer-border bg-cartographer-panel-raised text-cartographer-ink hover:border-cartographer-signal hover:bg-cartographer-signal/10 hover:text-cartographer-signal-soft focus-visible:outline-cartographer-signal",
    solid:
      "border border-cartographer-signal bg-cartographer-signal text-cartographer-canvas hover:bg-cartographer-signal-soft focus-visible:outline-cartographer-signal",
    selected:
      "border border-cartographer-signal bg-cartographer-signal/15 text-cartographer-signal hover:bg-cartographer-signal/25 focus-visible:outline-cartographer-signal",
    subtle:
      "border border-transparent text-cartographer-muted hover:border-cartographer-border hover:bg-cartographer-panel-raised hover:text-cartographer-ink focus-visible:outline-cartographer-signal",
  }
</script>

<button
  {...rest}
  {type}
  class={cn(
    "px-3 py-1.5 transition-colors focus-visible:outline-2 focus-visible:outline-offset-1 disabled:cursor-not-allowed disabled:opacity-60",
    variants[variant],
    className,
  )}
>
  {@render children?.()}
</button>
