// The storyboard's words: what the narrator says in each scene and what the
// game's text box prints under the map. This file imports nothing, so
// `scripts/voice.mjs` can read it under Node to make the voice-over.

export type Scene = {
  id: string
  /** What the voice says. Empty for a scene with music only. */
  narration: string
  /** What the dialog box prints, two lines at most; none for a title card. */
  caption?: string[]
  /** The least the scene lasts, in seconds; it runs longer when the voice needs it. */
  seconds: number
}

export const SCENES: Scene[] = [
  {
    id: "hook",
    narration: "Pokémon Wayfarer is one Pokémon world, and you cross it however you like.",
    seconds: 6,
  },
  {
    id: "lands",
    narration:
      "Kanto, Johto and Hoenn, with Sinnoh, Sinjoh, the Sevii Islands and Alola. Seven lands on one sea, in one save.",
    caption: ["Seven lands, one sea,", "one save."],
    seconds: 9,
  },
  {
    id: "start",
    narration:
      "Begin in Pallet Town, New Bark Town or Littleroot. Your party, your Bag and your Pokédex go everywhere you do.",
    caption: ["Start in KANTO, JOHTO", "or HOENN."],
    seconds: 12,
  },
  {
    id: "gyms",
    narration:
      "Twenty-four Gyms, in any order. Earn any eight badges, and a League phones you. Win Indigo and the Hoenn League, and the Masters opens on Seven Island.",
    caption: ["24 Gyms, any order."],
    seconds: 11,
  },
  {
    id: "leaders",
    narration:
      "Gym Leaders and the Elite Four keep their own routines around the world. Make friends, and they'll give you their number.",
    caption: ["Gym Leaders on the road."],
    seconds: 8,
  },
  {
    id: "day",
    narration:
      "A Wayfarer day is two hours of play, morning to night. Time runs only while you play.",
    caption: ["A two-hour day."],
    seconds: 9,
  },
  {
    id: "ships",
    narration:
      "The S.S. Aqua, Anne and Tidal sail real loops, and the Anne goes on to Alola. When your port is called, walk down the gangway. Stay aboard, and you sail on.",
    caption: ["Ships you actually ride."],
    seconds: 11,
  },
  {
    id: "daily",
    narration:
      "Routes change from day to day. Trainers and items move, and a trainer you've beaten may call you for a rematch.",
    caption: ["Routes that change daily."],
    seconds: 8,
  },
  {
    id: "open",
    narration:
      "Free, and made in the open on GitHub. Pokémon Wayfarer. January the twentieth, twenty twenty-seven.",
    seconds: 8,
  },
]
