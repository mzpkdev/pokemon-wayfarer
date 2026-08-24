# Pokemon Wayfarer

Pokemon Wayfarer keeps its project-specific setup at the repository root. The game
source is included in `game/` as a local Git subtree copy that tracks upstream.

The planned upstream is
[`PokemonHnS-Development/pokehns-expansion`](https://github.com/PokemonHnS-Development/pokehns-expansion).

## Repository layout

| Path | Purpose |
| --- | --- |
| `docs/` | Project documentation and the subtree integration guide. |
| `game/` | The upstream game subtree, tracked locally in this repository. |

## Working in the repository

Keep project-specific files at the root or in a dedicated root-level directory. Changes
to `game/` remain local to this repository. Before updating it from upstream, read
[the subtree guide](docs/game-subtree.md).
