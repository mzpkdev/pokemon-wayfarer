# v0 evolution thresholds

`evolution.json` is game-owned content. Its 145 chains preserve the approved notable-trainer thresholds. The 145 `nonLevelEdges` rows are explicit first-pass thresholds for every other active nonlevel successor in the game graph. They are progression placeholders to tune through playtesting; they are not inferred at runtime. Stone and other item methods generally start at level 35, trade at 42, Alcremie forms at 30, and later stages rise at least ten levels above their predecessor.

Baby forms are listed separately and never become a step-down result. Numeric evolution thresholds come from active game data, including battle-only level evolutions.

Run `python3 game/tools/notable_trainers/evolution.py --check` from the repository root to verify full active-edge coverage, source agreement, unique predecessors, increasing levels, and the generated ROM table.
