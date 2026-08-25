<script lang="ts">
  import type { CatalogMap, CatalogObject, CatalogWarp } from "./catalog.js"
  import type { ObjectSelection, WarpSelection } from "./types.js"
  import { mapImageUrl } from "./urls.js"
  import { objectPlaceholderFor } from "./object-placeholders.js"
  import Button from "./ui-toolkit/Button.svelte"
  import CollapsibleSection from "./ui-toolkit/CollapsibleSection.svelte"
  import { cn } from "./lib/cn.js"

  type Props = {
    maps: readonly CatalogMap[]
    selectedMap: CatalogMap | null
    selectedWarp: WarpSelection | null
    selectedObject: ObjectSelection | null
    renderedMapNames: ReadonlySet<string>
    onSelectWarp?: (warp: CatalogWarp) => void
    onSelectObject?: (object: CatalogObject) => void
    onFocusMap?: (name: string) => void
  }

  let {
    maps,
    selectedMap,
    selectedWarp,
    selectedObject,
    renderedMapNames,
    onSelectWarp,
    onSelectObject,
    onFocusMap,
  }: Props = $props()

  const destinationFor = (warp: CatalogWarp): CatalogMap | null => {
    return (
      maps.find((map) => map.name === warp.destinationMap || map.id === warp.destinationMapId) ??
      null
    )
  }

  const objectFor = (map: CatalogMap, selection: ObjectSelection | null): CatalogObject | null => {
    if (selection?.sourceMapName !== map.name) return null
    return map.objects.find((object) => object.objectId === selection.objectId) ?? null
  }

  const evidenceLabel = (evidence: CatalogObject["kind"]["evidence"]): string => {
    return {
      "trainer-type": "trainer type",
      graphics: "graphics",
      script: "script",
      fallback: "fallback",
    }[evidence]
  }
</script>

<aside
  class="min-w-0 border border-cartographer-border bg-cartographer-panel p-4 shadow-cartographer-panel xl:min-h-[34rem]"
  aria-live="polite"
