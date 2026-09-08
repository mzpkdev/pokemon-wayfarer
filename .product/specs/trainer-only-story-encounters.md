# Trainer-only story encounters

PRD: [Trainer-only encounters](../prds/trainer-only-encounters.md)
Implemented: Yes

Status: Implemented for the current supported scene inventory. Scripted-wild
exclusion and the specified no-party guards are settled policy. Future ports
and exceptional defeat rules retain the explicit dependencies listed here;
source availability does not make unported scenes playable.
Implementation status does not claim those future integrations or shipping
balance approval. See the
[implementation validation](../research/trainer-only-implementation-validation.md).

## Scope

Define how Wayfarer handles rivals, local objective guards and public roadblocks
when the player has no usable Pokémon. This companion owns authored encounter
eligibility, deferred scenes, refusal dialogue and progression protection. The
[core encounter spec](trainer-only-encounters.md) owns wild encounters, party
eligibility, ordinary defeat, inventory and centralized blackout/recovery.
Ordinary supported losses retain their existing money charge exactly once. Later
trainer-only retaliation uses central recovery without another money deduction;
this does not authorize changes to excluded challenge, League or authored outcomes.

The [source encounter audit](../research/trainer-only-story-encounter-gates.md)
provides script locations and state writers for every family below, inspected at
`56fb1e68b4`. Its current HNS/Hoenn inventory and future FRLG/Sevii source inventory
are separate implementation scopes. Apply these policies to Wayfarer only.

Regional adventure dependencies remain owned by the
[Johto](../prds/johto-independent-story-beats.md),
[Hoenn](../prds/hoenn-independent-story-beats.md),
[FRLG Kanto](../prds/frlg-kanto-independent-story-beats.md) and
[Sevii](../prds/sevii-independent-story-beats.md) independence PRDs. Public travel
follows the [HNS](../prds/hns-open-world-region-traversal.md) and
[Hoenn](../prds/emerald-open-world-region-traversal.md) traversal PRDs.
This feature does not remove local rescue, item, password, trial or boss predicates.

## Behavior

### Eligibility and encounter policies

Use the core spec's canonical usable-party predicate, currently
`WayfarerCanStartOrdinaryBattle()`. At least one eligible living non-Egg Pokémon
provides protection. Empty, fully fainted, Eggs-only and mixed unusable parties
all take the same refusal/defer path; challenge-dead members do not qualify.

Assign policy to an encounter script/event, including all approaches and rematch
callers, rather than to a trainer class, sprite or character name. Each authored
entry must identify its trigger, narrative eligibility, battle outcome branches,
objective writes, dialogue ID and any retreat/reset staging. Unmapped authored
callers retain their existing safe exceptional behavior until classified; they
must not fall into ordinary bypass just because their trainer sprite is familiar.

| Policy | No usable party | When protection returns |
| --- | --- | --- |
| Ordinary trainer | Suppress sight/approach. Talk uses `NP_ORDINARY`; keep the undefeated actor. | Resume normal sight/talk eligibility after control and trigger cleanup. |
| Deferred rival battle | Suppress off-screen triggers silently. Hide persistent battle-only presence and collision transiently. | Restore the pending chapter when its own narrative prerequisites also hold. |
| Local objective guard | Keep the guard and local objective boundary. Talk or deliberate approach uses its assigned refusal; no battle starts. | Offer the existing encounter while its objective remains pending. |
| Non-battle actor or reward | Evaluate its normal local scene/item prerequisites. No general party refusal. | No party-dependent change unless its specific authored policy requires one. |
| Public roadblock removed by traversal design | Keep the approved public lane open, regardless of party. Any retained local battle has its own policy. | Do not rebuild the campaign roadblock. |
| Explicit exception | Follow the approved tutorial, League, scripted wild or challenge rule. | Do not infer ordinary rival or defeat policy. |

Check eligibility before locking control, spawning an approaching rival,
transforming a disguised guard, substituting a tutorial party, or playing an
irreversible scene animation. Recheck before battle startup if an intervening
menu or script can change party state. Refusal never invokes the emergency
empty-party recovery warp and never launches trainer-only combat against a trainer.

