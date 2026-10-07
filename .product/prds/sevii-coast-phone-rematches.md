# Sevii and Kanto coast phone rematches

Implemented: No

Design status: draft. Terms follow the [glossary](player-trainer-rating.md#glossary)
and [daily world slots](daily-world-slots.md#terms).

## Intent

Wayfarer ported FireRed and LeafGreen route trainers to Sevii and to Kanto's
coast (Routes 19–21), and wired their stronger rematch teams into the
Vs. Seeker. The Vs. Seeker can't be obtained in Wayfarer, so those teams are
never seen. Everywhere else, route trainers give you their number and call
you for rematches.

This design gives the FRLG trainers phone numbers and moves their existing
rematch teams onto the Pokégear, so route trainers work the same way in every
region. The Vs. Seeker retires, as [daily world
slots](daily-world-slots.md#vs-seeker) describes.

## Design

### Who

The trainers in Wayfarer's two FRLG rematch registries who have at least one
stronger team:

| Registry | Families | With a stronger team | Teams |
| --- | ---: | ---: | --- |
| Sevii | 64 | 32 | 27 with two teams, 5 with three |
| Kanto coast, Routes 19–21 | 9 | 9 | Two teams each |

That's 41 phone contacts. A family can be a pair, such as Twins, who share one
number and battle together.

The other 32 Sevii families have only their base team, as in FireRed and
LeafGreen, whose trainer data has no rematch team for any of them. The Vs.
Seeker registry simply repeats their base team. They get no number. They still wander and battle
you again under daily world slots, and regular trainer scaling still makes
them stronger as your Trainer Rating grows.

### On the phone

They behave exactly like HNS phone trainers:

- **Number:** after you beat them, they offer their number (see
  [daily world slots](daily-world-slots.md#phone-numbers-and-rematches)).
- **Calls:** they call now and then to chat, and call when they want a
  rematch.
- **Rematch teams:** each rematch uses the family's next team, in the order
  the Vs. Seeker registry already defines. After the last team, rematches keep
  using it. Levels follow [regular trainer scaling](trainer-party-scaling.md).
- **Gifts:** none.

### Call texts

FRLG trainers never had phone calls, so their texts are new. Each contact gets
the same set an HNS phone trainer has: three chats and one battle request.

- **Voice:** each trainer sounds like their FRLG self, based on their class
  and their original battle lines. A Swimmer talks about the sea, a Black Belt
  about training.
- **Place:** chats may mention their home island or route as somewhere they
  like. Battle requests name where the trainer is today, as daily world slots
  requires.
- **Content:** 41 contacts × 4 texts, about 164 new texts. They are reviewed
  like any other authored dialogue.

## Boundaries

- The rematch teams, their order and the trainers' battle dialogue stay as the
  Sevii and coast ports authored them.
- No new teams. Families without a stronger team stay without a number.
- The Vs. Seeker's retirement belongs to [daily world
  slots](daily-world-slots.md#vs-seeker).

## Constraints

- **Wayfarer only.**
- **Save:** each family already keeps its rematch stage and a "ready" bit in
  Wayfarer's own Sevii and coast save data, outside the shared rematch array.
  The phone reuses those, and the "ready" bit now comes from a call instead of
  the Vs. Seeker. Phone registration adds one flag per contact, 41 in total.
- **Contact list:** the Pokégear lists ordinary contacts by scanning the HNS
  rematch table. It must also list these registries' contacts.
- **Shared capacity:** with HNS's Johto and Kanto trainers, the
  [Hoenn port](hoenn-phone-rematches.md) and these 41, there are about 140
  ordinary contacts plus the special ones. The Pokégear list holds 99 rows
  today and must grow. The Hoenn design covers that change, and this design
  depends on it.

## Playtesting

- Do the new texts read as the same trainers you fought?
- With about 140 route trainers able to call, does the phone ring too often?

## References

- [Daily world slots](daily-world-slots.md)
- [Hoenn phone rematches](hoenn-phone-rematches.md)
- [Sevii trainer restoration](../specs/sevii-trainer-restoration.md)
- [Regular trainer scaling](trainer-party-scaling.md)
