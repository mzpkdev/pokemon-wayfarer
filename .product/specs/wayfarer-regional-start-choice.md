# Wayfarer regional start choice

PRD: [Johto and Hoenn starting regions](../prds/wayfarer-regional-start-choice.md)
Implemented: No

## Scope

Implement one Wayfarer-only professor choice that dispatches to native Johto
or Hoenn new-game initialization. This specification owns the choice, saved
origin, opening scripts, minimum visitor adaptations, initial recovery, and
Hoenn-origin access to the existing Aqua circuit.

The runtime foundation continues to own the composite map catalog, persistent
namespaces, and active-region dispatch. Existing progression specifications
continue to own Trainer Rating, encounters, field moves, and League rules.

This is the regional-start extension anticipated by the Hoenn integration and
League documents. On implementation it supersedes their Johto-only entry
assumption and the blanket exclusion of native Hoenn arrival scripts. Their
visitor rules remain applicable to Johto-origin players, and their existing
maiden-voyage requirement remains applicable to Johto-origin Aqua travel.

## Behavior

### Origin profiles and authoring boundary

Store an origin identity independently of geography. Register the two initial
profiles as `ORIGIN_NEW_BARK` and `ORIGIN_LITTLEROOT`; the first release's menu
labels remain `JOHTO` and `HOENN`. References to Johto-origin and Hoenn-origin
players below mean these specific profiles, not every future origin located
in those regions.

Each profile supplies the following contract through ordinary engine data,
script entry points, and C callbacks where needed:

| Profile responsibility | Contract |
| --- | --- |
| Identity | Stable origin ID and selection label; IDs do not depend on menu order or region enum values. |
| Entry | Initial map/warp and its region, plus a validated initial recovery destination. A house or home identity is optional. |
| Initialization | One-time origin setup after shared defaults, with explicitly owned state and declared regional baseline changes. |
| Opening | Its own script/callback entry and persistent progress, including any acquisition, scenes, and eventual handoff it needs. There is no fixed list or order of stages. |
| Regional integration | Explicit handling of relevant household, professor, rival, starter, and campaign entry points; it can use existing native/visitor handlers or authored alternatives. Geography alone never selects those handlers. |
| Shared services and travel | Its own handoffs and eligibility queries for equipment and travel. These need not depend on a starter or professor scene. |

The shared launcher resolves a confirmed ID, initializes the common world,
applies that profile once, and enters its opening. It must not orchestrate a
universal `house -> Mom -> rival -> professor -> starter` sequence or require
those actors to exist. The Oak menu is the v1 selection front end, separate
from the origin launcher; a later custom flow may replace that presentation
without duplicating the new-game save initialization.

A profile may start with an authored party, arrange acquisition later, omit
a conventional starter, or have no home. Empty-party openings must prevent
ordinary battles until the authored flow supplies a valid battle participant;
they need not fake a professor or starter-received flag to pass engine checks.
Recovery is always required and can be a camp, service, or other safe authored
destination. Optional home/family identity is not the recovery contract.

Persist origin-specific milestones in allocated Wayfarer-owned flags/variables
or a declared save-state extension. They have names and ownership tied to
that scenario; do not reuse Elm/Birch quest flags as generic opening stages.
No global `openingComplete` or `starterReceived` milestone is required to
unlock every custom origin. Each callback checks its own actual prerequisites.
Continue resumes saved map/script state and never reapplies profile setup.

Route shared questions through profile-backed helpers, such as initial
recovery, regional scene handling, and regular Aqua eligibility. Keep the
concrete New Bark/Littleroot policies in their profile handlers rather than
scattering origin-ID or starting-region comparisons across shared systems.
Adding a future origin requires its registered profile, authored content,
state allocation, and integration tests. It must not fall through an
`else = Johto` or `else = Hoenn` default. This is an engine extension boundary,
not a new plugin loader, origin editor, or scripting language.

### Build and presentation