### Deferred rivals and scene recovery

Persistent battle-only rivals disappear without dialogue while unavailable.
Remove their collision too, but do not write canonical hide/completion flags to
achieve temporary suppression. Derive temporary visibility from narrative
eligibility and usable-party state at map load and after relevant party changes.
An actor still needed for a gift or non-battle conversation remains available
for that role; only its battle branch is gated. If that branch is deliberately
selected without protection, use `NP_RIVAL` as a fallback. It is not a message to
show every time the player crosses a suppressed trigger.

A deferred chapter remains at its approved location and in its narrative order,
subject to its owning regional campaign's eligibility and explicit retirement rules.
Crossing the trigger must not simulate either victory or an authored decline,
consume a daily rematch, give the chapter's reward, or move the rival to the next
chapter. Rival state must not share a progression writer with a host adventure
that can finish independently. Split those predicates before enabling deferral.

Completing an occupation or taking a ship must not erase a pending encounter.
Provide a returnable version of its existing location/staging under the regional
story design. If a port cannot preserve that access yet, its rival support is
blocked on the host adaptation; do not silently relocate or complete the chapter.
Only one eligible chapter may own an actor/trigger at a time.

Recovery while standing on a trigger or hidden actor position must not spawn an
actor over the player or immediately lock control. Re-arm coordinate/sight
encounters after the player leaves their activation area and enters again.
Delay overlapping object restoration until its tile is clear; a map reload must
also handle an overlapping saved player position safely. Delayed restoration
never makes a pending rival permanently absent.

When trainer-only item recovery restores a usable party, ordinary trainers already
watching the return position must also allow a safe field return. Defer their
automatic approaches until the player leaves their sight area. Direct talk still
allows an explicit retry; this does not mark a trainer defeated or change rewards.

### Objective boundaries, outcomes and retreat

Protect local objectives with their actual state/item predicates as well as
geometry. A different entrance, suppressed sight check or temporarily hidden
actor cannot grant a password, key, hostage rescue, machine shutdown or boss
reward. Routine grunts become mandatory only where a mapped local security or
reward predicate actually requires their victory. Preserve approved public lanes
around that local boundary and leave an exit/retreat path available.

No-party dialogue is a repeatable refusal: release locks and restore field input
without rewards, chapter writes, actor departure or lasting scene changes. A
coordinate refusal fires once per approach, not every frame while the player
stands still. Re-arm only after leaving its activation area. A safe local approach
tile can be authored where needed, without healing or a recovery warp.

For supported authored battles, inspect the explicit battle outcome before every
success continuation. Code after `trainerbattle` must not implicitly mean victory.
Only the required victory can set trainer-defeated state, issue its earned reward,
remove a guard, open security, advance a chapter or resolve an occupation. Preserve
an existing intentional pre-battle reward order where one is authored; do not
reclassify that reward as a victory prize.

If a caller supports the core spec's field return after loss, stop its success
continuation, restore retryable actor/map staging, release control and show
`NP_RETREAT`. Return to the approach side of a local objective boundary when the
scene moved the player inside it. Any local reposition must be documented for
that scene and keep a walkable exit; it is not a Center warp. Charge the existing
loss consequence once through the core owner. A no-party refusal on the next
approach is separate from the loss cleanup and cannot loop automatically.

Any future field-return adaptation of chained/partner sequences must check party
and outcome between fights. Exhaustion must stop that adapted sequence without
awarding its final reward. Preserve already earned partial victories only where
the authored adventure defines them; do not create new checkpoints implicitly.
Tutorials, League, Celebi and exceptional challenge losses keep their existing
policy until explicitly supported. Classification as
an objective guard settles no-party entry, not every exceptional loss consequence.

### Defeat support by family

The tables below select no-party entry for every listed family. Ordinary single
story battles adopt safe field return only after their explicit outcome branch
and retreat/reset staging meet this spec. The following exceptions keep their
existing defeat behavior until separately adapted:

