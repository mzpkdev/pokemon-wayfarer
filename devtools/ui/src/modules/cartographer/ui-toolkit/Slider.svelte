<script lang="ts">
  import { Slider as ArkSlider } from "@ark-ui/svelte/slider"

  type Props = {
    label: string
    minimum: number
    maximum: number
    step?: number
    value: number
    onValueChange?: (value: number) => void
  }

  let { label, minimum, maximum, step = 1, value, onValueChange }: Props = $props()

  const handleValueChange = (details: { value: number[] }): void => {
    const next = details.value[0]
    if (next !== undefined) onValueChange?.(next)
  }
</script>

<ArkSlider.Root
  class="grid min-w-48 gap-2"
  min={minimum}
  max={maximum}
  {step}
  value={[value]}
  onValueChange={handleValueChange}
>
  <div class="flex items-baseline justify-between gap-4">
    <ArkSlider.Label class="text-xs font-medium text-cartographer-muted">{label}</ArkSlider.Label>
    <ArkSlider.ValueText
      class="font-cartographer-mono text-sm font-semibold text-cartographer-signal"
    />
  </div>
  <ArkSlider.Control class="relative flex h-5 touch-none items-center">
    <ArkSlider.Track class="h-1 w-full bg-cartographer-border-strong">
      <ArkSlider.Range class="h-full bg-cartographer-signal" />
    </ArkSlider.Track>
    <ArkSlider.Thumb
      index={0}
      class="size-4 rounded-full border-2 border-cartographer-canvas bg-cartographer-signal shadow-cartographer-panel focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-cartographer-signal"
    >
      <ArkSlider.HiddenInput />
    </ArkSlider.Thumb>
  </ArkSlider.Control>
</ArkSlider.Root>
