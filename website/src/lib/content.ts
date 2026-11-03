/** The page's copy. Only shipped features are described. */

export const REPOSITORY_URL = "https://github.com/mzpkdev/pokemon-wayfarer"

/** The patcher the FAQ points to for applying the .ups patch in a browser. */
export const PATCHER_URL = "https://www.marcrobledo.com/RomPatcher.js/"

/**
 * Your level cap and wild levels by badges earned, from the wild level
 * scaling spec's v0 table (.product/specs/wild-level-scaling.md). Rows are
 * Wayfarer's reaches by the names players read them by: Outlands, Wilds, Road.
 */
export const LEVELS = {
  badges: [0, 4, 8, 16, 24],
  /** The column phones leave out, so the table fits at whole GBA pixels. */
  hiddenOnPhones: 1,
  rows: [
    { label: "LV. CAP", levels: [15, 28, 50, 75, 100] },
    { label: "FAR AWAY", levels: [13, 28, 50, 74, 98] },
    { label: "OFF ROAD", levels: [9, 25, 44, 65, 85] },
    { label: "ON ROAD", levels: [5, 20, 38, 56, 74] },
  ],
}

export type Highlight = {
  title: string
  body: string
  /** A key item icon from public/game/items. */
  icon: string
}

/**
 * The world's size, as the world tour's last card counts it (see
 * WorldSection.astro), plus the Gyms.
 */
export const NUMBERS: { value: string; label: string }[] = [
  { value: "62", label: "towns and cities" },
  { value: "114", label: "routes" },
  { value: "24", label: "Gyms" },
  { value: "7", label: "ports" },
]

/** What the world tour showed, in short, for anyone who scrolled past it. */
export const HIGHLIGHTS: Highlight[] = [
  {
    title: "Seven lands, one save",
    body: "Start in Kanto, Johto or Hoenn. Your party, Bag and Pokédex go everywhere you do.",
    icon: "town-map",
  },
  {
    title: "Every land, its own Pokémon",
    body: "Kanto and Johto are home to Gens I and II, Hoenn to III and V, Alola to VII and Sevii to Galar's VIII. Kalos is split across the three Safari Zones.",
    icon: "safari-ball",
  },
  {
    title: "24 Gyms, any order",
    body: "Earn any 8 badges and a league phones. Win Indigo and the Hoenn League in either order, and the Masters opens on Seven Island.",
    icon: "master-ball",
  },
  {
    title: "Gym Leaders on the road",
    body: "Gym Leaders and the Elite Four keep their own routines around the world. Make friends and they'll give you their number.",
    icon: "soothe-bell",
  },
  {
    title: "Ships you actually ride",
    body: "The S.S. Aqua, Anne and Tidal sail real loops, and the Anne goes on to Alola. When your port is called, walk down the gangway. Stay aboard and you sail on to the next.",
    icon: "ss-ticket",
  },
  {
    title: "Routes that change daily",
    body: "Trainers and items move from day to day, ships' passengers too. Beat a trainer and they may give you their number for a rematch.",
    icon: "vs-seeker",
  },
  {
    title: "A two-hour day",
    body: "Morning, Day, Evening and Night pass in two hours of play. Time only runs while you play.",
    icon: "sun-stone",
  },
  {
    title: "Free, made in the open",
    body: "A fan game built on Pokémon Heart & Soul, itself built on pokeemerald-expansion, with every change on GitHub.",
    icon: "rare-candy",
  },
]
