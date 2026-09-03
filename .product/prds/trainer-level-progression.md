# Trainer Level progression

## Intent

Let players explore Johto, Kanto, and Hoenn in any order without ordinary
battles pushing their party far ahead of the world. Wayfarer measures progress
through completed accomplishments, raises one global Trainer Level, and uses
that level to pace the world and Pokemon growth wherever the player travels.

The system is a progression model, not a difficulty option. It should support
players who follow one regional story, alternate between several stories, or
spend most of their time on optional quests.

## Design

### Trainer Experience and Trainer Level

Wayfarer has one global Trainer Level from 10 through 100. A new game begins at
Trainer Level 10 with no Trainer Experience toward the next level.

Badges, first regional League clears, approved story milestones, and approved
quests grant Trainer Experience. Every 100 Trainer Experience raises Trainer
Level by one. Excess Experience carries into the next level, and one reward can
raise more than one level. Trainer Experience cannot be lost, Trainer Level
cannot decrease, and Experience stops accumulating at Trainer Level 100.

Ordinary Pokemon battles, catches, rematches, travel, location discovery, item
collection, and other repeatable actions do not grant Trainer Experience. The
player advances by completing durable accomplishments rather than by grinding.

All three regions contribute to the same Trainer Experience total. Regional
badge, story, and Champion states remain independent for their own scripts,
but their approved accomplishments advance the same Trainer Level. No region
has a separate Trainer Level.

The reward schedule is intentionally badge-heavy:

| Accomplishment | Trainer Experience |
| --- | ---: |
| First acquisition of any regional Gym Badge | 300 |
| First regional League or Champion clear | 600 |
| Registered major story milestone | 100 |
| Registered substantial quest | 50 |

Each reward is granted once from a durable completion fact. Repeatable content
and events without a permanent completion fact are ineligible. The shipped
reward manifest must include at least 1,800 Trainer Experience from story and
quest accomplishments, with at least 600 available in each region before that
region's Champion clear. Reaching Trainer Level 100 therefore does not require
every registered quest or every regional League clear.

Trainer Rating is not part of Wayfarer. Standalone Emerald, FireRed,
LeafGreen, and HNS builds may retain their existing Trainer Rating behavior.

### World scaling

Trainer Level is Wayfarer's only global progression input for wild, Trainer,
Gym, and boss scaling. Scaling never reads the current party level, highest
party level, or party average. Changing the party therefore cannot lower the
world or make an encounter easier.

Each consumer may translate Trainer Level into an appropriate effective level.
Ordinary wild Pokemon retain the current projection's distinction between
safer and more dangerous authored areas. Some high-authored profiles may remain
above Trainer Level, while ordinary Trainers, Gyms, and major bosses use their
own offsets and roster rules. Trainer Level does not make every encounter
exactly the same level and is not a universal enemy-level ceiling.

The existing wild encounter projection remains the basis for ordinary wild
levels. Wayfarer reparameterizes its Rating 10 through 80 progression domain
across Trainer Levels 10 through 100. This preserves the existing curve,
authored-area retention, profile offsets, predecessor resolution, species
floors, and monotonic results without storing or exposing Trainer Rating.

Detailed Trainer party projection, roster tiers, moves, items, and battle
formats belong to a separate Trainer scaling specification. That specification
must use Trainer Level as its progression input and must preserve authored
Trainer identity.

The soft growth limit cannot ship until that Trainer scaling specification and
its required boss-path validation are implemented. A player following any
supported progression route must never need to grind through the reduced
Experience bands merely to reach the level expected by the next required
battle.

### Pokemon growth

Trainer Level is also the efficient training limit for each Pokemon. Pokemon
below it receive normal battle Experience. Pokemon at or above it continue to
receive Experience at a reduced rate. The reduction applies to each party
member independently, so a lower-level Pokemon can catch up while established
members grow slowly.

The limit is soft. It never removes levels, blocks a capture or trade, prevents
a Pokemon from entering battle, or makes an over-level Pokemon disobey. A
player may deliberately train beyond Trainer Level, but ordinary exploration
should not do so quickly by accident.

Rare Candies and Experience Candies are deliberate bypasses. They retain their
normal effect after warning the player when their use would raise a Pokemon
above Trainer Level. Daycare is passive and stops adding Experience once a
Pokemon reaches Trainer Level.

Wayfarer always uses this growth model. The HNS challenge menu's OFF, NORMAL,
and HARD level-cap choices do not appear in Wayfarer and do not combine with
Trainer Level.

## Boundaries

- Trainer Level does not order regional stories, gate regional travel, or
  require the player to finish one region before beginning another.
