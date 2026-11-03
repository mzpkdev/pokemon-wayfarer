/** The page's copy. Only shipped features are described. */

export const REPOSITORY_URL = "https://github.com/mzpkdev/pokemon-wayfarer"

/** The patcher the FAQ points to for applying the .ups patch in a browser. */
export const PATCHER_URL = "https://www.marcrobledo.com/RomPatcher.js/"

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
    body: "A fan game built on Pokémon Heart & Soul, itself built on pokeemerald-expansion, with every change on GitHub.",
    icon: "rare-candy",
  },
]
