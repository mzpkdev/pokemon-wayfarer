# Pallet Town Kanto origin opening

Status: Draft design for implementation. No runtime changes are included.
Specification: [Wayfarer Kanto origin opening](../specs/wayfarer-kanto-origin-opening.md)

## Intent

Let a new Wayfarer journey begin in Pallet Town with the recognizable FireRed
and LeafGreen opening. The player takes Red's narrative place, receives a
Kanto first partner from Professor Oak, battles Blue in the lab, travels to
Viridian City, receives Oak's Parcel from the Mart, and returns it to Oak for
the FRLG Pokédex and five-Poké-Ball handoff.

This is a small, standalone origin opening. It establishes the player's Kanto
home, partner choice, and relationship with Blue without deciding the later
Kanto campaign.

## Design

### Red's role with the player's chosen appearance

Selecting the Pallet origin makes the player the protagonist of the FRLG
opening: Pallet is their home, the resident of Red's house is their mother,
Oak knows them, and Blue is their childhood rival. "Red" describes this story
role, not a forced name, gender, or sprite.

The existing professor introduction remains authoritative for player name,
appearance, gender, and challenge settings. All four existing appearance
choices remain available: Gold, Kris, Brendan, and May. The opening uses the
selected HNS or Emerald player graphics and the matching existing gender
branch. It does not add, import, or force an FRLG Red or Leaf player sprite.

### Opening sequence

Use the existing HNS Pallet exterior and interiors rather than importing the
FRLG Pallet layouts. The opening follows the FRLG story order on those maps:

1. The player begins upstairs in Red's house, can explore the home, and can
   speak with their mother downstairs.
2. Attempting to leave Pallet to the north causes Oak to stop the player
   because the tall grass is unsafe without a Pokémon. The HNS southern coast
   exit remains unavailable to this origin until Parcel delivery is complete
   so it cannot bypass the authored opening route.
3. Oak escorts the player to his lab, where Blue is waiting and three Kanto
   partners are available.
4. The player chooses Bulbasaur, Charmander, or Squirtle under default
   settings. Blue takes the counter-choice tied to the selected local slot.
5. Blue challenges the player before they leave the lab. The story continues
   after either a win or a loss.
6. The player travels north through Route 1 to Viridian City.
7. On the first eligible visit to the Viridian Mart, its clerk asks the player
   to take a delivery to Oak and gives them Oak's Parcel.
8. The player returns to Pallet through Route 1 and delivers the Parcel in
   Oak's lab. Blue arrives as in FRLG; Oak gives both Trainers their Pokédexes
   and gives the player five Poké Balls.

The feature ends after Oak has accepted the Parcel and the Pokédex and five
Poké Balls have been successfully handed off. Daisy's Town Map is a separate
follow-up and is not part of this opening.

### Starter and challenge behavior

The three displayed local choices are Bulbasaur, Charmander, and Squirtle.
Existing challenge settings remain active and may substitute the Pokémon
actually delivered according to their current rules. The selected local slot,
not the resulting species, permanently determines Blue's counter-starter and
the Kanto rival branch.

Starter confirmation is a one-time choice. Saving, reloading, re-entering the
lab, losing the first battle, or revisiting a starter object must never grant a
second Pokémon or allow a different choice. Receipt is recorded only after
successful delivery; an interrupted or failed delivery retries the same
committed choice.

The first Blue battle uses the opening's early-rival behavior. Winning or
losing completes that encounter and leaves the player able to continue north.
No victory-only story gate or ordinary trainer retry rule is added.

### A quiet first trip to Viridian

The first Route 1 trip should read as the FRLG errand setup, not as an HNS
visitor route. HNS sight Trainers and unrelated Blue appearances must not
interrupt this opening. Existing wild encounters, ordinary walking, and the
Pallet-to-Viridian connections remain available in both directions. The
southern coast is an HNS-only escape from the FRLG sequence and stays
origin-gated until the Parcel delivery handoff is complete; this is a temporary
opening boundary, not a later regional travel decision.

The opening must not reveal later HNS or Kanto story scenes through Daisy's
house, Viridian's exterior, or the HNS Viridian Gym. Later specifications can
replace, release, or further adapt the suppressed content when they define the
rest of the Kanto-origin campaign.

### Home and recovery

