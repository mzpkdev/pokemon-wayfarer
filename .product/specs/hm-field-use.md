# HM field use

PRD: [HM field use](../prds/hm-field-use.md)
Implemented: Yes

## Scope

This specification defines field HM eligibility, performer selection, field-action access, HM move management, and the required dialogue changes for Emerald, FireRed/LeafGreen, and HNS. It covers every player-available HM field move in each build, including HNS Whirlpool.

It does not relocate HMs, change how they are awarded, alter their battle behavior, or change non-HM field moves. Badge effects unrelated to field HMs also remain unchanged.

## Behavior

### Field-use eligibility

A badge is never required to use an HM in the field. This applies to central field-move checks, automatic overworld triggers, party-menu actions, and build-specific map scripts.

For a requested HM, the game resolves an automatic field user in this order:

1. Use the first non-Egg party Pokémon that knows the move. The HM item is not required in this case.
2. If no party Pokémon knows the move, require the corresponding HM in the Bag and use the first non-Egg party Pokémon whose species can learn the move from that HM.
3. If neither rule finds a user, the request fails.

Party order determines the first matching Pokémon. A fainted Pokémon qualifies. An Egg never qualifies, even if corrupt or migrated data gives it the move.

The HMs Overwrite challenge option adds one fallback after the normal rules. When the player owns the HM but has no known or compatible user, the first non-Egg party Pokémon may perform the field move regardless of compatibility. The option does not remove HM ownership requirements and does not allow an Egg to perform the move.

An explicit party-menu selection overrides automatic party ordering. If the selected Pokémon knows the move, or qualifies through HM ownership and compatibility, that Pokémon performs Fly or Flash. With HMs Overwrite active, any selected non-Egg Pokémon qualifies while the HM is owned.

Using an unlearned HM in the field does not:

- teach or replace a move;
- consume the HM;
- consume move PP;
- change friendship, experience, or any other Pokémon state.

The resolver must return enough information to distinguish these failure states:

- no party Pokémon knows the move and the HM is not in the Bag;
- the HM is in the Bag, but no party Pokémon qualifies;
- a user qualifies, but the move cannot be used in the current field context.

Non-HM field actions retain their existing eligibility. In particular, move-only actions without a machine item must not acquire an HM ownership requirement.

HNS designates Whirlpool rather than Dive as HM08. HNS Dive remains a non-HM field action and keeps its existing eligibility, including its current badge condition; this feature must not make it badge-free or require a nonexistent Dive HM in that build.

FireRed/LeafGreen compiles the shared non-HNS HM08 Dive item and move mapping, but no current FireRed/LeafGreen content awards or uses it. That latent mapping is outside this feature and does not require field-use or playtest coverage unless another feature makes Dive available in those builds.

### Field-action access

Cut, Surf, Strength, Rock Smash, Waterfall, Dive, Whirlpool, and other terrain-bound HMs keep their contextual overworld interactions. Those interactions use the automatic resolver before asking for confirmation or starting the field effect.

Fly and Flash appear in a selected Pokémon's party actions under these conditions:

- the Pokémon is not an Egg; and
- it knows the move, or the player owns the HM and the Pokémon is compatible, or the player owns the HM and HMs Overwrite is active.

Knowing Fly or Flash is sufficient even if the HM item is absent. Compatibility without ownership is not sufficient. Selecting an HM in the Bag continues to teach the move; the Bag does not become a second field-action menu.

Existing context rules remain in force. Fly still requires a valid map and destination, Flash still requires a valid dark area, Surf still requires surfable water, and each terrain interaction keeps its existing follower, direction, map, animation, and confirmation behavior.

Surf availability and all other automatic checks must ignore Eggs and accept fainted Pokémon. They must use the same ownership, compatibility, and HMs Overwrite rules as scripted interactions.

HNS Whirlpool must be registered or otherwise routed through the same HM resolver as the other field moves. Encountering a whirlpool without a badge is allowed, but traversal only begins after a valid Whirlpool user is resolved. Before movement begins, the script buffers that user and displays the HNS-style "{Pokémon} used Whirlpool!" message. No new field-move banner or animation is required. The current badge-only path with its user check disabled is not acceptable.

### Feedback and tutorials

Successful field use keeps the active build's existing Pokémon nickname, move name, animation, sound, and follower presentation where those elements already exist. The standard success message remains valid when the move is used without being learned because the selected Pokémon is still the performer. Whirlpool adopts the named-user message described above because its current script does not identify a performer.

Failure feedback states the actionable cause:

- when nobody knows the move and the HM is missing, tell the player that the HM is needed;
- when the HM is owned but no party Pokémon qualifies, tell the player that none of their Pokémon can use the move;
- when a user qualifies but the context is invalid, keep the move's existing contextual failure text.

