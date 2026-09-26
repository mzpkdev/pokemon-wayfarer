# Gym Leader scaling

Implemented: No for trainer-owned world progression. The earlier player-TR
scaler exists in code but its general build switch is disabled by default;
Giovanni's Wayfarer finale has a separate runtime path.

## Intent

Let players challenge the supported singles Gym Leaders in different orders.
Each leader starts with an approachable authored team and develops through a
personal curve as the world gains badges and first league clears. Their
strength comes from their own effective TR, independently of player TR and
party levels.

## Design

Use the shared [trainer world progression](trainer-world-progression.md)
contract. Each canonical trainer has a starting `baselineTR`, personal badge
checkpoints, first-clear growth, and authored party stages. Resolve current
global milestones when preparing a battle, select the applicable stage, and
convert the trainer's effective TR to levels through the shared NPC curve.

The baseline is not a permanent battle rating. Do not reuse the previous
player-TR party-size thresholds or select a prefix from one immutable team.
Author complete opening, developing, and competitive stages. Species changes
between stages are explicit content; there is no automatic evolution or
predecessor substitution. Teams keep recognizable aces and valid support at
every supported world point.

Original FRLG, Emerald, and repository HNS parties are references, not mandatory
opening difficulty. Later Gym Leaders need low starting ratings too. Blue's
personal curve reaches Champion strength sooner while retaining an approachable
opening; that experiment does not create a new Blue badge encounter.

Freeze the selected rating and profile for the battle and any reconstruction.
A retry at unchanged milestones has the same authored party and levels. If the
player earns another badge or first league clear before retrying, the leader
uses the new world point. Award the challenged Gym's badge after its battle,
so that award never strengthens the opponent during the fight.

## Coverage

Wayfarer still has 24 badge encounters. The proposed singles policy covers
23 canonical badge opponents: Brock, Misty, Lt. Surge, Erika, Janine, Sabrina,
Blaine, Giovanni; Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine, Pryce, Clair;
Roxanne, Brawly, Wattson, Flannery, Norman, Winona, and Juan. The inventory must
map actual Wayfarer battle IDs and selectable variants, including Giovanni's
bespoke Viridian finale.

Tate and Liza remain outside the singles pool. Their double Gym battle and its
badge retain the existing policy; this proposal neither removes that badge
nor converts the battle to singles. The explorer's 24 `gymEligible` records
include Blue and must not be mistaken for the 24 badge encounter inventory.

Canonical pool membership never enrolls all of a character's encounters.
Blue's opening/rival/Dojo battles, leader rematches, Giovanni's villain scenes,
facilities, partners, and story battles keep their separate policies unless
explicitly added to the coverage inventory. Standalone builds remain unchanged.

## Teams and construction

Each stage supplies its exact species/forms, move policies, held items,
abilities, IVs, EVs, natures, aces, battle order, and level offsets. Review new
stage content deliberately rather than retaining an inappropriate endgame
moveset on an opening Pokémon. `AUTHORED` moves keep their exact reviewed tuples;
`LEVEL_UP` moves use the selected species' normal learnset at its effective level.

Keep original source-member identity when constructing in battle order, so
moves, items, abilities, and gimmicks remain attached to the correct member.
An authored ace-last order does not force AI replacement or switching behavior.
Preserve encounter rewards, prize-money basis, badge scripts, and AI unless
reviewed stage content explicitly changes the relevant battle field.

Trainer-species randomization keeps its existing complete construction path;
it bypasses the new stage transformation. Other explicit challenge and move
randomizer options keep their current precedence.

## Balance and acceptance

Use the NPC growth/level rules in the shared specification. The explorer's
TR-0 level-12 anchor, stage thresholds, and personal checkpoints remain
provisional. The player's cap stays separate. Historical Gym order, current
party size, and training do not supply hidden difficulty adjustments.

Exhaust badges 0–24 and first clears 0–3 for every enrolled variant. Check
approachable opening parties, every stage transition, postponed leaders,
source metadata, battle reconstruction, and explicit randomizer bypass.
Check that Gym members under their ordinary player-TR policy do not consistently
outclass their leader. Validate the exceptional Tate/Liza battle separately.

The new policy requires reviewed content and ROM playtesting before enablement.
The browser explorer predicts parties and levels; it does not establish combat
balance or replace the production encounter inventory.

## References

- [Gym Leader scaling specification](../specs/gym-leader-scaling.md)
- [Trainer world progression specification](../specs/trainer-world-progression.md)
- [Ordinary Trainer and Gym-member scaling](trainer-party-scaling.md)
- [Player party progression](../specs/trainer-rating-party-progression.md)