- It does not promise that every encounter is equally difficult. Species,
  moves, team composition, items, AI, EVs, and available resources still
  matter.
- It does not scale Battle Frontier or other battle-facility content, link
  battles, secret-base teams, or other explicitly level-normalized formats.
- Fixed, gift, legendary, roaming, hidden DexNav, scripted, outbreak, Feebas,
  Battle Pike, and Battle Pyramid Pokemon keep their existing authored or
  mode-specific level behavior unless another specification enrolls them.
- Trainer party composition and tier selection require their own specification.
- Each regional content owner must register its exact story and quest reward
  manifest before Trainer Level progression is enabled in a release build.
- Standalone Emerald, FireRed, LeafGreen, and HNS behavior remains unchanged.

## Balance

A regional set of eight badges and its first League clear awards 3,000 Trainer
Experience, equal to 30 Trainer Levels. Across Johto, Kanto, and Hoenn, badges
and first League clears provide enough Experience to reach Trainer Level 100
from the starting level. Story and quest rewards let players reach the same
destination through a less completionist mix of regional content.

Battle Experience uses this starting schedule:

| Pokemon level before an Experience portion | Battle Experience received |
| --- | ---: |
| Below Trainer Level | 100% |
| Equal to Trainer Level | 25% |
| One level above Trainer Level | 10% |
| Two or more levels above Trainer Level | 5% |

The schedule should make one or two deliberate extra levels possible while
making sustained accidental over-leveling unlikely. It should not interfere
with bringing a newly caught or rotated Pokemon up to the current world band.

## Presentation

The Trainer Card shows Trainer Level, Trainer Experience toward the next level,
and the current efficient Pokemon training limit. At Trainer Level 100 it shows
MAX instead of an Experience remainder.

An eligible accomplishment reports the Trainer Experience earned. If the
reward raises Trainer Level, the level increase is shown after the reward and
before control returns to the player.

The first reduced battle-Experience award explains that Pokemon at or above
Trainer Level grow more slowly. Later awards use the normal Experience
presentation without repeating the tutorial.

## Interactions

### Wild encounters

All current ordinary-population consumers use the Trainer-Level projection,
including regular encounters, ordinary land and water DexNav populations,
Pokedex area checks, Match Call and radio species selection, and local ambient
species selection. The selected profile, slot weights, encounter method, time
of day, rod partition, ability effects, lure behavior, and randomizer ownership
remain unchanged.

### Trainer battles

Trainer Level is sampled when a Trainer battle begins. The resulting enemy
party remains fixed for that battle. A badge or Champion reward earned after
the victory affects later encounters; it does not retroactively change the
battle or its Pokemon Experience calculation.

### Pokemon above Trainer Level

Caught, gifted, scripted, and traded Pokemon retain their received level. An
over-level Pokemon receives the reduced battle-Experience rate appropriate to
its own level. The system does not alter its stats, moves, friendship, EVs, or
eligibility for other forms of training.

## Constraints

Trainer Experience and Trainer Level are global Wayfarer state rather than
regional Hoenn state. They must survive travel, save and reload, blackout, and
regional League completion.

Wayfarer has not established public save compatibility. The implementation may
replace the prerelease Trainer Rating state and revise the Wayfarer save layout
without migrating prerelease saves. Future released save formats must preserve
Trainer Experience and derive the same or a higher Trainer Level after a data
update.

The system must fit within the existing save-sector, EWRAM, ROM, generated-data,
and release-headroom limits. Reward registration and wild projection must be
data-driven and auditable.

## Playtesting

- Does free exploration stop pushing an established party far beyond Trainer
  Level while still rewarding ordinary battles?
- Can a newly caught or rotated Pokemon catch up without changing the world
  level for the rest of the party?
- Can players reach useful Trainer Levels by completing content in different
  regional orders, and can they reach 100 without completing every quest?
- Can a prerequisite-valid route reach Trainer Level 100 while leaving at
  least one regional Champion undefeated?
- Do repeated interactions, rematches, losses, reloads, and shared scripts
  avoid duplicate Trainer Experience?
- Do ordinary wild populations preserve their current ecological identity at
  every Trainer Level from 10 through 100?
- Do high-authored profiles that exceed Trainer Level remain understandable as
  dangerous places rather than appearing to ignore world scaling?
- Do Candy warnings, daycare limits, and the first reduced-Experience tutorial
  make deliberate and incidental over-leveling understandable?

## References

- [Trainer Level progression specification](../specs/trainer-level-progression.md)
- [Trainer Rating wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Wayfarer Hoenn integration](wayfarer-hoenn-integration.md)
- [HNS open-world regional traversal](hns-open-world-region-traversal.md)
