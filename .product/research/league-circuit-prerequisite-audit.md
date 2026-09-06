# League circuit implementation and prerequisites

The circuit is not ready to enable. `WAYFARER_LEAGUE_CIRCUIT_ENABLED` stays
false until the native openings and travel acceptance below are complete.
The global badge helpers, Rating producer, utility learnsets, and fixed League
data can be verified independently. The guarded scripts and Trainer Card
status prepare the remaining integration without enforcing certification caps.

## Missing opening contracts

The [circuit specification](../specs/wayfarer-interregional-league-circuit.md)
requires an exact approved opening and starter transaction before a region can
appear as a new-game choice. The merged contracts define the existing Johto
opening, but neither Kanto nor Hoenn has that native-start transaction.

- Kanto needs an opening on the included HNS Kanto maps, a starter selection
  and branch-state transaction, and a safe recovery/release point. Standalone
  FireRed opening scripts are not included in Wayfarer.
- Hoenn needs its initial party and native starter transaction, opening scene,
  and safe recovery/release point. The [Hoenn content port](../specs/wayfarer-hoenn-content-port.md)
  explicitly says its existing-party Birch rescue is a visitor branch and
  cannot define Hoenn's native new-game opening.
- Both starts need an explicit initial S.S. Aqua entitlement. The approved
  loop is Olivine to Vermilion to Slateport to Olivine. Every regular departure
  checks `VAR_SSAQUA_STATE >= 8` and the S.S. Ticket; only the current Johto
  opening can reach the maiden voyage before obtaining those credentials.
  Choosing initial voyage state and ticket delivery must accompany the opening
  transactions. The direction and existing port hooks remain unchanged.

No alternative opening, starter gift, ship story, or new start choice is
invented by this implementation. All three clean-save starting journeys remain
unverified and block full circuit acceptance.

## Actual League venues and Trainer identities

Wayfarer includes HNS and Emerald maps. It does not include the standalone
FireRed Route 23 guards or FireRed Hall of Fame. Kanto Tier 1 and Johto Tier 2
reuse the shared HNS Indigo Plateau League rooms. Their regional results must
come from the circuit tier, rather than the shared venue's geographic region.

Kanto uses the existing HNS first parties at levels 48 through 56. Johto uses
the existing second parties at 66 through 70. Hoenn's five existing League
parties receive fixed Wayfarer levels 86 through 92. The source species,
items, moves, AI, party sizes, and numeric Trainer IDs remain intact.

The machine-readable [tier inventory](../../game/tools/wayfarer_league_tiers/inventory.json)
records all 15 IDs and the complete level lists. The parallel ordinary Trainer
scaling task must rerun its generated classification inventory after integration.
These League IDs remain excluded. No ordinary or Gym Leader roster is
redesigned here. Increasing authored levels establishes a tier order; it does
not establish gameplay balance without playtesting.

## Threshold locations and travel evidence

Every possible threshold badge belongs to one of these regional Gym groups:

| Region | Gym locations |
| --- | --- |
| Kanto | Pewter, Cerulean, Vermilion, Celadon, Saffron, Fuchsia, Seafoam Islands (Blaine), Viridian |
| Johto | Violet, Azalea, Goldenrod, Ecruteak, Cianwood, Olivine, Mahogany, Blackthorn (badge delivered in Dragon's Den) |
| Hoenn | Rustboro, Dewford, Mauville, Lavaridge, Petalburg, Fortree, Mossdeep, Sootopolis |

The existing HNS map connections provide the mainland approach through
Viridian, Route 22, Route 26 North, Reception Gate, Kanto Victory Road,
Route 23, and Indigo Plateau. Johto also connects through the Route 27 spine.
The original Reception Gate checks Johto Badge 8 and the Ecruteak theater
story; the circuit branch removes those travel checks. Qualified players
bypass the forced Victory Road Silver scene without completing its story.
League-room admission uses the circuit predicates. These changes are directly
authorized by the circuit spec and require no additional story decision.

The retained [HNS traversal contract](../specs/hns-open-world-region-traversal.md)
provides settlement roads and native Surf connections, including Cianwood and
Cinnabar. Late Blackthorn/Dragon's Den and Blaine's Seafoam badge location need
explicit return-route acceptance as well; the opening-settlement contract
alone is not proof for those thresholds.

Hoenn threshold locations use the existing network to Slateport and the
directional Aqua leg to Olivine for the first two Leagues. The final League
requires the existing ocean approach to Ever Grande and Victory Road. Ordinary
field-move rules allow prepared native users without another badge, but static
map adjacency alone does not prove collision, elevation, script, or recovery
behavior. The Hoenn opening traversal contract explicitly excludes Ever Grande.
Sootopolis, its authorized Dive return, Ever Grande's Waterfall approach, and
Victory Road need focused runtime travel verification before caps can enable.

## Clears and releases

The circuit clear helper validates the pending tier, commits only that
region's Champion/game-clear state, and raises the Rating high-water mark.
An explicit transient handoff supplies the recorded region to `GameClear`;
that routine must not infer Kanto or Johto completion from the shared map.
The circuit script skips the old HNS rematch reset heap, which respawns
unrelated encounters and rewrites another region's story state.

The intended post-credits recovery destinations are the existing Indigo
Plateau Center for both HNS tiers and Ever Grande League Center for Hoenn.
These avoid replaying a native home opening or implicitly transporting the
player between regions. Tests of the stored continuation warp supplement,
but do not replace, a real credits/save/reload and departure journey.

## Acceptance still required

Before enabling the switch, approve and implement the missing native starts,
verify all three clean new-game saves and Aqua entitlements, and exercise
threshold-to-League and post-clear return routes for every location above.
The postponed Whitney and Clair rewards need interaction-level retry coverage
as well as the shared admission tests. Fixed League parties need balance
playtesting across mixed routes. The integrated ordinary-scaling inventory
must classify the final League sources and aliases again.

Compilation, source tests, and deterministic ROM mechanics tests are reported
separately from emulator travel or balance playtesting. This audit makes no
claim that the latter has been completed.
