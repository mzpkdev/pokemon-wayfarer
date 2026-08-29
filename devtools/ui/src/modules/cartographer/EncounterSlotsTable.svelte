<script lang="ts">
  import { mapImageUrl } from "./urls.js"
  import type { ResolvedEncounterSlot } from "./encounters.js"

  type Props = {
    slots: readonly ResolvedEncounterSlot[]
    trainerRating: number
  }

  let { slots, trainerRating }: Props = $props()

  const levelLabel = (minimum: number, maximum: number): string => {
    return minimum === maximum ? `Lv. ${minimum}` : `Lv. ${minimum}–${maximum}`
  }

  const percentage = (weight: number | null): string => {
    return weight === null ? "Unavailable" : `${(weight * 100).toFixed(1).replace(/\.0$/, "")}%`
  }
</script>

<div class="cartographer-scrollbar min-w-0 overflow-x-auto border border-cartographer-border">
  {#if slots.length > 0 && slots.every((slot) => !slot.eligible)}
    <p class="m-0 border-b border-cartographer-border px-3 py-2 text-sm text-cartographer-muted">
      All authored slots are unavailable at Trainer Rating {trainerRating}.
    </p>
  {/if}
  <table class="w-full min-w-[34rem] border-collapse text-left text-sm">
    <thead class="bg-cartographer-panel-raised text-cartographer-muted">
      <tr class="font-cartographer-mono text-[0.68rem] tracking-[0.06em]">
        <th scope="col" class="border-b border-cartographer-border px-3 py-2 font-medium">Slot</th>
        <th scope="col" class="border-b border-cartographer-border px-3 py-2 font-medium"
          >Authored source</th
        >
        <th scope="col" class="border-b border-cartographer-border px-3 py-2 font-medium"
          >Effective at rating {trainerRating}</th
        >
        <th scope="col" class="border-b border-cartographer-border px-3 py-2 text-right font-medium"
          >Selection weight</th
        >
      </tr>
    </thead>
    <tbody>
      {#each slots as slot (slot.source.slotIndex)}
        <tr class="border-b border-cartographer-border last:border-b-0">
          <td class="px-3 py-3 align-top font-cartographer-mono text-xs text-cartographer-muted"
            >{slot.source.slotIndex + 1}</td
          >
          <td class="px-3 py-3 align-top">
            <div class="flex min-w-0 items-center gap-2 font-medium text-cartographer-signal-soft">
              {#if slot.source.sprite}
                <img
                  class="size-7 shrink-0 object-contain [image-rendering:pixelated]"
                  src={mapImageUrl(slot.source.sprite.path)}
                  alt=""
                  width={slot.source.sprite.widthPixels}
                  height={slot.source.sprite.heightPixels}
                />
              {/if}
              <span class="min-w-0 break-words"
                >{slot.source.speciesLabel ?? slot.source.speciesId}</span
              >
            </div>
            <p class="mb-0 mt-1 font-cartographer-mono text-[0.68rem] text-cartographer-muted">
              Authored {levelLabel(slot.source.minLevel, slot.source.maxLevel)}
              {#if slot.source.minLevel !== slot.source.runtimeMinLevel || slot.source.maxLevel !== slot.source.runtimeMaxLevel}
                · runtime envelope {levelLabel(
                  slot.source.runtimeMinLevel,
                  slot.source.runtimeMaxLevel,
                )}
              {/if}
              · source weight {slot.source.slotRate}
            </p>
          </td>
          <td class="px-3 py-3 align-top">
            {#if slot.outcomes.length === 0}
              <span class="text-cartographer-muted">Projection unavailable</span>
            {:else}
              <ul class="m-0 grid list-none gap-2 p-0">
                {#each slot.outcomes as outcome (`${outcome.speciesId}-${outcome.authoredMinimumLevel}`)}
                  <li class:opacity-60={!slot.eligible} class="flex min-w-0 items-center gap-2">
                    {#if outcome.sprite}
                      <img
                        class="size-7 shrink-0 object-contain [image-rendering:pixelated]"
                        src={mapImageUrl(outcome.sprite.path)}
                        alt=""
                        width={outcome.sprite.widthPixels}
                        height={outcome.sprite.heightPixels}
                      />
                    {/if}
                    <span class="min-w-0">
                      <span class="font-medium">{outcome.speciesLabel}</span>
                      <span
                        class="block font-cartographer-mono text-[0.68rem] text-cartographer-muted"
                      >
                        Authored {levelLabel(
                          outcome.authoredMinimumLevel,
                          outcome.authoredMaximumLevel,
                        )}
                        → projected {levelLabel(
                          outcome.projectedMinimumLevel,
                          outcome.projectedMaximumLevel,
                        )}
                        {#if !outcome.eligible}
                          · requires ordinary wild level {outcome.minimumOrdinaryWildLevel}
                        {/if}
                      </span>
                    </span>
                  </li>
                {/each}
              </ul>
            {/if}
          </td>
          <td
            class="px-3 py-3 text-right align-top font-cartographer-mono text-xs"
            class:text-cartographer-muted={!slot.eligible}
          >
            {percentage(slot.selectionWeight)}
          </td>
        </tr>
      {/each}
    </tbody>
  </table>
</div>
