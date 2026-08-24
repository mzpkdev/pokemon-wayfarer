# Game subtree

The game source belongs in `game/`. It is a local copy of
[`PokemonHnS-Development/pokehns-expansion`](https://github.com/PokemonHnS-Development/pokehns-expansion)
managed as a Git subtree. No separate GitHub fork is needed.

The initial import from upstream `master` is already present in `game/`.

## Upstream remote

The main project repository retains its existing `origin` remote. Each clone that will
pull game updates also needs this local remote:

```sh
git remote add game-upstream git@github.com:PokemonHnS-Development/pokehns-expansion.git
```

The upstream repository's default branch is `master`.

## Initial import

The subtree has already been imported. Do not run this command again unless you are
recreating the repository from scratch.

```sh
git fetch game-upstream master
git subtree add --prefix=game game-upstream master --squash
```

## Pulling upstream changes

Commit any local changes to `game/` first, then run:

```sh
git fetch game-upstream master
git subtree pull --prefix=game game-upstream master --squash
```

Resolve any conflicts in `game/`, test the integrated game, then commit the resulting
subtree update in this repository. Local changes stay here unless you later choose to
create a GitHub fork and publish them.
