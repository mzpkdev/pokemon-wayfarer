import { Tabs } from "@ark-ui/react/tabs"
import { useState } from "react"

import { DialogBox } from "~/components/DialogBox"

type Feature = {
  id: string
  name: string
  icon: string
  lines: string[]
}

/** What Wayfarer offers, as key items in the BAG. Only shipped features are listed. */
const features: Feature[] = [
  {
    id: "one-save",
    name: "ONE WORLD",
    icon: "town-map",
    lines: [
      "KANTO, JOHTO and HOENN share one world.",
      "Your party, BAG and POKéDEX go everywhere you do.",
    ],
  },
  {
    id: "ferries",
    name: "FERRIES",
    icon: "ss-ticket",
    lines: [
      "The S.S. AQUA, ANNE and TIDAL sail four-port loops.",
      "Ports change ships at 8:00 and 20:00. The ANNE sails on to ALOLA.",
    ],
  },
  {
    id: "clock",
    name: "WORLD CLOCK",
    icon: "sun-stone",
    lines: [
      "A day lasts two hours of play:",
      "MORNING, DAY, EVENING and NIGHT. Time only runs while you play.",
    ],
  },
  {
    id: "routes",
    name: "LIVING ROUTES",
    icon: "vs-seeker",
    lines: [
      "Trainers and items on routes change every day.",
      "Beat a trainer and they may give you their number for a rematch.",
    ],
  },
  {
    id: "rating",
    name: "TRAINER RATING",
    icon: "rare-candy",
    lines: [
      "Every badge, from any region, raises your TRAINER RATING.",
      "Wild POKéMON, trainers and POKé MART shelves grow with it.",
    ],
  },
  {
    id: "leagues",
    name: "LEAGUES",
    icon: "master-ball",
    lines: [
      "Climb from INDIGO to the SEVII MASTERS to the HOENN LEAGUE.",
      "Once you qualify, the leagues phone you with invitations.",
    ],
  },
  {
    id: "notables",
    name: "GYM LEADERS",
    icon: "soothe-bell",
    lines: [
      "GYM LEADERS and the ELITE FOUR keep their own routines on the road.",
      "Make friends with them and they will give you their number.",
    ],
  },
  {
    id: "sevii",
    name: "SEVII ISLANDS",
    icon: "tri-pass",
    lines: [
      "Explore all seven SEVII ISLANDS,",
      "with their own stories, trainers and the TRAINER TOWER.",
    ],
  },
]

/** Wayfarer's features as the BAG's KEY ITEMS pocket: a list, and the selected item's description. */
export const FeatureBag = () => {
  const [value, setValue] = useState(features[0]!.id)

  const selected = features.find((feature) => feature.id === value) ?? features[0]!

  return (
    <Tabs.Root
      value={value}
      onValueChange={(details) => setValue(details.value)}
      orientation="vertical"
      className="grid gap-[calc(4*var(--px))] sm:grid-cols-[auto_1fr]"
    >
      <div
        className="gba-window hidden place-items-center self-stretch p-[calc(8*var(--px))] sm:grid"
        aria-hidden="true"
      >
        <img
          src={`/game/items/${selected.icon}.png`}
          alt=""
          width={24}
          height={24}
          className="pixelated size-[calc(48*var(--px))]"
        />
      </div>

      <Tabs.List aria-label="Features" className="gba-window">
        {features.map((feature) => (
          <Tabs.Trigger
            key={feature.id}
            value={feature.id}
            className="gba-text group flex w-full cursor-pointer items-center pr-[calc(8*var(--px))] text-left whitespace-nowrap outline-none"
          >
            <span
              aria-hidden="true"
              className="inline-block w-[calc(8*var(--px))] opacity-0 group-data-selected:opacity-100"
            >
              ▶
            </span>
            <span className="group-focus-visible:underline group-focus-visible:decoration-[length:var(--px)] group-focus-visible:underline-offset-[calc(2*var(--px))]">
              {feature.name}
            </span>
          </Tabs.Trigger>
        ))}
      </Tabs.List>

      <div className="min-h-[calc(84*var(--px))] sm:col-span-2 md:min-h-[calc(52*var(--px))]">
        {features.map((feature) => (
          <Tabs.Content key={feature.id} value={feature.id}>
            <DialogBox key={value} speed={1} lines={feature.lines} />
          </Tabs.Content>
        ))}
      </div>
    </Tabs.Root>
  )
}
