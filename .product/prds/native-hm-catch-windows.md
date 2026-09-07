# Native HM catch windows

Status: Core implementation complete; full route acceptance pending.
Implementation: [Production changes and acceptance evidence](../research/native-hm-windows/revisions/implementation/README.md)
Specification: [Native HM catch windows](../specs/native-hm-catch-windows.md)
Current design: [Nearby-access distribution and route audit](../research/native-hm-windows/revisions/nearby-access/README.md)
Attachments: [Original distribution and research package](../research/native-hm-windows/attachments.md)

## Intent

Let players find field-move users among a wider range of compatible wild
Pokemon. Which species knows a utility when caught should change with its
level, while the world continues to offer useful options across Trainer Rating
(TR) 0-80.

The existing design repeats utility entries to keep a few anchor species
useful at every level. This crowds their learnsets and concentrates too many
roles on individual species. Replace that requirement with overlapping catch
windows across the roster. A player who keeps an already-known move can keep
using that Pokemon beyond its wild-catch window.

## Scope and authority

This PRD replaces the roster, repeated-entry, permanent-anchor and successor
reminder requirements of [Native HM utility learnsets](native-hm-learnsets.md)
for Wayfarer. Wayfarer includes Johto, HNS Kanto and Hoenn together. Standalone
Emerald, FireRed, LeafGreen and HNS builds remain outside this revision.

The selected nearby-access revision contains 121 species and 154 species/utility
roles. Its exact modern and legacy levels, caught-level windows, locations,
enumerated encounter replacements and route evidence are attached. Its
`proposal.json` is the current distribution authority. Preserve the original
117-species, 148-role approval as a historical attachment, not the implementation
target. Static simulation passing does not establish production acceptance.

## Design

A fresh catch receives its ordinary level-derived moves. A young Tentacool may
know Surf, while a higher-level Staryu supplies that role later. The player can
catch a suitable companion for their current Rating without searching for one
permanent designated HM species.

Utility moves remain ordinary battle moves. They use normal move slots and may
be kept, replaced or forgotten. The design preserves original native learnset
entries, not every original four-move wild moveset: adding a utility can still
displace another move on a particular fresh catch.

Owning an HM continues to provide the existing compatible-party field-use
route without requiring the move to be known. Native utility distribution is
an alternative acquisition route, not a replacement for HM rewards.

## Distribution requirements

- Add an assigned utility only once in each learnset mode. If it already occurs
  natively, preserve its original occurrence instead of adding another.
- Preserve all original native moves, levels and relative order. Remove the
  previous feature's injections, not unrelated native moves or other features.
- Allow at most two authored native utility types per selected species,
  including pre-existing utilities. Evolution paths involving the roster must
  not introduce a third type through inherited and successor-native moves.
- Use existing species compatibility. Prefer plausible anatomy, behavior or
  elemental identity; numerical coverage alone is not enough.
- Stagger levels and overlap catch windows across species. No species must
  retain every utility in its generated wild moveset at every later level.
- Support both modern and legacy learnsets. The roles stay consistent between
  modes, but learning levels and catch windows may differ.
- Keep original utility occurrences elsewhere unchanged, including unrelated
  baseline duplicates. The rule prohibits newly introduced repetition.

The eight counted utilities are Cut, Flash, Surf, Strength, Rock Smash,
Waterfall, Whirlpool and Dive. Fly remains excluded. The cap governs authored
native roles; it does not restrict a player's manually taught, bred or copied
moveset.

The effective roster attachments include native roles as well as additions.
Their counts describe the selected revision, not permanent quotas for future
balancing.

## Coverage and travel

Maintain regional encounter-table coverage for all eight utilities at every
integer TR from 0 through 80 in both modes. Qualify practical acquisition
separately: a claimed source must be reachable without the utility it is
supposed to provide, and the player must be able to return. A detour through
nearby routes or the next town is acceptable. Do not require same-town catches
when an accessible nearby source is sufficient. Record longer cave detours,
earlier traversal prerequisites and rod availability rather than assuming them
away. A required source must work at the current time; waiting for night or day
is not part of this guarantee.

Coverage is a property of the available roster, not each individual species.
The old two-places-per-anchor rule and the requirement that Chinchou or Wailmer
personally cover every Rating no longer apply. Evolution-only, optional and
special-area entries may enrich the roster but cannot stand in for ordinary,
reachable acquisition evidence.

Native Surf may continue to support the existing approved crossings. Test
both land-connected sides of Olivine/Cianwood, mainland Kanto/Cinnabar, Route
118, and Lilycove/Mossdeep/Pacifidlog. A Surf encounter on water does not prove
that a player on shore can obtain Surf. The design assumes a prepared player
with capture supplies and a reachable rod; it does not add emergency recovery
after depositing, releasing or forgetting the last Surf user.

