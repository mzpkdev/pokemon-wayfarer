import { Tabs } from "@ark-ui/react/tabs"
import { useState } from "react"

import { DialogBox } from "~/components/DialogBox"

type Origin = {
  id: string
  region: string
  town: string
  lines: string[]
}

const origins: Origin[] = [
  {
    id: "johto",
    region: "JOHTO",
    town: "NEW BARK TOWN",
    lines: ["Wake up in NEW BARK TOWN.", "PROF. ELM is waiting in his lab."],
  },
  {
    id: "hoenn",
    region: "HOENN",
    town: "LITTLEROOT TOWN",
    lines: ["Start in LITTLEROOT TOWN and choose", "TREECKO, TORCHIC or MUDKIP."],
  },
  {
    id: "kanto",
    region: "KANTO",
    town: "PALLET TOWN",
    lines: ["Set out from PALLET TOWN with a", "first partner from PROF. OAK."],
  },
]

/** PROF. OAK's question from the new game intro, as the game's choice menu. */
export const OriginChoice = () => {
  const [value, setValue] = useState(origins[0]!.id)

  return (
    <Tabs.Root
      value={value}
      onValueChange={(details) => setValue(details.value)}
      orientation="vertical"
      className="grid gap-[calc(4*var(--px))] md:grid-cols-[1fr_auto] md:items-end"
    >
      <DialogBox
        className="md:col-span-2"
        lines={["Where will your journey begin?", "Your POKéMON adventure starts here!"]}
      />

      <div className="min-h-[calc(52*var(--px))]">
        {origins.map((origin) => (
          <Tabs.Content key={origin.id} value={origin.id}>
            <DialogBox key={value} lines={origin.lines} />
          </Tabs.Content>
        ))}
      </div>

      <Tabs.List aria-label="Starting region" className="gba-window justify-self-end">
        {origins.map((origin) => (
          <Tabs.Trigger
            key={origin.id}
            value={origin.id}
            className="gba-text group flex w-full cursor-pointer items-center pr-[calc(8*var(--px))] text-left outline-none"
          >
            <span
              aria-hidden="true"
              className="inline-block w-[calc(8*var(--px))] opacity-0 group-data-selected:opacity-100"
            >
              ▶
            </span>
            <span className="group-focus-visible:underline group-focus-visible:decoration-[length:var(--px)] group-focus-visible:underline-offset-[calc(2*var(--px))]">
              {origin.region}
            </span>
          </Tabs.Trigger>
        ))}
      </Tabs.List>
    </Tabs.Root>
  )
}