Guard the feature with `IS_WAYFARER`, including runtime origin dispatch inside
shared HNS or Emerald code. Wayfarer remains an HNS-engine build with both
content sets. Selecting Hoenn must not change a build version or engine rule.

Extend `game/src/oak_speech_hns.c` at the professor's return after challenge
setup, before printing `gText_Oak_YourePlayer`. Retain the existing appearance,
naming, and challenge menus. The presentation order is:

```text
Oak's welcome and Pokémon introduction
Appearance -> name -> name confirmation
Challenge setup -> existing challenge acknowledgement
Existing platform transition brings Oak and his Pokémon back
Origin question -> selection -> town confirmation -> travel remark
Existing "{PLAYER}{KUN}, are you ready?" and adventure send-off
Existing player transition, farewell, and shrink into the chosen opening
```

`Task_NewGameHnsSpeech_ReshowProfessorMon` currently restores the professor
and Pokémon and immediately prints `gText_Oak_YourePlayer`. For Wayfarer,
route this point into the new exchange instead. Wait for the sprite fade and
question printing to finish before opening the list. Keep Oak and his Pokémon
visible through selection, confirmation, and the travel remark. Only then
print `gText_Oak_YourePlayer` once and resume
`Task_NewGameHnsSpeech_WaitForSpriteFadeInAndTextPrinter` and the original
send-off. Do not start that task's outgoing fade while a choice is pending.

Leave `Task_NewGameHnsSpeech_WaitForTextAfterChallengeMenu`'s existing
transition back to Oak intact; do not place the question on the player/settings
return screen. Do not replay the welcome or invoke the standalone Birch
speech after selecting Hoenn.

| Step | Required behavior |
| --- | --- |
| Question | Print `Now, tell me...` followed by `Where will your journey begin?` in the existing speech textbox. |
| List | Show `JOHTO`, then `HOENN`. First entry into this list highlights Johto. Up/Down select; A opens confirmation. B keeps the list open without committing. |
| Johto confirmation | Print `Ah, JOHTO! You'll begin in NEW BARK TOWN, then?` and open the existing Yes/No menu after printing finishes. |
| Hoenn confirmation | Print `Ah, HOENN! You'll begin in LITTLEROOT TOWN, then?` and open the same Yes/No menu after printing finishes. |
| Reconsider | No or B restores the question and list with the candidate still highlighted; add no rejection dialogue or earlier speech replay. |
| Commit | Yes records the confirmed pending origin, closes the menu, and prints `Perhaps your travels will take you to other regions, too!` before the existing send-off. |

Use these two-line textbox breaks for the new dialogue, with the existing
font and ordinary text-printer behavior:

```text
Now, tell me...
Where will your journey begin?

Ah, JOHTO! You'll begin in
NEW BARK TOWN, then?

Ah, HOENN! You'll begin in
LITTLEROOT TOWN, then?

Perhaps your travels will take
you to other regions, too!
```

Each blank-separated block is one textbox page; only the selected origin's
confirmation appears. Validate the actual rendered font/window bounds and
Yes/No placement. Retain the existing music, textbox frame, text speed,
sprites, and menu styling, with no separate selection scene or new assets.
Consume each menu selection before opening the next menu so the same A press
cannot choose a region and confirm it. Advance dialogue using the existing
speech controls; pending choices require their defined menu input.

Do not display technical region IDs, initialization state, a difficulty claim,
or the old UI-style `Begin in...` / `You can visit other regions later.` text.
Fast-intro settings may shorten existing presentation but must still expose
this mandatory choice. Resetting or abandoning new-game setup clears the
pending choice. Continue and saved-game recovery never show the list.

### State ownership

Use a dedicated Wayfarer new-game context in RAM to carry the candidate and
confirmed origin across speech callbacks. Reset it when entering a fresh
new-game flow, rather than relying on power-on zeroing. Do not use a menu task
slot that naming/challenge callbacks destroy, or read origin from a previous
save. The normal UI cannot start field initialization without confirmation.
Test/debug new-game entry points must supply an explicit valid origin.