No field-use path may display the existing message that another badge is required.

Each build has one early HM tutorial at or near its first HM acquisition. The tutorial explains that:

- badges are not required for field use;
- the move does not need to be learned while the HM is in the Bag and a compatible party Pokémon is present;
- teaching the move remains optional, and an HM move can later be forgotten or replaced.

The intended tutorial locations are the Rustboro Cutter's House for Emerald, the S.S. Anne captain's HM Cut handoff for FireRed/LeafGreen, and the Ilex Forest HM Cut reward for HNS. Other NPCs may give move-specific hints, but they must not repeat the complete system tutorial.

Gym leaders and badge explainers must not claim that a badge authorizes an HM or that the field move must be learned. Their badge rewards, HM rewards, and other accurate badge effects may remain. If defeating a Gym Leader is still the condition for receiving an HM, dialogue may describe the HM as a reward for that victory without presenting the badge as permission to use it.

The affected Gym dialogue includes:

- Emerald: Roxanne, Brawly, Wattson, Flannery, Norman, Winona, Tate and Liza, and Juan.
- FireRed/LeafGreen: Brock, Misty, Lt. Surge, Erika, Koga, and the Cerulean Badge Guy explanations.
- HNS: Falkner, Bugsy, Whitney, Morty, Chuck, and Pryce.

Supporting dialogue and denial text must also be revised wherever it says or implies that a Pokémon must learn an HM, a badge enables field use, or HM moves cannot be forgotten. This includes the Granite Cave Flash hint, Rustboro school and Cutter's House explanations, Route 118 and Route 119 hints, the S.S. Anne captain, the Route 2 Flash aide, Chuck's wife, Lance's Whirlpool explanation, both Move Deleters, and the global HM-forgetting messages.

### Content implementation inventory

All map script paths in this inventory are relative to `game/data/maps/`.

The mapped Gym and badge explanations are:

- Emerald: `RustboroCity_Gym/scripts.inc`, `DewfordTown_Gym/scripts.inc`, `MauvilleCity_Gym/scripts.inc`, `LavaridgeTown_Gym_1F/scripts.inc`, `PetalburgCity_Gym/scripts.inc`, `FortreeCity_Gym/scripts.inc`, `MossdeepCity_Gym/scripts.inc`, and `SootopolisCity_Gym_1F/scripts.inc`.
- FireRed/LeafGreen: `PewterCity_Gym_Frlg/scripts.inc`, `CeruleanCity_Gym_Frlg/scripts.inc`, `VermilionCity_Gym_Frlg/scripts.inc`, `CeladonCity_Gym_Frlg/scripts.inc`, `FuchsiaCity_Gym_Frlg/scripts.inc`, and `CeruleanCity_House1_Frlg/scripts.inc`.
- HNS: `VioletCity_Gym_hns/scripts.inc`, `AzaleaTown_Gym_hns/scripts.inc`, `GoldenrodCity_Gym_hns/scripts.inc`, `EcruteakCity_Gym_hns/scripts.inc`, `CianwoodGym_hns/scripts.inc`, and `MahoganyTown_Gym_hns/scripts.inc`.

The supporting map scripts are:

- Emerald: `GraniteCave_1F/scripts.inc`, `RustboroCity_CuttersHouse/scripts.inc`, `RustboroCity_PokemonSchool/scripts.inc`, `Route118/scripts.inc`, `Route119/scripts.inc`, and `LilycoveCity_MoveDeletersHouse/scripts.inc`.
- FireRed/LeafGreen: `SSAnne_CaptainsOffice_Frlg/scripts.inc`, `Route2_EastBuilding_Frlg/scripts.inc`, and `Route14_Frlg/scripts.inc`.
- HNS: `IlexForest_hns/scripts.inc`, `CianwoodCity_hns/scripts.inc`, `RocketHideout_B2F_hns/scripts.inc`, `BlackthornCity_House3_hns/scripts.inc`, and the Surf removal in `AzaleaTown_hns/scripts.inc`.

The shared engine strings are `gText_CantUseUntilNewBadge` and `gText_HMMovesCantBeForgotten2` in `game/src/strings.c`, plus `STRINGID_HMMOVESCANTBEFORGOTTEN` in `game/src/battle_message.c`. Remove or replace every reachable use so none of these strings can teach the old behavior.

Dialogue replacement preserves the speaker's surrounding reward or tutorial context. Gym text removes HM permission claims; regional tutorial text explains the new rule once; move-specific hints stop saying the move must be learned; HM-forgetting text is deleted or replaced with accurate ordinary move-management guidance.

