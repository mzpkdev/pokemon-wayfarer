# Quality of life

Specifications: [Fast battles, full-speed music](../specs/battle-speed.md)

Implemented: No.
Design status: Draft. Fast battles is agreed as the first feature. The other
conveniences list what the game already has; each change to them waits on an
[open question](#open-questions).

## Intent

ROM hack players expect a set of modern conveniences, and they named them in
[the player-demand research](#references). The loudest request by far is
faster battles with the music at normal speed. Wayfarer already has several of
the others from pokeemerald-expansion and Heart and Soul. This PRD says which
conveniences Wayfarer has, which it adds, and which are the owner's call.

## Design

| Convenience | Status today |
| --- | --- |
| [Fast battles, full-speed music](#fast-battles-full-speed-music) | Missing. FAST BATTLES only skips pauses |
| [Best ball button](#best-ball-button) | Already present |
| [Natures from the party menu](#natures-from-the-party-menu) | Missing. Mints do the job from the bag |
| [Party-menu move relearner](#party-menu-move-relearner) | In the engine, switched off |
| [Exp. All toggle](#exp-all-toggle) | Already present |
| [Type effectiveness in battle](#type-effectiveness-in-battle) | Already present |
| [Passing time](#passing-time) | Ruled out by the in-game clock |

### Fast battles, full-speed music

Status: missing. This is the first feature, specified in
[Fast battles, full-speed music](../specs/battle-speed.md).

- A new **BATTLE SPEED** option makes battles run 1×, 2×, 3× or 4× as fast.
- Animations, text, HP and Exp. bars and pauses all speed up.
- Music and sound effects play at normal speed and pitch.
- Only battles speed up. The overworld always runs at normal speed.
- Link battles always run at 1×.

The existing **FAST BATTLES** option (Heart and Soul) only skips the scripted
pauses after battle messages. It does not make anything else faster. It stays
as it is, and the two options combine.

### Best ball button

Status: already present. In a wild battle, R at the action menu throws the
last ball used. Hold R and press the D-Pad to pick another ball. The **BALL
PROMPT** option turns the button off, and it is on by default.

- Engine: `B_LAST_USED_BALL` and `B_LAST_USED_BALL_CYCLE` in
  `include/config/battle.h`, `CanThrowLastUsedBall` in `battle_interface.c`.
- No change proposed.

### Natures from the party menu

Status: missing. No party-menu action changes a nature. Mints do it: you use
a mint from the bag, then pick a Pokémon from the party. The MINTS mode on the
new-game challenge screen is on by default.

- Mints are sold only at the Goldenrod Flower Shop, after the third badge,
  and one Modest Mint lies on Route 42. Johto is the only place to get them.
- A player who starts in Kanto or Hoenn has no mints until they reach Goldenrod.

### Party-menu move relearner

Status: in the engine, switched off. The party menu can offer a MOVES action
that relearns moves, but `P_PARTY_MOVE_RELEARNER` is `FALSE`
(`include/config/summary_screen.h`). The summary-screen relearner and egg, TM
and tutor relearning are also off.

- Today the relearners are NPCs in Fallarbor Town and Blackthorn City.
- Turning it on is a config change, with no new design work.

### Exp. All toggle

Status: already present. Every new game gets the Gen 6 Exp. Share key item
(the Heart and Soul new-game script). Using it from the bag turns it on or off
for the whole party.

- It starts off. Only Elm's lab scene turns it on, so only Johto-origin
  players start with it on (`FLAG_EXP_SHARE_ENABLED`).

### Type effectiveness in battle

Status: already present. When you choose a move, the PP line shows how
effective it is against the target (`B_SHOW_EFFECTIVENESS` is
`SHOW_EFFECTIVENESS_ALWAYS`). Type icons beside the HP bars are off
(`B_SHOW_TYPES`).

- No change proposed.

### Passing time

Status: ruled out in v0. Players asked to sleep or wait until a chosen time
of day, so evening players can see daylight. The [in-game clock](in-game-clock.md)
rules out setting or skipping time in v0. The only exception is sleeping
through a ferry voyage. This PRD does not overturn that rule. See the
[open questions](#open-questions).

## Boundaries

- No overworld speed-up. Developers in the request threads warn that it causes
  bugs, and it rushes past the hand-made maps.
- DexNav belongs to [Researcher Rating](researcher-rating.md).
- Difficulty modes and level caps are out of scope.
- No change to the clock rules owned by [In-game clock](in-game-clock.md).

## Constraints

- RAM: the release build's EWRAM is 587 bytes under its ceiling, and the
  mechanics-test build is within 32 bytes of the 256 KB hardware limit. Battle
  speed adds no EWRAM. Any later item must follow the repository's RAM rules.
- Save: new options go in the existing option bits. Prerelease saves need no
  migration.

## Playtesting

- Does 2× feel like the speed most players settle on? Is 4× ever too fast to read?
- Do overlapping sound effects at 3× and 4× sound broken or acceptable?
- Do Kanto and Hoenn players miss mints and the Exp. Share before Johto?

## Open questions

1. **Battle speed default.** 1× or 2× for a new game?
   *Recommendation:* 1×, and make it easy to find in the options. Players who
   want it will look for it.
2. **Natures.** Add a party-menu nature changer, or keep mints?
   *Recommendation:* keep mints. Sell them in one shop per region, after the
   same badge count as Goldenrod's, so every origin can get them. A free
   changer adds a menu for something mints already do.
3. **Party-menu relearner.** Switch it on?
   *Recommendation:* yes, for level-up moves only and at no cost. It is a
   config change. The NPCs keep their role for players who never open the menu.
4. **Exp. Share at the start.** Should it start on for every origin?
   *Recommendation:* yes. A player's origin should not decide a convenience
   setting. The bag toggle stays.
5. **Passing time.** Should a player be able to pass time to a chosen time of
   day? It conflicts with the in-game clock's v0 rule.
   *Recommendation:* not in v0. Playtest the 20-minute morning, evening and
   night first. If evening players still miss daylight, allow sleeping in your
   own bed until the next morning. Time would only move forward, as with the
   ferry cabins.

## References

- Battle speed threads on r/PokemonROMhacks:
  [1w0ih7i](https://www.reddit.com/r/PokemonROMhacks/comments/1w0ih7i/) (1,770 points),
  [1vwwdk8](https://www.reddit.com/r/PokemonROMhacks/comments/1vwwdk8/) (1,005 points),
  [1vojpfw](https://www.reddit.com/r/PokemonROMhacks/comments/1vojpfw/) (120 points).
- Hacks with battle speed-up, as named by players in these threads: SoulGold,
  Emerald Rogue, R.O.W.E., Pink Recharged, Yellow Recharged, Dreamstone
  Mysteries and Zennpachii's Emerald QoL hack.
- [In-game clock](in-game-clock.md)
- [Researcher Rating](researcher-rating.md)
