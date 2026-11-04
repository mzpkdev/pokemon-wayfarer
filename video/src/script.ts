// The storyboard's words: what the narrator says in each scene, which the
// game's text box under the map prints as they say it. This file imports nothing, so
// `scripts/voice.mjs` can read it under Node to make the voice-over.
//
// The WAYFARER narrates: a traveler who has walked all seven lands, glad
// to see an old face back. The script unfolds the way a player's hopes do:
// start anywhere, then the question every FRLG player had (where was the
// S.S. Anne going?), and its answer opens the whole world. Nobody from the games; a character the project owns. The text box
// prints their words as ordinary NPC dialogue, so the cut without a voice
// still reads as them talking.

export type Scene = {
  id: string
  /**
   * What the voice says and the text box prints. Empty for a scene with music
   * only. A line break (\n) turns the box's page there; the voice reads on.
   */
  narration: string
  /** The least the scene lasts, in seconds; it runs longer when the words need it. */
  seconds: number
}

/** Names the text box prints in capitals, as the game does; the longer of two overlapping names first. */
export const BOX_CAPS = [
  "Pokémon Wayfarer",
  "Pokémon",
  "Pallet Town",
  "New Bark Town",
  "Littleroot",
  "Kanto",
  "Johto",
  "Hoenn",
  "Sinnoh",
  "Sevii Islands",
  "Alola",
  "S.S. Anne",
  "Vermilion",
  "Falkner",
  "Sprout Tower",
  "Erika",
  "Celadon",
  "Goldenrod",
  "League",
  "Gym Leaders",
  "Elite Four",
  "Gyms",
]

export const SCENES: Scene[] = [
  {
    id: "pallet",
    narration: "Well, look who's back. Last time, you set out from Pallet Town. Everyone did.",
    seconds: 7,
  },
  {
    id: "newbark",
    narration: "This time? Maybe New Bark Town.",
    seconds: 4,
  },
  {
    id: "littleroot",
    narration: "Or Littleroot. Start wherever you like.",
    seconds: 5,
  },
  {
    id: "anne",
    narration:
      "Remember the S.S. Anne? She sailed out of Vermilion, and you never saw her again. Ever wonder where she was going?",
    seconds: 12,
  },
  {
    id: "lands",
    narration:
      "Alola. And now you can sail with her.\nAnd Kanto was never the whole map. Seven lands, one journey, and you can walk to every one. Each keeps its own Pokémon. Galar's, even. Try the Sevii Islands.",
    seconds: 13,
  },
  {
    id: "leaders",
    narration:
      "See those? Gym Leaders. Elite Four. Out living their lives.\nFalkner's up Sprout Tower this morning.\nErika's off to the pond in Celadon. Get friendly, and they might give you their number.",
    seconds: 11,
  },
  {
    id: "night",
    narration: "Come evening, the lights go on in Goldenrod, and folks head home.",
    seconds: 8,
  },
  {
    id: "daily",
    narration:
      "Walk the same road tomorrow, and it won't be the same road. New faces, new finds, and someone you beat, calling for a rematch.",
    seconds: 10,
  },
  {
    id: "gyms",
    narration:
      "Twenty-four Gyms. Win any eight, in any order you like. Then the phone rings. It's the League.",
    seconds: 10,
  },
  {
    id: "open",
    narration:
      "Pokémon Wayfarer. Free, out January twentieth. Good to have you back. See you out there.",
    seconds: 9,
  },
]
