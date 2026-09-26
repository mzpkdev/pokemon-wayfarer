# Wayfarer Kanto origin opening

PRD: [Pallet Town Kanto origin opening](../prds/wayfarer-kanto-origin-opening.md)
Implemented: No

## Scope

Implement the Wayfarer-only `ORIGIN_PALLET` opening on selected HNS maps. This
specification owns origin registration, entry and initial recovery, Pallet
household framing, Oak's interception, the Kanto starter choice, Blue's first
lab battle, opening-only interference suppression, Oak's Parcel receipt in the
Viridian Mart, and the return delivery to Oak with the FRLG Pokédex and
five-Poké-Ball handoff.

The scope starts after the shared professor introduction has committed the
origin and initialized the new game. It ends immediately after Oak's Parcel is
successfully delivered to Oak and every reward in that lab scene has completed.
The regional-start framework owns the common origin selection and launcher.
Later Kanto specifications own Daisy's Town Map and every subsequent story
beat.

## Behavior

### FRLG dialogue fidelity

Every opening line, branch, and interaction the FRLG opening shows between the
bedroom and Oak's Pokédex and Poké Ball handoff appears verbatim on its HNS
host, in FRLG order, for the Pallet origin. The
[FRLG Pallet opening inventory](../research/frlg-pallet-opening-inventory.md)
is the oracle. Ambient lines a player meets on that path, Oak's in-battle
tutorial for the first Blue battle, and the Viridian Mart Parcel scene are
included. FRLG `{RIVAL}` is shown as `BLUE`, because HNS `{RIVAL}` names the
Johto rival. Only paths FRLG never reaches, such as a full Bag, a failed gift,
or HNS geometry with no FRLG counterpart, keep Wayfarer text.

### Origin profile and build boundary

Register stable origin ID 3 as `ORIGIN_PALLET`. Do not derive this ID from menu
position or `REGION_KANTO`. The profile declares:

| Responsibility | Pallet contract |
| --- | --- |
| Entry region | Kanto |
| Initial map | HNS Red's House 2F, at a safe bedroom warp/coordinate selected during implementation |
| Initial recovery | HNS Red's house, using a validated recovery warp that can execute before any Pokémon Center visit |
| Opening handler | The persistent Pallet opening state machine defined below |
| Narrative identity | The player occupies Red's FRLG story role; name, gender, and appearance remain the intro selections |
| Initial catalog | Kanto, through the shared origin profile's existing catalog initialization contract |
| Johto/Hoenn scene policy | Use the existing non-native visitor handlers; do not impersonate either stock origin |
| Interim Aqua policy | Regular service is ineligible and Slateport grants no Ticket until later travel work supplies a Kanto policy |

Use the shared profile launcher and validation. Add Pallet to the origin
selection front end without changing the stable IDs of New Bark or Littleroot.
An unrecognized ID must remain invalid rather than falling through to Pallet.

Compile origin registration, dispatch, map adaptations, and persistence only
for `IS_WAYFARER`. Standalone builds must not reference Wayfarer-only Pallet
state or change their existing behavior.

### Appearance and player identity

Do not add another appearance decision inside the opening. Preserve the intro's
selected Gold, Kris, Brendan, or May appearance ID, its matching existing
gender value, name, palette, field graphics, battle graphics, and all other
player presentation. Pallet scripts use ordinary player placeholders and the
matching existing gender dialogue branch where one is needed.

The selected origin changes story relationships only:

- Red's HNS house is the player's home.
- The woman downstairs is the player's mother, not the mother of an absent
  character named Red.
- Oak knows the player as the local child beginning their journey.
- Blue is the player's Pallet rival and uses the fixed Wayfarer character
  identity `BLUE`.

No script may test for a particular appearance to decide whether the player is
the Kanto protagonist.

### Persistent state and ownership

Allocate named Wayfarer-owned Kanto opening state during implementation. Do
not invent numeric flag or variable assignments in this specification. At
minimum, the implementation must distinguish these facts independently:

| State | Meaning |
| --- | --- |
| Opening phase | The next authored scene or terminal handoff state |
| Starter slot committed | One of the three local Kanto slots has been confirmed and cannot change |
| Starter received | The selected/substituted first partner was successfully delivered |
| First Blue battle resolved | The lab battle finished by win or loss |
| Parcel received | `ITEM_OAKS_PARCEL` was successfully placed in the Bag at the Viridian Mart |
| Parcel accepted | Oak received the Parcel and the item is no longer in the Bag |
| Pokédex handoff complete | The shared Pokédex is available without clearing or downgrading existing records |
| Poké Balls received | Exactly five Poké Balls from this scene were successfully delivered |
| Opening complete | All Parcel-delivery presentation and rewards completed; this spec is terminal |

