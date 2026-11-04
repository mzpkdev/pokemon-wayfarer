// The storyboard's words: what the narrator says in each scene and what the
// game's text box prints under the map. This file imports nothing, so
// `scripts/voice.mjs` can read it under Node to make the voice-over.
//
// PROFESSOR OAK narrates, as he opens every one of these games, speaking to
// a trainer who has been away: warm, a little grand, and glad to have them
// back. The text box prints his words the way the game would, so the cut
// without a voice still reads as Oak talking.

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
      "Hello again. Welcome back to the world of Pokémon. It is a good deal bigger than you remember.",
    caption: ["Hello again! Welcome back", "to the world of POKéMON."],
    seconds: 8,
  },
  {
    id: "starts",
    narration:
      "Pallet Town, New Bark, Littleroot. Begin wherever your own story began. The roads out are open, all of them.",
    caption: ["Begin wherever your own", "story began."],
    seconds: 8,
  },
  {
    id: "anne",
    narration:
      "Ah, the S.S. Anne. You always did wonder where she went. Every evening at half past eight she sails for Alola, by way of the islands, and I'm afraid she won't wait for you.",
    caption: ["She sails for ALOLA at half", "past eight. She won't wait."],
    seconds: 12,
  },
  {
    id: "lands",
    narration:
      "Kanto and Johto, one land. Beyond them Hoenn, Sinnoh, the Sevii Islands, Alola. Seven lands on one sea, and each with its own Pokémon, living where they always have.",
    caption: ["Seven lands, one sea, and", "each with its own POKéMON."],
    seconds: 13,
  },
  {
    id: "leaders",
    narration:
      "And the people you remember have lives of their own. Falkner trains at Sprout Tower after lunch. Erika is out shopping in Celadon. Meet them on the road, and they may well give you their number.",
    caption: ["FALKNER trains at SPROUT", "TOWER after lunch, I hear."],
    seconds: 11,
  },
  {
    id: "night",
    narration:
      "Evening comes, the lights go on in Goldenrod, and everybody heads home. Time passes only while you play, mind.",
    caption: ["Evening comes, and everyone", "heads home."],
    seconds: 8,
  },
  {
    id: "daily",
    narration:
      "Come back in a week, and it isn't the same route. New faces, new finds, and an old acquaintance on the phone, wanting a rematch.",
    caption: ["Come back in a week. It", "won't be the same route."],
    seconds: 10,
  },
  {
    id: "gyms",
    narration:
      "Earn eight badges, any eight you please, and the telephone rings. The League wants a word with you.",
    caption: ["Eight badges, any eight,", "and the LEAGUE will call."],
    seconds: 10,
  },
  {
    id: "open",
    narration:
      "Pokémon Wayfarer. Free, and made in the open. Now then. Are you ready? Your very own adventure is about to unfold. Once more, on the twentieth of January.",
    seconds: 9,
  },
]
