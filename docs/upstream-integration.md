# Upstream Integration

## Game Subtree

`game/` is a local Git subtree of
[`PokemonHnS-Development/pokehns-expansion`](https://github.com/PokemonHnS-Development/pokehns-expansion).
Changes inside it are regular `pokemon-wayfarer` commits and appear in its PRs.

`npm install` adds the `upstream` remote. If lifecycle scripts are disabled, add it yourself:

```sh
git remote add upstream git@github.com:PokemonHnS-Development/pokehns-expansion.git
```

Updates use ordinary subtree merges, without `--squash`. The upstream baseline
is connected to Wayfarer's history, so Git can distinguish upstream changes from
Wayfarer changes even though upstream files live at the repository root.

In a clean uberepo task based on current `origin/main`, fetch and inspect the target:

```sh
git fetch upstream
git rev-parse upstream/master
git log --oneline 8cb27a9fb01a0f80063c6285fa5805ae9c594329..upstream/master
```

Record that full target SHA, then run `git subtree merge --prefix=game <target-sha>`.
Resolve conflicts, regenerate affected data, test, and commit the merge. Review
automatically merged changes as well: upstream progression rules can disagree with
Wayfarer's League circuit, trainer scaling, and native HM progression.

Use **Create a merge commit** when landing an upstream-update PR. GitHub squash
merging and rebase merging discard the ancestry this workflow requires. After
landing, verify that the imported SHA is an ancestor of `origin/main`; merging the
same SHA again in a clean task must report that it is already up to date.

## Imported revision and Wayfarer behavior

The imported HNS revision is `8cb27a9fb01a0f80063c6285fa5805ae9c594329`.
The preceding baseline was `93c5eee2abb78f0baaf4050da4a65572bbdcfb66`.

- Wayfarer keeps its existing trainer IDs, League selection, and Dojo rematches.
  Upstream post-OBC teams have distinct IDs; their HNS postgame routing does not
  replace Wayfarer's rating-based progression.
- Native HM learnsets retain Wayfarer's conditional utility moves alongside
  upstream move additions. Wayfarer excludes the new Donphan Play Rough at level
  52 and Tauros Blaze Kick at level 50 to preserve their Strength catch windows;
  standalone HNS retains those moves.
- The summary screen retains its removed Pokédex shortcut. Physical L still opens
  EVs on the Skills page when the L=A option is enabled.
- Dynamic Punch tutors use separate taught messages for Mossdeep and HNS Route 4,
  so the upstream Cerulean text does not replace Hoenn's Mossdeep text.

## Historical squash imports

Earlier GitHub squash merges retained `git-subtree-*` trailers but discarded the
original subtree parents. Using `--squash` with those trailers selected an incorrect
base and attempted to undo unrelated local changes.

A tree-preserving merge connects Wayfarer to the actual previously imported
`93c5eee2abb78f0baaf4050da4a65572bbdcfb66` upstream commit. This repairs ancestry
without rewriting published commits. Ordinary subtree merges use that ancestry
and do not consult the misleading historical squash trailers. Do not repeat the
repair for later updates or bridge to an upstream revision that has not been imported.

Reverting an update restores files but leaves its upstream ancestry in history.
Before retrying a reverted update, deliberately restore the reverted integration
and reconcile the original problem; a repeated subtree merge alone cannot reapply
changes Git already considers merged.
