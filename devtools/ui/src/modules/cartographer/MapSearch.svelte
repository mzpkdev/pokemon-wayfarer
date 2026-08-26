<script lang="ts">
  import type { CatalogMap } from "./catalog.js"
  import SearchCombobox, { type SearchOption } from "./ui-toolkit/SearchCombobox.svelte"

  type Props = {
    maps: readonly CatalogMap[]
    query?: string
    onSelectMap?: (name: string, focus: boolean) => void
  }

  let { maps, query = $bindable(""), onSelectMap }: Props = $props()

  const search = (catalogMaps: readonly CatalogMap[], searchQuery: string): CatalogMap[] => {
    const needle = searchQuery.toLocaleLowerCase("en").replace(/[^a-z0-9]+/g, "")
    if (!needle) return []
    return catalogMaps
      .filter((map) =>
        [map.name, map.mapSection ?? ""].some((value) =>
          value
            .toLocaleLowerCase("en")
            .replace(/[^a-z0-9]+/g, "")
            .includes(needle),
        ),
      )
      .sort(
        (left, right) =>
          left.name.localeCompare(right.name, "en") || left.id.localeCompare(right.id, "en"),
      )
  }

  let results = $derived(search(maps, query))
  let options = $derived<SearchOption[]>(
    results.map((map) => ({
      value: map.name,
      label: map.name,
      description: map.mapSection ?? "No map section",
    })),
  )
</script>

<section
  class="border border-cartographer-border bg-cartographer-panel p-3 shadow-cartographer-panel"
  aria-label="Map search"
>
  <h2
    class="mb-3 border-b border-cartographer-border pb-2 text-sm font-semibold text-cartographer-signal"
  >
    Find a map
  </h2>
  <SearchCombobox
    bind:query
    emptyText="No source maps or map sections match."
    label="Name or map section"
    {options}
    placeholder="e.g. Route29 or MAPSEC..."
    onSelect={(name) => onSelectMap?.(name, true)}
  />
</section>