| Family | Defeat routing |
| --- | --- |
| Cherrygrove Silver #1, Route 103 rival, Oak's Lab rival, Wally tutorial | Preserve authored startup/tutorial policy; no generic loss conversion. |
| Kimono trial, Oceanic Museum grunt sequence, Space Center, Three Island gang, any required consecutive or partner fight | Preserve existing chained/partner outcome policy until explicit adaptation, even though no-party entry is refused. |
| Tohjo Giovanni/Celebi episode | Preserve authored episode loss/recovery policy until explicitly adapted. |
| Victory Road Silver/Wally and Champion | Keep League admission/outcome constraints explicit; only independently supported chapter battles may adopt ordinary field return. |
| Challenge/facility configurations and scripted wild objectives | Preserve core exclusions and approved exceptional outcomes. |
| All unaudited Gym-map callers, including members, leaders, Blue and future Giovanni | Preserve existing defeat routing until individually audited and added to the core spec's caller allowlist. No badge, TM, TR/League or story credit on loss/abort. |
| Other listed single rival/objective trainer battles | Opt into field return per caller only after success-only writes, safe retreat, actor reset and no retrigger loop are implemented and tested. |

At a directly requested excluded tutorial battle, use `NP_ORDINARY` before party
substitution or scene staging. Silently suppressed rival triggers remain silent. At League battle admission, use `NP_LEAGUE`, without changing
other admission rules. No-party refusal applies before entering an excluded
sequence; it does not change what a loss inside that sequence does. A multi-guard objective with independently
started single battles is not automatically a chained battle. Check actual callers.

### No-party dialogue

These are proposed English text resources, not claims about existing strings.
IDs are semantic names; implementation may apply the repository's naming prefix.
Wrap and paginate using the normal dialogue renderer. Ordinary refusals must work for empty, fainted and Eggs-only parties without
claiming ownership or fainting. Hostile characters dismiss or threaten the player;
they do not invite a fair rematch. Reuse the encounter's existing text when it fits
an unresolved objective and no-party refusal. Otherwise write a short variant in
that character's established voice. The hostile resources below are fallbacks,
not replacements for suitable existing dialogue. Refusal grants no passage,
password, reward or attack against the trainer.

For ordinary trainers, `NP_ORDINARY` resolves to one of the four lines below.
Use a stable assignment per trainer encounter, with an explicit dialogue override
where the trainer's personality calls for it. For unassigned ordinary encounters,
use a deterministic index from a stable encounter key into the ordered four-line
pool; rematches share their base encounter's assignment. Do not consume gameplay
RNG, rotate on repeated talk, or store a new dialogue counter in the save. The same
trainer keeps the same line across talks and reloads. Explicit tutorial callers
use the neutral first line directly.

Dialogue selection is separate from encounter policy. Optional evil grunts still
allow ordinary bypass, but use their existing hostile text or faction fallback
instead of a friendly line. Optional Rocket, Aqua and Magma grunts may reuse
`NP_ROCKET_GUARD`, `NP_AQUA_GUARD` and `NP_MAGMA_GUARD` respectively as text
fallbacks only. The resource name does not assign encounter policy or collision;
these trainers remain optional and do not become objective guards.

