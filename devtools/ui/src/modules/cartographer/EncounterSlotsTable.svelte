<script lang="ts">
  import { mapImageUrl } from "./urls.js"
  import type { CatalogWildEncounterSlot } from "./catalog.js"

  type Props = {
    slots: readonly CatalogWildEncounterSlot[]
  }

  let { slots }: Props = $props()

  const levelLabel = (minimum: number, maximum: number): string => {
    return minimum === maximum ? `Lv. ${minimum}` : `Lv. ${minimum}–${maximum}`
  }
</script>

<div class="cartographer-scrollbar min-w-0 overflow-x-auto border border-cartographer-border">
  <table class="w-full min-w-[28rem] border-collapse text-left text-sm">
    <thead class="bg-cartographer-panel-raised text-cartographer-muted">
      <tr class="font-cartographer-mono text-[0.68rem] tracking-[0.06em]">
        <th scope="col" class="border-b border-cartographer-border px-3 py-2 font-medium">Slot</th>
        <th scope="col" class="border-b border-cartographer-border px-3 py-2 font-medium"
          >Species</th
        >
        <th scope="col" class="border-b border-cartographer-border px-3 py-2 font-medium">Levels</th
        >
        <th scope="col" class="border-b border-cartographer-border px-3 py-2 text-right font-medium"
          >Weight</th
        >
      </tr>
    </thead>
    <tbody>
      {#each slots as slot (slot.slotIndex)}
        <tr class="border-b border-cartographer-border last:border-b-0">
          <td class="px-3 py-2 font-cartographer-mono text-xs text-cartographer-muted"
            >{slot.slotIndex + 1}</td
          >
          <td class="px-3 py-2 font-medium text-cartographer-signal-soft">
            <div class="flex min-w-0 items-center gap-2">
              {#if slot.sprite}
                <img
                  class="size-7 shrink-0 object-contain [image-rendering:pixelated]"
                  src={mapImageUrl(slot.sprite.path)}
                  alt=""
                  width={slot.sprite.widthPixels}
                  height={slot.sprite.heightPixels}
                />
              {:else}
                <span
                  class="grid size-5 shrink-0 place-items-center border border-cartographer-border-strong font-cartographer-mono text-[0.6rem] text-cartographer-muted"
                  aria-hidden="true">?</span
                >
              {/if}
              <span class="min-w-0 break-words">{slot.speciesLabel ?? slot.speciesId}</span>
            </div>
          </td>
          <td class="px-3 py-2">{levelLabel(slot.minLevel, slot.maxLevel)}</td>
          <td class="px-3 py-2 text-right font-cartographer-mono text-xs">{slot.slotRate}</td>
        </tr>
      {/each}
    </tbody>
  </table>
</div>
