# FRLG Kanto independent story beats

Status: Agreed product design. Implemented through the per-adventure
specifications that cite it: [Celadon Rocket Hideout](../specs/frlg-kanto-celadon-hideout.md),
[Mt. Moon](../specs/frlg-kanto-mt-moon.md),
[Nugget Bridge](../specs/frlg-kanto-nugget-bridge.md),
[Safari teeth](../specs/frlg-kanto-safari-teeth.md),
[Silph liberation](../specs/frlg-kanto-silph-liberation.md),
[Silph President and Master Ball](../specs/silph-president-master-ball.md),
[Pokémon Tower](../specs/frlg-kanto-pokemon-tower.md),
[Viridian finale](../specs/frlg-kanto-viridian-finale.md),
[Power Plant](../specs/frlg-kanto-power-plant.md), and
[S.S. Anne](../specs/frlg-kanto-ss-anne-adventure.md). Mewtwo's readiness
gate is the one reward decision still open: the encounter exists in Wayfarer's
Cerulean Cave with no TR readiness check (see Open questions).

## Intent

Make FRLG Kanto's local adventures playable in different orders while preserving
their original characters, locations, objectives, battles, rewards, and most
dialogue. Retain short causal connections and Giovanni's final confrontation
rather than rewrite every scene as an unrelated episode.

