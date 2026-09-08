# Regional story beat independence audit

Audited local main revision `9ab8058a6e` on 2026-09-07. Scope: Wayfarer's HNS
Johto/Kanto and imported Emerald Hoenn. FRLG Kanto is not the active Kanto
campaign. Unmerged tasks, including regional-start design, are outside this audit.

## Verdict

The three regions do not yet meet the proposed independent-story-beat standard.
Kanto is closest; Johto has several good local episodes but retains major campaign
dependencies; Hoenn mostly retains Emerald's sequential campaign inside an open
travel network.

For this audit, a beat may contain several ordered scenes and span nearby towns.
It passes when a player can discover its premise locally, complete it without
unrelated earlier campaign flags, understand its characters and stakes without
having played earlier episodes, and finish it without invalidating other episodes.
An explicitly connected regional finale can retain its own internal sequence,
but should not be described as several independently playable episodes.

This is a stronger requirement than the current product contract. The
[League circuit PRD](../prds/wayfarer-interregional-league-circuit.md) explicitly
retains regional quest and Gym prerequisites. The
[Hoenn content specification](../specs/wayfarer-hoenn-content-port.md) preserves
the Emerald campaign in its intended order. Most dependencies below are design
gaps against the proposed criterion, rather than regressions against those specs.

## Beat inventory

| Region | Beat or arc | Assessment |
| --- | --- | --- |
| Johto | Kurt / Slowpoke Well | Mostly a self-contained local rescue. |
| Johto | Burned Tower introduction | Locally introduced; later rival and Suicune developments remain sequential. |
| Johto | Theater Rocket / Surf reward | Short local episode. |
| Johto | Lighthouse / medicine / Jasmine | Cohesive two-town episode; medicine-first works mechanically but needs alternate introductory dialogue. |
| Johto | Lake of Rage / Lance / Mahogany hideout | Coherent internally ordered arc, suitable as one larger episode. |
| Johto | Radio Tower takeover | Not independent: Mahogany progression and Chuck, Jasmine, Pryce badges are prerequisites. |
| Johto | Clair / Dragon's Den | Still requires those three badges before Gym access. |
| Johto | Kimono Girls / Lugia or Ho-Oh | Still activated through Clair, Dragon's Den, and a return to Elm. |
| Kanto | S.S. Aqua granddaughter rescue | Local introduction to regional travel, independent of the Johto League. |
| Kanto | Power Plant theft | Mechanically self-contained, but its ending assumes Johto's Rocket defeat. |
| Kanto | Misty, Copycat, radio / Snorlax | Remain downstream of Power Plant repair; not separate freely ordered episodes. |
| Kanto | Most Gym challenges / Blue invitation | Largely local; Blue's former fifteen-badge requirement is bypassed. |
| Kanto | Mt. Moon Silver | Locally available, but assumes earlier losses and character development. |
| Kanto | Suicune | Ordered cross-region pursuit with an extra order-dependent Misty requirement. |
| Hoenn | Birch / starter choice / rival | Adapted visitor opening; later rival scenes still depend on its starter choice. |
| Hoenn | Devon / Peeko / deliveries | Original linked quest; Slateport can falsely complete delivery early. |
| Hoenn | Meteor Falls / Mt. Chimney | Reasonable two-location arc with internal ordering. |
| Hoenn | Weather Institute | Largely local rescue; nearby rival scene has separate earlier prerequisites. |
| Hoenn | Route 120 Kecleon / Devon Scope | Strong independent episode, including an early/late Steven greeting. |
| Hoenn | Desert survey / Go-Goggles | Designed as a local alternative to the old Gym reward gate. |
| Hoenn | Norman | Requires Wally tutorial and four Hoenn badges; retains the original father premise. |
| Hoenn | Mt. Pyre / hideouts / submarine | Preserved regional sequence, not independently ordered location episodes. |
| Hoenn | Mossdeep / Dive / Seafloor / Sootopolis | Preserved late campaign sequence leading to Juan. |

## Confirmed problems

### 1. Slateport can complete a delivery the player never accepted

A new Hoenn visitor arrives in Slateport with pre-campaign state. Dock's script
recognizes Devon Goods without checking possession and removes the museum queue.
Museum admission does not check the Parts. Stern then thanks the player for them,
runs the Aqua battles, narrates a handover, and commits delivery completion.
The actual item comes from the earlier Rusturf rescue.

This is a false premise and an out-of-order completion, not a standalone museum
story. Either introduce a local museum-defense branch for visitors or give the
delivery branch a clear, discoverable prerequisite and a separate local welcome.

Evidence: `game/src/wayfarer_persistence.c:348`;
`game/data/maps/SlateportCity_SternsShipyard_1F/scripts.inc:4`;
`SlateportCity_OceanicMuseum_1F/scripts.inc:22`;
`SlateportCity_OceanicMuseum_2F/scripts.inc:4`, `:69`;
`RusturfTunnel/scripts.inc:306`. Map paths after the first share `game/data/maps/`.

### 2. Optional earlier events are still asserted as history

