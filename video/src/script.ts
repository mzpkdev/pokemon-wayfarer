// The storyboard's words: what the narrator says in each scene and what the
// game's text box prints under the map. This file imports nothing, so
// `scripts/voice.mjs` can read it under Node to make the voice-over.
//
// The WAYFARER narrates: a traveler who has walked all seven lands, met at
// a Pokémon Center counter, a little weathered and glad to see a familiar
// face. Nobody from the games; a character the project owns. The text box
// prints their words as ordinary NPC dialogue, so the cut without a voice
// still reads as them talking.

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
    id: "pallet",
    narration:
      "Pallet Town. You know every tile of it. One Pokémon in your pocket, a whole world past the trees... and a road that only ever went one way. Not anymore.",
    caption: ["Pallet Town. You know every", "tile of it."],
    seconds: 10,
  },
  {
    id: "starts",
    narration:
      "Kanto. Johto. Hoenn. Start in any one of them. And from there? Anywhere. No badge to earn first, no guard with a cough blocking the road. Just a world, and you in it.",
    caption: ["KANTO, JOHTO or HOENN.", "Then anywhere you like."],
    seconds: 10,
  },
  {
    id: "anne",
    narration:
      "See the Anne? Folks used to say she sailed off and never came back. She comes back. Every evening, half past eight, out of Vermilion for Alola by way of the islands. Be on the pier, or don't be. She won't wait.",
    caption: ["The ANNE? Half past eight,", "for ALOLA. She won't wait."],
    seconds: 12,
  },
  {
    id: "lands",
    narration:
      "Kanto and Johto are one land, always were. Past them, Hoenn. Sinnoh. The Sevii Islands. Alola. Seven lands, one sea, and each one keeps its own Pokémon, right where they've always been.",
    caption: ["Seven lands, one sea, each", "with its own POKéMON."],
    seconds: 13,
  },
  {
    id: "leaders",
    narration:
      "The famous names? They've got lives. Falkner trains at Sprout Tower after lunch. Erika's shopping in Celadon. Catch them on the road, and you might walk away with their number.",
    caption: ["FALKNER? SPROUT TOWER,", "after lunch. Try there."],
    seconds: 11,
  },
  {
    id: "night",
    narration:
      "Evening comes, the lights go on in Goldenrod, and everybody heads home. Clock only runs while you're out here, mind.",
    caption: ["Gets dark out here.", "Everybody heads home."],
    seconds: 8,
  },
  {
    id: "daily",
    narration:
      "Come back in a week, it won't be the same route. New faces, new finds, and somebody you beat once ringing you up for another go.",
    caption: ["Come back in a week. Won't", "be the same route, I swear."],
    seconds: 10,
  },
  {
    id: "gyms",
    narration:
      "Eight badges. Any eight you like. Then your phone rings, and it's the League asking.",
    caption: ["Eight badges, any eight,", "and the LEAGUE rings you."],
    seconds: 10,
  },
  {
    id: "open",
    narration:
      "Pokémon Wayfarer. Free, made in the open. Road opens the twentieth of January. I'll see you out there.",
    seconds: 9,
  },
]
