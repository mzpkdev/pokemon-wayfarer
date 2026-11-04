// The storyboard's words: what the narrator says in each scene and what the
// game's text box prints under the map. This file imports nothing, so
// `scripts/voice.mjs` can read it under Node to make the voice-over.
//
// The voice speaks to the player, in the present tense, and leads with what
// they remember: the walk out of Pallet Town, the S.S. Anne that sailed away.
// Then it widens, land by land, and fills the world with the people and the
// days that make it live. The text box is the world's own voice, the kind of
// thing people say in a Pokémon Center.

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
      "Remember the first time you walked out of Pallet Town with one Pokémon in your pocket, and the whole world was out there?",
    seconds: 8,
  },
  {
    id: "starts",
    narration:
      "Or New Bark. Or Littleroot. Start wherever your story started. This time, the whole world really is out there.",
    caption: ["The road's open, y'know.", "Always was, if you ask me."],
    seconds: 8,
  },
  {
    id: "anne",
    narration:
      "Remember the S.S. Anne? Ever wonder where she went? She sails every day, on a real loop, and she doesn't wait. Stay aboard long enough, and she'll take you to Alola.",
    caption: ["The ANNE sails at half past", "eight. ALOLA, by the islands."],
    seconds: 12,
  },
  {
    id: "lands",
    narration:
      "Kanto and Johto, one land. Then Hoenn, Sinnoh, the Sevii Islands, Alola. Seven lands on one sea, and every one keeps its own Pokémon, living where they always have.",
    caption: ["KALOS Pokémon? Only in the", "SAFARI ZONES, they say."],
    seconds: 13,
  },
  {
    id: "leaders",
    narration:
      "And the people you remember are out here too. Falkner trains at Sprout Tower after lunch. Erika's shopping in Celadon. Catch them on the road, and they might give you their number.",
    caption: ["FALKNER? Not at the GYM.", "Try SPROUT TOWER after lunch."],
    seconds: 11,
  },
  {
    id: "night",
    narration: "Then evening comes, the lights go on in Goldenrod, and everybody heads home.",
    caption: ["Gets dark around here.", "Watch your step on the routes."],
    seconds: 8,
  },
  {
    id: "daily",
    narration:
      "Come back a week later, and it isn't the same route. New faces, new finds, and an old rival on the phone, wanting a rematch.",
    caption: ["Those trainers weren't on", "ROUTE 15 yesterday, I swear!"],
    seconds: 10,
  },
  {
    id: "gyms",
    narration: "Earn eight badges, any eight you like, and the phone rings. The League wants you.",
    caption: ["A LEAGUE calling you?", "Well, well."],
    seconds: 10,
  },
  {
    id: "open",
    narration: "Pokémon Wayfarer. Free, made in the open. The road opens January the twentieth.",
    seconds: 8,
  },
]
