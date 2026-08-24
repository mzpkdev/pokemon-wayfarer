# Pokemon Wayfarer

Pokemon Wayfarer keeps its project-specific setup at the repository root. The game
source will be imported into `game/` as a Git subtree.

The planned upstream is
[`PokemonHnS-Development/pokehns-expansion`](https://github.com/PokemonHnS-Development/pokehns-expansion).

## Repository layout

| Path | Purpose |
| --- | --- |
| `docs/` | Project documentation and the subtree integration guide. |
| `game/` | The upstream game subtree. This directory is intentionally absent until the subtree is added. |

## Working in the repository

Keep project-specific files at the root or in a dedicated root-level directory. Treat
`game/` as imported upstream source. Before changing it or updating it from upstream,
read [the subtree guide](docs/game-subtree.md).