| ID | Text | Assignment |
| --- | --- | --- |
| `NP_ORDINARY` | Select the assigned line below. | Ordinary trainer talk, including the optional S.S. Aqua sailor. |
| `NP_ORDINARY_NEUTRAL` | "Come back when you have a Pokémon that can battle." | Neutral; first pool entry and direct tutorial fallback. |
| `NP_ORDINARY_RELAXED` | "No Pokémon ready? We can battle another time." | Relaxed; second pool entry. |
| `NP_ORDINARY_EAGER` | "I was hoping for a battle! Maybe next time." | Eager; third pool entry. |
| `NP_ORDINARY_KIND` | "Don't worry about a battle. Take care out there!" | Kind; fourth pool entry. |
| `NP_RIVAL` | "We'll battle when you've got a Pokémon ready." | Fallback battle branch of a rival retained for another interaction; hidden/off-screen rivals remain silent. |
| `NP_ROCKET_GUARD` | "You've got no Pokémon to stop us. Get lost!" | Rocket security, rescue and occupation guards listed below. |
| `NP_DISGUISED_DIRECTOR` | "This is no place for you without a Pokémon that can battle. Leave." | Fake Director before transformation; do not expose the disguise in the refusal. |
| `NP_AQUA_GUARD` | "You're in over your head. Beat it!" | Aqua local objective guards. |
| `NP_MAGMA_GUARD` | "Stay out of our way." | Magma local objective guards. |
| `NP_BIKER_GUARD` | "Keep walking, unless you want trouble!" | Three Island gang refusal before the sequence. |
| `NP_TRIAL` | "Return with a Pokémon that can battle. The trial will wait." | Elder Li and the Kimono Girls trial entry. |
| `NP_GYM` | "Come back with a Pokémon ready to battle before you challenge this Gym." | Current Blue and any approved future Gym objective owner. |
| `NP_STERN` | "Please bring a Pokémon that can battle before we handle these parts." | Captain Stern before museum confrontation staging. |
| `NP_PARTNER` | "We need a Pokémon that can battle before we take them on." | Steven's Space Center battle offer. |
| `NP_FOSSIL` | "Want a fossil? Bring a Pokémon that can battle first!" | Mt. Moon fossil Super Nerd. |
| `NP_RECRUITER` | "Come back with a Pokémon ready to battle. Then we'll talk." | Nugget Bridge recruitment battle entry, before that sequence starts. |
| `NP_GIDEON` | "The Sapphire stays with me. Now get out!" | Warehouse Gideon. |
| `NP_SELPHY` | "A battle? Come back with a Pokémon that's ready!" | Lost Cave Selphy, before her rescue battle staging. |
| `NP_LEAGUE` | "You need a Pokémon that can battle before you enter this challenge." | League battle admission; other League prerequisites still apply. |
| `NP_INVESTIGATE` | "You need a Pokémon that can battle before you investigate." | Settled Route 120 scripted-encounter refusal without a usable party. |
| `NP_RETREAT` | "You can't keep battling. You step back." | Supported authored defeat return, after safe retreat staging. |

No message is needed for walking along an open public lane. Retained bystanders,
rescued staff, item pickups and completed trainers keep their ordinary eligible
text. A completed guard must not revert to a hostile refusal simply because the
player deposits their party. Doors retain their normal locked/key/password text;
a door does not use an NPC no-party taunt or teach its missing password.

### Current HNS/Johto rival coverage

All starter variants in a numbered Silver family share its policy. `Silent`
means the deferred-rival policy above, including hidden battle-only contact actors.

| Family | Required separation/recovery | No-party dialogue |
| --- | --- | --- |
| Cherrygrove Silver #1 | Suppress all off-screen approaches; keep chapter pending and public lane open without town-state completion. | Silent |
| Azalea Silver #2 | Split rival chapter from Well/Ilex and shared town progression; retain Ilex travel. | Silent |
| Burned Tower Silver #3 | Decouple fall/discovery access from rival victory; retain a later battle at the Tower. | Silent |
| Goldenrod Underground Silver #4 | Split Radio occupation state from rival chapter; finishing the rescue cannot erase the chapter. | Silent |
| Victory Road Silver #5 | Defer chapter without faking Route 27 progress; preserve existing League eligibility pending explicit League integration. | Silent for deferred chapter; League admission uses `NP_LEAGUE`; retain authored loss policy. |
| Mt. Moon Silver #6 | Preserve optional contact battle and its later Indigo unlock; no victory or decline writer on suppression. | Silent |
| Indigo Center Silver #7 | Restore optional rematch under normal daily eligibility; suppression spends no rematch. | Silent |
| Sprout Tower Silver non-battle scene | Keep independently eligible conversation/staging; no blanket rival hide. | Existing scene text |
| Mahogany B3F Silver non-battle scene | Separate shared Mahogany state if narrative order defers it; hideout remains independently completable. | Existing scene text when eligible |

