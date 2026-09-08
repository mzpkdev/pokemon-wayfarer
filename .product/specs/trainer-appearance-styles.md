# Trainer appearance styles

PRD: [Trainer appearance styles](../prds/trainer-appearance-styles.md)
Implemented: No

## Scope

Replace Wayfarer's initial boy/girl choice with six appearance styles and carry
that choice through every local-player rendering surface. This spec owns the
intro picker, appearance identity, persistence, graphics lookup, required asset
completion, and acceptance checks. Starting-origin behavior remains owned by
[Wayfarer regional start choice](wayfarer-regional-start-choice.md).

The feature is guarded by `IS_WAYFARER`. Standalone HNS, FRLG, and Emerald keep
their existing intro, gender selection, assets, and rendering behavior. Changing
appearance after starting a game, independent pronoun selection, clothing
customization, and multiplayer protocol redesign are outside this release.

## Behavior

### Selection and controls

Oak asks `What do you look like?` at the existing appearance-selection point,
before naming. Replace the boy/girl question and menu only in Wayfarer. Preserve
the surrounding introduction, naming, challenge setup, origin choice, and final
send-off order.

| Label | Character and source art | Proposed saved ID | Existing gender value |
| --- | --- | --- | --- |
| Style 1 | Gold, HNS | `APPEARANCE_GOLD = 1` | `MALE` |
| Style 2 | Kris, HNS | `APPEARANCE_KRIS = 2` | `FEMALE` |
| Style 3 | Red, FRLG | `APPEARANCE_RED = 3` | `MALE` |
| Style 4 | Leaf, FRLG | `APPEARANCE_LEAF = 4` | `FEMALE` |
| Style 5 | Brendan, Emerald | `APPEARANCE_BRENDAN = 5` | `MALE` |
| Style 6 | May, Emerald | `APPEARANCE_MAY = 6` | `FEMALE` |

The labels and order are fixed. Display the highlighted character's trainer
portrait and a small animated walking sprite together. Names and source games
in the table identify assets for implementers; they are not extra menu labels.

Use one vertical six-row list beside the preview, within the GBA's 240 by 160
viewport. Reserve the existing bottom dialogue area. A proposed layout uses
8-pixel-aligned windows: list at x=8, y=8 with a 72 by 96 content area; portrait
within x=104..167, y=8..71; walking sprite centered near x=192, y=80. Adjust exact
coordinates to the existing frames and sprite dimensions after rendering, while
keeping all six rows, cursor, both previews, and dialogue unobscured. Use the
existing menu font and styling. Do not shrink the labels or add scrolling.

- A fresh new-game flow highlights Style 1. Up/Down move one row and wrap between
  Style 1 and Style 6. Left/Right do nothing.
- A confirms the visible style and advances into the existing naming flow. B
  leaves the picker open, matching the current noncancelable choice.
- Update portrait, walking sprite, palette, and highlighted label as one visible
  selection. During a preview transition, suppress confirmation until both
  previews match the candidate. Consume the transition's input; a held or queued
  A must not confirm a different style when animation finishes.
- If name rejection returns to appearance selection, retain the last confirmed
  style and restore both previews. Naming and challenge callback round trips,
  the final trainer display, and the shrink transition must preserve that style.
- Fast-intro settings still expose this mandatory choice. Continue skips it.
  Abandoning setup and beginning another new game resets to Style 1.

