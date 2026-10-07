# Hoenn phone rematches

Implemented: No

Design status: draft. Terms follow the [glossary](player-trainer-rating.md#glossary)
and [daily world slots](daily-world-slots.md#terms).

## Intent

Johto and Kanto route trainers such as Joey give you their number, call you on
the Pokégear and ask for rematches with stronger teams. Hoenn's route trainers
had the same relationship in Emerald through Match Call, but in Wayfarer it
does nothing. Their scripts still say "Registered … in the POKéGEAR", yet
nothing is registered and no rematch ever happens.

This design brings Hoenn's rematch trainers onto the Pokégear phone so that
every region's route trainers work the same way.

## Design

### Who

The 64 ordinary trainers in Emerald's rematch table, from Rose on Route 118 to
Kira and Dan on the Abandoned Ship. Five of them are pairs, such as Lila and
Roy. Emerald's special Match Call contacts (Wally, the Gym Leaders, the Elite
Four and the Champion) are out of scope. Notable trainers own them.

### On the phone

A Hoenn phone trainer behaves exactly like an HNS phone trainer:

- **Number:** after you beat them, they offer their number (see
  [daily world slots](daily-world-slots.md#phone-numbers-and-rematches)).
- **Calls:** they call now and then to chat, and call when they want a
  rematch.
- **Rematch teams:** each rematch uses their next authored Emerald team, up to
  five. Teams Emerald skips, such as Cindy's second, stay skipped. Levels
  follow [regular trainer scaling](trainer-party-scaling.md), which already
  picks the rematch team first and then scales it.
- **Gifts:** none. Emerald's Hoenn trainers never gave gifts by phone, and
  this design doesn't add any.

### Call texts

The calls reuse Emerald's own Match Call texts. Emerald gives each trainer one
personal chat plus their own pick from shared sets of battle chats and battle
requests. Each trainer keeps that same selection, converted into the
Pokégear's per-trainer call format.

- References to the PokéNav or Match Call become the Pokégear.
- Lines that only make sense in Emerald's story order, such as the few that
  mention Team Magma or Team Aqua, get neutral wording.
- Battle requests name where the trainer is today, as
  [daily world slots](daily-world-slots.md#phone-numbers-and-rematches)
  requires. Emerald's "different route" requests already do this ("You can
  find me around {place}").

### Before daily world slots

If this ships before daily world slots, Hoenn phone trainers stay at their
home spots and readiness works as it does for HNS phone trainers today: 255
steps on their home map. Battle requests then name the home map.

## Boundaries

- No new trainers, teams or maps. All 64 trainers, every rematch team and the
  call texts already exist and are compiled into Wayfarer.
- HNS phone trainers keep their current texts, gifts and team order.
- The PokéNav's other Emerald features, such as the condition and ribbon
  screens, stay out.

## Content

- **Trainers:** 64 rematch entries. Every team they reference is already
  defined in Wayfarer's trainer data.
- **Texts:** Emerald's 319 Match Call text blocks, which Wayfarer already
  includes but never shows. They need converting and auditing, as above.
- **Hoenn map scripts** already run the register and rematch steps. They start
  working once the trainers are in the rematch table.

## Constraints

- **Wayfarer only.**
- **Rematch table:** Wayfarer compiles HNS's table, which reuses Emerald's
  entry names for different people (Emerald's Rose slot holds Joey). Hoenn
  trainers get their own new entries and never reuse an HNS one. Entries 28–63
  are empty today and can take 36 of them. The other 28 need the table to grow
  past its 90 entries.
- **Growing the table** hits three limits, all of which must be raised:
  - The phone registration flags sit right before the HNS extended content
    flags and must move to a larger free flag range.
  - The saved rematch progress array holds 100 entries.
  - The Pokégear contact list holds 99 rows, special contacts included, with no
    bounds check.
- **Save layout:** Wayfarer has no released saves, so moving these blocks
  needs no migration (see `AGENTS.md`). Cost: one byte and one flag per new
  trainer, under 100 bytes in total.
- **Badge level ceiling:** HNS caps rematch team levels by badges with a table
  tuned to Johto and Kanto. Regular trainer scaling already sets levels, so the
  ceiling must not cut Hoenn teams short. Check this during implementation.

## Playtesting

- With about 100 route trainers able to call, does the phone ring too often?
- Do converted Emerald texts read naturally next to HNS's?

## Open questions

- Should call frequency drop as the contact list grows, so more contacts don't
  mean more interruptions?

## References

- [Daily world slots](daily-world-slots.md)
- [Regular trainer scaling](trainer-party-scaling.md)
- [Notable trainers](notable-trainers.md)
