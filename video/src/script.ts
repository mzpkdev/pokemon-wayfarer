// The storyboard's words: what the narrator says in each scene, which the
// game's text box under the map prints as they say it. This file imports nothing, so
// `scripts/voice.mjs` can read it under Node to make the voice-over.
//
// The WAYFARER narrates: a traveler who has walked all seven lands, met at
// a Pokémon Center counter, a little weathered and glad to see a familiar
// face. Nobody from the games; a character the project owns. The text box
// prints their words as ordinary NPC dialogue, so the cut without a voice
// still reads as them talking.

export type Scene = {
  id: string
  /** What the voice says and the text box prints. Empty for a scene with music only. */
  narration: string
  /** The least the scene lasts, in seconds; it runs longer when the words need it. */
  seconds: number
}

/** Names the text box prints in capitals, as the game does; the longer of two overlapping names first. */
export const BOX_CAPS = [
  "Pokémon Wayfarer",
  "Pokémon",
  "Pallet Town",
  "Kanto",
  "Johto",
  "Hoenn",
  "Sinnoh",
  "Sevii Islands",
  "Alola",
  "Anne",
  "Vermilion",
  "Falkner",
  "Sprout Tower",
  "Erika",
  "Celadon",
  "Goldenrod",
  "League",
]

export const SCENES: Scene[] = [
  {
    id: "pallet",
    narration: "Pallet Town. Every journey used to start right here.",
    seconds: 7,
  },
  {
    id: "newbark",
    narration: "Then it started here.",
    seconds: 4,
  },
  {
    id: "littleroot",
    narration: "And here. Now? You choose where.",
    seconds: 5,
  },
  {
    id: "anne",
    narration:
      "See the Anne? Folks used to say she sailed off and never came back. She comes back. Every evening, half past eight, out of Vermilion. Where to? Somewhere you've never been. Be on the pier, or don't be. She won't wait.",
    seconds: 12,
  },
  {
    id: "lands",
    narration:
      "Kanto and Johto are one land, always were. And past them... Hoenn. Sinnoh. The Sevii Islands. Alola. Seven lands, one sea, and every one of them keeps its own Pokémon, right where they've always been.",
    seconds: 13,
  },
  {
    id: "leaders",
    narration:
      "The famous names? They've got lives. Falkner trains at Sprout Tower after lunch. Erika's shopping in Celadon. Catch them on the road, and you might walk away with their number.",
    seconds: 11,
  },
  {
    id: "night",
    narration:
      "Evening comes, the lights go on in Goldenrod, and everybody heads home. Clock only runs while you're out here, mind.",
    seconds: 8,
  },
  {
    id: "daily",
    narration:
      "Come back in a week, it won't be the same route. New faces, new finds, and somebody you beat once ringing you up for another go.",
    seconds: 10,
  },
  {
    id: "gyms",
    narration:
      "Eight badges. Any eight you like. Then your phone rings, and it's the League asking.",
    seconds: 10,
  },
  {
    id: "open",
    narration:
      "Pokémon Wayfarer. Free, made in the open. Road opens the twentieth of January. I'll see you out there.",
    seconds: 9,
  },
]