Add a saved `u16 startingOriginId` under the Wayfarer SaveBlock3 state. Reserve
0 as invalid/unconfirmed, assign 1 to `ORIGIN_NEW_BARK` and 2 to
`ORIGIN_LITTLEROOT`, and append stable IDs for later origins. Never renumber
or reuse an assigned ID. Validate saved IDs against registered profiles.
The ID is immutable after new-game initialization. Read starting geography
from its profile using existing `REGION_*` constants, rather than persisting
a redundant starting-region field. Current map region, `currentRegion`, and
visited-region bits remain separate and continue to change during travel.

Remove Wayfarer's unconditional Johto result from `GetRegionVisitedState`.
For Wayfarer, both the Johto getter and setter must use the Johto bit of
`visitedRegions`, as Hoenn already uses its bit. Retain the existing Kanto
flag contract and its current callers. Initialize the bitset to only the
selected profile's entry region after baseline setup; the first real Johto
map entry then marks Johto visited through current-region dispatch. Merely setting the
latent HNS context must not count as visiting Johto. Standalone HNS keeps
its existing unconditional Johto result.

Capture the confirmed value before `ClearSav3()` in `NewGameInitData`, then
write it after clearing/initializing persistent storage, alongside the existing
preservation of challenge settings. Audit any call that resets the full
Wayfarer persistent structure so Continue cannot overwrite the selected ID
with the New Bark default.
Use the repository's save-version policy for the new layout; do not add a
prerelease migration or infer origin from the player's saved map. An invalid
origin in a current-version save is invalid data, not a request to run a new
opening or repair it by resetting regional progress.
Reject that save through the game's corrupt/incompatible-save handling before
field entry. In particular, the current invalid-magic fallback through
`WayfarerInitPersistentStateFromSavedMap` must not invent an origin or clear
the current-version save's state. Keep new-game initialization separate from
Continue validation.

Keep the existing Hoenn choice and receipt state. Add a Johto starter-choice
committed flag and a Johto starter-received flag in the Wayfarer-owned state
or allocated Wayfarer flag namespace. `VAR_STARTER_MON` remains the Johto
choice value used by Silver; the explicit committed flag distinguishes an
unchosen zero from a valid first slot. Hoenn uses `VAR_HOENN_STARTER_CHOICE`
and `FLAG_HOENN_STARTER_RECEIVED`. Region travel never edits either choice.

### New-game initialization order

Perform shared initialization exactly once. Refactor the current ordering
where necessary so later blanket clears cannot erase regional initialization:

1. Capture confirmed origin and challenge settings; clear the ordinary new-game
   save state, party, inventory, records, and event storage as today.
2. Initialize shared Wayfarer state, Trainer Rating, money, PC items, berries,
   and other global defaults once. Preserve the current default money of 3,000.
3. Establish the HNS world baseline once for both origins. Initialize the
   separate Hoenn bank as uninitialized with no starter choice or receipt.
4. Store origin and apply the selected profile below, including the dormant
   Johto pre-Elm state for Hoenn-origin visitors. Native Hoenn baseline writes
   stay in the Hoenn bank. Shared equipment/system flags change only when
   their authored handoffs occur.
5. Set the initial map, current-region state, visited-region bit, and initial
   recovery destination before any map callback can execute. Then enter the
   map and let its authored opening run.

These profile values and shared starting defaults define the two v1 origins.
They do not require future origins to begin at home or with an empty party.
Any later profile's initial grants and regional state adjustments must be
declared and applied once after shared defaults, never through a second reset.

`EventScript_ResetAllMapFlagsHnS` currently runs after `WarpToTruck` and ends
by setting New Bark respawn and resetting berries. Move its state writes
before the selected regional profile and warp, or split those writes from
the legacy wrapper. Its shared item and berry setup still executes once;
do not also repeat it in the common initializer. No late HNS reset may
overwrite Hoenn's home, visited bits, or opening variables.

