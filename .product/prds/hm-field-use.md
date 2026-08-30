# Badge-free HM field use

## Intent

Make HMs useful in an open-world campaign without making Gym order or move slots part of the traversal cost. A player who has found an HM and brought a Pokémon capable of learning it can use that field move immediately. Teaching the move remains available, but it is a battle-build choice rather than a field-use requirement.

This behavior applies to Emerald, FireRed and LeafGreen, and HNS.

## Design

Badges never authorize HM field use. Earning or missing a badge does not change whether Cut, Fly, Surf, Strength, Flash, Rock Smash, Waterfall, Dive, or Whirlpool can be used outside battle in a build where that HM exists.

When the player attempts to use an HM in the field, the game selects its user in this order:

1. The first non-Egg party Pokémon that knows the move.
2. If no party Pokémon knows it, the first non-Egg party Pokémon that can learn the move, provided the matching HM is in the Bag.
3. If neither rule finds a user and HMs Overwrite is active, the first non-Egg party Pokémon, provided the matching HM is in the Bag.

The selected Pokémon may be fainted. Eggs never qualify. A Pokémon that knows the move does not need the HM to be present in the Bag. A Pokémon that does not know the move needs both the HM and normal HM compatibility unless the HMs Overwrite exception applies.

Using an unlearned HM does not teach the move, replace another move, consume the HM, or change PP. The Pokémon still appears as the field-move user in the same places where a learned user would.

For example, a player with no badges, HM01 in the Bag, and a compatible Pokémon may Cut even when no party Pokémon knows Cut. A player whose Pokémon already knows Surf may Surf without a badge or an HM ownership check. A player with HM03 but only Eggs or incompatible Pokémon cannot Surf unless an eligible non-Egg Pokémon is added to the party or HMs Overwrite applies.

HM moves follow the same move-management rules as ordinary moves. They may be forgotten by the Move Deleter or replaced through normal move learning. A Pokémon may also be catch-swapped, sent to the PC, or released even when it knows an HM or is the party's only current HM user.

HM items remain reusable and non-discardable. Once obtained, an HM remains in the Bag; map initialization and story scripts do not confiscate or silently remove it.

## Boundaries

Existing HM acquisition points, story conditions, and reward gates remain unchanged. A badge may still be required to receive an HM when that badge is part of the existing reward sequence. For example, Chuck's wife may still wait for the Storm Badge before giving Fly, but the dialogue must present Fly as a reward rather than a field-use permission granted by the badge.

This feature does not change Pokémon compatibility tables, HM numbering, move effects in battle, map obstacles, puzzles, animations, or destination rules such as which Fly locations have been visited. It does not add Bag commands for field use or change non-HM field moves.

HNS uses Whirlpool rather than Dive as HM08. This feature does not reclassify HNS Dive or change its non-HM field-use rules.

Badges keep all unrelated effects, including progression, obedience, battle, level-cap, shop, and rematch behavior defined elsewhere.

The open-world regional traversal designs continue to define which settlements must be reachable without HMs. This feature changes how optional HM routes and shortcuts work after the player obtains the relevant machine; it does not make HMs part of the core settlement network.

## Balance

HM acquisition remains the progression gate for each field ability. Party composition also matters in a standard playthrough because the player needs a compatible Pokémon when nobody knows the move. Removing the badge check lets players use an HM as soon as they discover it, while allowing unlearned use removes the permanent move-slot tax.

The HMs Overwrite exception protects challenge runs whose party restrictions or randomized movesets could otherwise remove every compatible user. It does not grant an HM the player has not obtained.

## Content

The rule covers every HM field interaction compiled into each build:

| Build | HM field moves |
| --- | --- |
| Emerald | Cut, Fly, Surf, Strength, Flash, Rock Smash, Waterfall, and Dive |
| FireRed/LeafGreen | Cut, Fly, Surf, Strength, Flash, Rock Smash, and Waterfall |
| HNS | Cut, Fly, Surf, Strength, Flash, Rock Smash, Waterfall, and Whirlpool |

Gym victory dialogue must stop claiming that a badge permits field use. The affected speakers are:

| Build or region | Gym dialogue to revise |
| --- | --- |
| Emerald, Hoenn | Roxanne, Brawly, Wattson, Flannery, Norman, Winona, Tate and Liza, and Juan |
| FireRed/LeafGreen, Kanto | Brock, Misty, Lt. Surge, Erika, and Koga |
| HNS, Johto | Falkner, Bugsy, Whitney, Morty, Chuck, and Pryce |

The remaining victory speech may describe the badge's other effects, the Gym Leader's reaction, or the HM reward where one exists. It must not imply that a badge licenses an HM, even if the HM is received at the same point.

Supporting dialogue and messages that teach or enforce the old rules also need revision:

