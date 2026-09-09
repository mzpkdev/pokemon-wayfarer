# FRLG Kanto story delivery milestones

PRDs: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md), [independent story beats](../prds/frlg-kanto-independent-story-beats.md)

Status: Partial implementation of the umbrella story design.

## PR #86 delivery boundary

The user has limited the scope of [PR #86](https://github.com/mzpkdev/pokemon-wayfarer/pull/86)
to the eight completed story milestones below and the completed Silph Master
Ball reward. Future adventures and their product decisions are separate
deliveries and do not block this PR. Each linked specification defines its
own implemented behavior; this index does not mark the full Kanto PRDs complete.

“Implemented” means present on `task/frlg-kanto-story-implementation` for PR
review. It does not mean merged, released, or that every validation has passed.
Shipping still requires review, required CI checks, and a passing production
ROM budget. This scope decision does not authorize merge or deployment.

| Milestone | Implemented scope | Status |
| --- | --- | --- |
| [Machine Part chronology](frlg-kanto-machine-part-chronology.md) | Repair, Cape date, and Misty's Gym return in their retained order | Implemented in PR #86 |
| [Persistent S.S. Anne](frlg-kanto-persistent-anne.md) | Interiors, captain's Cut, any SS Ticket boarding, appended Vermilion slot 6, safe dock return | Implemented in PR #86 |
| [Celadon Hideout](frlg-kanto-celadon-hideout.md) | Rocket investigation, Giovanni, and Scope | Implemented in PR #86 |
| [Fuchsia Safari and Teeth](frlg-kanto-safari-teeth.md) | Safari Surf and Gold Teeth, visiting Baoba's Strength exchange | Implemented in PR #86 |
| [Mt. Moon](frlg-kanto-mt-moon.md) | Four Rockets, Miguel, and persistent fossil choice | Implemented in PR #86 |
| [Cerulean burglary](frlg-kanto-cerulean-burglary.md) | Robbed household, Rocket, and recovered TM | Implemented in PR #86 |
| [Nugget Bridge](frlg-kanto-nugget-bridge.md) | Ordered challenge, prize, and recruitment scene alongside HNS scenes | Implemented in PR #86 |
| [Silph liberation](frlg-kanto-silph-liberation.md) | Floors, Card Key puzzle, occupation battles, Giovanni, Lapras, and restored lobby services | Implemented in PR #86 |
| [Silph Master Ball](silph-president-master-ball.md) | Immediate post-liberation President reward with full-pocket retry; Elm's later reward remains | Implemented in PR #86 |

Steven has acknowledgement-only dialogue in this delivery. His universal
starter reward is approved as a future feature, but its unfinished draft and
capacity behavior are not part of this PR's implemented scope.

## Future deliveries

Each row requires its own bounded implementation specification and validation.
The order may follow resolved dependencies, with Cinnabar last by user request.
No row below is an acceptance requirement for PR #86.

| Future milestone | Settled direction and remaining decisions |
| --- | --- |
| [Steven starter reward draft](silph-steven-starter-reward.md) | One Hoenn starter for any origin, independently of Silph liberation. Full-party plus full-PC behavior remains unresolved. |
| Bill rescue | Preserve rescue and a no-duplicate Ticket reward, including for existing Ticket holders. Resolve Bill/grandfather placement while preserving requests and the Galarian machine. |
| Tower, Fuji, and physical Flute | Preserve the Tower adventure and Fuji reward. A ground-floor radio studio with persistent upper floors was discussed, but the layout and transition are not finalized. |
| Snorlax and legendary sites | Resolve retained Vermilion Snorlax, Route 12/16 physical-Flute encounters, Power Plant layout, and legendary readiness. Silph Master Ball timing is already settled and implemented. |
| Giovanni finale and Celebi | Viridian Gym Giovanni uses the FRLG leader roster with Wayfarer scaling and awards the initial Earth Badge. Celebi gates require the actual finale and, where relevant, Goldenrod completion. Adapt Tohjo time/identity dialogue; Blue succession remains future work. |
| Kanto League and Blue | Separate Kanto and Johto lineups while preserving circuit guarantees. Resolve Blue Champion parties and Koga/Janine roles. Personal Blue rivalry is reserved for planned Kanto origin; current Johto/Hoenn origins are visitors. |
| Cinnabar, last | Complete intact FRLG island and selected interiors, Mansion/Key/Blaine, fossil services, and HNS route seams. No eruption obligation. |

Kanto opening/Pallet origin, Anne travel-network expansion, and the Sevii
story/map port remain separately scoped outside this PRD delivery.

## Contract precedence

The broad PRDs retain their future requirements. Their unimplemented features
must not be inferred from an existing FRLG source asset or from completing one
of the local adventures above. Local wins and rewards do not complete a host
campaign, Giovanni's finale, or a future Blue chapter.

The approved Silph Master Ball specification supersedes the PRDs' earlier
open Master Ball readiness question for that reward only. Legendary readiness
remains open. The [implementation sequence](../research/frlg-kanto-implementation-sequence.md)
retains historical measurements; fresh checks on the PR revision establish
shipping readiness.