The implementation may encode compatible facts compactly, but it must not
infer receipt from party contents, species, current map, a battle-defeat flag,
or possession of unrelated global features. The local slot is stored
separately from the delivered species so starter-changing challenges cannot
change Blue's branch.

Do not reuse or overwrite:

- Johto's `VAR_STARTER_MON`, Johto committed/received flags, or Silver state;
- Hoenn's starter choice/receipt state;
- `VAR_PALLETTOWN_LABSTATE` or other HNS late-game Oak/Mt. Silver state;
- FRLG map-scene flags that are absent, aliased, or zero-valued in HNS;
- a broad global `openingComplete` flag shared by unrelated origin profiles.

New-game initialization clears and initializes Pallet state once, after shared
defaults, when `ORIGIN_PALLET` is committed. Continue resumes saved state and
never reapplies setup. Map entry and interaction handlers must be idempotent at
every phase.

### State sequence

Use named implementation constants for an equivalent monotonic sequence:

```text
HOME
  -> OAK_INTERCEPTION_AVAILABLE
  -> LAB_STARTER_CHOICE
  -> STARTER_RECEIVED
  -> FIRST_BLUE_BATTLE_RESOLVED
  -> PARCEL_RECEIVED
  -> PARCEL_ACCEPTED
  -> POKEDEX_HANDOFF_COMPLETE
  -> POKE_BALLS_RECEIVED
  -> OPENING_COMPLETE (terminal for this specification)
```

Saving and reloading must preserve the exact committed facts. A handler may
repair staging objects or resume an interrupted presentation from those facts,
but may never move backward, change the starter slot, or repeat a successful
grant. Only successful delivery commits a receipt state.

`PARCEL_RECEIVED` means the return errand is active, not complete.
`OPENING_COMPLETE` requires Parcel acceptance, Pokédex availability, five
Poké Balls, and the authored lab presentation. It must not imply Town Map
receipt or any later rival or campaign progress.

### Home and Pallet entry

