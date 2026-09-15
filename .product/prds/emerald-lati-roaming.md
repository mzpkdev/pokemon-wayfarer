# Emerald Lati roaming

Status: Approved product design. Not implemented.

## Intent

Restore Emerald's postgame Latias or Latios hunt in Wayfarer. The player's TV
answer should release the selected Pokémon into Hoenn, where it behaves like the
original persistent roamer, without replacing or leaking into Wayfarer's Johto
and Kanto roaming encounters.

Preserve Southern Island as the companion reward: an Eon Ticket lets the player
encounter the sibling they did not release into Hoenn.

## Design

### Postgame choice

After the player first clears the Hoenn League, retain the existing return-home
scene. Norman gives the S.S. Ticket, the television reports an unidentified
flying Pokémon, and Mom asks which color the report named.

| Answer | Hoenn roaming encounter |
| --- | --- |
| Red | Level 40 Latias |
| Blue | Level 40 Latios |

The answer is final for that Hoenn playthrough. It controls both the roaming
species and the Southern Island counterpart. Rewatching the news, revisiting the
house, clearing another League, or repeating the Hall of Fame must not reroll the
choice, recreate a resolved encounter, or change another region's roamers.

Preserve Emerald's presentation and wording wherever it remains accurate. The
scene does not identify the Pokémon in the Pokédex merely because the player saw
the report; normal Pokédex seen and caught behavior begins with an actual
encounter.

### Hoenn roaming behavior

The selected Pokémon roams only across Emerald's authored Hoenn route network.
It moves as the player changes maps and can appear through the existing roaming
encounter rules when player and Pokémon occupy the same eligible route.

Its individual identity persists for the life of the encounter. Personality,
individual values, current HP, status, and other saved battle attributes survive
fleeing and regional travel. Catching or defeating it resolves only this Hoenn
roamer and prevents it from returning.

Roaming encounters retain their authored level and remain outside ordinary wild
level scaling. This feature does not make Latias or Latios easier to locate,
prevent them from fleeing, or change their battle behavior.

### Regional coexistence

Wayfarer treats Johto, Kanto, and Hoenn as separate roaming territories. Each
roamer remains in the routes authored for its own regional quest. Starting,
restarting, encountering, catching, or defeating a roamer in one territory must
not alter a roamer in another.

The supported simultaneous set is:

- Johto's Raikou and Entei;
- HNS Kanto's selected Latias or Latios; and
- Emerald Hoenn's selected Latias or Latios.

The Kanto and Hoenn Lati choices are independent. They may select the same
species. Wayfarer preserves both regional quests and rewards rather than forcing
one campaign's answer to complete or rewrite the other.

### Hoenn Southern Island

After the Hoenn television choice is committed, owning the Eon Ticket makes
Hoenn's Southern Island destination available from the Emerald S.S. Tidal
service. The island contains the opposite sibling from that choice:

| Hoenn roamer | Southern Island encounter |
| --- | --- |
| Latias | Level 50 Latios holding Soul Dew |
| Latios | Level 50 Latias holding Soul Dew |

This is a static scripted encounter rather than a roamer. Running or otherwise
leaving without resolving the battle keeps it available. Catching or defeating
it resolves the Hoenn Southern Island encounter once. A later Hall of Fame clear
must not restore a resolved encounter.

The Eon Ticket is a shared Wayfarer travel credential: the same owned Key Item
may authorize each campaign's own Southern Island destination. Each island uses
its own regional choice and encounter state. This feature does not introduce a
new way to obtain the ticket.

## Boundaries

- Apply these changes to Wayfarer. Standalone Emerald and standalone HNS retain
  their existing behavior.
- Preserve the source species, levels, route network, movement rules, encounter
  odds, battle behavior, Soul Dew reward, dialogue, and audiovisual presentation.
- Do not redesign Eon Ticket acquisition, S.S. Tidal's unrelated destinations,
  ordinary wild encounters, Pokédex rules, Trainer Rating, or legendary
  readiness policy.
- Do not merge Kanto and Hoenn Lati choice, capture, defeat, or island state.
- Do not require a prerelease save migration. The project has no public save
  compatibility baseline.

## Interactions

The feature must remain recoverable through save and reload, blackout, fleeing,
regional travel, and either order of regional campaign completion. A player can
activate the Hoenn roamer before or after the Johto and Kanto roamers and obtain
the same four independent encounters.

The television scene commits its answer and completion only after the Hoenn
roamer exists. An internal capacity or initialization failure ends the one-time
cutscene safely and leaves a short retry interaction with Mom. The player can
free a roaming slot and choose again; no species is committed until creation
succeeds.

Ticket presentation state belongs to the dock showing it. Showing the Eon Ticket
at a Hoenn dock must not consume it or resolve either island encounter.

## Playtesting

- Clear the Hoenn League and choose each color on separate saves. Confirm the
  corresponding level-40 species appears only on eligible Hoenn routes.
- Activate Hoenn first and HNS first. Confirm Raikou, Entei, the Kanto Lati, and
  the Hoenn Lati coexist and retain their own locations and battle state.
- Flee after damaging or inflicting status on the Hoenn roamer, travel between
  regions, save and reload, and encounter it again with the same identity and
  battle state.
- Catch and defeat each regional roamer separately. Confirm only the resolved
  encounter disappears and no Hall of Fame or TV replay recreates it.
- Obtain the Eon Ticket and use the Hoenn S.S. Tidal service. Confirm Southern
  Island contains the opposite species at level 50 with Soul Dew for each choice.
- Obtain the Eon Ticket before making the Hoenn television choice. Confirm the
  Hoenn Southern Island destination and encounter remain unavailable.
- Run from the Southern Island encounter and retry it. Catch or defeat it and
  confirm it does not return, including after another Hoenn Hall of Fame clear.

## References

- [Wayfarer Hoenn integration](wayfarer-hoenn-integration.md)
- [Wayfarer Hoenn content port](../specs/wayfarer-hoenn-content-port.md)
- [Emerald open-world region traversal](../specs/emerald-open-world-region-traversal.md)
