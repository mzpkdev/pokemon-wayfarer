<script lang="ts">
  import { Combobox, createListCollection } from "@ark-ui/svelte/combobox"

  export type SearchOption = {
    description: string
    label: string
    value: string
  }

  type Props = {
    emptyText: string
    label: string
    options: readonly SearchOption[]
    placeholder: string
    query?: string
    onSelect?: (value: string) => void
  }

  let { emptyText, label, options, placeholder, query = $bindable(""), onSelect }: Props = $props()

  let collection = $derived(createListCollection({ items: options }))

  const handleValueChange = (details: { items: SearchOption[] }): void => {
    const selected = details.items[0]
    if (selected) onSelect?.(selected.value)
  }
</script>

<Combobox.Root
  bind:inputValue={query}
  {collection}
  closeOnSelect
  inputBehavior="autohighlight"
  openOnClick
  selectionBehavior="preserve"
  onValueChange={handleValueChange}
>
  <Combobox.Label class="mb-2 block text-xs font-medium text-cartographer-muted"
    >{label}</Combobox.Label
  >
  <Combobox.Control>
    <Combobox.Input
      class="w-full border border-cartographer-border bg-cartographer-field px-2.5 py-2 text-sm text-cartographer-ink placeholder:text-cartographer-muted-soft focus-visible:border-cartographer-signal focus-visible:outline-2 focus-visible:outline-offset-1 focus-visible:outline-cartographer-signal"
      {placeholder}
      type="search"
    />
  </Combobox.Control>
  {#if query.trim()}
    <Combobox.Positioner class="z-10 w-full">
      <Combobox.Content
        class="mt-1 max-h-72 overflow-auto border border-cartographer-border bg-cartographer-panel p-1.5 shadow-cartographer-float"
      >
        {#if options.length > 0}
          <Combobox.List class="grid list-none gap-1 p-0">
            {#each options as option (option.value)}
              <Combobox.Item
                class="flex cursor-pointer items-center justify-between gap-3 px-3 py-2 text-left text-sm outline-none transition-colors hover:bg-cartographer-signal/10 data-[highlighted]:bg-cartographer-signal/10 data-[state=checked]:bg-cartographer-signal/15"
                item={option}
              >
                <Combobox.ItemText>{option.label}</Combobox.ItemText>
                <small
                  class="text-right break-words font-cartographer-mono text-[0.68rem] text-cartographer-muted"
                  >{option.description}</small
                >
              </Combobox.Item>
            {/each}
          </Combobox.List>
        {:else}
          <Combobox.Empty class="block px-3 py-2 text-sm text-cartographer-muted"
            >{emptyText}</Combobox.Empty
          >
        {/if}
      </Combobox.Content>
    </Combobox.Positioner>
  {/if}
</Combobox.Root>
