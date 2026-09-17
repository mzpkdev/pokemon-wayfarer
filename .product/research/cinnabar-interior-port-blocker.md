# Cinnabar interior port warp investigation

## Scope

The task adds HNS-addressable, FRLG-layout copies of Cinnabar's Center, Mart,
Lab, Gym, and Mansion. They are empty and unlocked: no trainers, encounters,
story, rewards, keys, quizzes, or switches. Center healing/respawn and the Mart
inventory are local services.

## Findings

The exterior Mansion entrance was reported as blocked, but its tile has the
same `0x60` nonanimated-door behavior in FRLG and HNS. A SkyEmu journey enters
it by walking north from `(8,4)` to `(8,3)`. All five exterior entrances and
their ordinary return exits work.

The actual interior failures had two causes. FRLG assigns different behavior
IDs to some functional tiles, so FRLG behavior attributes need semantic
translation before HNS field code consumes them. Separately, several copied
port layouts declare warp events on tiles whose source behavior is normal or
cave. Behavior translation cannot make those tiles into warp tiles. The
compatibility fallback is limited to declared warp events in the twelve new
Cinnabar interior layouts; other FRLG maps keep their normal warp rules.

The Center stair has elevation 4 and is entered from the east, while the
southern neighbor has elevation 3 and correctly blocks movement. Mansion
directional stairs likewise require east or west entry according to their
tile behavior.

## Verification and remaining release gate

`cinnabar-interior-port.e2e.ts` checks exterior entrances and returns, wide
side exits, Lab rooms, Center stairs and healing, Mart purchase, and Mansion
floor and basement passages through SkyEmu. The playable E2E ROM links at
33,418,432 bytes, leaving 136,000 physical bytes free. The separate 512 KiB
production reserve is therefore 388,288 bytes short at this E2E size; a
release build and size recovery are still required before merging this port.