### HM moves, items, and party management

An HM move can be forgotten or replaced anywhere an ordinary move can be forgotten or replaced. This includes ordinary move learning, the Move Deleter, and move replacement during evolution or other supported move-learning flows. The special protection for the last Pokémon that knows Surf is removed.

Knowing an HM does not prevent a Pokémon from being swapped out after a catch, deposited in a Box, moved between party and storage, or released. Other restrictions unrelated to HM knowledge remain unchanged.

HM items remain reusable, non-consumable, and non-discardable. Their teaching behavior and battle move data remain unchanged. No map initializer, transition, or story script may silently remove an owned HM. The HNS first-entry Azalea initialization must stop removing HM Surf while preserving its unrelated state initialization.

Existing acquisition and reward conditions remain unchanged. For example, an HM may still be awarded after a Gym victory or story event. Once acquired, its field use is governed only by this specification.

### Save behavior

The feature adds no persistent state and requires no save migration. Existing saves immediately use the new rules based on their current party, known moves, HM inventory, challenge options, and field context. Badge state is ignored only for field HM permission.

### Validation

Deterministic tests must cover the shared resolver and every caller-specific rule:

| Case | Expected result |
| --- | --- |
| No badges, first party Pokémon knows the HM move | That Pokémon is selected even when the HM item is absent. |
| Earlier compatible Pokémon, later Pokémon knows the move | The later known user is selected for an automatic interaction. |
| Nobody knows the move, HM owned, multiple compatible Pokémon | The first compatible non-Egg Pokémon is selected. |
| Compatible Pokémon present, HM absent | Field use fails because the HM is missing. |
| HM owned, no compatible Pokémon | Field use fails because no Pokémon qualifies. |
| HM owned, no compatible Pokémon, HMs Overwrite active | The first non-Egg Pokémon is selected. |
| HM owned, party contains only Eggs | Field use fails in normal and HMs Overwrite modes. |
| Egg has a known HM move in migrated or corrupt data | The Egg is not selected automatically, even when the HM item is absent. |
| Egg knows Fly or Flash | No Fly or Flash party action appears. |
| HMs Overwrite active and an Egg is selected | No Fly or Flash action appears and field use cannot start. |
| Only qualifying Pokémon is fainted | Field use succeeds. |
| Unlearned HM use succeeds | Moves, PP, inventory, and Pokémon state remain unchanged. |
| Selected Pokémon qualifies for Fly or Flash | Its party action appears and uses that selected Pokémon. |
| Selected Pokémon is merely compatible but HM is absent | Fly or Flash does not appear. |

The missing-HM and no-qualified-Pokémon cases must assert their distinct failure categories and player messages, not only that traversal failed. HNS script coverage must make the same distinction rather than falling back to one generic obstacle message.

Tests must also verify that every player-available HM unlock ignores badges, that non-HM and latent field moves keep their prior rules, and that automatic Surf checks use the shared eligibility behavior.

Script or integration coverage must exercise at least one contextual obstacle, Surf, Fly, Flash, and HNS Whirlpool. Whirlpool coverage must assert the selected user, the named-user message, and traversal. HNS coverage must confirm that each legacy badge or received-HM permission check has been removed without removing unrelated story state. Carry HM Surf across the first Route 33 to Azalea trigger and confirm that the item remains in the Bag.

Party-management coverage must confirm that HM moves can be replaced, the last Surf user can use the Move Deleter, HM users can be catch-swapped and stored, and no HM-specific release restriction remains.

Build or compile the affected field-move, party-menu, storage, and script objects for Emerald, FireRed, LeafGreen, and HNS. Complete ROM playtesting must cover each build's available HMs with badges unset, including known, unlearned-compatible, missing-item, incompatible-party, Egg-only, fainted-only, and HMs Overwrite cases. The dialogue pass must confirm that no reachable text still teaches the old badge, learned-move, or non-forgettable rules.

## References

- [Field-move registry](../../game/src/field_move.c)
- [Script HM-user resolution](../../game/src/scrcmd.c)
- [Party actions and field-move callbacks](../../game/src/party_menu.c)
- [Automatic Surf eligibility](../../game/src/field_player_avatar.c)
- [Automatic terrain triggers](../../game/src/field_control_avatar.c)
- [Shared field-move scripts](../../game/data/scripts/field_move_scripts.inc)
- [HNS field-move scripts](../../game/data/scripts/field_move_scripts_hns.inc)
- [HM forgetting configuration](../../game/include/config/pokemon.h)
- [Catch-swap configuration](../../game/include/config/battle.h)
- [Storage release restrictions](../../game/src/pokemon_storage_system.c)