| Build or region | Content to revise |
| --- | --- |
| Emerald, Hoenn | Cutters House HM tutorial, Granite Cave Flash NPC, Scott at the Rustboro Pokémon School, Route 118 Surf NPC, Route 119 rival Fly explanation, and the Lilycove Move Deleter |
| FireRed/LeafGreen, Kanto | S.S. Anne captain, Route 2 Flash aide, Cerulean Badge Guy, Route 14 HM-forgetting advice, and any Move Deleter text shared with other builds |
| HNS, Johto | Chuck's wife, Lance's Whirlpool explanation, the Blackthorn Move Deleter, Azalea's first-entry Surf removal, and any shared HM tutorial text |
| Shared text | Messages that say HM moves cannot be forgotten, that another badge is needed, or that protect the last Surf user |

Each build should explain the complete rule once at an early, ordinary HM handoff or tutorial touchpoint. That explanation should say that the player can either teach the move or carry the HM with a compatible Pokémon in the party, and that badges are not required. Gym Leaders and later HM givers should not repeat the full tutorial.

## Presentation

Terrain-bound moves remain contextual. The player initiates Cut, Surf, Strength, Rock Smash, Waterfall, Dive, and Whirlpool by interacting with the relevant terrain or obstacle. The interaction identifies the selected Pokémon using the active build's existing field-HM presentation, whether or not it knows the move. HNS Whirlpool names its selected user before traversal like other HNS terrain HMs.

Fly and Flash appear as party actions on a compatible Pokémon when the corresponding HM is owned. A Pokémon that already knows Fly or Flash retains the action without relying on Bag ownership. HMs Overwrite exposes the action on any selected non-Egg party Pokémon when the HM is owned. This explicit selection overrides the automatic user order.

Failure feedback should tell the player what is missing. Attempts without the HM and without a Pokémon that knows the move should point to the missing HM. Attempts with the HM but no eligible Pokémon should say that none of the party Pokémon can use it. No message should mention a badge requirement.

## Interactions

- Fainted Pokémon qualify for field use, including traversal moves that can otherwise strand the player.
- Eggs do not qualify even when their underlying species would be compatible.
- A later party Pokémon that knows the move takes priority over an earlier Pokémon that merely can learn it.
- Among multiple Pokémon in the same eligibility tier, party order decides the user.
- HMs Overwrite changes only the compatibility requirement. It still requires HM ownership when no Pokémon knows the move and excludes Eggs. Automatic interactions use the first eligible party slot; an explicit Fly or Flash party action uses the selected Pokémon.
- Forgetting, replacing, depositing, releasing, or catch-swapping the only Pokémon that knows an HM is allowed. The next field-use attempt evaluates the current party and Bag again.
- Teaching an HM remains reusable and follows the existing compatibility rules. Field use does not silently modify a Pokémon's moveset.
- Fly still follows the existing destination and map restrictions. Flash and terrain moves still follow their existing map and obstacle checks after a user is selected.

## Playtesting

Test each HM available in Emerald, FireRed/LeafGreen, and HNS with no badges and with the badge that previously authorized it. Both states should produce the same result when the party and Bag are otherwise identical.

For every build, cover these eligibility cases:

- A Pokémon knows the move and the HM is absent.
- The HM is owned and a Pokémon can learn the move but does not know it.
- The HM is owned, but the only compatible Pokémon is fainted.
- The HM is owned, but the only compatible species is an Egg.
- The HM is owned and no Pokémon is compatible, both with and without HMs Overwrite.
- An earlier party member is compatible while a later party member knows the move.
- Several Pokémon qualify in the same tier.
- The party has no eligible Pokémon and the HM is missing.

Confirm that contextual obstacles, Surf entry, Fly, Flash, save and reload, map transitions, and repeat use all select the intended Pokémon without teaching the move or changing PP. Check that failures distinguish a missing HM from a missing eligible Pokémon.

Use the Move Deleter and ordinary move replacement on every HM move available in each build. Deposit and catch-swap the only Pokémon that knows an HM, including the only Surf user, and confirm no HM-specific protection remains.

Replay every affected Gym reward and HM handoff. HM items must still be awarded at their existing points, while no dialogue or system message says that badges authorize field use or that HM moves cannot be forgotten. New players should be able to explain the Bag-plus-compatible-Pokémon rule after the centralized tutorial without relying on Gym dialogue.

Carry HM Surf through the first Route 33 to Azalea trigger in HNS and confirm that the transition does not remove it from the Bag.

## References

- [Technical specification](../specs/hm-field-use.md)
- [Emerald open-world regional traversal](emerald-open-world-region-traversal.md)
- [FireRed and LeafGreen open-world regional traversal](frlg-open-world-region-traversal.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
