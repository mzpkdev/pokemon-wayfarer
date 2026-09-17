# Cinnabar interior port blocker

## State

The task adds HNS-addressable, FRLG-layout copies of Cinnabar's Center, Mart,
Lab, Gym, and Mansion. They are empty and unlocked: no trainers, encounters,
story, rewards, keys, quizzes, or switches. Center healing/respawn and the Mart
inventory are local services.

`make -C game -j4 e2e CXX=g++` links successfully at 33,417,388 bytes (99.59%
of the 32 MiB ROM).

## Blocker

On Cinnabar's FRLG exterior, stepping into the Mansion entrance does not run its
declared warp. The same mismatch previously prevented Route 20's FRLG Seafoam
entrances: source FRLG metatile behavior values are interpreted through the HNS
behavior table, so door tiles are not recognized as doors.

The imported graphics, layout data, coordinates, and warp events are present.
The required next change is a behavior translation for the FRLG tile values used
by the imported Cinnabar exterior and interior layouts. It must cover at least
doors, stairs, holes, and Mansion/Gym interactive barriers, then be verified in
SkyEmu before restoring an end-to-end journey.

## Takeover order

1. Implement and test the behavior translation.
2. Re-enable an emulator journey for exterior entrances and every interior warp.
3. Validate Center healing and Mart purchase.
