# FRLG Nugget Bridge on HNS Route 24

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)  
Dependencies: [FRLG Kanto independent story beats](../prds/frlg-kanto-independent-story-beats.md), [Wayfarer runtime foundation](wayfarer-runtime-foundation.md), [Trainer-only story encounters](trainer-only-story-encounters.md), [Trainer party scaling](trainer-party-scaling.md)  
Implemented: Yes

Implemented on `task/frlg-kanto-story-implementation` in [PR #86](https://github.com/mzpkdev/pokemon-wayfarer/pull/86); not yet merged or released. See the [milestone index](frlg-kanto-story-milestones.md) for the bounded delivery scope.

## Scope and map integration

Adapt the FRLG Nugget Bridge challenge to the existing `MAP_ROUTE24_HNS`.
The episode is six direct, optional interactions: five ordered Trainers followed
by the recruiting Team Rocket Grunt. It restores the local challenge, one
Nugget prize, and the recruitment attempt without making any other Rocket
adventure, badge, origin, regional campaign, or Bill event a prerequisite.

This milestone adds Wayfarer-only object events and scripts. It does not add a
map, layout, connection, warp, coordinate trigger, tile, collision, elevation,
tileset, or sprite asset. In particular, no generated `map.bin` is edited.
The map events must be schema-valid `wayfarer_only` additions in
`Route24_hns/map.json`; its static terrain remains owned by Porymap.

The current HNS layout is 30 by 22. An emulator and screenshot audit confirmed
the dry wood bridge at the positions below. Add six fixed, downward-facing
event objects with
elevation zero, `TRAINER_TYPE_NONE`, and sight zero. The specified interaction
tile is directly south of each actor. All six actor and interaction tiles are
validated wood bridge tiles. Elevation zero matches the current HNS Route 24
event convention even though the raw bridge tiles have elevation three.

| Order | Object | Position | South approach | Overworld graphic |
| ---: | --- | --- | --- | --- |
| 1 | Cale | `(17,19)` | `(17,20)` | `OBJ_EVENT_GFX_BUG_CATCHER_HNS` |
| 2 | Ali | `(19,16)` | `(19,17)` | `OBJ_EVENT_GFX_LASS_HNS` |
| 3 | Timmy | `(19,13)` | `(19,14)` | `OBJ_EVENT_GFX_YOUNGSTER_HNS` |
| 4 | Reli | `(17,12)` | `(17,13)` | `OBJ_EVENT_GFX_LASS_HNS` |
| 5 | Ethan | `(17,6)` | `(17,7)` | `OBJ_EVENT_GFX_CAMPER_HNS` |
| Recruiter | Rocket Grunt | `(19,3)` | `(19,4)` | `OBJ_EVENT_GFX_ROCKET_M_HNS` |

The events must be direct manual interactions with `TRAINER_TYPE_NONE` and
sight zero. They must not use coordinate triggers, player movement, automatic
facing, or an event lock while the player merely travels. Keep the centre
bridge lane at `x=18` free of new Nugget Bridge objects; the existing Machine
Part scene retains its separate reserved use below. The existing Fisherman
occupies `(19,12)` and must remain there. The retained normal route is
`x=18`: in HNS
state 3, northbound movement from `(18,12)` invokes the existing Machine Part
scene and advances it to state 4. In state 4, the retained scene actors occupy
`x=16, y=7..9`, while `x=18` remains physically traversable north. The six
new actors do not occupy that centre route. The challenge neither changes the
trigger nor requires a bypass around the Fisherman or the existing scene;
after the retained scene finishes, ordinary passage continues. Completing the
challenge never unlocks travel.

Reserve the existing Machine Part scene and its movement space exactly as it
is. Do not place new actors, coordinate triggers, or scripted movement in
`x=16..18`, `y=7..11`, or change terrain there. The player may stand at
Ethan's interaction tile `(17,7)` during ordinary free movement. Preserve the
HNS actors at `(16,7)`, `(16,8)`, and the original
Rocket's initial `(18,8)` and scene `(16,8)` positions, its three coordinate
triggers at `(16,11)`, `(17,11)`, and `(18,11)`, or its movements. Preserve
`VAR_KANTO_ROCKET_STORY_STATE`, `TRAINER_GRUNT_31_HNS`,
`FLAG_HIDE_CERULEAN_CAPE_ROCKET`, the Machine Part reveal, and all three HNS
scene states. The bridge Rocket is a separate local recruiter and may remain
available before or after every one of those scenes and after Giovanni's finale.
Its dialogue must not claim a global Rocket outcome.

Keep all eight declared Route 24 object events: the original Rocket, Abra, Man,
Woman, day Surskit, night Spoink, berry tree, and Fisherman at `(19,12)`.
Preserve their positions, flags, behavior, scripts, day/night selection,
connections, and the map's total 14 declared object events after the six
additions, below the 15 non-player object-event capacity. Route 24 has no
warps, and its down connection to `MAP_CERULEAN_CITY_HNS` and up connection to
`MAP_ROUTE25_HNS` remain unchanged.

FRLG's Route 24 Camper Shane is an unrelated sixth side Trainer, not one of
the five challengers or the recruiter. He is outside this milestone. Do not add
his FRLG party, event, or reward now, and do not change the unrelated existing
`TRAINER_SHANE_HNS` on HNS Route 42. Standalone FRLG Route 24, including Shane
and its TM45 item, remains unchanged.

## Ordered challenge and reward state

The order is Cale, Ali, Timmy, Reli, then Ethan. Direct interaction with the
first undefeated Trainer in that order offers its source-derived challenge.
Speaking to a later undefeated Trainer first gives a short order-preserving
message that names the next challenger and starts no battle. A defeated Trainer
uses post-battle dialogue. This uses the five individual ordinary Trainer defeat
states as the ordered progression; no new scene variable or shared Rocket
variable is needed.

Every offered Trainer battle first calls
`WayfarerCanStartOrdinaryBattleForScript` before it locks control, faces the
player, or starts a battle. Each uses the ordinary no-party behavior. An empty
party, fainted-only party, Eggs-only party, or party containing only fainted
Pokemon and Eggs does not start a battle or alter challenge state. A party with
at least one non-Egg Pokemon with HP may battle. The trainer script must read
`GetBattleOutcome` after every battle: only `B_OUTCOME_WON` advances the
order. A loss, forfeit, escape, or other non-win leaves the current Trainer
available and every later Trainer unavailable. No non-win may award the Nugget
or permit the recruitment battle.

After Ethan's win, the recruiter becomes eligible. The recruiter resolves the
Nugget prize before its recruitment battle, so receipt does not require a
usable party:

1. If the local prize receipt flag is clear, check normal `ITEM_NUGGET` space
   in the Items pocket. If no entry or existing Nugget stack can accept one
   item, show a make-room message and stop. Do not grant the prize, set the
   receipt flag, present recruitment dialogue, or start a battle.
2. If space exists, grant exactly one `ITEM_NUGGET` and confirm the add
   succeeded. Only then set the receipt flag. An already-owned Nugget does not
   suppress this source's one-copy reward: an existing stack with capacity
   receives one additional Nugget even when the pocket has no free entry.
3. After successful receipt, release the prize transaction's control lock.
   Apply `WayfarerCanStartOrdinaryBattleForScript` with the existing
   `NP_RECRUITER` no-party wording before locking control again, facing the
   player, or presenting recruitment dialogue. A no-party refusal preserves
   the already received Nugget and leaves the recruiter battle retryable.
4. On a usable party, present the Rocket recruitment dialogue and start the
   ordinary Rocket battle. Only
   `B_OUTCOME_WON` completes the recruiter's ordinary Trainer defeat state and
   enables its post-battle dialogue. Any non-win leaves the recruiter present
   and retryable. The prize receipt remains set and is never repeated.

The prize receipt and recruiter defeat are deliberately independent. Travel,
save/reload, a full Items pocket, a no-party refusal after receipt, and a lost
recruiter battle must not duplicate the prize, consume it, or skip the pending
battle. Do not use `FLAG_ROUTE25_GOT_NUGGET`, FRLG
`FLAG_HIDE_NUGGET_BRIDGE_ROCKET`, `VAR_MAP_SCENE_ROUTE24`, or any zero-valued
HNS/FRLG alias. The HNS Route 25 gauntlet and Kevin's existing `ITEM_BIG_NUGGET`
reward under `FLAG_ROUTE25_GOT_NUGGET` remain a separate episode with no shared
flags, item receipt, or prerequisite.

Allocate exactly one Wayfarer-local receipt flag, following the Cerulean
burglary allocation:

| Flag | Name | Meaning |
| ---: | --- | --- |
| `0x4C3` | `FLAG_NUGGET_BRIDGE_NUGGET_RECEIVED_HNS` | This episode's one `ITEM_NUGGET` was successfully granted. |

Define the flag as `0x4C3` only in Wayfarer and zero in other products, using
the established HNS-content flag convention. It is set only after the item add
succeeds. The existing appended Trainer defeat state holds every battle result;
no new save field, migration, or global item policy is required.

## Trainer identity and scaling

Append the following six Wayfarer Trainers after the Cerulean burglary Grunt's
compact slot 695 and runtime ID 1549. Preserve the source class, portrait,
gender metadata, encounter music, single-battle type, `Check Bad Move` AI, and
zero-IV authored normal parties. All six are ordinary scripted Trainers and
use the existing `ORDINARY` Trainer Rating scaling policy. None is an objective
boss exclusion and this milestone does not alter shared scaling rules.

| Compact slot / runtime ID | Wayfarer symbol | FRLG source | Authored normal party |
| --- | --- | --- | --- |
| 696 / 1550 | `TRAINER_NUGGET_BRIDGE_CALE_HNS` | `TRAINER_BUG_CATCHER_CALE` | Caterpie 10, Weedle 10, Metapod 10, Kakuna 10 |
| 697 / 1551 | `TRAINER_NUGGET_BRIDGE_ALI_HNS` | `TRAINER_LASS_ALI` | Pidgey 12, Oddish 12, Bellsprout 12 |
| 698 / 1552 | `TRAINER_NUGGET_BRIDGE_TIMMY_HNS` | `TRAINER_YOUNGSTER_TIMMY` | Sandshrew 14, Ekans 14 |
| 699 / 1553 | `TRAINER_NUGGET_BRIDGE_RELI_HNS` | `TRAINER_LASS_RELI` | Nidoran M 16, Nidoran F 16 |
| 700 / 1554 | `TRAINER_NUGGET_BRIDGE_ETHAN_HNS` | `TRAINER_CAMPER_ETHAN` | Mankey 18 |
| 701 / 1555 | `TRAINER_NUGGET_BRIDGE_ROCKET_HNS` | `TRAINER_TEAM_ROCKET_GRUNT_6` | Ekans 15, Zubat 15 |

Use the existing HNS Bug Catcher, Lass, Youngster, Camper, and male Rocket
overworld graphics. This is a visual adaptation only: preserve each source
Trainer's class, portrait, encounter music, and gender metadata, including the
FRLG Lass source records' male metadata with female encounter music. Do not
make a FRLG object graphic Wayfarer-available or import an asset for this
milestone. Set
`TRAINERS_COUNT_NUGGET_BRIDGE_WAYFARER` to 6 and extend the accumulated
Wayfarer count to 1556. Add range assertions proving that Cale immediately
follows `TRAINER_CERULEAN_BURGLARY_GRUNT_HNS` and that the Rocket's runtime ID
plus one equals `TRAINERS_COUNT_WAYFARER`. The new records use existing
appended-HNS defeat storage and do not enter Hoenn's fixed runtime bank.
Standalone HNS remains at its current Trainer count and emits none of these
records or objects.

## Validation and budget

Add focused static and generator coverage for the 30 by 22 Route 24 layout,
each exact actor and south interaction tile, dry bridge behavior, elevation
zero, manual interaction configuration, the new-object-free `x=18` lane,
object capacity, and the retained `x=18` connection path with the new actors
present. It must prove that all eight existing Route 24 objects, including the
Fisherman at `(19,12)`, day/night actors, berry, connections, Machine Part
actor and trigger lane, scripts, variables, and map binary hashes remain
unchanged. It must prove state-3 northbound movement at `(18,12)` invokes the
unchanged Machine Part scene, transitions to state 4, and then permits
northbound travel on `x=18`; no new Nugget Bridge state or object may change
that sequence. Compare standalone HNS preprocessing and generated Route 24
output with fixed baseline `7a70900126`, and prove that standalone FRLG source
maps and scripts are unchanged.

Native tests and generator audits must cover runtime IDs 1550 through 1555,
total count 1556, compact-to-runtime translation, exact source-party parity,
`ORDINARY` classification, and separate Trainer defeat storage. Exercise the
full order, each wrong-order interaction, completed-Trainer dialogue, every
non-win retry, and each no-party shape: empty, fainted-only, Eggs-only,
fainted-and-Egg-only, and a mixed party with one usable Pokemon. Prove the
recruiter's prize transaction occurs before its party guard; a full Items
pocket leaves both prize and recruitment pending; freeing space permits exactly
one prize; and a pre-owned Nugget stack gains exactly one source-local copy when
it has capacity. Cover save/reload before the prize, after prize before battle,
and after a lost recruiter battle. Confirm a recruiter loss never duplicates the
Nugget and that the Route 25 Big Nugget and `FLAG_ROUTE25_GOT_NUGGET` remain
unchanged.

Emulator coverage must visually confirm all six actors at the stated positions,
their south interaction tiles, that no new bridge actor occupies `x=18`, the
retained Fisherman at `(19,12)`, and both map connections. In state 3, walk
north from `(18,12)`, exercise the existing Machine Part trigger through its
state-4 transition, then continue north on `x=18`. Exercise states 4 and 5
directly as well, to prove no new actor overlaps or blocks the retained scene
or its ordinary path. The visual check must confirm that every selected tile is
dry, walkable in its intended direction, and renders at elevation zero over the
elevation-three bridge. Run focused map, story, ordinary-Trainer, item-pocket,
scaling, and relevant Machine Part regression checks, then obtain native critic
review. Record the incremental release-ROM size and remaining physical free
space after the milestone; do not silently cut content or begin unrelated
optimization if the reserve is insufficient.

## References

- [FRLG Kanto story conflicts](../research/frlg-hns-kanto-story-conflicts.md)
- [FRLG Kanto implementation sequence](../research/frlg-kanto-implementation-sequence.md)
- [Machine Part chronology](frlg-kanto-machine-part-chronology.md)
- [Cerulean burglary](frlg-kanto-cerulean-burglary.md)
- [FRLG Route 24 source](../../game/data/maps/Route24_Frlg/scripts.inc)
- [HNS Route 24](../../game/data/maps/Route24_hns/map.json)
- [HNS Route 25](../../game/data/maps/Route25_hns/scripts.inc)