Red's house is the Pallet-origin player's home and initial recovery location.
An early loss, including the first Blue battle where the engine returns to the
field, must recover safely in the house rather than New Bark, Littleroot, or an
uninitialized fallback. Later valid local healing destinations continue to
follow Wayfarer's shared recovery rules.

## Boundaries

- Wayfarer only. Standalone HNS, Emerald, FireRed, and LeafGreen retain their
  existing starts, maps, scripts, sprites, and state.
- The scope ends after returning Oak's Parcel, Blue's non-battle lab arrival,
  Pokédex receipt, and the five-Poké-Ball handoff. It does not include Daisy's
  Town Map or any later rival encounter.
- This feature does not define the Kanto Gym journey, Rocket adventures,
  Snorlax encounters, Indigo League, Blue's Champion role, Viridian Gym,
  Sevii, or later interregional travel. Their owning PRDs and specifications
  remain authoritative.
- Existing separate Red NPCs or encounters outside the selected opening maps
  remain untouched. Reconciling the narrative player role with those duplicate
  world appearances is later work.
- No later Kanto milestone, badge, League clear, campaign completion, or
  visitor reward may be fabricated to make this opening work. The Pokédex and
  five Poké Balls are authored rewards of Parcel delivery, not later progress.
- The opening adapts FRLG's story beats to HNS map geometry. It does not
  replace Pallet, Route 1, Viridian, or their interiors with FRLG layouts, and
  it does not promise coordinate-identical optional room props.
- Existing challenge settings are preserved. This feature adds no new starter
  roster, difficulty mode, appearance, sprite family, or rival naming screen.
- Prerelease save compatibility is not required, consistent with repository
  policy. Continue must nevertheless resume every committed opening state
  without replaying grants or changing origin.

## Content

The origin uses these selected HNS locations:

- Pallet Town
- Red's house, first and second floors
- Professor Oak's lab
- Route 1
- Viridian City
- Viridian City Mart

The required characters are the player, their Pallet mother, Professor Oak,
Blue, the lab assistants needed by the scene, and the Viridian Mart clerk.
The required rewards are one first partner, temporary possession of Oak's
Parcel, the Kanto Pokédex handoff, and five Poké Balls.

## Interactions

Register Pallet as the third playable origin through the shared origin-profile
framework. Its identity and progress are separate from current map region,
Johto's starter state, Hoenn's starter state, and HNS Pallet's late-game lab
state. Visiting Pallet from another origin retains the existing visitor-facing
behavior and must not start, imitate, or advance this opening.

The broader [FRLG Kanto story on HNS maps](frlg-kanto-story-on-hns-maps.md)
owns later Kanto continuity. This origin provides the narrative precondition
that a Kanto-origin player is Blue's rival, but it does not implement any of
that campaign's later encounters.

## Constraints

All new behavior and state must be origin-gated and Wayfarer-only. Reuse the
selected HNS maps and existing HNS/Emerald player assets. Allocate dedicated
Wayfarer-owned state during implementation; do not alias unavailable FRLG
flags, Johto starter variables, or HNS late-game Pallet state.

Pokémon, item, and Pokédex grants must be exact-once transactions. A receipt
marker may only commit after its delivery API succeeds, and a retry must
preserve the already committed local choice or completed portions of the
Parcel handoff. Oak must not consume the Parcel and then replay or duplicate
completed rewards after an interruption.

## Playtesting

- Does every existing appearance and gender choice remain visible and
  consistent from the professor introduction through Pallet, the lab battle,
  Route 1, and Viridian?
- Does the HNS geography still communicate the expected FRLG sequence without
  an unrelated Trainer or Blue scene interrupting it?
- Do all three local starter slots produce the correct Blue counter branch
  under default and starter-substituting challenge settings?
- After either first-battle result, can the player safely receive exactly one
  Parcel, return it once, and receive exactly one Pokédex handoff and five Poké
  Balls?
- Can saves resume before selection, after choice, after starter delivery,
  after the battle, after Parcel receipt, and during/after delivery without
  replay or duplication?
- Do non-Pallet origins retain their current Pallet, Route 1, Viridian, and
  Oak-lab behavior?

## References

- [Wayfarer regional start choice](wayfarer-regional-start-choice.md)
- [FRLG Kanto story on HNS maps](frlg-kanto-story-on-hns-maps.md)
- [HNS Kanto conflict inventory](../research/frlg-hns-kanto-story-conflicts.md)
- [Custom origin framework exercise](../research/custom-origin-framework-exercise.md)
