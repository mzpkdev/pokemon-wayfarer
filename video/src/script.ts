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
  "Kalos",
  "Safari Zones",
  "Alola",
  "S.S. Anne",
  "Vermilion",
  "Goldenrod",
  "League",
  "Gym Leaders",
  "Elite Four",
  "Brock",
  "Lt. Surge",
  "Game Corner",
  "Wallace",
  "Route 119",
  "Day-Care",
  "Golem",
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
      "Alola. And now you can sail with her.\nKeeps her own hours, mind. She won't wait.\nAnd Kanto was never the whole map. Seven lands. Walk every one of them. Each keeps its own Pokémon. Galar's on the Sevii Islands. Kalos hides in the Safari Zones.",
    seconds: 13,
  },
  {
    id: "leaders",
    narration:
      "Ever wonder what Gym Leaders do when nobody's at the door?\nBrock's off to the Day-Care in Johto. Trade with him sometime.",
    seconds: 8,
  },
  {
    id: "surge",
    narration: "Lt. Surge? Game Corner. Again.",
    seconds: 3.5,
  },
  {
    id: "wallace",
    narration: "Wallace is fishing Route 119.\nHas been for days.",
    seconds: 5,
  },
  {
    id: "night",
    narration: "Come evening, the lights go on in Goldenrod, and folks head home.",
    seconds: 6,
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
      "Twenty-four Gyms. Any eight, any order.\nWord gets around. Soon the names you grew up with know yours.\nThen the phone rings. It's the League.\nAnd who's in the Elite Four this year? Brock. With a Golem.\nWonder where he got that.",
    seconds: 10,
  },
  {
    id: "open",
    narration:
      "Pokémon Wayfarer. Free, out January twentieth. Good to have you back. See you out there.",
    seconds: 9,
  },
]
