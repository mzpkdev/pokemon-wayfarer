# Game subtree

The game source belongs in `game/`. It will come from a fork of
[`PokemonHnS-Development/pokehns-expansion`](https://github.com/PokemonHnS-Development/pokehns-expansion).

Do not add placeholder files to `game/` before importing the subtree. Git requires the
target directory to be absent or empty when the subtree is first added.

## Planned remotes

The main project repository will retain its existing `origin` remote. Add these remotes
when the fork is created:

```sh
git remote add game-fork git@github.com:<your-account>/pokehns-expansion.git
git remote add game-upstream git@github.com:PokemonHnS-Development/pokehns-expansion.git
```

Replace `<your-account>` with the GitHub account that owns the fork. The upstream
repository's default branch is `master`.

## First import

```sh
git fetch game-fork master
git subtree add --prefix=game game-fork master --squash
```

## Pulling upstream changes

Commit any local changes to `game/` first, then run:

```sh
git fetch game-upstream master
git subtree pull --prefix=game game-upstream master --squash
```

Resolve any conflicts in `game/`, test the integrated game, then commit the resulting
subtree update in the main project repository.
