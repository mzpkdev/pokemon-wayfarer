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
//
// A scene is one picture's worth of words: each punchline has its own scene,
// so the picture can act it out (`cast` in `timeline.ts`) and cut on it.

export type Scene = {
  id: string
  /**
   * What the voice says and the text box prints. Empty for a scene with music
   * only. A line break (\n) turns the box's page there; the voice reads on.
   * A beat (|) holds a moment before the words after it, as before a
   * punchline: the voice pauses and the box waits to print on (|| holds longer).
   */
  narration: string
  /** The least the scene lasts, in seconds; it runs longer when the words need it. */
  seconds: number
  /** False keeps the text box away, as over the title card, which says it itself; the voice still speaks. */
  boxed?: boolean
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
  "Safari Zone",
  "Galar",
  "Wooloo",
  "Potions",
  "Alola",
  "S.S. Anne",
  "Vermilion",
  "League",
  "Gym Leaders",
  "Elite Four",
  "Brock",
  "Bug Catcher",
  "Route 30",
  "Clair",
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
    narration: "Well, look who's back. | Last time, you set out from Pallet Town. | Everyone did.",
    seconds: 7,
  },
  {
    id: "newbark",
    narration: "This time? | Maybe New Bark Town.",
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
      "Remember the S.S. Anne? She sailed out of Vermilion, and you never saw her again. | Ever wonder where she was going?",
    seconds: 12,
  },
  {
    id: "alola",
    narration: "Alola. | And now you can sail with her.",
    seconds: 6.5,
  },
  {
    id: "wait",
    narration: "Keeps her own hours, mind. | She won't wait.",
    seconds: 6,
  },
  {
    id: "lands",
    narration:
      "And Kanto was never the whole map. | Seven lands. Walk every one of them. Each keeps its own Pokémon.",
    seconds: 8,
  },
  {
    id: "galar",
    narration: "Folk on the Sevii Islands swear they're Galar now. | Ask the Wooloo.",
    seconds: 7,
  },
  {
    id: "kalos",
    narration:
      "Never been to Kalos myself. Too far. | The Safari Zones brought a bit of it home, though.",
    seconds: 6,
  },
  {
    id: "leaders",
    narration:
      "Ever wonder what Gym Leaders do when nobody's at the door?\nBrock's helping out at the Day-Care in Johto. | Trade with him sometime.",
    seconds: 13.5,
  },
  {
    id: "surge",
    narration: "Lt. Surge? | Game Corner. | Again.",
    seconds: 3.5,
  },
  {
    id: "wallace",
    narration: "Wallace is fishing Route 119.\n| Has been for days.",
    seconds: 5,
  },
  {
    id: "night",
    narration: "Stray off the road after dark?\n| Pack more Potions than you think.",
    seconds: 5,
  },
  {
    id: "don",
    narration: "That Bug Catcher you beat on Route 30?",
    seconds: 3.5,
  },
  {
    id: "donagain",
    narration: "He's moved on. You'll run into him again.\n| He hasn't forgotten you.",
    seconds: 5,
  },
  {
    id: "clair",
    narration: "Twenty-four Gyms. Fancy Clair's first?\nGo on. | It'll be a fair fight.",
    seconds: 7,
  },
  {
    id: "fame",
    narration:
      "Word gets around. Soon the names you grew up with know yours.\nThen the phone rings. | It's the League.",
    seconds: 8,
  },
  {
    id: "golem",
    narration:
      "And who's in the Elite Four this year? | Brock. | With a Golem.\n| Wonder where he got that.",
    seconds: 8,
  },
  {
    id: "open",
    narration:
      "Pokémon Wayfarer. Free, out January twentieth. | Good to have you back. | See you out there.",
    seconds: 9,
    boxed: false,
  },
]