### Current Hoenn rival coverage

| Family | Required separation/recovery | No-party dialogue |
| --- | --- | --- |
| Route 103 starter rival | Authored starter/tutorial policy remains explicit; no skipped chapter or Oldale completion. | `NP_ORDINARY` before battle/tutorial staging; retain authored loss policy |
| Rustboro optional rival | Separate battle from eligible Match Call/conversation state; suppression executes no decline writer. | Silent battle-only presence; `NP_RIVAL` if retained for conversation |
| Route 110 rival | Defer all approach triggers; retain chapter and earned Dowsing Machine without blocking road access. | Silent |
| Route 119 rival | Defer both approaches; retain chapter and earned Fly without gating bridge/Institute access. | Silent |
| Lilycove optional rival | Preserve decoration, departure and meeting/decline flags until their real conditions occur. | Silent |
| Mauville Wally | Keep optional chapter pending; no Wally/uncle relocation or defeat/call flags from bypass. | Silent battle-only presence; other actors keep eligible text |
| Victory Road Wally | Separate first chapter from corridor access; keep League entry policy explicit. Optional rematches retain normal eligibility. | Silent for battle-only chapter/rematch; `NP_LEAGUE` at League admission |
| Lavaridge Go-Goggles scene | Non-battle gift retains actual eligibility, bag-full retry and desert survey policy. | Existing gift text |
| Petalburg Wally tutorial | Party-substitution tutorial is excluded from generic no-party battle conversion. | `NP_ORDINARY` before battle/tutorial staging; retain authored loss policy |

### Current local objective coverage

| Family / objective | Preserved requirement and boundary | Refusal |
| --- | --- | --- |
| Slowpoke Well Proton | Rescue victory before Rocket removal, Slowpoke/Kurt restaging and rescue reward flow. | `NP_ROCKET_GUARD` |
| Mahogany password guards/Petrel and confrontation | Real password/security and confrontation predicates; split Silver state. Electrode shutdown keeps its separate scripted encounter policy. | `NP_ROCKET_GUARD` for trainer guards |
| Radio fake Director/Petrel | Basement Key and occupation state earned; guard before disguise transformation. | `NP_DISGUISED_DIRECTOR` |
| Radio executives and Archer | Occupation victory, staff rescue and Wing handoff; remove unrelated Mahogany road dependency. | `NP_ROCKET_GUARD` |
| Underground Director and Radio reward actors | Actual rescue/key/reward prerequisites, independent of deferred Silver. | Existing eligible handoff text; no party gate |
| Theater Rocket | Rescue victory before Rocket departure and Surf reward eligibility. | `NP_ROCKET_GUARD` |
| Kimono Girls trial | Wing/TR prerequisites and real trial victories before Bell/legendary progression. | `NP_TRIAL` |
| Elder Li | Local challenge victory before Flash. | `NP_TRIAL` |
| Kanto Power Plant Route 24 grunt | Real encounter/part discovery before restoration; public Route 24 stays open. | `NP_ROCKET_GUARD` |
| Power Plant manager | Actual returned Machine Part and restoration prerequisites. | Existing eligible item/quest text; no party gate |
| Viridian Blue | Existing Gym victory/badge/TM objective; future Giovanni identity is separate. | `NP_GYM` |
| Tohjo Falls Giovanni/Celebi | Preserve optional authored episode and retry/outcome policy; never merge with future FRLG finale. | `NP_ROCKET_GUARD` at safe trainer entry; broader episode policy remains explicit |
| Oceanic Museum Captain Stern/grunt sequence | Guard before spawning the confrontation. Preserve actual Parts, local battle sequence and delivery predicates; existing chained defeat policy remains until adapted. No Route 110 road dependency. | `NP_STERN` at Stern entry; any direct objective-grunt entry uses `NP_AQUA_GUARD` |
| Rusturf Aqua grunt | Peeko/Goods rescue and actual item recovery; independent public ferry/travel. | `NP_AQUA_GUARD` |
| Mt. Chimney Maxie | Local meteorite conflict, with planned theft prerequisite; cable car/Jagged Pass remain open. | `NP_MAGMA_GUARD` |
| Magma Hideout Maxie | Guard before Groudon awakening animation; victory writer controls local completion and subsequent branches. | `NP_MAGMA_GUARD` |
| Magma Hideout other trainers | Explicitly mapped security guards use objective policy; routine fights alone do not establish a new gate. | `NP_MAGMA_GUARD` for objective guards, otherwise ordinary bypass with the same hostile tone |
| Aqua Hideout Matt | Actual confrontation/submarine escape progression, separate from public Lilycove sea lane. | `NP_AQUA_GUARD` |
| Weather Institute Shelly/security | Staff rescue and Castform eligibility; Route 119 bridge remains open. | `NP_AQUA_GUARD` |
| Space Center grunt sequence and Maxie/Tabitha | Local invasion sequence and Dive eligibility; keep retreat and planned Magma prerequisite. Keep existing chained/partner defeat policy until adapted. | `NP_MAGMA_GUARD`; Steven offer uses `NP_PARTNER` |
| Seafloor Archie | Both planned branches and local finale victory before crisis/weather progression. | `NP_AQUA_GUARD` |
| Mt. Pyre, Harbor theft, Sootopolis farewell | Non-battle causal scenes keep their own eligibility and local state. | Existing scene text; no blanket villain refusal |

