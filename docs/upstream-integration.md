# Upstream Integration

## Game Subtree

`game/` is a local Git subtree of
[`PokemonHnS-Development/pokehns-expansion`](https://github.com/PokemonHnS-Development/pokehns-expansion).
Changes inside it are regular `pokemon-wayfarer` commits and appear in its PRs.

`npm install` adds the `upstream` remote. If lifecycle scripts are disabled, add it yourself:

To bring in upstream `master`, commit your local work first, then run:

```sh
git fetch upstream master
git subtree pull --prefix=game upstream master --squash
```

Resolve conflicts, test, and commit the update.
