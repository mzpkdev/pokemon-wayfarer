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

| Script         | What it does                           |
| -------------- | -------------------------------------- |
| `pnpm dev`     | Start the dev server                   |
| `pnpm build`   | Build the static site into `dist/`     |
| `pnpm preview` | Serve the built site                   |
| `pnpm check`   | Type check, lint, and check formatting |
| `pnpm format`  | Format files with oxfmt                |

## Type checking

`astro check` only supports TypeScript 5 and 6, so `pnpm typecheck` runs `astro sync` and then TypeScript 7's `tsc` instead. `tsc` checks `.ts` and `.tsx` files but not the frontmatter of `.astro` files. Keep logic in TypeScript modules and React components, and keep `.astro` files to markup and props.