### Public roadblocks and optional fights

| Lane / host adventure | Required no-party behavior |
| --- | --- |
| Cherrygrove/Azalea and other approved rival through-lanes | Silent rival deferral; no substitute invisible blocker or chapter completion. |
| Route 110 Aqua wall | Approved public lane stays open. A retained off-lane objective guard uses `NP_AQUA_GUARD`; ordinary optional trainers use `NP_ORDINARY`. |
| Route 119 Aqua bridge wall | Bridge remains independent of Institute rescue and rival Fly chapter; use the same off-lane dialogue distinction. |
| Route 112 cable car, Mt. Chimney and Jagged Pass | Preserve approved cable-car/travel lanes without clearing the meteorite objective. Local Magma guards use `NP_MAGMA_GUARD`. |
| Lilycove sea route | Keep open-ocean travel independent from Matt/submarine completion. |
| Mahogany onward road | Radio occupation/Wing progress cannot recreate the removed public road gate. |
| S.S. Aqua maiden rescue | Retain the actual rescue episode. Its moved-aside sailor is optional, uses `NP_ORDINARY`, and cannot be promoted to a mandatory guard. |
| Future Kanto Bill/captain/Fuji/Silph access | Deferred rivals cannot gate host adventures; each local rescue/item dependency still applies. |
| Future Sevii travel and Dotted Hole | Ruby does not gate travel; Icefall/Lorelei completion does not gate Dotted Hole. Warehouse passwords remain local requirements. |

An open public lane does not promise immunity to eligible wild encounters or a
safe trip to recovery. Route 120's scripted Kecleon is the explicit exception
below; this table does not silently settle that conflict.

### Future FRLG/Sevii port coverage

These requirements activate with the corresponding Wayfarer story port. FRLG
source maps are evidence, not an instruction to expose those maps in the current
ROM. Rival chapters retain their approved locations; ship, Lavender/Tower and
Giovanni/Blue adaptation must be resolved by that port first.

