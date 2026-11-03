/** The page's copy. Only shipped features are described. */

export const REPOSITORY_URL = "https://github.com/mzpkdev/pokemon-wayfarer"

export type Feature = {
  title: string
  body: string
  /** A key item icon from public/game/items. */
  icon: string
}

export const FEATURES: Feature[] = [
  {
    title: "One world, one save",
    body: "Kanto, Johto and Hoenn share a single world. Your party, Bag and Pokédex go everywhere you do.",
    icon: "town-map",
  },
  {
    title: "Ferries that keep a schedule",
    body: "The S.S. Aqua, Anne and Tidal sail four-port loops and change ships at 8:00 and 20:00. The Anne sails on to Alola.",
    icon: "ss-ticket",
  },
  {
    title: "A clock of its own",
    body: "A day lasts two hours of play, from Morning through Day and Evening to Night. Time only runs while you play.",
    icon: "sun-stone",
  },
  {
    title: "Routes that change daily",
    body: "Trainers and items move around from day to day. Beat a trainer and they may give you their number for a rematch.",
    icon: "vs-seeker",
  },
  {
    title: "Trainer Rating",
    body: "Every badge, from any region, raises your rating. Wild Pokémon, trainers and Poké Mart shelves grow with it.",
    icon: "rare-candy",
  },
  {
    title: "A league circuit",
    body: "Any 8 badges qualify you, with one from the league's own side. Indigo and Hoenn phone with invitations and can be won in either order; the Sevii Masters opens once you've won both.",
    icon: "master-ball",
  },
  {
    title: "Gym Leaders on the road",
    body: "Gym Leaders and the Elite Four keep their own routines around the world. Make friends and they'll give you their number.",
    icon: "soothe-bell",
  },
  {
    title: "The Sevii Islands",
    body: "All seven islands are open to explore, with their own stories, trainers and the Trainer Tower.",
    icon: "tri-pass",
  },
]

export type Origin = {
  region: string
  town: string
  body: string
}

export const ORIGINS: Origin[] = [
  {
    region: "Kanto",
    town: "Pallet Town",
    body: "Set out with a first partner from Professor Oak.",
  },
  {
    region: "Johto",
    town: "New Bark Town",
    body: "Wake up next door to Professor Elm's lab.",
  },
  {
    region: "Hoenn",
    town: "Littleroot Town",
    body: "Choose Treecko, Torchic or Mudkip.",
  },
]

export type Stat = { value: string; label: string }

export const STATS: Stat[] = [
  { value: "3", label: "starting regions" },
  { value: "3", label: "ships at sea" },
  { value: "7", label: "ports of call" },
  { value: "24", label: "Gym Badges" },
  { value: "2 h", label: "in a day" },
]