| State before first map scripts | Johto | Hoenn |
| --- | --- | --- |
| Initial map | `MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_2F_HNS`, existing warp 1 | `MAP_INSIDE_OF_TRUCK`, existing native truck entry |
| Saved current region | Johto | Hoenn |
| Visited regions | Johto only | Hoenn only |
| HNS region context | Johto | Johto as a latent HNS context, without marking it visited |
| Hoenn initialized | False | True, after its native baseline has been installed |
| Home recovery | Existing New Bark home heal location | Existing gender-appropriate Littleroot player-house heal location |
| Party, badges, League clears | Empty party; zero badges and clears | Empty party; zero badges and clears |
| Starter state | Both choices uncommitted; both receipts false | Both choices uncommitted; both receipts false |
| Aqua maiden-voyage state | Existing fresh HNS value | Same fresh value; do not write 8 |

Use source-fixed Hoenn script operands for its baseline, as
`WayfarerHoennEntry_EventScript_InitializeBaseline` already does. Calling the
generic Emerald reset while the current map is HNS can write the wrong bank.
Neither origin initializer may rerun the shared berry initializer or clear
the other region's story state later.

Hoenn's native profile must establish the truck/house/rival pre-arrival state,
not the visitor arrival profile. Reuse baseline code only where its results
match that state; isolate any native-versus-visitor differences. Mark Hoenn
initialized last, after all native baseline writes succeed. Subsequent calls
to `WayfarerPrepareHoennEntry` update arrival region and local recovery only,
preserving the native campaign.

### Native Hoenn content and starter flow

`InsideOfTruck` is currently excluded by
`game/tools/wayfarer_hoenn_content/classification.json`. Include it as required
native-opening content. Include its layout, graphics, step callback, dynamic
exit, scripts, source aliases, and heal-location dependencies in the generated
Wayfarer content closure. Retain existing map IDs; append any genuinely new
catalog entries. Regenerate manifests and audit expectations through the
existing tools rather than editing generated output or weakening exclusions.

Keep the truck arrival, household introduction, clock interaction, rival
meeting, and default native reward order. Run appearance/name/challenge setup
only in the shared professor flow. The household clock is the native clock
interaction; it must not invoke a second new-game reset, clear played time,
or restart global RTC initialization. Check both existing appearance/gender
branches and preserve the selected Wayfarer player graphics throughout.

Dispatch Route 101's rescue on saved origin and local rescue state:

- A Hoenn-origin player without a starter uses the native bag selection and
  first-battle flow. The selection gives one level-5 partner under default
  settings and records the Hoenn choice and successful receipt.
- A Hoenn-origin player resuming after that grant uses the already granted
  partner. Resuming must not permit another grant or a changed choice.
- A Johto-origin visitor retains the existing-party rescue and optional lab
  gift defined by the Hoenn content port.

Adapt `ChooseStarter`/`CB2_GiveStarter` and their callbacks explicitly. The C
callback currently writes `VAR_STARTER_MON`, outside the Hoenn script alias
boundary. For native Hoenn in Wayfarer, write the dedicated Hoenn choice and
receipt state, never the HNS starter variable. The choice is the selected
local slot even if a configured challenge substitutes the actual species.
Keep the existing challenge restrictions and first-battle presentation; do
not create a new challenge mode or permit the menu to silently select an origin.

Commit starter receipt only after successful delivery. Retrying, saving and
reloading at any permitted point, or re-entering the lab must not duplicate
the Pokémon. Native first-battle return keeps the authored continuation: the
Route 101 script heals the party and takes the player to Birch's lab, including
after a loss. `CB2_EndFirstBattle` returns to that script; it does not itself
heal the party. Preserve that native first-battle exception rather than
applying the visitor's non-win retry branch. If an existing challenge ends
the run instead of returning to the script, retain that policy. Never grant
another partner as part of recovery or continuation.

In Birch's lab, native Hoenn follows the acknowledgement/nickname branch for
the already received partner, then continues the local rival/Pokédex story.
It bypasses Wayfarer's visitor starter-choice/gift branch. The Route 103 rival
becomes available from the committed Hoenn choice; all later Hoenn starter
consumers use that same choice. Johto's choice remains unset until Elm's
local choice occurs.