Enter on [HNS Red's House 2F](../../game/data/maps/PalletTown_RedsHouse_2F_hns/map.json)
with the chosen player graphics already active. Preserve ordinary bedroom
navigation and harmless HNS furnishing interactions. Do not replay naming,
appearance, gender, challenge, clock, or new-game setup.

Origin-gate the [first-floor household](../../game/data/maps/PalletTown_RedsHouse_1F_hns/scripts.inc):

- before the starter sequence, Mom gives a Kanto-origin home/first-journey
  response consistent with the FRLG opening and never describes Red as an
  absent third person;
- after starter receipt, her interaction remains coherent and does not alter
  the committed choice, grant another partner, or advance later Kanto story;
- visitors from other origins retain the existing absent-Red dialogue and
  behavior.

The existing Daisy-house visitor introduction to Blue must not run for
`ORIGIN_PALLET`, set its introduction state, or direct the player to a later
Viridian Blue role. Preserve unrelated Daisy-house services where their
dialogue is coherent; later Kanto campaign work owns a native replacement for
that introduction.

Initialize both the active recovery destination and the profile fallback to a
safe Red's-house warp before field control. An early recoverable loss must not
fall back to New Bark or Littleroot. Once another valid local healing
destination is established, shared Wayfarer recovery precedence remains
unchanged.

### Oak interception on HNS Pallet

Add an origin- and phase-gated north-exit interception to
[HNS Pallet Town](../../game/data/maps/PalletTown_hns/map.json). It must cover
every walkable northbound path into Route 1 without blocking doors.

While the Pallet origin is in `HOME` or
`OAK_INTERCEPTION_AVAILABLE` and has no received starter:

1. Approaching the north exit locks field input.
2. Oak warns that wild Pokémon live in the grass and the player needs a
   partner, following the FRLG scene's meaning.
3. Oak approaches using movement authored for the HNS coordinates.
4. Oak leads or transfers the player into HNS Oak's lab with a safe staging
   position.
5. The phase advances to `LAB_STARTER_CHOICE` only after the lab scene is
   staged so interruption cannot strand the player between maps.

Before interception, entering the lab manually may show Blue waiting and Oak
absent, but it must not permit starter selection or mutate late-game lab state.
After the interception, a reload or lab re-entry resumes the eligible starter
choice rather than replaying the town escort.

For other origins and for Pallet saves past this scene, the new interception
is inert. Preserve `FLAG_VISITED_PALLET_TOWN` and the selected HNS map's normal
connections.

The HNS southern coast provides an alternate exit that FRLG's opening does not
have. For `ORIGIN_PALLET`, prevent that connection or its port handoff from
leaving Pallet until `OPENING_COMPLETE`. The refusal must be safe and must not
advance a travel, coast, or opening milestone. Release only this temporary
opening gate after the complete Parcel-delivery handoff; other origins retain
the existing coast connection throughout.

### HNS lab staging and starter choice

Adapt [HNS Oak's lab](../../game/data/maps/PalletTown_Lab_hns/map.json) rather
than selecting the FRLG lab map. During the active Pallet opening, origin-owned
object visibility and scripts stage Oak, Blue, the three choices, and any
assistants needed for the scene. For other origins, retain the current HNS Oak
interaction, assistant dialogue, National Pokédex testing behavior, and
late-game badge/Mt. Silver state. After the Pallet origin completes this
opening, use origin-specific post-delivery Oak dialogue without falling through
to the visitor/postgame state machine.

The choice order and local slot identity are:

| Local slot | Default displayed/delivered species | Blue's counter slot |
| --- | --- | --- |
| Bulbasaur | Bulbasaur | Charmander |
| Charmander | Charmander | Squirtle |
| Squirtle | Squirtle | Bulbasaur |

Follow the FRLG interaction pattern: the player inspects a choice, sees its
name/type presentation supported by the existing starter UI, confirms or
cancels, and may inspect another until confirming. A canceled confirmation
does not commit a slot or advance state.

On confirmation:

1. Persist the local Kanto slot once.
2. Resolve the delivered Pokémon through the existing active challenge rules.
3. Attempt delivery through the normal first-partner API.
4. Mark starter receipt only after delivery succeeds.
5. On failure or interruption, retain the committed slot and retry delivery of
   that same choice; do not reopen the three-way choice.

The expected fresh save has room for the partner, but transaction handling
must still be exact-once. Re-interacting after receipt cannot give another
Pokémon. Set shared party-availability/system state only through the same
successful handoff used by the other Wayfarer origins; do not claim Johto or
Hoenn starter receipt.

Blue chooses from the counter-slot column after successful player receipt.
His actual party construction may apply the same supported challenge rules as
other rival parties, but every later consumer reads the committed Kanto local
slot. It must not infer the branch from the player's species or party order.

### First Blue battle

When the player attempts to leave the lab after both starter receipts are
staged, Blue approaches and starts the FRLG first-rival tutorial battle. Select
the roster from the counter branch above. Prevent the battle until the player
has a usable received partner.

Both battle outcomes converge on the authored post-battle exchange and Blue's
exit. Mark the encounter resolved after the battle result returns and before
field control can start it again. A loss is not a retry gate and must not undo
starter receipt or choice. If an existing challenge mode replaces ordinary
loss recovery with its own run-ending policy, preserve that global policy;
otherwise recovery is safe and continuation remains possible from Red's
house/lab state.

After resolution, Oak's opening dialogue directs the player toward Viridian.
Do not award a Pokédex, Poké Balls, Town Map, Parcel, or later rival progress
in this scene.

### Route 1 and Viridian interference suppression

The player uses the normal HNS Pallet-to-Route-1 connection, HNS Route 1, and
normal HNS Route-1-to-Viridian connection. Preserve wild encounters, collision,
warps, time-based encounter setup, signs, and ordinary map visitation state.

While `ORIGIN_PALLET` is in this specification's opening phases, suppress the
sight behavior and battle entry of the two HNS Route 1 Trainers in
[Route 1](../../game/data/maps/Route1_hns/map.json). Hidden objects or inert
opening-safe interactions are both acceptable, but walking through their sight
lines must not start an unrelated battle on either leg of the Parcel errand.
Restore their ordinary behavior after `OPENING_COMPLETE`; do not mark either
Trainer defeated.

Suppress the existing HNS/coastal Blue introduction object and trigger in
[Viridian City](../../game/data/maps/ViridianCity_hns/map.json). Do not set its
introduced/defeated flags, play its dialogue, or identify Blue as Viridian's
Gym Leader as a side effect of the opening. Parcel receipt must not reveal or
advance that scene. Its eventual Kanto-origin treatment belongs to the later
Blue/campaign specifications.

The HNS Viridian Gym's Blue interaction is also a visitor/later-story path and
must not be reachable by `ORIGIN_PALLET`. Prevent this origin from entering or
starting that interaction without setting its introduction, battle, badge, TM,
or Gym state. Do not substitute a Giovanni scene in this specification. The
dedicated Viridian-finale work owns the native Kanto-origin Gym and the future
release of this boundary; other origins retain their existing behavior.

Audit every Viridian interaction reachable during this opening for the same
future-role leak. In particular, give `ORIGIN_PALLET` neutral variants for the
exterior Gym gramps and sign, the Pokémon Center visitor dialogue, and the Mart
NPC that describes the current Gym Leader. Through this specification's handoff
they must not say that Blue leads or runs the Gym, imply the player has met its
Leader, or mutate later Gym/Blue state. Other origins retain their existing
text and branches. The later Viridian-finale specification owns their native
Kanto replacements.

Do not use object visibility alone as authoritative opening progress. Save and
reload must reconstruct the correct staging from origin-owned state. Visitors
from New Bark, Littleroot, or future profiles retain their existing Route 1
and Viridian behavior.

### Viridian Mart Parcel receipt

Add a Pallet-origin branch to the
[HNS Viridian Mart](../../game/data/maps/ViridianCity_Mart_hns/map.json) while
preserving its normal clerk and NPC behavior for visitors and post-opening
states.

The Parcel scene is eligible only when all of the following are true:

- starting origin is `ORIGIN_PALLET`;
- the Kanto starter was successfully received;
- the first Blue battle was resolved;
- the Parcel receipt is not committed.

On the first eligible Mart visit, use a map-entry scene or an unmissable clerk
interaction to reproduce the FRLG beat: the clerk recognizes the player is
from Pallet, explains that Oak's order arrived, asks the player to deliver it,
and attempts to give `ITEM_OAKS_PARCEL`.

Commit `Parcel received` only after the item-delivery command reports success.
If the grant cannot complete, retain eligibility and provide a retry without
replaying or advancing unrelated Mart state. If the item is already present
because an interrupted handoff completed before its receipt marker committed,
reconcile to one receipt rather than granting a duplicate. After commitment,
the clerk uses ordinary/post-receipt dialogue and the entry scene never runs
again.

The successful item fanfare/message and return of field control inside the
Mart advances to the return leg. The clerk and opening guidance direct the
player back to Oak without warping them. Do not grant the Pokédex, Poké Balls,
Town Map, or later Blue progress at the Mart.

### Return to Pallet and Parcel delivery

After `PARCEL_RECEIVED`, the player walks back through HNS Route 1 and enters
HNS Oak's lab. The delivery scene is eligible only for `ORIGIN_PALLET` with the
committed Parcel receipt and an incomplete delivery. It takes precedence over
the visitor/postgame Oak script and must not read or write
`VAR_PALLETTOWN_LABSTATE`.

Reproduce the FRLG delivery sequence on the HNS lab staging:

1. Oak recognizes the errand and accepts `ITEM_OAKS_PARCEL`.
2. Oak acknowledges his order; Blue arrives for this non-battle scene.
3. Oak explains the Pokédex request and gives Pokédex access to the player and
   Blue. If the shared Pokédex is already available in a test or resumed edge
   case, acknowledge it without clearing records, modes, or catalog progress.
4. Oak gives the player exactly five Poké Balls and completes the authored
   catching/Pokédex explanation.
5. Blue leaves after his authored warning about obtaining a Town Map from
   Daisy. That line does not grant the map or mark it received in this
   specification.
6. Commit `OPENING_COMPLETE` only after all owned presentation and rewards
   have completed successfully.

Treat this as a resumable multi-step transaction. Accepting the Parcel,
enabling the Pokédex, and granting the Poké Balls have independent exact-once
receipts. A save, interruption, or failed item grant resumes at the first
unfinished step without restoring a consumed Parcel, replaying Blue's arrival,
clearing Pokédex data, or duplicating Poké Balls. If the Bag cannot accept the
five Poké Balls, keep the handoff pending and offer a retry through Oak; do not
commit `OPENING_COMPLETE`.

After `OPENING_COMPLETE`, Oak uses Kanto-origin post-delivery dialogue and
continues to avoid the HNS visitor/postgame state machine. This specification
does not implement Daisy's Town Map interaction or a later Blue encounter.
Opening-suppressed Viridian Blue/Gym content remains under its later owning
specification rather than being activated by delivery.

Existing separate Red NPCs and encounters outside the selected opening maps
remain unchanged. This specification accepts that temporary duplicate world
identity and does not hide, rewrite, or reconcile it.

### Validation

Add targeted mechanics/state tests and emulator coverage. Validation must
include:

| Area | Required evidence |
| --- | --- |
| Origin | ID 3 resolves only to `ORIGIN_PALLET`; entry region, bedroom spawn, initial Kanto catalog, visited state, and Red's-house recovery are correct. Invalid IDs remain rejected. |
| Appearance | Gold, Kris, Brendan, and May choices retain their selected field/battle graphics and matching gender state through the full opening. No FRLG player sprite is required or selected. |
| Household | Pallet-origin Mom dialogue treats the player as her child; visitor dialogue still describes absent Red. Home interaction and early recovery do not replay setup. Daisy's visitor Blue introduction cannot run or mutate its state for the Pallet origin. |
| Interception | Every northbound HNS Pallet exit path is covered before starter receipt; the southern coast cannot bypass the opening before Parcel delivery completes; manual lab entry, save/reload, and interrupted staging cannot bypass or repeat the escort. Other origins retain the coast connection, and Kanto receives it after the complete handoff. |
| Starters | All three local slots commit once, deliver once, and produce the correct Blue counter. Cancel, reload, failed delivery, and repeat interaction preserve exact-once behavior. |
| Challenges | Each supported starter-substituting setting preserves the selected local slot and correct Blue branch while delivering the configured species. Existing non-starter challenge behavior remains unchanged. |
| Battle | The first Blue encounter cannot begin empty-party, runs once, and reaches the same continuation after win or ordinary loss. Recovery uses the Pallet home when applicable. |
| Route | HNS Route 1 wild encounters and connections work in both directions; both HNS sight Trainers cannot interrupt either Parcel leg; save/reload reconstructs suppression; their normal undefeated behavior returns after `OPENING_COMPLETE`. |
| Viridian | The unrelated exterior and Gym Blue paths cannot run or mutate introduction, battle, badge, TM, or Gym state for the Pallet origin. Exterior, sign, Center, and Mart dialogue does not identify Blue as Gym Leader. Visitor behavior is unchanged. |
| Parcel receipt | Eligible Mart entry presents the clerk scene and gives exactly one `ITEM_OAKS_PARCEL`. Failure/interruption retries safely; reload and repeat entry cannot duplicate it. |
| Parcel delivery | Eligible Oak interaction consumes the one Parcel, stages Blue's non-battle arrival once, enables or preserves the Pokédex, and grants exactly five Poké Balls. Save/reload and failure at every step resume without loss or duplication and never write HNS late-game lab state. |
| Terminal boundary | The player finishes in Oak's lab with the Parcel removed, Pokédex available, and five Poké Balls received. Town Map, later Blue progress, badges, and campaign flags remain untouched. |
| State isolation | No Johto/Hoenn starter state, `VAR_PALLETTOWN_LABSTATE`, HNS late-game Oak state, absent FRLG alias, or unrelated origin profile changes. |
| Dialogue | A full on-foot playthrough on every supported appearance asserts every field message verbatim and in order, with no unasserted message, and every first-battle message, including Oak's tutorial on the win, loss, and naturally fought paths. |
| Build isolation | Wayfarer compiles and passes map/script validation with the new profile. Standalone HNS, Emerald, FireRed, and LeafGreen compile with their existing opening behavior and no Wayfarer-only state reference. |

Run ROM builds that share generated map outputs serially. Record source/manifest
checks separately from emulator acceptance. Keep `Implemented: No` until the
behavior lands on the main branch and all required validation passes.

## References

- [Regional start framework](wayfarer-regional-start-choice.md)
- [Runtime foundation](wayfarer-runtime-foundation.md)
- [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)
- [HNS Pallet scripts](../../game/data/maps/PalletTown_hns/scripts.inc)
- [HNS Red's House 1F scripts](../../game/data/maps/PalletTown_RedsHouse_1F_hns/scripts.inc)
- [HNS Oak lab scripts](../../game/data/maps/PalletTown_Lab_hns/scripts.inc)
- [HNS Route 1 scripts](../../game/data/maps/Route1_hns/scripts.inc)
- [HNS Viridian scripts](../../game/data/maps/ViridianCity_hns/scripts.inc)
- [HNS Viridian Mart scripts](../../game/data/maps/ViridianCity_Mart_hns/scripts.inc)
- [FRLG Pallet opening inventory](../research/frlg-pallet-opening-inventory.md)
- [FRLG Pallet source sequence](../../game/data/maps/PalletTown_Frlg/scripts.inc)
- [FRLG Oak lab source sequence](../../game/data/maps/PalletTown_ProfessorOaksLab_Frlg/scripts.inc)
- [FRLG Viridian Mart Parcel source](../../game/data/maps/ViridianCity_Mart_Frlg/scripts.inc)
