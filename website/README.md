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

| Script         | What it does                             |
| -------------- | ---------------------------------------- |
| `pnpm dev`     | Start the dev server                     |
| `pnpm build`   | Build the static site into `dist/`       |
| `pnpm preview` | Serve the built site                     |
| `pnpm assets`  | Re-render the game art in `public/game/` |
| `pnpm check`   | Type check, lint, and check formatting   |
| `pnpm format`  | Format files with oxfmt                  |

## Game art

Everything on the page is drawn with the game's own graphics, rendered from
`../game/graphics` by `scripts/game-assets.mjs`:

- the opening's four parallax layers and Poké Ball, which `TitleScene` composes
  on a canvas at GBA resolution and scales by whole pixels
- the title screen's logo, wordmark, PRESS START and passing sprites
- the 20 text window frames, map name popups and key item icons
- `Wayfarer Text`, a web font traced from the game's normal Latin font, with
  glyph widths from `game/src/fonts.c` and characters from `game/charmap.txt`
- PROF. OAK, his oval and the POKéMON he sends out, from the new game intro
- the region maps for the world section. The game has no Town Map art for
  Sinnoh, so that one comes from the project its maps were ported from
  (`scripts/source/sinnoh`). Alola and Sinjoh have none anywhere: Alola's
  islands are traced from their overworld maps, and Sinjoh sits on a northern
  coast that closes the Johto map's cut-off top. Both are painted in colors
  sampled from the region maps.

The output is committed, so building the site does not need the game tree.
Run `pnpm assets` after the source art changes. It also writes
`src/generated/game-art.json`, the trimmed sizes and offsets that place the
title sprites where the game draws them. The scene's timings and positions in
`src/lib/scene.ts` come from `game/src/intro.c` and `game/src/title_screen.c`.

The release date lives in `src/lib/release.ts`.

## Type checking

`astro check` only supports TypeScript 5 and 6, so `pnpm typecheck` runs `astro sync` and then TypeScript 7's `tsc` instead. `tsc` checks `.ts` and `.tsx` files but not the frontmatter of `.astro` files. Keep logic in TypeScript modules and React components, and keep `.astro` files to markup and props.