### Johto opening and Hoenn-origin visitors

Johto-origin players keep the existing Mom, Elm, first starter, Mr. Pokémon,
Silver, police, and related local scenes. Update native starter acquisition to
set the Johto committed/received markers at their actual commit points.

For Hoenn-origin players, initialize the dormant Johto opening to the
pre-Elm state without requiring New Bark's Mom interaction: town state 2,
lab state 0, with the corresponding pre-theft NPC visibility. Apply this once
as part of the Hoenn new-game profile after the HNS baseline; do not apply it
on later arrival. Keep unrelated HNS world state at its normal fresh baseline.

Elm's visitor introduction recognizes the existing party and offers the local
choice before starting any story that reads `VAR_STARTER_MON`. A committed
choice advances the same local quest state as the native selection: lab state
2 and town state 3. The optional gift is a separate transaction:

| Situation | Result |
| --- | --- |
| Cancel before choosing | No choice, gift, or quest advancement. The player can leave and return. |
| Confirm a slot | Store Johto choice once; enable its Silver branch and local errand. |
| Accept gift with room | Give the selected level-5 Pokémon using existing party/PC delivery rules; mark received only on success. |
| Decline or no storage room | Preserve the committed choice and quest advancement; leave the gift unreceived. |
| Return for gift | Offer the same selected Pokémon without repeating the intro, changing the choice, or resetting the quest. |

Guard Silver events that require the choice when it is still uncommitted;
they wait without blocking unrelated traversal. Once committed, the existing
Johto starter branch controls every Silver party. Never default it from the
Hoenn starter index, current party, or current region.

Replace uses of the shared `FLAG_SYS_POKEMON_GET` as proof of Elm starter
selection with the appropriate Johto-local committed/received condition in
the opening's lab, starter-ball, and rival scripts. Retain the global flag
for shared party availability. Audit direct Elm interaction as well as the
coordinate-triggered introduction so both reach the same visitor branch.

Hoenn-origin visitors bypass New Bark household intro triggers, including
the upstairs clock setup, before those triggers can move the player, change
the shared clock, or write town/lab state. Keep a visitor-safe
household interaction and any existing healing service; do not start Mom's
savings or family tutorial for a visitor. Johto-origin visitors likewise
retain Hoenn's visitor household behavior. Audit Norman, Mom, Oak, and rival
dialogue that assumes home origin and adapt only affected passages. Shared
Pokédex receipt is idempotent and never clears, replaces, or downgrades records.

### Shared equipment and local recovery

Birch's native starter acknowledgement gives a Hoenn-origin player the shared
Pokégear availability currently granted by New Bark's Mom. Use one shared
equipment helper for the menu/Match Call system flags, callable from both
native handoffs. Region-specific contacts register through their own story
events; Birch must not fabricate an introduction to Silver. Keep the existing
Hoenn running-shoes handoff. Elm's visitor flow does not repeat either gift.

Audit Hoenn's native Pokédex and running-shoes scripts against the shared HNS
system flags after source aliasing. Their handoffs must enable the actual
Wayfarer UI/actions, not just Hoenn-local receipt flags. Conversely, do not
grant Hoenn-local campaign rewards merely because a shared feature is already
available. National/regional catalog behavior remains owned by the Pokédex
specifications.

Initialize home recovery before the first map scripts, even though the truck
later sets its own gender-specific home warp. Verify the home healer and
whiteout cutscene can execute under the HNS engine on Hoenn maps. Early loss
or Teleport must not fall back to New Bark for a Hoenn starter. The empty
last-heal fallback in `overworld.c` must ask the selected profile for its
initial recovery destination (the home for these two profiles) only
when there is no valid established local heal destination. Normal local
recovery and the existing Hoenn whiteout exceptions remain unchanged.

After travel, set destination recovery before returning control, as the Aqua
specification already requires. A normal blackout in Hoenn after an Olivine
or Vermilion visit still uses the established Hoenn local heal destination;
origin is never a blanket override for a valid local heal warp.