The Kanto port design, `frlg-kanto-story-on-hns-maps.md` (PR #82), specifies
an origin-specific Blue campaign with forward chapter retirement. Its gameplay
port is absent from this implementation base; those chapters remain a dependency.
The older FRLG independence PRD and this inventory must not override the eventual
port-owned origin/chapter rules.

For that proposed model, only Kanto-origin players have personal Blue chapters.
Visitors keep the port's introduction and Champion roles without acquiring pending
personal fights. No-party suppression never counts as starting a later chapter
and therefore never retires another chapter. Actually starting an eligible later
Blue scene retires earlier ones under the port's rules, even if that battle is
lost. Recovery restores only still-eligible, unretired chapters; it cannot resurrect
a retired scene. Retirement gives no skipped victory, reward or host-adventure
completion. These Blue-specific rules do not change Silver or Hoenn rivals.

References below to preserving a Blue chapter mean preserving it while the owning
campaign still makes it eligible. Temporary-host access remains a port dependency;
this feature does not promise restoration of every past chapter or settle ship
geography. Reconcile against the accepted port specification before implementing.

| Future family | Required policy | No-party dialogue |
| --- | --- | --- |
| Oak's Lab starter rival | Explicit tutorial outcome/entry policy; no generic conversion. | `NP_ORDINARY` before startup staging; retain authored loss policy |
| Route 22 early/late rivals | Defer in narrative order while leaving approved road access. | Silent |
| Cerulean rival | Defer chapter/Fame Checker progression without blocking Bill. | Silent |
| S.S. Anne corridor rival | Keep captain access independent; preserve chapter after host departure at its approved location. | Silent; implementation waits for ship adaptation |
| Tower 2F rival | Keep Fuji investigation independent; preserve pending chapter in approved Tower adaptation. | Silent |
| Silph 7F rival | Separate rival chapter from occupation clearance and temporary staging. | Silent |
| Four Island rival appearance | Preserve non-battle scene arbitration and future recoverability; do not infer a battle from actor identity. | Existing scene text when eligible |
| Champion rival | League finale, excluded from ordinary rival deferral. | `NP_LEAGUE` before battle admission; retain authored loss policy |
| Cerulean burglary Rocket | Local stolen-item/TM objective, no regional travel gate. | `NP_ROCKET_GUARD` |
| Nugget Bridge recruiter | Preserve authored reward/recruitment order; no regional road lock. Refuse before starting this sequence without a party. | `NP_RECRUITER` |
| Mt. Moon fossil Super Nerd | Real victory before fossil choice. | `NP_FOSSIL` |
| Celadon Lift Key grunt and paired door guards | Real key acquisition and both required guard victories before security opens. | `NP_ROCKET_GUARD` |
| Celadon Giovanni | Local boss victory/Scope reward and investigation state. | `NP_ROCKET_GUARD` |
| Silph Card Key/doors | Actual item pickup and door predicates; Card Key is not an invented battle reward. | Existing item/locked-door text |
| Silph Giovanni and President | Boss victory before occupation cleanup and President reward eligibility. | Giovanni: `NP_ROCKET_GUARD`; President: existing eligible text |
| Viridian Giovanni finale | Preserve planned investigation/Silph dependencies and Gym outcome; resolve Blue ownership before port. | `NP_GYM` for approved Gym owner |
| Three Island biker sequence | Gang victory before removal/rescue progression; check exhaustion between fights. | `NP_BIKER_GUARD` |
| Icefall poachers/Lorelei rescue | Local rescue battle stays meaningful; independent from Dotted Hole. | Poachers: `NP_ROCKET_GUARD`; Lorelei's non-battle roles retain eligible text |
| Mt. Ember Rocket pair | Local access/password sequence requires its real prerequisites. | `NP_ROCKET_GUARD` |
| Warehouse entrance | Both actually learned passwords; independent Ruby and Dotted Hole branches. | Existing password-door text |
| Warehouse admins | Actual security/cage/arrow state changes from required victories. | `NP_ROCKET_GUARD` |
| Warehouse Gideon | Real Sapphire recovery battle/reward. | `NP_GIDEON` |
| Lost Cave Selphy | Guard before on-frame rescue battle staging; no house relocation or rescue completion on refusal. | `NP_SELPHY` |
| Selphy home requests and other non-battle reward actors | Retain local completed-rescue/request predicates. | Existing eligible dialogue |

### Settled scripted-wild exclusions

Route 120 Steven/Kecleon is a scripted wild encounter whose existing non-loss
resolution, including fleeing, grants the Scope and clears the bridge. The core
mode excludes scripted wild encounters. This is the settled behavior: require a
usable party and otherwise preserve a safe, retryable guard. Do not grant the
Scope, mark completion, hide Steven indiscriminately or promise no-party passage.
Use `NP_INVESTIGATE` with the same once-per-approach rearming rule. The player
returns after recovering a usable Pokémon; no trainer-only conversion is pending.

Tower Marowak, Lostelle and Mahogany Electrode objectives likewise retain scripted
wild policies. Preserve their local completion predicates without treating them
as ordinary trainer guards or automatically converting them into trainer-only mode.
This policy is settled; applying it to unported content remains part of its port.

### Other exceptional policies and future ports

League entry/finales, starter/catching tutorials, Celebi episode losses and
exceptional challenges retain their explicit policies. Future map/character
adaptation and temporary-host recoverability must be implemented and tested before
the corresponding scene is advertised as supported. The current Blue Gym and
future Giovanni finale are distinct until the port resolves their ownership.

## Validation

| Area | Required cases |
| --- | --- |
| Party predicate | Empty, six fainted, Eggs-only, mixed unusable and challenge-dead parties agree. One usable member restores eligible normal battles. No refusal claims fainting/ownership incorrectly. |
| Rival suppression | Exercise every approach/starter variant; no spawn, lock, collision, reward, decline write, completion write or daily-rematch consumption. |
| Rival restoration | Recover in field/on trigger/on hidden object tile; leave/reenter; save/reload; visit out of chapter order. No immediate forced trigger or permanent disappearance. |
| Host independence | Complete Well/Ilex, Tower discovery, Radio occupation and ported Silph/captain/Fuji objectives with rival deferred; return and complete the pending chapter at its approved location. |
| Objective protection | Approach each guard from both sides and alternate entrances. Refusal keeps objective unresolved but permits retreat; doors still require actual keys/passwords. |
| Irreversible staging | No-party entry at fake Director, Magma awakening and Selphy on-frame scene refuses before transformation, awakening or relocation. Retry with protection works. |
| Outcomes | Supported loss/forfeit/abort cannot execute victory continuation, issue prizes, clear occupations or run a second loss charge. Preserve authored pre-battle reward order. |
| Retry and chains | Supported defeat restores actors/map/control to a safe approach; no frame/coordinate loop. Excluded chains retain authored defeat routing; any future field-return adaptation stops on exhaustion without final rewards. |
| Public lanes | Walk approved Route 110/119, cable-car/Jagged Pass, rival and sea lanes both ways; objective remains pending. Aqua sailor stays optional. |
| Non-battle roles | Gifts, rescued staff, Card Key pickup, passwords, manager handoffs and rival conversations obey their real prerequisites without a blanket no-party gate. |
| Dialogue | Check all four ordinary variants, stable assignment/overrides, rematches, reload and unchanged gameplay RNG. Check hostile existing-text reuse, wrapping/pagination, repeat approach, completed-state text and silent rivals; no hostile refusal from a resolved objective actor. |
| Exceptions and ports | Confirm excluded scripted wild, tutorials, League/challenge and unported scenes keep safe explicit handling. Test each port against resolved geography and host access before enabling it. |
| Regression | Usable-party scenes retain intended story order/rewards; standalone HNS/FRLG/Emerald are unchanged. No unrelated traversal gate returns. |

Cover each authored entrance and distinct lifecycle in the emulator. Distribute
starter, gender and unusable-party variants across those cases instead of taking
their full Cartesian product when the same guard runs before variant selection.
Mechanics tests own exhaustive predicate and arithmetic combinations; source
audits still check every registered caller and starter branch. Keep separate
emulator cases for different staging, restoration, continuation and retreat
behavior, even when their refusal text is identical.

Map/event inspection must accompany emulator walkthroughs: script evidence alone
cannot prove collision, elevation or alternate-entry safety. Record supported
scene/configuration coverage explicitly; a shared predicate test does not validate
all authored callers. Follow the core spec's sequential build requirements.
