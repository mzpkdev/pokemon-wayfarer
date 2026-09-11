# Four trainer appearance styles

Status: Approved four-style v1 scope (2026-09-09); implementation and validation in progress.
Specification: [Trainer appearance styles](../specs/trainer-appearance-styles.md)

## Intent

Let players choose their trainer's appearance from the protagonists of HNS
and Emerald. Oak asks about appearance, and the player sees
what each choice looks like before starting their Wayfarer journey.

## Design

### Four choices

Replace Oak's boy/girl question in the Wayfarer introduction with:

> What do you look like?

Present these four choices in this fixed order. Character names in this table
identify the artwork; the menu displays only the style label.

| Label | Appearance | Source |
| --- | --- | --- |
| Style 1 | Gold | HNS |
| Style 2 | Kris | HNS |
| Style 3 | Brendan | Emerald |
| Style 4 | May | Emerald |

Saved IDs remain Gold=1, Kris=2, Brendan=5, and May=6. Menu labels are
Style 1 through Style 4; a row number is not a saved appearance ID. IDs 3 and 4
remain reserved for deferred Red/Leaf and are invalid in v1 saves and setup.

All four styles are available immediately. None requires a particular origin,
region, achievement, or unlock. Style 1 is selected on first entry.

The highlighted choice shows a trainer portrait and a small overworld preview.
Up and Down move through the four rows and wrap at either end. A confirms the
visible style and continues to name entry. B leaves this required choice open.
There is no separate style confirmation question.

Returning to appearance selection after rejecting the name preserves the last
selected style. The player can keep it or choose another. Naming and Oak's
closing animation must continue to show the selected character.

### One appearance for the journey

The selected style persists into the opening, normal play, saving, and Continue.
It stays the same when travelling between Johto, Kanto, and Hoenn, changing
movement modes, entering battles, or opening player information screens.

The appearance covers walking and running, cycling, surfing, diving, fishing,
field actions, the naming preview, battle sprites, Trainer Card portrait, and
Hall of Fame presentation. Each style must retain its selected character through
every supported action. Substituting a different protagonist is not an acceptable
fallback. Red and Leaf are deferred from v1; completing and validating their
missing action animations is outside this release.

Styles do not change movement capabilities, battle performance, encounter
rules, progression, or access to regions. Selecting Brendan in Johto does not
select a Hoenn origin or start a different story.

### Existing story behavior

This feature changes the appearance question and avatar selection. Existing
gender-dependent dialogue, default-name lists, household setup, and rival
selection retain their current male/female behavior: Gold and Brendan
use the existing male behavior; Kris and May use the existing female
behavior. For example, the existing Hoenn rival rule still applies to a player
using a Hoenn protagonist's appearance.

This is an explicit scope decision for the first version. It does not introduce
a separate pronoun choice or rewrite gendered dialogue across the adventure.
The selected appearance has its own identity, so rendering does not infer the
character from these story rules or from the current region.

## Boundaries

- Applies to Wayfarer. Standalone HNS, FRLG, and Emerald introductions and
  avatar behavior retain their existing choices.
- Includes exactly these four complete appearances. Custom outfits, recoloring,
  additional protagonists, and changing style after starting are outside scope.
- Keeps the existing introduction order, name entry, challenge setup, regional
  origin choice, and story content except for the appearance prompt and previews.
- Does not change NPC or rival artwork to match the player's style.
- Does not redesign multiplayer or recorded-battle formats. Representations
  that only carry legacy gender data retain their existing remote appearance;
  the local player's normal game screens must honor the saved style.
- Does not migrate prerelease saves. This follows the repository's current save
  compatibility policy.

## Presentation

Use the introduction's existing pixel art, textbox, font, sounds, and background.
Fit all four labels into one vertical menu on the GBA screen without scrolling
or clipping the question. Place the portrait and overworld preview beside it.
Keep enough separation that the cursor and both previews remain readable.

Switch previews as the cursor moves. During transitions, prevent a confirmation
from accepting a style different from the one visibly presented. The final
selection uses the same character in the portrait and overworld preview, with
correct palettes and no leftover pixels from the previous choice.

Preserve the original protagonists' designs, including outfit, colors,
proportions, and the action being performed.
The implementation specification owns animation coverage and validation.

## Constraints

The implementation must fit the existing ROM size limit and sprite/palette
budgets. Reuse existing art where it supports the required animation correctly.
Any missing frames are part of implementing the four styles, rather than a
reason to silently reduce the available actions or ship a partial character.

Existing saves from the implemented version must restore the style reliably.
Starting another new game resets the choice to Style 1; it must not inherit a
previous save's appearance.

## Playtesting

- Can a player distinguish all four appearances and understand the choice
  without seeing the source-game or character names?
- Are the four labels, cursor, portrait, and overworld preview legible together
  at native GBA resolution?
- Does quickly navigating and confirming always select the style shown?
- Do all four styles look consistent while biking, diving, watering, and using
  field moves, as well as while walking and battling?
- Does rejecting a name, choosing either origin, saving, and travelling retain
  the expected style throughout?

## References

- [Regional start choice](wayfarer-regional-start-choice.md)
- [Repository save compatibility policy](../../AGENTS.md#save-compatibility)
- [Oak's current introduction](../../game/src/oak_speech_hns.c)
- [Existing avatar graphics aliases](../../game/include/constants/event_objects.h)