### Aqua travel for both origins

Introduce a single Wayfarer predicate for regular-circuit eligibility that
delegates to the selected profile's policy:

```text
CanUseRegularAqua() = GetStartingOriginProfile().CanUseRegularAqua()

New Bark policy:   HNS VAR_SSAQUA_STATE >= 8
Littleroot policy: Hoenn starter successfully received
```

These are the two v1 policies. A custom origin can supply a different
eligibility condition without setting either stock opening's milestone.
Ticket acquisition is likewise selected by the profile's port interaction
policy; the Littleroot rule below is not inferred from its entry region.

Every regular leg also requires `ITEM_SS_TICKET` in the Bag. This predicate
is derived from existing origin/receipt/voyage state; do not store another
mutable circuit-unlocked flag. Implement the HNS voyage read in a C helper
so a Hoenn script cannot alias it into the Hoenn bank. Replace the raw HNS
`0x408B` check at Slateport and the corresponding regular-service checks at
Olivine and Vermilion with the common predicate under `IS_WAYFARER`.

For a Hoenn-origin player with a received starter and no Ticket, the dedicated
Slateport Aqua attendant explains the circuit and grants one free Ticket
before offering departure. A failed item grant leaves the player at the port
and is retryable. If the Ticket is already in the Bag, skip the grant. Do not
consume the Ticket on travel. Canceling departure after accepting the Ticket
keeps it. This is not a general ticket replacement service for Johto starters.

At Olivine, Hoenn-origin players enter the regular destination menu and use
the existing Vermilion route. They never enter the maiden-voyage boarding or
resume branches, even though `VAR_SSAQUA_STATE` retains its fresh value. At
Vermilion they use the existing Slateport route, which recognizes Hoenn is
already initialized. No helper may write maiden-voyage completion, reveal
its quest NPCs, or award its quest rewards to make regular travel work.

Audit other consumers of the maiden-voyage variable that affect required
port access or the wider League journey. For those access checks, use the
same origin-aware travel contract. Keep actual ship quest progression and
optional destination entitlement checks attached to their existing states.
Hoenn-origin maiden-voyage play/replay is excluded from this feature.

Keep existing port maps, attendant positions, travel direction, safe landing
coordinates, and destination heal locations. Do not route through S.S. Tidal,
change its flags or requirements, or set Hoenn Champion state to open travel.

### Integration and verification

The implementation must include an audit of affected script entry points and
generated operands, with source paths and before/after state ownership. Cover
native and visitor profiles separately; compilation alone cannot establish
that a mixed-region script writes the intended bank.

| Area | Required evidence |
| --- | --- |
| Intro | Both choices, confirmation cancellation, callback round trips, fresh setup after abandoning a previous choice, fast intro, and Continue without a prompt. Oak and his Pokémon are visible during the exchange; town confirmation fits the window; the travel remark precedes one uninterrupted send-off; no input carries from list selection into confirmation. |
| New-game state | Both origins, both appearance/gender mappings, preserved challenge settings, zero TR/badges/clears, correct visited bits, correct home, and correct initialized marker. |
| Native starters | All three local choices in each region, exact-once delivery, native first-battle loss continuation versus visitor rescue retry, correct local rival branch, and no writes to the other starter choice. |
| Shared services | Running shoes, Pokégear, first Pokédex, later professor receipt, and pre-Center home recovery work through the actual Wayfarer UI and scripts. |
| Visitor stories | Johto-to-Hoenn rescue still uses the existing party; Hoenn-to-Johto Elm choice can be postponed, declined as a gift, retried with full storage, and reclaimed later without quest rewind. |
| Persistence | Save/reload at permitted opening checkpoints, after starter choice, after local rewards, in another region, and after completing the full circuit; no opening replay or origin change. |
| Travel | Fresh Hoenn reaches Slateport, obtains Ticket, visits Olivine and Vermilion, and returns to progressed Hoenn with zero badges/clears; Johto still completes its maiden voyage and all three regular legs. |
| Recovery | Early first-battle loss, ordinary blackout before first Center, local blackout after each travel leg, Teleport where available, and existing Hoenn special whiteout cases. |
| Progression | Hoenn-origin access to all 24 badges and the existing League circuit, including deferred Elm/Birch story and return journeys; no hidden Johto-origin or maiden-voyage gate. |
| Build isolation | Wayfarer includes truck closure and both runtime profiles, fits its save-block limits and the 32 MiB ROM budget; standalone HNS/Emerald/FRLG retain their existing flow and compile without Wayfarer-only state references. |
| Authoring boundary | A test-only custom profile in the same region as a stock profile can enter a non-house map, use its own milestones and recovery, and resume without invoking Mom, rival, professor, or stock starter setup. Its own travel predicate works with both stock starter/voyage milestones unset. Unknown IDs are rejected, and menu reordering preserves IDs. |