This document treats FRLG Kanto on its own. It prepares the story requirements
for a future Wayfarer port; it does not decide how FRLG and HNS Kanto coexist or
merge. References to Viridian, Lavender, the rival, and other content below mean
their FRLG versions. The [Wayfarer Kanto story PRD](frlg-kanto-story-on-hns-maps.md#lavender-during-the-radio-conversion)
selects Lavender's shared radio and memorial building; the
[Tower specification](../specs/frlg-kanto-pokemon-tower.md) owns that adaptation.

## Design

### Independent adventures with minimal changes

An adventure may contain several ordered scenes and span several locations.
Remove unrelated Gym badges, campaign flags, and distant activation errands from
its prerequisites. Keep direct causes such as obtaining a ticket before boarding
a ship, or using the Silph Scope to identify a ghost.

Introduce each adventure through existing local characters. Preserve dialogue
when true; use brief first-meeting variants or conditional callbacks when an
earlier event did not happen. No new NPCs, factions, maps, replacement plots, or
quest-selection interface are required.

An unavailable successor scene must leave ordinary travel and unrelated adventures
available. Visiting early neither completes its predecessor nor consumes the scene.
Returning after the real prerequisite is satisfied makes it available normally.

### Adventure boundaries

| Adventure | Preserved content | Required independence or retained connection |
| --- | --- | --- |
| Mt. Moon fossils | Rocket encounters, the Super Nerd battle, and the existing fossil choice. | Treat as a local cave adventure. No later Rocket adventure requires its completion. Fossil rewards remain attached to their existing interactions. |
| Cerulean burglary | Investigate the robbed house, confront the Rocket, and recover the TM. | Keep independent of Bill, travel to Vermilion, and the major Rocket stories. |
| Nugget Bridge | Trainer challenge, prize, and Rocket recruitment attempt. | Preserve the local challenge order without requiring another Rocket adventure. |
| Bill and the S.S. Anne | Help Bill and receive his Ticket; board the ship, help the captain, and receive Cut. | Bill is one route to the Ticket, not a boarding prerequisite when the player already owns one. The [dedicated Anne specification](../specs/frlg-kanto-ss-anne-adventure.md) owns the persistent, one-time ship adventure. |
| Celadon and Pokémon Tower | Discover the hideout, defeat Giovanni, obtain the Scope, resolve Marowak's ghost, rescue Fuji, and receive the Flute. | Keep as a connected investigation, independent of Silph Co., Mt. Moon completion, and specific Gyms. |
| Snorlax | Use the Poké Flute for the existing Route 12 and Route 16 encounters. | Retain the Flute prerequisite. Either encounter may be completed first; neither gates ordinary regional travel. |
| Silph Co. | Infiltrate the occupied company, solve its local access puzzles, defeat Giovanni, and rescue the staff. | Remove Mr. Fuji's rescue as an entrance prerequisite. Keep this adventure independently available in Saffron. |
| Safari Zone | Reach the Secret House for Surf; find the Gold Teeth and return them to the Warden for Strength. | Keep as parallel local objectives. Neither requires the other or a distant adventure. Koga's challenge remains separate. |
| Pokémon Mansion and Blaine | Explore the mansion, find the Secret Key, and unlock Blaine's Gym. | The [FRLG Cinnabar and Seafoam port](frlg-cinnabar-seafoam-port.md) owns this complete local adventure and its independence from unrelated campaigns. |
| Giovanni in Viridian | Discover Giovanni as Gym Leader, confront him, and receive the Earth Badge and Earthquake TM. | For Wayfarer, [the finale specification](../specs/frlg-kanto-viridian-finale.md) requires completed Celadon Hideout and Silph liberation in either order. Tower/Fuji is independent; no badge or TR gate applies. |

The [Kanto origin opening](wayfarer-kanto-origin-opening.md) keeps its local
FRLG sequence through receiving and returning Oak's Parcel, Blue's lab arrival,
Pokédex receipt, and five Poké Balls. Daisy's Town Map and later campaign beats
are separate work. Neither the bounded opening nor its Parcel state becomes an
unrelated prerequisite for every later adventure.

### Bill and the S.S. Anne

Bill remains the Ticket giver, and the captain remains the Cut reward giver.
The [S.S. Anne adventure port](../specs/frlg-kanto-ss-anne-adventure.md)
supersedes this document's former temporary-ship and strict Bill-first design.
The ship remains accessible so unfinished one-time content cannot disappear,
but it offers no transport, recurring service, rematch, or renewable reward.
Its Blue scene must not become an implicit requirement for obtaining Cut.

### Celadon, Lavender, and the Flute

Preserve the sequence:

```text
Game Corner investigation -> Rocket Hideout -> Silph Scope
    -> Pokémon Tower / Marowak -> Fuji rescue -> Poké Flute
        -> Route 12 and Route 16 Snorlax, in either order
```

A player may discover Lavender's problem first. Existing local dialogue must
provide enough direction to investigate the missing identification tool without
requiring knowledge of the original campaign. Exploring the Tower early does
not resolve Marowak, rescue Fuji, or award the Flute.

Do not grant the Scope merely for entering the hideout or skip its normal reward
event. Keep the Tower's ghost encounter, Rocket rescue, and Fuji's return as one
recoverable sequence. Receiving the Flute does not automatically resolve either
Snorlax encounter. For Wayfarer, radio access and its Machine Part upgrade stay
independent of this sequence; Fuji's physical Flute remains its own reward.

### Two investigations and Giovanni's finale

The proposed structure is:

```text
Celadon Hideout ------------------+
                                  +-> Viridian Giovanni
Silph Co. rescue -----------------+
```

Either investigation can be completed first, and the player can alternate between
them. Silph's entrance and local puzzles do not require Fuji, the Scope, the Flute,
or completing Celadon. Giovanni's first-meeting and repeat-meeting dialogue must
reflect the order actually played, including victory lines that currently assume
an earlier loss.

Viridian's Giovanni confrontation becomes available after Celadon's Giovanni
has been defeated and its Scope obtained, and Silph's occupation has ended.
Fuji's rescue is independent. Outstanding ordinary
reward collection must remain recoverable; claiming the Master Ball is
not a finale prerequisite. Neither Snorlax encounter is required.

Remove the existing specific-Kanto-badge access condition for this finale. Do not
replace it with TR, a total badge count, or a league win. This is a direct story
dependency preserving Giovanni's reveal and withdrawal, not a late-reward gate
on an ordinary badge.

Keep his final battle, Earth Badge, and Earthquake TM reward sequence intact. The main
operations must already be resolved so he does not announce Team Rocket's end
while still running Silph or the Celadon hideout.

Smaller Rocket incidents, including Mt. Moon, Nugget Bridge, and the burglary,
remain optional and available afterward. Review their dialogue, visibility, and
the finale's global cleanup so they remain coherent and are not silently removed.
Prefer small conditional variants. Do not add all minor encounters as mandatory
prerequisites merely to preserve an unconditional line about Rocket's defeat.

### Late-game rewards

Use the [late-game reward exception](johto-independent-story-beats.md): delay
specifically named powerful rewards through Trainer Rating (TR) while leaving
their ordinary adventures playable. TR determines eligibility; it is not spent.

| Reward | Required treatment |
| --- | --- |
| Silph Master Ball | Require the resolved Silph rescue only. The President remains the reward giver. The [Master Ball specification](../specs/silph-president-master-ball.md) (Contract, "There is no Trainer Rating (TR), badge, League, legendary-readiness, origin, or other story threshold") overrides the earlier TR-gated treatment: the ball is immediate after liberation, and a full Ball pocket leaves it pending for retry. |
| Silph Lapras | Keep as an ordinary local gift with its existing giver and one-time delivery behavior. No legendary-tier TR gate is added. |
| Articuno and Zapdos | Preserve their local exploration and puzzles. Gate each encounter by legendary readiness, without requiring the other bird or a Rocket story. The coastal port owns FRLG Seafoam and Articuno's placement; the [Power Plant specification](../specs/frlg-kanto-power-plant.md) selects Wayfarer's Zapdos hall, TR 55 battle threshold, and FRLG encounter outcomes. |
| Mewtwo | Use a higher TR readiness threshold than the birds, replacing the unrelated Celio Ruby/Sapphire campaign dependency. Preserve Cerulean Cave and the existing encounter rather than add a new capture quest. |
| Cut, Surf, Strength, Flute, fossils, and ordinary gifts | Retain their local acquisition requirements without late-game TR gates. |

Ending Silph's occupation, returning its civilians, receiving ordinary rewards,
and proceeding toward Giovanni must not wait for Master Ball eligibility. The
President's deferred reward survives travel, save/reload, and a failed Bag handoff.

The player may discover and explore the birds' sites before capture readiness.
Keep Articuno's boulder/current puzzle in the full FRLG Seafoam port and the
applicable field-move requirements.
A high encounter level alone is not a gate. Unmet readiness must not consume or
permanently hide an encounter.

For Mewtwo, the port specification must identify the appropriate local access
boundary in Cerulean Cave. It must enforce the readiness requirement without
requiring completion of Sevii's unrelated delivery chain. No new league-win
condition is introduced by this PRD. Today the encounter is HNS's level-80
legendary battle in `CeruleanCave_B2F_hns` (`CeruleanCave3_EventScript_MewtwoNew`,
shown by `FLAG_HIDE_MEWTWO`), with no TR or story check. The only thing in front
of it is HNS's Cerulean Cave guard, which a Kanto-badge Indigo League clear
removes (`LeagueCircuit_EventScript_IndigoHallOfFame` calling
`PokemonLeague_HallOfFame_EventScript_SetGameClearFlags`). The FRLG
`CeruleanCave_B1F_Frlg` copy is unused.

Articuno and Zapdos share the legendary threshold of TR 55 on today's scale
(v0: TR 120, about 16 badges): `WAYFARER_LEGENDARY_TR` and
`WAYFARER_BIRD_CAPTURE_TR` in `game/include/wayfarer_sevii_story.h`, consumed by
`WayfarerCoast_IsArticunoEligible` and `WayfarerPowerPlant_IsZapdosEligible` in
`game/src/wayfarer_persistence.c`. The Master Ball has no threshold. Mewtwo
has no readiness threshold yet; when one is chosen it should be higher than the
birds'. All alternate activation paths must obey the final reward rules.

### Rival continuity

Keep FRLG's rival scenes at their existing locations except for Blue's
one-time Cinnabar introduction. The historical coastal port moved it to the
Viridian Gym entrance; the implemented [Viridian finale](../specs/frlg-kanto-viridian-finale.md)
now removes that introduction and Blue's Gym role. The v0
[notable trainers](notable-trainers.md) design gives Blue and Giovanni their
own TR, growing with world progress, for every battle with them; it assigns
Blue no badge. Preserve genuinely necessary
character-development order by withholding later scenes until their predecessors
are complete. Do not relocate chapters or select a different chapter dynamically
based on where the player arrives.

Deferred rival scenes must not block Bill, the captain, Pokémon Tower, or Silph.
Completing a host adventure must not advance an unseen rival chapter, rewind a
completed one, or make a required chapter unrecoverable. The same care applies to
Lapras and other rewards located near rival scenes.

The specification must identify which claims really need a preceding chapter and
which need only a short dialogue variant. Temporary locations and the rival's
League appearance require particular review. If preserving a scene at its original
location conflicts with the host adventure's independent completion, record the
exact issue for a product decision instead of silently deleting it or restoring
the whole original campaign order.

## Boundaries

- This is a future Wayfarer FRLG Kanto story contract, not a claim that the story
  port exists or that standalone FireRed/LeafGreen behavior changes now.
- HNS Kanto coexistence, replacement, shared locations, NPC identities, badge
  ownership, and transport integration remain outside this document. The
  [coastal port](frlg-cinnabar-seafoam-port.md) owns its named Cinnabar,
  Seafoam, Blaine, Blue-introduction, and Groudon-removal decisions; other
  merge choices require their own designs.
- Sevii's story, Lostelle, Celio's deliveries, and its transport design are outside
  scope. Removing Celio's dependency from Mewtwo does not complete or delete those
  quests. Moltres is at Mt. Ember in FRLG and belongs to that separate Sevii scope.
- Preserve existing characters, maps, puzzles, ordinary rewards, and battle
  identities. No broad rewriting, new factions, quest journal, or map redesign.
- No change to TR calculation, trainer scaling, encounter levels, or League
  qualification. Giovanni's badge remains subject to the normal badge accounting
  of the eventual port.
- Do not change unrelated Gym challenges. Blaine's local Secret Key requirement
  and Giovanni's named story finale are explicit retained connections.
- Exact dialogue, script state, map events, and port architecture belong in a
  follow-up implementation specification.

## Interactions

Passing through an area does not complete its story. Preserve the open-traversal
principle while retaining field preparation and optional shortcuts. The proposed
Snorlax travel contract includes both encounters; it is broader than the existing
FRLG traversal pass's selected Route 12 bypass. The specification must reconcile
Route 16 access without treating either capture as a regional road requirement.

Each adventure must preserve its own progress and unclaimed rewards through losses,
travel, save/reload, and Bag or party capacity failures. Reordered Giovanni battles
must not regress Saffron, reset the hideout, erase the Tower rescue, or award the
Earth Badge prematurely. Viridian completion must not suppress unfinished minor
Rocket incidents.

These requirements guide the future story port alongside the
[FRLG traversal design](frlg-open-world-region-traversal.md). That document's
existing script assumptions are not proof of narrative independence. The future
port must adopt the named changes here explicitly and keep its story state
distinct from unrelated regional progression.

## Playtesting

- Discover and complete Mt. Moon, the burglary, and Nugget Bridge before and after
  the major Rocket investigations and Giovanni finale. No incident is erased or
  attributes an unplayed encounter to the player.
- Help Bill and complete the captain's adventure without unrelated Gym or rival
  progress. Verify Ticket and Cut handling, persistent access to outstanding
  one-time content, and the absence of recurring ship utility.
- Discover the Tower problem before visiting Celadon. Follow locally understandable
  guidance, obtain the Scope normally, and return to complete the rescue and Flute.
- Finish Silph before Celadon/Lavender, then test the opposite order and interrupted
  interleaving. Giovanni's dialogue and each adventure's state remain truthful.
- Approach Viridian with neither investigation resolved, with each alone resolved,
  and with both resolved. Only both enable its finale; badge count and TR do not
  add requirements. Unclaimed Master Ball and unfinished Snorlax encounters do not
  prevent the finale.
- Finish Silph with a full Ball pocket. The city recovers and other stories
  proceed. Return to receive the reward once, including after save/reload.
- Complete both Safari objectives in either order and verify Surf and Strength
  come from their respective givers. Neither objective requires Koga or another arc.
- Complete the separately specified Mansion/Secret Key/Blaine adventure
  without unrelated campaign progress.
- Resolve the Snorlax encounters in either order after obtaining the Flute. Travel
  without resolving either must not require a capture or story victory.
- Test Articuno and Zapdos discovery below readiness and capture eligibility at
  TR 55. Neither requires the other. Once Mewtwo has a readiness gate, test its
  higher boundary without Celio's quest, including alternate access and
  save/reload paths.
- Visit later rival locations early, finish their host adventures where possible,
  then progress the rival and return. No chapter or reward is silently lost.

Players unfamiliar with FRLG should understand each local problem and its next
step. Players familiar with it should recognize the original adventures and
Giovanni's finale without a replacement plot.

## Settled by implementation

- Finding the Tower first: the 2F clue and Lavender dialogue point toward the
  Silph Scope without granting it. The Scope gates only the Marowak spirit and
  passage to Fuji, not entry or the radio
  ([Pokémon Tower](../specs/frlg-kanto-pokemon-tower.md), Design).
- Optional Rocket incidents after Giovanni withdraws: his defeat must not run the
  source's broad `FLAG_HIDE_MISC_KANTO_ROCKETS` cleanup or mark optional
  incidents complete ([Viridian finale](../specs/frlg-kanto-viridian-finale.md)).
- Rival chapters and Silph: Blue's S.S. Anne scene is a one-time visitor scene
  with no battle, rivalry, or forward progress
  ([S.S. Anne](../specs/frlg-kanto-ss-anne-adventure.md), "Blue"); the Tower's
  Blue scene never gates the stair, Scope, Marowak, Rockets, Fuji, or Flute.
  The rival's League appearance has not been reviewed by any specification.
- Master Ball threshold: none; the President hands it over after liberation and
  retries after a full Ball pocket
  ([Master Ball](../specs/silph-president-master-ball.md), Contract). Because
  there is no deferral, no deferred-collection line is needed.
- Articuno and Zapdos threshold: TR 55 (see Late-game reward exception).

## Open questions

1. Should Mewtwo get a TR readiness gate above the birds' TR 55, and where in
   Cerulean Cave would it be enforced? The encounter exists: a level-80
   legendary battle in `CeruleanCave_B2F_hns`
   (`game/data/maps/CeruleanCave_B2F_hns/map.json`, `LOCALID_CERULEANCAVE_MEWTWO`;
   `CeruleanCave3_EventScript_MewtwoNew` in its `scripts.inc`), with no TR or
   story check. The cave is Brutal in
   [reach assignments](../specs/reach-assignments.md), which sets wild levels,
   not access. Its entrance is behind HNS's guard, removed by a Kanto-badge
   Indigo League clear. The
   [Pokédex obtainability audit](../research/pokedex-obtainability.md) lists
   this as Mewtwo's one live source. The FRLG `CeruleanCave_B1F_Frlg` copy is
   unused. No threshold has been chosen.
2. Which presentation explains each legendary's readiness requirement to a
   player below the threshold? No specification settles this for the birds or
   Mewtwo.

## References

- [Johto story design and late-game reward exception](johto-independent-story-beats.md)
- [Hoenn independent story beats](hoenn-independent-story-beats.md)
- [FRLG open-world regional traversal](frlg-open-world-region-traversal.md)
- [Kanto origin opening](wayfarer-kanto-origin-opening.md)
- [FRLG traversal specification](../specs/frlg-open-world-region-traversal.md)
