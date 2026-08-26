<script lang="ts">
  import { Checkbox as ArkCheckbox } from "@ark-ui/svelte/checkbox"
  import type { Snippet } from "svelte"

  type Props = {
    checked: boolean
    children?: Snippet
    onCheckedChange?: (checked: boolean) => void
  }

  let { checked = $bindable(), children, onCheckedChange }: Props = $props()

  const handleCheckedChange = (details: { checked: boolean | "indeterminate" }): void => {
    onCheckedChange?.(details.checked === true)
  }
</script>

<ArkCheckbox.Root
  bind:checked
  class="flex cursor-pointer items-center gap-1.5 text-xs font-medium whitespace-nowrap text-cartographer-muted"
  onCheckedChange={handleCheckedChange}
>
  <ArkCheckbox.Control
    class="pointer-events-none flex size-[1.1rem] shrink-0 items-center justify-center border border-cartographer-border bg-cartographer-field leading-none text-cartographer-canvas transition-colors data-[state=checked]:border-cartographer-signal data-[state=checked]:bg-cartographer-signal focus-visible:outline-2 focus-visible:outline-offset-1 focus-visible:outline-cartographer-signal"
  >
    <ArkCheckbox.Indicator>
      <svg aria-hidden="true" class="block size-3" viewBox="0 0 12 12"
        ><path d="m2 6 2.5 2.5L10 3" fill="none" stroke="currentColor" stroke-width="2" /></svg
      >
    </ArkCheckbox.Indicator>
  </ArkCheckbox.Control>
  <ArkCheckbox.HiddenInput />
  <ArkCheckbox.Label>{@render children?.()}</ArkCheckbox.Label>
</ArkCheckbox.Root>
