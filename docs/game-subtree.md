# Game subtree

The game source belongs in `game/`. It is a local copy of
[`PokemonHnS-Development/pokehns-expansion`](https://github.com/PokemonHnS-Development/pokehns-expansion)
managed as a Git subtree. No separate GitHub fork is needed.

Do not add placeholder files to `game/` before importing the subtree. Git requires the
target directory to be absent or empty when the subtree is first added.

## Upstream remote

The main project repository retains its existing `origin` remote. Add the upstream
remote before importing the subtree:

```sh
git remote add game-upstream git@github.com:PokemonHnS-Development/pokehns-expansion.git
```

The upstream repository's default branch is `master`.

## First import

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
