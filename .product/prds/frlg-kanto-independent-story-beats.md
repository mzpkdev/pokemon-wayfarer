# FRLG Kanto independent story beats

Status: Partially implemented through the bounded [story milestones](../specs/frlg-kanto-story-milestones.md).
PR #86 delivers the completed milestones listed there; future milestones and
their unresolved decisions do not block that scoped PR. The umbrella design
remains incomplete and is not yet released. Legendary readiness and bounded
scene decisions remain open; the separate Silph Master Ball reward is implemented.

## Intent

Make FRLG Kanto's local adventures playable in different orders while preserving
their original characters, locations, objectives, battles, rewards, and most
dialogue. Retain short causal connections and Giovanni's final confrontation
rather than rewrite every scene as an unrelated episode.

This document treats FRLG Kanto on its own. It prepares the story requirements
for a future Wayfarer port; it does not decide how FRLG and HNS Kanto coexist or
merge. References to Viridian, Lavender, the rival, and other content below mean
their FRLG versions.

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
| Bill and the S.S. Anne | Help Bill, receive his Ticket, board the ship, help the captain, and receive Cut. | Keep this direct sequence. No unrelated badge or rival chapter is required to reach Bill or complete the captain's adventure. |
| Celadon and Pokémon Tower | Discover the hideout, defeat Giovanni, obtain the Scope, resolve Marowak's ghost, rescue Fuji, and receive the Flute. | Keep as a connected investigation, independent of Silph Co., Mt. Moon completion, and specific Gyms. |
| Snorlax | Use the Poké Flute for the existing Route 12 and Route 16 encounters. | Retain the Flute prerequisite. Either encounter may be completed first; neither gates ordinary regional travel. |
| Silph Co. | Infiltrate the occupied company, solve its local access puzzles, defeat Giovanni, and rescue the staff. | Remove Mr. Fuji's rescue as an entrance prerequisite. Keep this adventure independently available in Saffron. |
| Safari Zone | Reach the Secret House for Surf; find the Gold Teeth and return them to the Warden for Strength. | Keep as parallel local objectives. Neither requires the other or a distant adventure. Koga's challenge remains separate. |
| Pokémon Mansion and Blaine | Explore the mansion, find the Secret Key, and unlock Blaine's Gym. | Retain this local connection without unrelated campaign prerequisites. |
| Giovanni in Viridian | Discover Giovanni as Gym Leader, confront him, and receive the Earth Badge through the existing battle/reward sequence. | Make this the shared finale of the Celadon/Lavender and Silph adventures, replacing specific-badge access requirements. No TR gate applies to the badge. |

The opening and Oak's Parcel keep their own local sequence where included by the
future port. They must not become unrelated prerequisites for every later adventure.
Player origin and starter selection belong to the separate opening design.

### Bill and the S.S. Anne

Bill remains the Ticket giver, and the captain remains the Cut reward giver.
Preserve the ship's normal local progression and departure after the captain's
adventure. Boarding and departure must not advance a rival chapter that was not
played or change another travel service's entitlement.

The ship is a temporary location. A deferred rival scene or outstanding reward
must not silently disappear when the ship departs. Resolve that scene ownership
in the specification; do not make the entire previous rival itinerary an implicit
requirement for obtaining Cut.

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
Snorlax encounter.

### Two investigations and Giovanni's finale

The proposed structure is:

```text
Celadon Hideout -> Pokémon Tower / Fuji rescue --+
                                               +-> Viridian Giovanni
Silph Co. rescue ------------------------------+
```

Either investigation can be completed first, and the player can alternate between
them. Silph's entrance and local puzzles do not require Fuji, the Scope, the Flute,
or completing Celadon. Giovanni's first-meeting and repeat-meeting dialogue must
reflect the order actually played, including victory lines that currently assume
an earlier loss.

Viridian's Giovanni confrontation becomes available after both investigations
are resolved: Celadon's Giovanni has been defeated and its Scope obtained, Fuji
has been rescued, and Silph's occupation has been ended. Outstanding ordinary
reward collection must remain recoverable; claiming the Silph Master Ball is
not a finale prerequisite. Neither Snorlax encounter is required.

Remove the existing specific-Kanto-badge access condition for this finale. Do not
replace it with TR, a total badge count, or a League clear. This is a direct story
dependency preserving Giovanni's reveal and withdrawal, not a late-reward gate
on an ordinary badge.

Keep his final battle, Earth Badge, and normal reward sequence intact. The main
operations must already be resolved so he does not announce Team Rocket's end
while still running Silph or the Celadon hideout.

Smaller Rocket incidents, including Mt. Moon, Nugget Bridge, and the burglary,
remain optional and available afterward. Review their dialogue, visibility, and
the finale's global cleanup so they remain coherent and are not silently removed.
Prefer small conditional variants. Do not add all minor encounters as mandatory
prerequisites merely to preserve an unconditional line about Rocket's defeat.

### Late-game rewards

