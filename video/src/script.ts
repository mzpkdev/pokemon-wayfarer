// The storyboard's words: what the narrator says in each scene and what the
// game's text box prints under the map. This file imports nothing, so
// `scripts/voice.mjs` can read it under Node to make the voice-over.
//
// The voice speaks to the player, in the present tense, through one
// trainer's first week on the road: leave home, see the world, catch a ship,
// a day passes, meet a Gym Leader out and about, come back to a changed
// route, the League calls. The text box is the world's own voice, the kind
// of thing people say in a Pokémon Center.

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
    narration:
      "Remember the first time you walked out of town with one Pokémon in your pocket, and the whole world was out there? This time, it really is.",
    seconds: 9,
  },
  {
    id: "start",
    narration:
      "Begin in Kanto, Johto or Hoenn. Walk to the edge of town, and keep going. Nobody steps in your way.",
    caption: ["The road's open, y'know.", "Always was, if you ask me."],
    seconds: 12,
  },
  {
    id: "lands",
    narration:
      "Seven lands on one sea, and every one keeps its own Pokémon, living where they always have.",
    caption: ["KALOS Pokémon? Only in the", "SAFARI ZONES, they say."],
    seconds: 9,
  },
  {
    id: "ships",
    narration: "Miss the boat, and it sails without you. Stay aboard, and you wake up in Alola.",
    caption: ["The AQUA's in port till three.", "Then she's off to VERMILION."],
    seconds: 10,
  },
  {
    id: "day",
    narration:
      "Morning turns to evening turns to night while you play. The lights come on in Goldenrod, and somewhere, a Gym Leader heads home.",
    caption: ["Gets dark around here.", "Watch your step on the routes."],
    seconds: 11,
  },
  {
    id: "leaders",
    narration:
      "Because they have lives. Falkner trains at Sprout Tower after lunch. Erika's shopping in Celadon. Catch them on the road, and they might give you their number.",
    caption: ["FALKNER? Not at the GYM.", "Try SPROUT TOWER after lunch."],
    seconds: 11,
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
