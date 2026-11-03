/** The page's copy. Only shipped features are described. */

export const REPOSITORY_URL = "https://github.com/mzpkdev/pokemon-wayfarer"

/** The patcher the FAQ points to for applying the .ups patch in a browser. */
export const PATCHER_URL = "https://www.marcrobledo.com/RomPatcher.js/"

export type Concern = {
  /** What a player asks on seeing an open world, printed in a text window. */
  question: string
  answer: string
}

/**
 * The worries an open world raises, which the map can't answer. Wayfarer is
 * open but not frictionless: a blocked road always has a way around it.
 */
export const CONCERNS: Concern[] = [
  {
    question: "Can I really go anywhere?",
    answer:
      "Yes, from the first step. Wayfarer doesn't take every obstacle away: a road can still be blocked, a sea too wide or a door shut. But a blocked road is never the only one. There's always another way around.",
  },
  {
    question: "Won't I outlevel everything?",
    answer:
      "Every badge, from any region, raises your level cap, and wild Pokémon, trainers and Poké Mart shelves grow with it. Early on, the grass off the road is dangerous. Later, routes become home ground and the Gym Leaders and leagues bring the real fights.",
  },
  {
    question: "Do I need badges for HMs?",
    answer:
      "No. Wild Pokémon can already know field moves when you catch them, so a Pokémon that knows Surf lets you surf straight away. Once you find an HM, it works whenever someone in your party could learn it, without teaching it to them.",
  },
  {
    question: "What happens to the story?",
    answer:
      "It's still there. The original games' stories are split into standalone beats, each of which makes sense whatever order you play them in.",
  },
]

export type Highlight = {
  title: string
  body: string
  /** A key item icon from public/game/items. */
  icon: string
}

/** What the world tour showed, in short, for anyone who scrolled past it. */
export const HIGHLIGHTS: Highlight[] = [
  {
    title: "Seven lands, one save",
    body: "Start in Kanto, Johto or Hoenn. Your party, Bag and Pokédex go everywhere you do.",
    icon: "town-map",
  },
  {
    title: "24 Gyms, any order",
    body: "Any 8 badges qualify you. Win Indigo and the Hoenn League in either order, and the Sevii Masters opens.",
    icon: "master-ball",
  },
  {
    title: "Routes that change daily",
    body: "Trainers and items move from day to day. Beat a trainer and they may give you their number for a rematch.",
    icon: "vs-seeker",
  },
  {
    title: "Ferries that keep a schedule",
    body: "The S.S. Aqua, Anne and Tidal sail four-port loops and change ships at 8:00 and 20:00. The Anne sails on to Alola.",
    icon: "ss-ticket",
  },
  {
    title: "A two-hour day",
    body: "Morning, Day, Evening and Night pass in two hours of play. Time only runs while you play.",
    icon: "sun-stone",
  },
  {
    title: "Gym Leaders on the road",
    body: "Gym Leaders and the Elite Four keep their own routines. Make friends and they'll give you their number.",
    icon: "soothe-bell",
  },
  {
    title: "The Sevii Islands",
    body: "All seven islands are open, with their own stories, trainers and the Trainer Tower.",
    icon: "tri-pass",
  },
  {
    title: "Free, made in the open",
    body: "A fan game built on pokeemerald-expansion, with every change on GitHub.",
    icon: "rare-candy",
  },
]
