# Cerulean burglary on Wayfarer

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)  
Dependencies: [FRLG Kanto independent story beats](../prds/frlg-kanto-independent-story-beats.md), [trainer party scaling](trainer-party-scaling.md), [trainer-only story encounters](trainer-only-story-encounters.md), [Wayfarer runtime foundation](wayfarer-runtime-foundation.md), [compressed map-layout runtime loading](compressed-map-layout-runtime-loading.md)  
Implemented: Yes

Implemented on `task/frlg-kanto-story-implementation` in [PR #86](https://github.com/mzpkdev/pokemon-wayfarer/pull/86); not yet merged or released. See the [milestone index](frlg-kanto-story-milestones.md) for the bounded delivery scope.

## Scope

Adapt the FRLG Cerulean burglary as one independent local episode in the existing
HNS city. The player learns in the adapted House 2 that the household
lost its TM for Dig, confronts the thief in `MAP_CERULEAN_CITY_HNS`, and recovers
`ITEM_TM_DIG`, which is named TM28. The episode is available
without Bill, the S.S. Anne, the Machine Part, the Power Plant, a Gym, a badge,
Kanto origin, a major Rocket investigation, or any League progress. Completing
it does not advance any of those stories.

## House and rear-route adaptation

Use the existing HNS city exterior and its House 2 address. Wayfarer selects
`MAP_CERULEAN_CITY_HOUSE2`, the intact FRLG robbed-house interior, for this
single story interior. Keep its authored rubble, front doors, and rear hole.
Preserve the HNS household's woman `(8,5)`, man `(1,2)`, and Diglett `(2,4)`
in the room, each at elevation 3. All three positions are passable in the
10 by 9 source layout. The man uses the source Hiker's position `(1,2)`
to keep `(4,2)`, the rear hole approach and arrival, clear. Verify all
interaction approaches in the emulator.
Standalone HNS and FRLG keep their original maps, actors, and behavior.

Opt the FRLG map and `LAYOUT_CERULEAN_CITY_HOUSE2` into Wayfarer through
`wayfarer_include`. Add `wayfarer_exclude` filtering to the event generator
and event-ID generator: the two original FRLG actors remain for standalone
FRLG only, while three appended `wayfarer_only` objects supply the preserved
HNS residents. The adult scripts use the local robbery/recovery dialogue;
Diglett retains its original HNS interaction. Talking to residents does not
activate the Rocket, grant a reward, or change story state.

| Transition | Wayfarer destination |
| --- | --- |
| HNS city front warp 2, `(35,22)` | Imported house front warp 1, `(3,7)` |
| Imported house front warps 0, 1, and 2 | HNS city front warp 2 |
| Imported house rear warp 3, `(4,1)` | Appended HNS city arrival warp at `(35,18)` |
| Inspect city wall `(35,19)` facing south from `(35,18)` | Scripted warp to imported house `(4,2)`, below its rear hole |

The source front doorway triggers only at its center tile `(3,7)`. Its
side tiles are ordinary floor despite the retained warp entries; players can
walk sideways to the center and leave normally. Preserve that authored behavior
and redirect all three destination records.

Resolve `wayfarer_dest_map` and a narrow `wayfarer_dest_warp_id` override
consistently. Keep all standalone destinations and indices. The exterior
rear warp is an arrival marker on grass, not a step-triggered doorway.
Its south-facing background event uses elevation 3 to match the player,
explains the hole, then enters the house. The interior hole retains its
normal warp behavior. Neither direction may loop or land in collision.

The HNS exterior currently has a tree between the house and its northern
approach. Append `LAYOUT_CERULEAN_CITY_BURGLARY_WAYFARER`, copying the
original city's blockdata, 58 by 51 dimensions, tilesets, border reference,
and HNS layout version. Select it through a narrow `wayfarer_layout` header
override while retaining the original map ID. Only Wayfarer includes this
layout, with `game_version: "wayfarer"` and `wayfarer_include: true`. The
existing explicit opt-in selects it for Wayfarer and source-version matching
excludes it from standalone HNS/FRLG. Every Wayfarer visit uses it without
a scene or history variable.

Use Porymap with HNS's 640 primary tiles/metatiles and seven palettes to
replace the tree at `(34..35,17..18)` with passable grass, elevation 3.
Select adjacent existing grass after visual inspection. Porymap must author
and save this change. No script outside Porymap, hex editor, or byte patch
may modify the layout binary. Original HNS and FRLG `map.bin` files remain
byte-identical. This opening connects rear arrival `(35,18)` to the existing
northern grass and pavement. Verify the exact changed-cell set and retain
before/after visual evidence. No other city terrain change is authorized.

Append the Rocket at city `(37,23)`, elevation 3, facing left, with
`LOCALID_CERULEAN_BURGLARY_GRUNT`, `OBJ_EVENT_GFX_ROCKET_M`, and no sight
trigger. Interact from `(36,23)` facing right; `(38,23)` is blocked. Leave
both house routes and normal city travel open. The Rocket graphic and
Building FRLG primary tileset are already present. Enable only the selected
BurgledHouse secondary tileset's headers, graphics, and metatiles for this
interior; do not enable unrelated FRLG content.

## Episode behavior

The three persistent states use the new Rocket's ordinary Trainer defeat state
and a separate receipt flag. They are intentionally distinct.

| State | Trainer defeat state | `FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS` | Rocket | Result |
| --- | --- | --- | --- | --- |
| Pending | Clear | Clear | Visible | Direct interaction can start the battle. House residents report the theft. |
| Defeated, reward pending | Set | Clear | Visible | The Rocket returns TM28. It must not start a second battle. |
| Complete | Set | Set | Hidden | The Rocket is gone and House 2 uses recovery dialogue. |

The Rocket's direct interaction checks the canonical Wayfarer usable-party
predicate before locking control, facing the player, or staging a battle. An
empty party, a fainted-only party, an Eggs-only party, and a party containing
only fainted Pokemon and Eggs each receive the ordinary repeatable no-party
refusal. A party with at least one non-Egg Pokemon with HP, including one mixed
with fainted Pokemon or Eggs, may battle. A refusal does not start a battle, set
the Trainer defeat state, grant an item, set the receipt flag, or alter object
visibility. This is a battle-safety guard, not a story, origin, badge, or
regional-progress gate.

With a usable party, run the ordinary single Trainer battle. The success path
must explicitly read `GetBattleOutcome`: only `B_OUTCOME_WON` may enter the
reward transaction. A loss, forfeit, escape, or every other non-win leaves the
Rocket present and retryable, with no receipt or reward change. A won battle
sets only the Rocket's own ordinary Trainer defeat state; it does not set an
HNS city scene, change `VAR_CERULEAN_CITY_STATE`, or touch Machine Part,
Cape, Gym, or major Rocket flags.

After a win, and whenever the defeated Rocket is spoken to, resolve the
transaction in this order:

1. Check space for exactly one `ITEM_TM_DIG` in its actual `POCKET_TM_HM`. A
   full TM/HM pocket with no capacity for that item shows a make-room message,
   leaves the Rocket visible, and leaves the receipt flag clear. Use ordinary
   item-capacity semantics: an existing `ITEM_TM_DIG` stack that can accept one
   more is sufficient even when the pocket has no free entry.
2. If there is space, add exactly one `ITEM_TM_DIG` and verify that the add
   succeeded. A failed add has the same retryable result as a full pocket.
3. Only after that successful add, set the receipt flag, hide the live Rocket
   object, and use the departure and recovery dialogue.

The receipt branch must be repeatable after travel, save/reload, and a
TM/HM-pocket-full handoff. It must never repeat the battle, create a second
copy of this local reward, or let the player lose the reward because the Rocket
was hidden early.

`ITEM_TM_DIG` retains its current consumable configuration, including
`I_REUSABLE_TMS = FALSE`. This episode must not change its item configuration,
add a field use for Dig, or alter the global TM policy.

Use `ITEM_TM_DIG`, whose current item record is named TM28 and belongs to
`POCKET_TM_HM`. Do not use the legacy numeric `ITEM_TM28` source item. Existing
ways to own the Dig TM, including the Hoenn Fossil Maniac's brother and the HNS
National Park item ball, do not activate, complete, or suppress the
burglary before the Rocket is defeated. After a win, prior ownership does not
replace this local one-copy reward: the player receives one additional
`ITEM_TM_DIG` when the TM/HM pocket has room. The receipt flag prevents only a
second copy from this burglary.

## Persistent state and trainer data

Allocate only the following new Wayfarer-local flag in the reserved HNS content
window:

| Flag | Name | Meaning |
| ---: | --- | --- |
| `0x4C2` | `FLAG_CERULEAN_BURGLARY_TM_RECEIVED_HNS` | The post-win local TM28 reward was granted, and the street Rocket must stay hidden. |

Define the flag as `0x4C2` only for Wayfarer and as zero for other products, as
the preceding local story flags do. It is not interchangeable with
`FLAG_GOT_TM28_FROM_ROCKET`, which belongs to the FRLG source and resolves to
zero in HNS, or with HNS item-source flags for Dig. No new saved variable or
save-block field is needed. The ordinary appended Trainer defeat bit records
the battle separately from this receipt flag.

Append one Wayfarer-only trainer after Mt. Moon's compact HNS slot 694:

| Compact slot / runtime ID | New symbol | Source trainer | Exact normal roster | Scaling policy |
| --- | --- | --- | --- | --- |
| 695 / 1549 | `TRAINER_CERULEAN_BURGLARY_GRUNT_HNS` | `TRAINER_TEAM_ROCKET_GRUNT_5` (255) | Machop 17, Drowzee 17 | `ORDINARY` |

Preserve the source trainer's Team Rocket FRLG class, male Rocket Grunt portrait,
male gender, Aqua encounter music, single-battle type, `Check Bad Move` AI, and
zero-IV authored party data. The source roster is an ordinary scripted Trainer,
so it uses the existing ordinary Trainer Rating scaling policy. Do not classify
it as an objective-boss exclusion or change the shared scaling rules.

Set `TRAINERS_COUNT_CERULEAN_BURGLARY_WAYFARER` to 1 and extend the accumulated
Wayfarer count to 1550 after the Anne, Celadon Hideout, and Mt. Moon additions:

```text
TRAINERS_COUNT_WAYFARER = TRAINERS_COUNT_HNS + TRAINERS_COUNT_EMERALD - 1
                        + TRAINERS_COUNT_SS_ANNE_WAYFARER
                        + TRAINERS_COUNT_CELADON_HIDEOUT_WAYFARER
                        + TRAINERS_COUNT_MT_MOON_WAYFARER
                        + TRAINERS_COUNT_CERULEAN_BURGLARY_WAYFARER
```

The slot uses the existing appended-HNS defeat storage. It must not enter
Hoenn's fixed runtime bank. Extend the exact range assertion, trainer-state
test, source-party parity fixture, and scaling inventory and classification
records. Standalone HNS keeps `TRAINERS_COUNT_HNS = 661` and does not emit this
trainer or its event.

## Validation and budget

Static and generator coverage must prove the selected Wayfarer layout, bounded
Porymap-authored changed cells, unchanged source hashes, and passable
Rocket/front/rear approaches using dimensions from `layouts.json`. Generate
standalone HNS, FRLG, and Wayfarer to verify stable existing IDs, source-product
output, closed included warps, event filtering, and three preserved household
actors. Verify BG-event elevation and retain all existing city connections.

Native tests and data audits must cover runtime ID 1549, total count 1550,
compact-to-runtime translation, source-party parity, `ORDINARY` scaling, and
independent defeat state. They must cover no-party refusal for empty,
fainted-only, Eggs-only, and mixed fainted-and-Egg parties, plus a mixed party
with one usable Pokemon that may battle; first battle; all non-win retry
outcomes; a win; post-win no-repeat battle; save/reload in the pending-reward
state; full TM/HM pocket retry; successful TM28 grant; pre-owned `ITEM_TM_DIG`
receiving this separate local copy through an available existing stack; one-time
receipt; live-object removal; and the correct pre- and post-recovery House 2
dialogue. The tests must prove that pre-owned TM28 does not skip the
confrontation or suppress the local reward, including when a full pocket can
still accept the existing TM28 stack, and that neither the Machine Part
chronology nor `VAR_CERULEAN_CITY_STATE` changes.

Emulator coverage must enter and leave the selected house through both routes,
inspect the exterior opening, approach the Rocket from `(36,23)`, and walk
around the encounter. Check both directions without warp loops or blocked
arrivals, the household dialogue and Diglett, and save/reload. Run focused
map, story, trainer, item-pocket, and scaling checks plus the relevant Machine
Part regression. Record incremental production ROM cost and remaining reserve
with the normal budget check enforced. Obtain native critic review before
accepting the milestone.

## References

- [FRLG Kanto story conflicts](../research/frlg-hns-kanto-story-conflicts.md)
- [FRLG Kanto implementation sequence](../research/frlg-kanto-implementation-sequence.md)
- [Mount Moon fossils on HNS](frlg-kanto-mt-moon.md)
- [Celadon Rocket Hideout on Wayfarer](frlg-kanto-celadon-hideout.md)
- [Wayfarer Hoenn entry](wayfarer-hoenn-entry.md)
- [Wayfarer interregional League circuit](wayfarer-interregional-league-circuit.md)
- [FRLG Cerulean Rocket script](../../game/data/maps/CeruleanCity_Frlg/scripts.inc)
- [HNS Cerulean city](../../game/data/maps/CeruleanCity_hns/map.json)
- [HNS Cerulean House 2](../../game/data/maps/CeruleanCity_House2_hns/map.json)