>
  {#if !selectedMap}
    <p
      class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.15em] text-cartographer-signal"
    >
      Inspector
    </p>
    <h3 class="mb-3 mt-3 text-lg font-semibold">No map selected</h3>
    <p class="m-0 leading-6 text-cartographer-muted">
      Select a plotted map or use the source search to inspect its layout and exits.
    </p>
  {:else}
    {@const objects = selectedMap.objects ?? []}
    {@const inspectedObject = objectFor(selectedMap, selectedObject)}
    {@const inspectedPlaceholder = inspectedObject ? objectPlaceholderFor(inspectedObject) : null}
    <p
      class="mb-1 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.15em] text-cartographer-signal"
    >
      Map inspector
    </p>
    <h3 class="mb-4 text-xl font-semibold tracking-[-0.02em]">{selectedMap.name}</h3>
    {#if inspectedObject}
      <section class="mb-4 border border-cartographer-signal bg-cartographer-signal/10 p-3">
        <p
          class="m-0 font-cartographer-mono text-[0.68rem] font-bold tracking-[0.15em] text-cartographer-signal"
        >
          Object inspector
        </p>
        <div class="mt-3 flex items-start gap-3">
          {#if inspectedObject.sprite}
            <div
              class="flex size-12 shrink-0 items-end justify-center border border-cartographer-border bg-cartographer-panel"
            >
              <img
                src={mapImageUrl(inspectedObject.sprite.path)}
                alt={inspectedObject.graphicsId}
                width={inspectedObject.sprite.widthPixels}
                height={inspectedObject.sprite.heightPixels}
                class="max-h-10 max-w-10 object-contain [image-rendering:pixelated]"
              />
            </div>
          {:else if inspectedPlaceholder}
            <div
              class="object-placeholder flex size-12 shrink-0 items-center justify-center border border-cartographer-border bg-cartographer-panel"
              data-kind={inspectedPlaceholder.kind}
              aria-label={inspectedPlaceholder.label}
            >
              <span aria-hidden="true"></span>
            </div>
          {/if}
          <div class="min-w-0">
            <h4
              class="m-0 break-words font-cartographer-mono text-sm font-semibold text-cartographer-signal-soft"
            >
              {inspectedObject.kind.label}
            </h4>
            <p
              class="mb-0 mt-1 break-all font-cartographer-mono text-[0.68rem] leading-5 text-cartographer-muted"
            >
              {inspectedObject.graphicsId} · ({inspectedObject.xMetatiles}, {inspectedObject.yMetatiles})
            </p>
          </div>
        </div>
        <dl class="m-0 mt-3 grid gap-2 border-t border-cartographer-border pt-3 text-xs">
          <div class="flex justify-between gap-3">
            <dt class="text-cartographer-muted">Kind</dt>
            <dd class="m-0 text-right">
              {inspectedObject.kind.label} · {evidenceLabel(inspectedObject.kind.evidence)}
            </dd>
          </div>
          {#if inspectedObject.kind.action}
            <div class="flex justify-between gap-3">
              <dt class="text-cartographer-muted">Action</dt>
              <dd class="m-0 break-all text-right">{inspectedObject.kind.action}</dd>
            </div>
          {/if}
          <div class="flex justify-between gap-3">
            <dt class="text-cartographer-muted">Script</dt>
            <dd class="m-0 break-all text-right">{inspectedObject.script}</dd>
          </div>
          <div class="flex justify-between gap-3">
            <dt class="text-cartographer-muted">Flag</dt>
            <dd class="m-0 break-all text-right">{inspectedObject.flag}</dd>
          </div>
          <div class="flex justify-between gap-3">
            <dt class="text-cartographer-muted">Movement</dt>
            <dd class="m-0 break-all text-right">
              {inspectedObject.movementType} · {inspectedObject.movementRange.x} × {inspectedObject
                .movementRange.y}
            </dd>
          </div>
          <div class="flex justify-between gap-3">
            <dt class="text-cartographer-muted">Trainer / state</dt>
            <dd class="m-0 break-all text-right">
              {inspectedObject.trainerType} · {inspectedObject.trainerSightOrBerryTreeId}
            </dd>
          </div>
          {#if inspectedObject.sprite}
            <div class="flex justify-between gap-3">
              <dt class="text-cartographer-muted">Sprite</dt>
              <dd class="m-0 text-right">
                {inspectedObject.sprite.widthPixels} × {inspectedObject.sprite.heightPixels}px
              </dd>
            </div>
            <div class="grid gap-1 border-t border-cartographer-border pt-2">
              <dt class="text-cartographer-muted">Resolved from</dt>
              <dd
                class="m-0 break-all font-cartographer-mono text-[0.68rem] text-cartographer-signal-soft"
              >
                {inspectedObject.sprite.source}
              </dd>
            </div>
          {:else}
            <div class="border-t border-cartographer-border pt-2">
              {#if inspectedPlaceholder}
                <p class="m-0 font-cartographer-mono text-[0.68rem] text-cartographer-muted">
                  {inspectedPlaceholder.label}
                </p>
              {/if}
              <p class="mb-0 mt-1 text-cartographer-garnet-400">
                {inspectedObject.diagnostic?.message ??
                  "No source sprite resolved for this object. The map uses the fallback marker."}
              </p>
            </div>
          {/if}
        </dl>
      </section>
    {/if}
    <dl class="m-0 grid gap-3 border-y border-cartographer-border py-3 text-sm">
      <div class="grid gap-1 md:grid-cols-[minmax(7rem,10rem)_minmax(0,1fr)] md:gap-3">
        <dt
          class="font-cartographer-mono text-[0.68rem] font-bold tracking-[0.08em] text-cartographer-muted"
        >
          Source ID
        </dt>
        <dd class="m-0 break-words font-cartographer-mono text-xs text-cartographer-signal-soft">
          <code>{selectedMap.id}</code>
        </dd>
      </div>
      <div class="grid gap-1 md:grid-cols-[minmax(7rem,10rem)_minmax(0,1fr)] md:gap-3">
        <dt
          class="font-cartographer-mono text-[0.68rem] font-bold tracking-[0.08em] text-cartographer-muted"
        >
          Section
        </dt>
        <dd class="m-0 break-words">{selectedMap.mapSection ?? "Not assigned"}</dd>
      </div>
      <div class="grid gap-1 md:grid-cols-[minmax(7rem,10rem)_minmax(0,1fr)] md:gap-3">
        <dt
          class="font-cartographer-mono text-[0.68rem] font-bold tracking-[0.08em] text-cartographer-muted"
        >
          Layout
        </dt>
        <dd class="m-0 break-words">
          {selectedMap.layout.widthMetatiles} × {selectedMap.layout.heightMetatiles} metatiles
        </dd>
      </div>
      <div class="grid gap-1 md:grid-cols-[minmax(7rem,10rem)_minmax(0,1fr)] md:gap-3">
        <dt
          class="font-cartographer-mono text-[0.68rem] font-bold tracking-[0.08em] text-cartographer-muted"
        >
          Render state
        </dt>
        <dd class="m-0 break-words">
          {renderedMapNames.has(selectedMap.name)
            ? "Rendered default-visible surface map"
            : "Not visible in the default cartographer"}
        </dd>
      </div>
    </dl>
    <CollapsibleSection title="Exits" count={selectedMap.warps.length}>
      {#if selectedMap.warps.length === 0}
        <p class="m-0 p-4 text-sm text-cartographer-muted">
          This map has no catalogued warp exits.
        </p>
      {:else}
        <ul class="m-0 grid min-w-0 list-none gap-1.5 p-4">
          {#each selectedMap.warps as warp (warp.warpId)}
            {@const destination = destinationFor(warp)}
            <li class="min-w-0">
              <Button
                class={cn(
                  "flex min-w-0 w-full items-center justify-between gap-3 border-cartographer-border px-3 py-2 text-left font-cartographer-mono text-xs hover:border-cartographer-signal hover:bg-cartographer-signal/10 focus-visible:outline-cartographer-signal",
                  selectedWarp?.sourceMapName === selectedMap.name &&
                    selectedWarp?.warpId === warp.warpId &&
                    "border-cartographer-signal bg-cartographer-signal/10",
                )}
                onclick={() => onSelectWarp?.(warp)}
              >
                <span class="shrink-0"
                  >Warp {warp.warpId} · ({warp.xMetatiles}, {warp.yMetatiles})</span
                >
                <small
                  class="min-w-0 flex-1 break-words text-right font-cartographer-mono text-[0.68rem] text-cartographer-muted"
                  >{destination?.name ?? warp.destinationMap ?? warp.destinationMapId}</small
                >
              </Button>
              {#if destination && renderedMapNames.has(destination.name) && selectedWarp?.sourceMapName === selectedMap.name && selectedWarp?.warpId === warp.warpId}
                <Button
                  class="mt-2 min-h-9 text-xs"
                  variant="solid"
                  onclick={() => onFocusMap?.(destination.name)}>Focus {destination.name}</Button
                >
              {/if}
            </li>
          {/each}
        </ul>
      {/if}
    </CollapsibleSection>
    <CollapsibleSection title="Objects" count={objects.length}>
      {#if objects.length === 0}
        <p class="m-0 p-4 text-sm leading-6 text-cartographer-muted">
          This map has no catalogued objects.
        </p>
      {:else}
        <ul class="m-0 grid min-w-0 list-none gap-1.5 p-4">
          {#each objects as object (object.objectId)}
            {@const selected =
              selectedObject?.sourceMapName === selectedMap.name &&
              selectedObject?.objectId === object.objectId}
            {@const placeholder = objectPlaceholderFor(object)}
            <li class="min-w-0">
              <Button
                class={cn(
                  "flex min-w-0 w-full items-center justify-between gap-3 border-cartographer-border px-3 py-2 text-left font-cartographer-mono text-xs hover:border-cartographer-signal hover:bg-cartographer-signal/10 focus-visible:outline-cartographer-signal",
                  selected && "border-cartographer-signal bg-cartographer-signal/10",
                )}
                onclick={() => onSelectObject?.(object)}
              >
                <span class="min-w-0 flex-1">
                  <span class="block"
                    >{object.kind.label} · ({object.xMetatiles}, {object.yMetatiles})</span
                  >
                  <small class="block break-all text-cartographer-muted">{object.graphicsId}</small>
                </span>
                <small
                  class="shrink-0 text-right font-cartographer-mono text-[0.68rem] text-cartographer-muted"
                  >{selected
                    ? "Selected"
                    : object.sprite
                      ? "Sprite ready"
                      : (placeholder?.label ?? "No sprite")}</small
                >
              </Button>
            </li>
          {/each}
        </ul>
      {/if}
    </CollapsibleSection>
  {/if}
</aside>

<style>
  .object-placeholder > span {
    display: block;
  }

  .object-placeholder[data-kind="stateful"] > span {
    width: 12px;
    height: 12px;
    background: var(--color-cartographer-object-stateful);
    border: 2px solid var(--color-cartographer-field);
  }

  .object-placeholder[data-kind="variable"] > span,
  .object-placeholder[data-kind="unresolved"] > span {
    width: 12px;
    height: 12px;
    border: 2px solid var(--color-cartographer-field);
  }

  .object-placeholder[data-kind="variable"] > span {
    background: var(--color-cartographer-object-variable);
  }

  .object-placeholder[data-kind="expression"] > span {
    width: 0;
    height: 0;
    border-bottom: 13px solid var(--color-cartographer-object-expression);
    border-left: 7px solid transparent;
    border-right: 7px solid transparent;
  }

  .object-placeholder[data-kind="unresolved"] > span {
    background: var(--color-cartographer-object-unresolved);
  }
</style>