Use the [late-game reward exception](johto-independent-story-beats.md): delay
specifically named powerful rewards through Trainer Rating while leaving their
ordinary adventures playable. TR determines eligibility; it is not spent.

| Reward | Required treatment |
| --- | --- |
| Silph Master Ball | The President offers it immediately after Silph liberation, with no TR threshold, a success-only receipt, and full-pocket retry. Follow the [implemented reward specification](../specs/silph-president-master-ball.md). Elm's later Master Ball remains; two are intentional. |
| Silph Lapras | Keep as an ordinary local gift with its existing giver and one-time delivery behavior. No legendary-tier TR gate is added. |
| Articuno and Zapdos | Preserve Seafoam and Power Plant exploration and local puzzles. Gate each capture encounter by legendary readiness, without requiring the other bird or a Rocket story. |
| Mewtwo | Use a higher TR readiness threshold than the birds, replacing the unrelated Celio Ruby/Sapphire campaign dependency. Preserve Cerulean Cave and the existing encounter rather than add a new capture quest. |
| Cut, Surf, Strength, Flute, fossils, and ordinary gifts | Retain their local acquisition requirements without late-game TR gates. |

Ending Silph's occupation, returning its civilians, receiving ordinary rewards,
and proceeding toward Giovanni must not wait for Master Ball collection. The
President's unclaimed reward survives travel, save/reload, and a failed Bag handoff.

The player may discover and explore the birds' sites before capture readiness.
Keep Articuno's boulder/current puzzle and the applicable field-move requirements.
A high encounter level alone is not a gate. Unmet readiness must not consume or
permanently hide an encounter.

For Mewtwo, the port specification must identify the appropriate local access
boundary in Cerulean Cave. It must enforce the readiness requirement without
requiring completion of Sevii's unrelated delivery chain. No new League-clear
condition is introduced by this PRD.

Exact thresholds for the birds and Mewtwo remain balance decisions against
the wider journey. Mewtwo's threshold is higher than the birds'. The Silph
Master Ball has no readiness threshold. All alternate legendary activation
paths must obey the final readiness rules.

### Rival continuity

Keep FRLG's rival scenes at their existing locations. Preserve genuinely necessary
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
  ownership, and transport integration are explicitly deferred. Do not resolve
  those questions by interpreting this document as an approved merge design.
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
Rocket incidents or the President's unclaimed Master Ball.

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
  progress. Verify Ticket, Cut, ship departure, outstanding rewards, and deferred
  rival handling together.
- Discover the Tower problem before visiting Celadon. Follow locally understandable
  guidance, obtain the Scope normally, and return to complete the rescue and Flute.
- Finish Silph before Celadon/Lavender, then test the opposite order and interrupted
  interleaving. Giovanni's dialogue and each adventure's state remain truthful.
- Approach Viridian with neither investigation resolved, with each alone resolved,
  and with both resolved. Only both enable its finale; badge count and TR do not
  add requirements. Unclaimed Master Ball and unfinished Snorlax encounters do not
  prevent the finale.
- Finish Silph at different Trainer Ratings. The city recovers and the President
  offers the Master Ball immediately at each rating. Receive it once, including
  after a full-pocket failure and save/reload. Preserve Elm's separate reward.
- Complete both Safari objectives in either order and verify Surf and Strength
  come from their respective givers. Neither objective requires Koga or another arc.
- Complete Mansion/Secret Key/Blaine without unrelated campaign progress.
- Resolve the Snorlax encounters in either order after obtaining the Flute. Travel
  without resolving either must not require a capture or story victory.
- Test Articuno and Zapdos discovery below readiness and capture eligibility at
  the chosen boundary. Neither requires the other. Test Mewtwo's higher boundary
  without Celio's quest, including alternate access and save/reload paths.
- Visit later rival locations early, finish their host adventures where possible,
  then progress the rival and return. No chapter or reward is silently lost.

Players unfamiliar with FRLG should understand each local problem and its next
step. Players familiar with it should recognize the original adventures and
Giovanni's finale without a replacement plot.

## Open questions

1. Which existing Lavender interactions give the clearest minimal lead toward the
   Scope when the player discovers the Tower first?
2. Which dialogue and visibility changes preserve optional minor Rocket incidents
   after Giovanni's withdrawal without adding mandatory prerequisites?
3. Which rival chapters genuinely depend on earlier scenes, and how can the S.S.
   Anne departure and other temporary scenes preserve them without blocking the
   local adventure? Review the League appearance separately from access to Silph.
4. What TR thresholds govern the birds and Mewtwo? Where should
   Mewtwo's readiness be enforced within the existing Cerulean Cave access flow?
5. Which existing local presentation explains each legendary readiness
   requirement?

## References

- [Johto story design and late-game reward exception](johto-independent-story-beats.md)
- [Hoenn independent story beats](hoenn-independent-story-beats.md)
- [FRLG open-world regional traversal](frlg-open-world-region-traversal.md)
- [FRLG traversal specification](../specs/frlg-open-world-region-traversal.md)