Add targeted mechanics/state tests for origin initialization, helper predicates,
starter isolation, and repeated arrival. Extend the existing E2E new-game
playbook to take an explicit origin and assert the resulting map/state rather
than relying only on a fixed sequence of A presses. Run the appropriate ROM
and Hoenn content/source-boundary audits serially, following the repository's
shared-generated-map build restriction. Validate supported starter-changing
challenge settings for both the displayed choice and the resulting rival slot.

Record source/manifest validation separately from emulator acceptance. Neither
the Hoenn start nor the PRD is implemented until both origins complete the
required opening, travel, and return checks. Keep `Implemented: No` for this
documentation change.

## Open questions

### Task-worktree implementation decisions

The implementation in `task/regional-start-implementation` uses
`wayfarer_origin.h` as the registration and launcher boundary. This section
records its concrete API; it does not change the main-branch implementation
status above. Emulator acceptance is recorded separately from source audits.

- `WayfarerOriginProfile` owns entry map/warp, initial recovery, one-time
  initialization, a field opening callback, four regional scene policies, and
  Aqua eligibility and ticket predicates. Opening presentation is independent
  of the regional rescue policy. The Oak front end maps its two menu entries
  to stable IDs explicitly.
- Scene interception points are Johto household, Johto professor, Hoenn
  household, and Hoenn rescue. `WayfarerDispatchOriginScene` reads the scene
  from `VAR_0x8004`. Native and visitor results select the corresponding stock
  handler. An authored result transfers to the profile's selected script, or
  ends that stock interaction when the callback returns no script. Authors
  own the saved first-contact/retry state and must explicitly yield to a stock
  policy when its prerequisites are established.
- The launcher validates registered IDs, map and warp bounds, entry geography,
  recovery, and required callbacks. It applies initialization only to an
  unassigned save. Test registration is compiled only with `TESTING`; test
  profiles are absent from Oak's menu and release builds.
- Johto choice commitment and successful delivery use
  `FLAG_JOHTO_STARTER_CHOICE_COMMITTED` and `FLAG_JOHTO_STARTER_RECEIVED` at
  HNS flag IDs `0x930` and `0x931`, only in Wayfarer. Hoenn retains its existing
  dedicated bank. A selected local slot remains independent of substituted
  challenge species and the other region's choice.
- Reward scripts own separate receipts for independently successful grants.
  Pokémon and item receipts commit after the delivery API reports success;
  retry uses the committed choice and does not rewind campaign state. Shared
  equipment helpers only set availability flags, and Pokédex acknowledgement
  preserves existing records and unlocked modes. There is no generic reward
  transaction engine or universal starter milestone.
- `WayfarerCanStartOrdinaryBattle` checks the actual usable party. Wild
  encounter generation and Trainer approaches reject empty parties before
  starting scenes. Unexpected forced entry abandons its script and returns
  to validated local recovery without a battle result or a reward. Authored
  openings must still prevent inappropriate access with their own map logic.
