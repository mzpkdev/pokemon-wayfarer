# Wayfarer website

The public site for Pokémon Wayfarer, built with Astro and React islands.

## Stack

- [Astro](https://astro.build) for pages and layouts, with [React](https://react.dev) for interactive islands
- [Ark UI](https://ark-ui.com) headless components, styled with [Tailwind CSS](https://tailwindcss.com) v4
- `cn()` in `src/lib/cn.ts` combines `clsx` and `tailwind-merge` for conditional classes
- [oxlint](https://oxc.rs/docs/guide/usage/linter) and [oxfmt](https://oxc.rs/docs/guide/usage/formatter) for linting and formatting
- TypeScript 7 (`tsc`) for type checking

## Commands

Run these from `website/`, or from the repository root with `pnpm --filter @wayfarer/website run <script>`.

| Script           | What it does                                        |
| ---------------- | --------------------------------------------------- |
| `pnpm dev`       | Start the dev server                                |
| `pnpm build`     | Build the static site into `dist/`                  |
| `pnpm preview`   | Serve the built site                                |
| `pnpm assets`    | Re-render the game art in `public/game/`            |
| `pnpm world-map` | Rebuild the world map's data in `public/world-map/` |
| `pnpm og`        | Re-render the link preview image, `public/og.png`   |
| `pnpm check`     | Type check, lint, and check formatting              |
| `pnpm format`    | Format files with oxfmt                             |

## Game art

Everything on the page is drawn with the game's own graphics, rendered from
`../game/graphics` by `scripts/game-assets.mjs`:

- the opening's four parallax layers and Poké Ball, which `TitleScene` composes
  on a canvas at GBA resolution and scales by whole pixels
- the title screen's logo, wordmark, PRESS START and passing sprites
- the 20 text window frames, the wood map name popup and key item icons
- a tile of Route 103's water and the Bag, for the page's sections
- `Wayfarer Text`, a web font traced from the game's normal Latin font, with
  glyph widths from `game/src/fonts.c` and characters from `game/charmap.txt`
- PROF. OAK, his oval and the POKéMON he sends out, from the new game intro

The output is committed, so building the site does not need the game tree.
Run `pnpm assets` after the source art changes. It also writes
`src/generated/game-art.json`, the trimmed sizes and offsets that place the
title sprites where the game draws them. The scene's timings and positions in
`src/lib/scene.ts` come from `game/src/intro.c` and `game/src/title_screen.c`.

The release date lives in `src/lib/release.ts`.

## The page

Below the title scene and the world map, PROF. OAK asks for your email, and
then each section (`Place.astro`) opens with the wooden map name popup and a
heading, on the same plain dark grass. Everything it says is in the game's
text windows, in the game's font:

- every word of copy sits in a text window (`gba-window gba-text`), with
  `window-title` lines in the game's blue ink; nothing is set in a web font
  except form input
- windows and sprites are drawn at 2 CSS pixels per GBA pixel (`--px: 2px`);
  headings and popups step up to 3 on wider screens
- the background stays plain, so the windows carry the page
- each section is one of the game's screens: the RIGHT NOW board,
  `WorldCard` (the Trainer Card's dotted rows and badge case), `BagScreen`
  (the BAG's pocket list, its ▶ cursor and the printed description), the
  FAQ as a menu window, the sign-up as the naming screen

The React islands:

- `StartMenu` is the page's navigation, drawn as the game's START menu: a
  MENU button once the title screen is behind (top right; a pill in the
  bottom corner on a phone, out of the way while the map is pinned), a text
  window of entries with the ▶ cursor and a window describing the one the
  cursor is on. Arrows, Enter and Esc or B work as on the GBA.
- `RightNow` is the living world off the map: the time of day, the ships and
  the Magnet Train on their legs, and every Gym Leader, Elite Four and
  Champion at what they are doing, read from the world map's own simulation
  on the page's clock (see `src/world-map/README.md`), so the board and the
  map always agree.
- `BagScreen` lists the highlights as the BAG's key items: pointing or the
  arrow keys move the cursor, the Bag shakes and the item's description
  prints in the message box.
- `OakStage`, `StoryShuffle` and `DialogBox` draw the sign-up and the
  open-world section's figures. `DialogBox`'s `usePrinter` is the game's
  text printer, shared with `BagScreen`.

## Link previews

`public/og.png` is the title screen at 1200x630, the size link previews ask
for. `pnpm og` re-renders it from the built site (run `pnpm build` first);
without Playwright's own Chromium, point `CHROME` at a Chrome or Chromium
executable. Previews need absolute addresses, so `Layout.astro` only emits
the image, `og:url` and the canonical link once `site` is set in
`astro.config.ts`.

## World map

The world section and its full-screen explorer draw the whole overworld from
the game's own metatiles. It is a module of its own in `src/world-map/`, data
builder included; see its README.

## Type checking

`astro check` only supports TypeScript 5 and 6, so `pnpm typecheck` runs `astro sync` and then TypeScript 7's `tsc` instead. `tsc` checks `.ts` and `.tsx` files but not the frontmatter of `.astro` files. Keep logic in TypeScript modules and React components, and keep `.astro` files to markup and props.
