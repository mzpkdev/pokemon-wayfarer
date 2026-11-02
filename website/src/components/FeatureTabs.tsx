import { Tabs } from "@ark-ui/react/tabs"

type Feature = {
  id: string
  label: string
  body: string
}

const features: Feature[] = [
  {
    id: "regions",
    label: "Regions",
    body: "Travel between Hoenn, Kanto, Johto and the Sevii Islands on one save.",
  },
  {
    id: "clock",
    label: "Clock",
    body: "A two-hour day with Wayfarer's own times of day shapes who you meet and where.",
  },
  {
    id: "ferries",
    label: "Ferries",
    body: "The S.S. Aqua, Anne and Tidal sail scheduled loops between ports.",
  },
]

export const FeatureTabs = () => (
  <Tabs.Root defaultValue={features[0]?.id} className="w-full max-w-xl">
    <Tabs.List className="flex gap-1 border-b border-white/10">
      {features.map((feature) => (
        <Tabs.Trigger
          key={feature.id}
          value={feature.id}
          className="-mb-px border-b-2 border-transparent px-4 py-2 text-sm text-wayfarer-50/70 transition-colors hover:text-wayfarer-50 data-selected:border-wayfarer-500 data-selected:text-wayfarer-50"
        >
          {feature.label}
        </Tabs.Trigger>
      ))}
    </Tabs.List>
    {features.map((feature) => (
      <Tabs.Content key={feature.id} value={feature.id} className="py-4 text-wayfarer-50/80">
        {feature.body}
      </Tabs.Content>
    ))}
  </Tabs.Root>
)