- `WayfarerReplaceRecoveryDestination` replaces both active and fallback
  recovery before an opening location becomes unavailable. Ordinary recovery
  retains a valid local destination; only an invalid destination uses the
  saved fallback. New Bark's initial destination uses its actual bedroom
  heal location, rather than the legacy town heal point outside Elm's lab.
- Stock profile initialization selects the matching initial Pokédex catalog
  after the shared reset: Johto for New Bark, Hoenn for Littleroot. Later
  professors, travel, and Continue preserve catalog selection and extensions.
  This supersedes the earlier unconditional Johto default for playable starts
  in the regional Pokédex specification.
- Shared defaults and the explicitly HNS-sourced baseline run before profile
  setup. Hoenn baseline scripts use fixed Hoenn operands. Save version 7
  rejects incompatible prerelease layouts and unknown origin IDs before
  Continue; no origin is inferred from a saved map.

The [paper exercise](../research/custom-origin-framework-exercise.md) originally
identified the questions below. The task decisions above define the engine
interfaces; each future origin still needs authored content, stock-campaign
prerequisite handling, and integration tests. These questions remain review
criteria for that work, rather than additional playable origins in this release:

- What are the named regional scene interception points and handler results,
  including precedence between custom first-contact scenes, visitor handlers,
  and later normal campaign progression?
- Where is empty-party battle prevention enforced, and what happens when an
  unexpected encounter or callback reaches it after reload?
- What transaction and retry contract covers arbitrary grants, including
  independently successful parts of a multi-reward handoff?
- How does a profile replace both its active recovery point and fallback
  before making an opening-only location unavailable?
- How are profiles exposed to selection front ends or restricted to tests,
  and how does each front end confirm a validated ID for the shared launcher?

## References

The [custom origin paper exercise](../research/custom-origin-framework-exercise.md)
tests the profile model with three different flows and records unresolved
general-authoring contracts. Those findings are design follow-ups, not
implemented support or additional v1 origins.

Source inspection baseline: `baa362db65`. Function and script labels below
are navigation anchors; this document specifies changes to their current behavior.

- [HNS professor speech](../../game/src/oak_speech_hns.c): challenge return and final send-off.
- [Oak's existing dialogue](../../game/data/text/oak_speech_hns.inc): the world introduction, name confirmation, challenge acknowledgement, and closing lines.
- [New-game initialization](../../game/src/new_game.c): `NewGameInitData`, `WarpToTruck`.
- [Wayfarer persistence](../../game/src/wayfarer_persistence.c): initialization, validation, and `WayfarerPrepareHoennEntry`.
- [Save state](../../game/include/global.h): `WayfarerHoennPersistentState`.
- [Native starter callbacks](../../game/src/battle_setup.c): `ChooseStarter`, `CB2_GiveStarter`, first-battle callbacks.
- [Starter choices](../../game/src/starter_choose.c): local slots and challenge substitutions.
- [Truck scripts](../../game/data/maps/InsideOfTruck/scripts.inc) and [Hoenn content classification](../../game/tools/wayfarer_hoenn_content/classification.json).
- [Route 101 rescue](../../game/data/maps/Route101/scripts.inc) and [Birch lab](../../game/data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc).
- [Elm lab](../../game/data/maps/NewBarkTown_Lab_hns/scripts.inc), [New Bark household](../../game/data/maps/NewBarkTown_PlayersHouse_1F_hns/scripts.inc), and [Silver's opening](../../game/data/maps/CherrygroveCity_hns/scripts.inc).
- [Slateport Aqua attendant](../../game/data/maps/SlateportCity_Harbor/scripts.inc), [Olivine port](../../game/data/maps/OlivineCity_PortInside_hns/scripts.inc), and [Vermilion port](../../game/data/maps/VermilionCity_PortInside_hns/scripts.inc).
- [Runtime foundation](wayfarer-runtime-foundation.md), [Hoenn content port](wayfarer-hoenn-content-port.md), [Aqua circuit](wayfarer-hoenn-entry.md), and [League circuit](wayfarer-interregional-league-circuit.md).