- Kanto's Rocket grunt responds to the player saying Team Rocket has disbanded,
  even if the Johto finale has not happened: `Route24_hns/scripts.inc:153`.
- Radio Tower Proton says the player interfered at Slowpoke Well, although the
  Well is not a takeover prerequisite: `GoldenrodCity_RadioTower_4F_hns/scripts.inc:90`.
- Mt. Moon Silver describes learning from a previous loss, although earlier
  battles can be bypassed: `MtMoon_Cave_hns/scripts.inc:17`, `:61`.
- Misty's date refers to the player's Johto badges even on a zero-badge visit:
  `Route25_hns/scripts.inc:387`.
- Norman still calls himself the visitor's father, including in the actual
  post-battle path: `PetalburgCity_Gym/scripts.inc:410`, `:1314`.

These need first-meeting or prior-completion variants. Route 120 already provides
a useful pattern: Steven selects his greeting using Letter delivery state without
making the Letter a prerequisite (`Route120/scripts.inc:153`).

### 3. Johto's major episodes still require distant progress

The Wayfarer takeover helper requires Mahogany state at least 15 and badges from
Chuck, Jasmine, and Pryce. Radio Tower therefore cannot be selected as an early
Goldenrod episode. Clair's Gym checks the same three badges. The legendary finale
then depends on Clair's completion and Elm's Master Ball handoff activating the
Kimono Girls.

Evidence: `MahoganyTown_Gym_hns/scripts.inc:103`;
`BlackthornCity_hns/scripts.inc:8`, `:27`;
`NewBarkTown_Lab_hns/scripts.inc:441`, `:456`.

Keep Lake of Rage and Mahogany hideout as one coherent arc. Decide explicitly
whether Radio Tower is that arc's finale or a separate locally introduced crisis.
Unrelated Gym badges should not silently stand in for narrative readiness if
independent episodes are the goal.

### 4. Hoenn retains long chains rather than local entry points

The Mt. Pyre reward opens Magma Hideout; its finale activates the harbor theft;
that theft removes Aqua Hideout's entrance guards. Separately, Mossdeep Gym
activates the Space Center invasion; its conclusion activates Steven's Dive reward,
whose handoff removes the Seafloor entrance guard. Seafloor starts the Sootopolis
crisis; Juan's door remains tied to its resolution.

Evidence: `MtPyre_Summit/scripts.inc:60`; `JaggedPass/scripts.inc:7`;
`MagmaHideout_4F/scripts.inc:60`; `SlateportCity_Harbor/scripts.inc:110`;
`MossdeepCity_Gym/scripts.inc:82`; `MossdeepCity_SpaceCenter_2F/scripts.inc:298`;
`MossdeepCity_StevensHouse/scripts.inc:87`; `SeafloorCavern_Room9/scripts.inc:118`;
`SootopolisCity/scripts.inc:10`, `:1371`.

These sequences can support a cohesive larger campaign. They do not support
playing each named location's story independently. Opening ocean routes or
obtaining a native field-move user does not remove the event-state dependencies.

### 5. Suicune gains an extra prerequisite depending on event order

Route 14 sets Suicune encounter state 5 and reveals the Route 25 encounter only
if Cerulean state is already at least 3. Misty's date sets Cerulean state 3 but
does not retry that check. Misty's Gym victory does retry it.

Thus, date then Route 14 allows the encounter without beating Misty; Route 14
then date requires her Gym victory through the traced normal progression.
Both events should evaluate the same readiness predicate if that extra Gym
requirement is unintended.

Evidence: `Route14_hns/scripts.inc:111`; `Route25_hns/scripts.inc:123`;
`CeruleanCity_Gym_hns/scripts.inc:210`.

## Recommended direction

Use independent small arcs with ordered scenes inside them. Each arc needs a
local invitation, enough context for a first meeting, its own completion state,
and a conclusion that remains true when other arcs are untouched. Shared characters
can remember completed episodes through optional dialogue branches.

Keep genuinely connected finales ordered, but name their prerequisite arcs
explicitly and make them discoverable. Avoid making every scene independent:
the Lake of Rage investigation and a regional legendary crisis benefit from
internal cause and effect.

Prioritize the false Slateport delivery, false-history dialogue, and Suicune
ordering asymmetry. Then decide the intended boundaries of Radio Tower, the
Kimono finale, the Kanto restoration quests, and Hoenn's villain campaign before
changing their prerequisite flags.

## Validation and limits

This is a source and narrative audit, not a full ROM playthrough. Reviewed regional
script traces were reconciled against the current specs and selected source paths.
Existing source checks passed: 24 HNS traversal tests and 9 Emerald traversal
tests. Those checks establish their traversal contracts, not narrative independence.

An additional unconfirmed risk is Azalea's shared state: late Slowpoke Well/Kurt
completion writes states 3 and 4 while Bugsy/Silver use later states. A resulting
softlock was not established and is not claimed here. Future runtime checks should
exercise late quest completion, reversed sibling episodes, travel away and back,
and save/reload without synthesizing prerequisites in the fixture.

No gameplay files were changed.
