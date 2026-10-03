# Notable ambience

Implemented: No

Specification: [Notable ambience specification](../specs/notable-ambience.md)

Design status: the expressive layer on top of the walkers that
[notable spots](notable-spots.md) and the
[world simulation](notable-world-simulation.md) put on screen. Terms follow
the [glossary](player-trainer-rating.md#glossary). Timings, cadences, and the
first pool of beats are placeholders for playtesting.

## Intent

Notable trainers now walk the regions and spend time at spots, but on screen
they still read as robots on an errand. They march at the player's walking
speed, take exact shortest paths, and stand at a spot doing one fixed loop.
Every trainer at a water's edge does the same thing, so Misty and Giovanni look
identical there.

Ambience is what makes a walker feel like a person out for the day. They stroll.
They stop to look around. They notice the player passing by and greet a friend
they bump into. Misty watches the water and Lt. Surge does push-ups in the
grass. Now and then a trainer's ace comes out of its Poké Ball to play. None of
it changes where anyone goes or what the player can do. It changes how the
world feels to walk through.

## Design

### Spots and ambience

Two layers answer two questions:

| Layer | Question | Timescale |
| --- | --- | --- |
| [Spots](notable-spots.md) and routines | *Where* is the trainer, and why? | minutes |
| Ambience | *What are they doing* right now? | seconds |

Spots stay exactly as they are. Ambience runs only on screen, only while the
player can see the trainer, and saves nothing.

### A calmer pace

Walkers stroll at half the player's walking speed by default. They only walk at
full speed when they have a reason to hurry: stepping aside for the player, or
leaving the screen through a map edge.

### Beats

A **beat** is one short moment of behaviour, one to a few seconds long:

- a glance around;
- a music note while walking;
- a ripple and a bite at the water;
- two friends stopping to say hello.

Beats happen at spots, on the way between spots, and in reaction to the player
or another trainer:

- **On the way:** Lt. Surge, walking to the Gym, stops halfway, looks left and
  right, then carries on.
- **Noticing the player:** Erika turns her head as the player passes.
- **Meeting each other:** Brock and Misty cross paths in Pewter, stop, and greet
  each other.
- **Arriving and leaving:** a trainer looks around for a moment on arrival, and
  turns and pauses before setting off.
- **At a spot:** Whitney hums at the Mart. A trainer at the water gets a bite.

### One shared pool

All beats live in **one pool** for every trainer and region, authored once as
data. Nothing is authored per spot, and only a little per trainer:

- **Context decides which beat can fire.** That covers where the trainer is (a
  spot kind, walking, arriving, leaving) and what is around them (water ahead,
  the player nearby, a friend on the map). A fishing bite only happens facing
  water, and a greeting only happens when someone is there to greet.
- **A beat's content is fixed.** It is a short sequence of moves, icons, and
  effects.
- **Doing nothing is the common case.** Beats are spaced out, so trainers aren't
  constantly emoting.
- **Choices are deterministic.** Nothing uses the random number generator, so
  ambience can never shift encounters or battles.

### Character without per-trainer scripts

Trainers feel different without unique scripts:

- **Body-language tags** let signature behaviour emerge from the shared pool.
  "athletic" trainers do push-ups, "dreamy" ones doze off, "water-loving" ones
  admire the water, and "stoic" ones mostly stand still. Giovanni's stillness
  is his signature.
- **A few preferred beats** per trainer, like spot favourites, tip the choice
  towards what suits them.
- **Relationships** pick the greeting when two trainers meet. A short list
  covers friends and family, such as Brock and Misty or Lance and Clair.
  Colleagues (Gym Leaders of one region, one League's Elite Four) and rivals
  (the Champions) follow on their own.

### The ace comes out

At some spots, the trainer's ace appears beside them for a moment: Starmie at
Misty's water's edge, Dragonite next to Lance. It hops, the trainer smiles, and
it goes back. It needs a spare object slot, so it only happens when the map has
room.

## Boundaries

- **Spots, routines, travel, and capacity don't change.** Ambience never moves
  a trainer to another spot or map, and never changes a record.
- **No saved state.** Ambience is lost on a reload and rebuilt from scratch.
- **No dialogue.** Talking to walkers belongs to the friendship work, which
  comes later.
- **The off-screen simulation is untouched.**
- **Fame and friendship reactions are left for later.** A trainer who has heard
  of the player, or is their friend, should react to them. This needs fame and
  friendship to exist at runtime first.
- **No new sprites.** Everything uses the existing overworld sprites, emote
  icons, and field effects.

## Presentation

- **Body:** slow walking; turning to look; jumping in place; spinning; a bow.
- **Icons:** "!", "?", "!!", "…", "X", a heart, and the follower emotion icons
  (happy, music note, love, curious, pensive, sad, angry, surprised).
- **Effects:** ripples and splashes, shaking grass, dust, and sparkles.
- **The companion:** the ace's overworld follower sprite.

## Interactions

- **Spots:** each spot kind's behaviour template keeps its movement (stand,
  roam an area, change shelves, just leaving). Its occasional emote becomes the
  first beat of the pool.
- **The player:** beats stop at once when the player needs the walker out of
  the way (pushing, a 1-tile corridor), and whenever a script or story scene
  runs.
- **The following Pokémon:** a companion never hides the player's following
  Pokémon. It is the first thing to go when object slots are short.
- **Performance:** beats cost a few bytes of RAM per walker and almost no CPU;
  they must not add lag frames.

## Constraints

- No save data. EWRAM only, per the repository's RAM rules. Small ROM tables.
- Deterministic: no random number generator.
- Per-frame cost stays within the walker's existing frame budget.

## Playtesting

- Do walkers still look like they're in a hurry?
- Are beats frequent enough to notice, and rare enough not to feel like noise?
- Can players tell trainers apart by how they behave?
- Does the companion read as charming or as clutter in busy towns?

## Open questions

- Exact cadences and pause lengths (placeholders in the spec).
- Which ace comes out when a trainer has several (the spec uses the first ace in
  roster order).

## References

- [Notable ambience specification](../specs/notable-ambience.md)
- [Notable spots](notable-spots.md)
- [Notable world simulation](notable-world-simulation.md)
- [Notable trainers](notable-trainers.md)
