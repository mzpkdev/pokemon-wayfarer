# Wayfarer legacy multiboot removal

## Intent

Wayfarer removes three hardware-specific multiboot features that do not serve
its core GBA experience: the Pokemon Colosseum/GameCube client, the Ruby and
Sapphire Berry Glitch Fix transmitter, and the e-Reader transfer program. The
three embedded payloads occupy 191,700 bytes before any related code and
graphics are counted. Recovering that space gives the combined-region build
more room under the 32 MiB ROM limit.

This is a Wayfarer product decision. It does not remove the payload files from
the source repository or require standalone Emerald, FireRed, LeafGreen, or
HNS builds to drop their original hardware compatibility.

## Design

Wayfarer does not include or offer any of these features:

| Feature | Embedded payload | Wayfarer user impact |
| --- | ---: | --- |
| Pokemon Colosseum/GameCube client | 163,840 bytes | A GameCube cannot start the client from Wayfarer's copyright screen. |
| Ruby/Sapphire Berry Glitch Fix | 15,348 bytes | The title-screen button chord no longer opens the Berry update transmitter. |
| e-Reader transfer program | 12,512 bytes | Wayfarer cannot send the program to an e-Reader or import Trainer Hill cards through it. |

The unavailable features are removed from the Wayfarer ROM and from every
Wayfarer user-facing route. A removed route must not appear as an empty menu
item, an unexplained communication error, or a screen that cannot complete.
The copyright, title, and main-menu flows continue normally when the related
hardware or button chord is present.

Standalone source builds may retain all three features. Each feature has its
own build-time capability flag, defaults to enabled for the standalone
products that support it today, and may be disabled independently. Wayfarer
forces all three capabilities off so that changing a local build option cannot
silently produce a ROM that conflicts with the Wayfarer product definition.

Removing the e-Reader transfer route makes e-Reader Trainer Hill card import
unavailable in Wayfarer. Existing prerelease save data does not create an
exception. Wayfarer ignores e-Reader Trainer Hill data and uses ordinary
built-in Trainer Hill content. The separate saved visiting-trainer format and
its Mystery Gift command remain available wherever current non-e-Reader paths
support them. This change does not add a save migration.

## Boundaries

- Ordinary GBA cable trading and battling remain available.
- Existing GBA Wireless Adapter trading, battling, and Union Room behavior
  remain available.
- Wireless Mystery Gift remains available wherever the current Wayfarer build
  supports it. The shared Mystery Gift screen and link callback are not part
  of the e-Reader removal.
- Cable Club rooms named "Colosseum" are ordinary GBA battle rooms. They are
  not the Pokemon Colosseum/GameCube client and must remain.
- Trainer Hill remains playable with its built-in challenge data.
- This decision does not remove Mystery Gift cards, ordinary event scripts,
  the Enigma Berry item behavior, or battle code merely because a symbol name
  mentions e-Reader.
- The three binary payload files remain in the repository for standalone
  builds. Wayfarer excludes them at compile and link time.
- This work does not redesign the title screen, main menu, link rooms, battle
  facilities, or save format beyond the fallbacks needed to make the removed
  routes unreachable and safe.

## Compatibility

Wayfarer intentionally loses compatibility with Pokemon Colosseum and Pokemon
XD workflows that depend on the embedded GameCube client, with sending the
official Berry Glitch Fix to Ruby or Sapphire, and with e-Reader card transfer.
A user who needs those archival hardware paths must use a compatible
standalone build.

No public Wayfarer save-compatibility promise exists. A prerelease save that
contains transferred Trainer Hill data may still load if its surrounding save
format is accepted, but Wayfarer does not expose that data. A separately
stored, valid visiting-trainer record may retain its existing behavior.
Ordinary party, inventory, story, Trainer Hill, trading, battling, and Mystery
Gift state must not be cleared as a side effect.

## Constraints

The raw payload saving is 191,700 bytes, or `0x2ECD4` bytes. The linked
Wayfarer ROM is expected to shrink by at least that amount when compared with
an otherwise identical build that enables the three payloads. Removing
feature-specific code and graphics may save more. Alignment may change where
the final linked address lands, so the size report and `__rom_end` are the
authorities for the accepted result.

The change must not rely only on linker garbage collection. Wayfarer's
compile-time configuration must prevent the payload definitions and their
entry points from being built as active features.

## Acceptance criteria

1. Normal and release Wayfarer builds succeed with all three capabilities
   forced off.
2. The Wayfarer map and symbol outputs contain none of the three payload
   start/end symbol pairs and no live route to their transmit or execute code.
3. The Wayfarer `__rom_end` decreases by at least 191,700 bytes against an
   otherwise identical payload-enabled baseline, and the ROM remains within
   the active Wayfarer size ceiling.
4. Copyright-screen startup, title-screen input, save loading, and the main
   menu work without the removed hardware paths.
5. The Berry update chord does not enter a hidden or incomplete screen, and
   GameCube link activity cannot divert Wayfarer startup.
6. No e-Reader option is shown. Stale Trainer Hill transfer data cannot replace
   built-in challenges, and invalid visiting-trainer data cannot open a room
   or start a battle.
7. Ordinary cable and wireless trading and battling still work, including the
   Cable Club battle rooms whose names contain "Colosseum".
8. Wireless Mystery Gift still reaches its existing supported receive and send
   flows and returns safely to the main menu.
9. Standalone builds compile with their default compatibility settings and
   with each legacy capability disabled independently.

## References

- [Wayfarer runtime foundation specification](../specs/wayfarer-runtime-foundation.md)