The selected revision tests 11 acquisition scenarios, including Blackthorn's
Surf approach and Dragon's Den Whirlpool requirement. Each must provide an
8% or better chance of a catch actually knowing the required move at one
reachable source, across TR 0-80, both learnset modes and both clock cases.
The qualifying source can be an ordinary land encounter or a successful Old
Rod fishing encounter. Sum eligible carrier outcomes within that source;
never add probabilities from different places or methods. For fishing, 8%
means 2% per unmodified cast at the Old Rod's 25% bite rate. Land-encounter
probability is conditional on encountering a Pokemon, not on each step.

Dragon's Den requires local Whirlpool acquisition before the shrine obstacle,
after Clair's defeat, with Surf already available. Blackthorn's Surf acquisition
also has a qualifying local source at every tested TR and time. The local carrier
handover must meet the same 8% floor; neither badge 8 nor the Rocket Hideout HM
may be assumed to obtain it. The optional Blackthorn detour through Ice Path
is allowed only with the cleared puzzle needed for a return trip; neither
mandatory Blackthorn acquisition depends on taking it. Whirl Islands
and the timing of their access are explicitly out of scope.

The code-traced nearby-access simulation passes the selected scenarios. The
original same-shore Lilycove TR 0-1 gap and sub-8% findings remain historical
evidence; they are addressed through the revised roster, nearby sources and
enumerated encounter edits, not an unresolved choice among the old Lilycove
slot alternatives. This is static evidence under documented route conditions,
not a universal save-state or emulator-play certificate.

Only `encounter_replacements` in the selected proposal are authorized ecology
changes. They are the limited exception to Standard Rod's original no-table-edit
boundary. Preserve global weights, bite rates, selection rules and all
unlisted encounter entries. Broad ecology edits remain outside this design.

## Evolution, reminders and presentation

Evolution preserves already-known moves. Listed descendants receive their own
single utility entries. Do not add blanket level-one reminder copies. An
unlisted descendant retains its native learnset, so an inherited utility may
not be relearnable after it is forgotten. That tradeoff is intentional; any
new descendant assignment must obey the same cap and single-entry rules.

Existing move lists and field-use prompts remain sufficient. This revision
does not require a new UI, tutorial or innate field-skill system. If habitat
hints are authored later, describe level-dependent availability rather than
claiming a species always knows the move.

## Boundaries

Do not change HM item locations, rewards, move power, accuracy, PP, battle
effects, terrain checks, Trainer Rating calculation or encounter projection.
Keep field-use eligibility and Dive authorization unchanged. A native Dive
move does not bypass Steven's Hoenn grant or the existing HNS authorization.
Do not activate a field action in maps where it is not supported merely because
the roster contains the move.

Do not add encounter-specific movesets, post-catch move injection, runtime
utility retention, protected party slots or a separate innate-ability system.
Randomized species/learnsets remain outside the authored coverage guarantee.
Existing owned Pokemon are not retroactively rewritten. No save migration is
required for this prerelease data change.

## Acceptance

Accept the implementation only when:

1. Both production learnsets match the attached effective distribution and
   preserve native entries, ordering, compatibility and table limits.
2. No new repeated utility entry or third authored family utility is introduced.
3. Production wild creation matches the declared catch windows and regional
   TR coverage. A Python model alone is not sufficient acceptance evidence.
4. Critical crossing sources are verified on both sides, including applicable
   times and rod qualities. Meet the 8% single-source floor in all 11 selected
   scenarios, including the local Den Whirlpool handover; verify return paths
   and the documented Ice Path condition.
5. Evolution, forgetting, reminder behavior, HM-owned fallback and Dive gates
   behave as specified; standalone builds retain their existing behavior.
6. The attachment package remains available and the shipped revision's results
   are distinguishable from the original research snapshot.

## References and attachments

- [Technical specification](../specs/native-hm-catch-windows.md)
- [Attachment index and authority](../research/native-hm-windows/attachments.md)
- [Selected distribution](../research/native-hm-windows/revisions/nearby-access/roster.md)
- [Selected catching locations](../research/native-hm-windows/revisions/nearby-access/locations.md)
- [Nearby-access findings and limitations](../research/native-hm-windows/revisions/nearby-access/README.md)
- [Original findings and distribution](../research/native-hm-windows/README.md)
- [HM field use](hm-field-use.md) and [Standard Rod fishing](standard-rod-fishing.md)