Source inspection baseline: `1e2c5c3122fe89eaa075241f716b842f832a9d47`.
The existing entry points are `Task_NewGameHnsSpeech_WaitToShowGenderMenu`,
`Task_NewGameHnsSpeech_ChooseGender`,
`Task_NewGameHnsSpeech_SlideOutOldGenderSprite`,
`Task_NewGameHnsSpeech_SlideInNewGenderSprite`, and
`NewGameHnsSpeech_ShowGenderMenu` in
[oak_speech_hns.c](../../game/src/oak_speech_hns.c). Their current two-value task
state cannot serve as the six-style identity. Audit every player-sprite restore
in this file, including returns from naming and challenge menus. Put the new
question in [Oak's text](../../game/data/text/oak_speech_hns.inc) without changing
standalone HNS dialogue.

### Identity and save lifecycle

Add a proposed `u8 playerAppearanceId` to the Wayfarer-owned persistent state in
[global.h](../../game/include/global.h), currently
`SaveBlock3.wayfarerHoenn` / `WayfarerHoennPersistentState`. Zero is invalid or
unconfirmed. The explicit IDs above are stable and must never depend on a menu
index, trainer-picture enum, object-graphics enum, or active region.

Keep `SaveBlock2.playerGender` with its current meaning and width. On appearance
confirmation, assign its value from the table. Existing dialogue substitutions,
name suggestions, rival choices, and story branches continue to use that gender.
The feature introduces no separate pronoun question. Appearance selection does
not alter starter options, challenge settings, origin, map version, or regional
progress. All six styles are available for both Johto and Hoenn origins and
remain identical after travel through Johto, Kanto, and Hoenn.

Use a dedicated pending new-game appearance context in RAM, separate from saved
state and transient menu tasks. It contains candidate ID, confirmed ID, and
confirmation state. Reset it explicitly at fresh setup entry, initialize the
candidate to Gold, and mark it unconfirmed. Do not source it from an existing
save. It survives naming callbacks and the reset performed by `NewGameInitData`.

Before `ClearSav3()` in [new_game.c](../../game/src/new_game.c), capture and
validate the confirmed pending appearance alongside the confirmed origin. After
`WayfarerInitPersistentState()` and shared defaults, write the captured appearance
and its mapped gender before origin setup or local-player rendering. Shared
persistent initialization may assign Gold as its valid default, but must not
silently satisfy the intro's confirmation requirement. Test/debug launchers
must supply an explicit valid appearance and origin. Failed validation must
stop initialization before clearing the save.

Extend the origin preflight in `CB2_NewGame` in
[overworld.c](../../game/src/overworld.c) to validate the confirmed appearance
as well. On failure, return to the title through the existing failed-origin
path before calling `NewGameInitData` or entering map setup. An early return
inside the currently `void` `NewGameInitData` alone is insufficient: its caller
must not continue into the opening after failed initialization. Test missing,
zero, and unregistered pending IDs at this caller boundary.

Normal reads after initialization use the saved appearance. Only intro and
naming preview paths use the pending context before that point; make this
choice explicit in their call sites rather than testing whether a task happens
to exist. Continue and regional initialization must never reset the appearance.

Extend `WayfarerPersistentStateIsValid` in
[wayfarer_persistence.c](../../game/src/wayfarer_persistence.c) to reject an
unregistered appearance ID before rendering or indexing descriptor tables.
Treat an inconsistent saved appearance/gender pair as invalid current-version
data. Follow the existing incompatible-save path; do not infer a character from
region or silently substitute another character. Bump Wayfarer's `SAVE_VERSION`
in [save.h](../../game/include/save.h) for the layout change and keep standalone
versions unchanged. There is no migration for prerelease saves.

### Appearance lookup and ownership

Introduce a proposed `wayfarer_appearance.h` / `wayfarer_appearance.c` module
with a registered descriptor for each ID. Each descriptor supplies the legacy
gender, front portrait ID, battle back picture ID, and graphics/palette metadata
for every supported avatar state. Proposed API names below describe the required
boundary; they are not existing symbols:

| Proposed API | Contract |
| --- | --- |
| `WayfarerGetAppearanceProfile(id)` | Return a descriptor for a registered ID; reject all other values. |
| `WayfarerGetPlayerAppearanceId()` | Read the validated persistent local identity. |
| `WayfarerGetConfirmedPendingAppearance()` | Read the confirmed setup identity without consulting an old save. |
| `WayfarerGetAppearanceGraphicsId(id, state)` | Resolve a complete, state-compatible graphics descriptor. |
| `WayfarerGetAppearanceFrontPic(id)` / `WayfarerGetAppearanceBackPic(id)` | Resolve the selected character's existing or normalized trainer assets. |

Keep object graphics IDs as `u16` throughout descriptors, return values, and
call sites. The appearance ID is a separate `u8`; it must not narrow the larger
object graphics namespace. Check trainer-picture widths against their enums too.

Keep explicit-ID lookups usable by the picker. Runtime local-player wrappers
read the saved ID. Preserve gender-only or explicit actor APIs for rivals, NPCs,
remote players, recorded partners, and arbitrary trainer rendering; callers must
state whether they render the local player. In particular,
`GetRivalAvatarGraphicsIdByStateIdAndGender` currently delegates to the player
gender helper in some builds. Globally changing that helper would replace rival
art with the user's appearance.

`GetPlayerAvatarGraphicsIdByStateId` and `InitPlayerAvatar` in
[field_player_avatar.c](../../game/src/field_player_avatar.c) are local-player
integration points. Audit `GetPlayerAvatarGraphicsIdByStateIdAndGender`,
`GetPlayerAvatarGenderByGraphicsId`, `GetPlayerAvatarStateTransitionByGraphicsId`,
`sPlayerAvatarGfxIds`, and `sPlayerAvatarGfxToStateFlag` as separate contracts.
Do not recover movement state by choosing the first matching graphics ID when
multiple states share a sheet. Preserve actual avatar state or use distinct
state descriptors so Acro Bike cannot turn into Mach Bike and underwater cannot
turn into surfing.

### Assets and animation coverage

Existing character art is under
[graphics/object_events/pics/people](../../game/graphics/object_events/pics/people)
in `gold`, `kris`, `red`, `leaf`, `brendan`, and `may`. Existing front and back
trainer IDs also cover all six characters. Asset presence alone does not prove
that every engine animation state is compatible.

Provide a checked manifest for all six styles covering normal walking/running,
Mach Bike, Acro Bike and tricks, surfing, underwater, field-move poses, fishing,
watering, and VS Seeker when available. For each combination, record graphics
and palette symbols, frame dimensions/counts, animation tables, and whether the
sheet is reused or authored. Validate every referenced frame index and expected
animation before accepting a reused sheet.

[Avatar graphics constants](../../game/include/constants/event_objects.h)
currently alias FRLG Mach/Acro to one bike sheet, underwater to surf, and watering
to field-move graphics. These aliases are audit targets, not proof of support.
Author or normalize Red and Leaf assets for missing states, including correct
Acro tricks, underwater movement, and watering poses wherever their current
sheets fail the state's animation contract. Keep the selected character in every
state. Substituting Gold/Kris or Brendan/May for an unsupported Red/Leaf action
is not acceptable. Reuse compatible same-character art only after verification.

### Required rendering audit

Audit local-player display paths beyond the field avatar. Record each affected
call site and whether it uses persistent appearance, pending appearance, or an
explicit remote/NPC identity. Shared functions must retain their other callers.

| Surface | Source anchors and required behavior |
| --- | --- |
| Naming | `NamingScreen_CreatePlayerIcon` in [naming_screen.c](../../game/src/naming_screen.c) uses pending appearance during setup and saved appearance for a local-player naming entry after setup. Its current call to `GetRivalAvatarGraphicsIdByStateIdAndGender` must be separated from real rival callers. Keep rival, Pokémon, and box icons unchanged. |
| Field palette | `LoadPlayerObjectEventPalette` in [event_object_movement.c](../../game/src/event_object_movement.c) and local calls in [field_effect.c](../../game/src/field_effect.c), [oras_dowse.c](../../game/src/oras_dowse.c), and [surfable.c](../../game/src/surfable.c) use matching character palettes. Check reflections and auxiliary surf/dive/fly graphics. |
| Battle back/front | [battle_controller_player.c](../../game/src/battle_controller_player.c) uses the selected back sprite, palette, animation, and any local front portrait. Preserve actor-specific behavior in [battle_controller_recorded_partner.c](../../game/src/battle_controller_recorded_partner.c). |
| Shared front mapper | `PlayerGenderToFrontTrainerPicId_Debug` and its wrapper in [trainer_pokemon_sprites.c](../../game/src/trainer_pokemon_sprites.c) currently map gender/build. Add local appearance resolution without hijacking explicit trainer/gender callers. |
| Trainer Card | [trainer_card.c](../../game/src/trainer_card.c) currently selects `sTrainerPicFacilityClass[cardType][gender]`. Local cards show the chosen portrait with the correct palette and bounds; card edition/layout alone must not change identity. |
| Hall of Fame and facilities | Audit [hall_of_fame.c](../../game/src/hall_of_fame.c), [hall_of_fame_frlg.c](../../game/src/hall_of_fame_frlg.c), and [battle_dome.c](../../game/src/battle_dome.c), including later record playback and any stored local portrait identity. |
| Other UI | Search local gender/build-based trainer displays, including intro restoration, menu portraits, and bag rendering. Character depictions follow appearance; an existing gender-based UI color theme may retain its legacy gender behavior. |

Keep link and recorded formats unchanged. When an incoming remote record has
only legacy gender/edition data, render it through the existing legacy mapping;
it must never borrow the current local appearance. Outgoing legacy records
continue carrying their existing fields, so remote appearance fidelity is not
promised by this feature. A locally stored display record that needs the local
character identity must retain or resolve that identity explicitly rather than
assuming a two-entry gender table.

In a live link battle, the local player's own controller still uses the saved
appearance. `PlayerGetTrainerBackPicId` currently sends `BATTLE_TYPE_LINK`
through `LinkPlayerGetTrainerPicId(GetMultiplayerId())`; split that local path
from remote actor resolution. Do not round-trip the local appearance through
gender/version-only link data. Recorded battles retain their existing format
and legacy rendering when they lack appearance identity, as allowed by the PRD;
never reinterpret an arbitrary recorded actor as the current save's player.

### Implementation and acceptance

Implement the appearance registry and save lifecycle first, then the picker
and local-rendering integrations, and complete the asset manifest and missing
art before acceptance. A picker-only change does not satisfy this spec.

| Area | Required evidence |
| --- | --- |
| Picker | Capture all six rows/previews at 240 by 160; exercise wraparound, B, held directions, rapid changes and A during transitions; confirm the displayed style is the saved style. |
| Callback lifecycle | Name rejection, naming return, challenge return, fast intro, abandoned setup followed by a new flow, and final shrink retain or reset selection as specified. |
| Initialization | Exercise all 12 style/origin combinations; assert saved ID, mapped gender, expected opening, unchanged challenge settings, and no appearance reset during origin setup. |
| Persistence | Save/reload each style; Continue omits selection. Invalid IDs and mismatched gender are rejected without out-of-bounds reads; obsolete prerelease saves follow version rejection. |
| Field state | For every style, inspect walking/running in all directions, both bikes and Acro tricks, surf entry/exit, dive/surface, fishing, watering, field moves, and VS Seeker where reachable. Check palettes, frame bounds, reflections, transitions, and restoration to the previous state. |
| Rendering | Verify naming icon, intro portraits, local Trainer Card, battle front/back animations, Hall of Fame and facility records against the chosen character. Explicit NPC/rival/remote records retain their own graphics. |
| Link boundary | In a live link battle, verify all six styles for the local player while remote opponents/partners use their own legacy data. Playback of legacy recorded battles must not borrow the current save's appearance. |
| Regions and story | Start each style in both origins, travel through Johto/Kanto/Hoenn and back, save/reload away from home, and verify appearance remains fixed. Verify existing gender-linked naming and rival/story branches for all six mappings. |
| Build isolation | Build Wayfarer and supported standalone HNS/FRLG/Emerald targets serially. Native menus and rendering retain their existing behavior without references to Wayfarer-only state. |
| Resource limits | Record ROM size and asset delta against the accepted baseline; the result must fit the 32 MiB ROM and current save-block budgets. Check intro sprite/window allocation, VRAM/palette use, and cleanup through repeated preview changes and callback returns. |

Add focused mechanics/state tests for descriptor completeness, ID validation,
gender mapping, pending-to-persistent handoff, and movement-state preservation
when sheets are shared. Extend the existing
[new-game E2E playbook](../../e2e/src/playbooks/new-game-intro.ts) to accept an
explicit style and origin and assert resulting state. The E2E README currently
describes an HNS ROM setup; confirm or add Wayfarer ROM support before claiming
these runs cover Wayfarer. Use actual emulator
captures for UI and animation acceptance; compilation and an asset inventory
alone cannot prove visible correctness. Follow the repository restriction on
serial builds because different map versions share generated files.

This documentation does not implement the feature. Keep `Implemented: No` until
the implementation lands on main with the required state and emulator evidence.

Keep existing production ROM reserve checks enabled when measuring the asset
delta. Fitting the physical ROM limit alone does not waive the project's reserve.
