# FRLG Cinnabar, Seafoam Islands, and coastal routes port

Status: Approved product direction. Full FRLG Route 19, Route 20, and both
Route 21 sections belong to the final port. The unlocked Cinnabar interior
preview and retirement of the six superseded HNS maps and the unreachable
HNS Route 21 payload merged in
[PR #110](https://github.com/mzpkdev/pokemon-wayfarer/pull/110); the full town
and coastal port is not implemented.

## Intent

Restore FireRed and LeafGreen Route 19, Route 20, Route 21 North and South,
Cinnabar Island, and Seafoam Islands as one playable stretch of Wayfarer's
Kanto coast. Cinnabar is a town with its
Mansion, Blaine's Gym, Pokémon Lab, Pokémon Center, and Mart. Seafoam is the
five-floor cave with its boulder and current puzzle and Articuno encounter.
Access to this coast does not depend on an unrelated regional story being completed.

The four route maps retain their FRLG Trainers, other NPCs, signs, hidden
items, and ordinary wild encounters. Route 19 connects Fuchsia to Route 20;
Route 20 connects Cinnabar, both Seafoam entrances, and Route 19; both Route
21 sections connect Pallet to Cinnabar. The HNS Kingler blockade and its
Blaine-dependent Route 19 dialogue do not control this coast.

The [unlocked interior preview](../specs/frlg-cinnabar-unlocked-interiors.md)
is an interim playable slice. It connects twelve FRLG-layout Cinnabar interiors
to the current coast preview, with Center healing and Mart shopping. Its empty
Gym, Mansion, and Lab do not complete the local stories and services below.

## Design

### Cinnabar Island

Replace Wayfarer's erupted HNS island with the full FRLG town exterior and its
connected interiors. The island stays intact throughout this feature. Keep the
Mansion's four floors, statue switches, diaries, Trainers, items, wild Pokémon,
and B1F Secret Key. The Secret Key opens the town Gym. Keep its six quiz doors,
Gym Trainers, Blaine battle, Volcano Badge, and TM38 reward. Blaine appears and
awards the badge only at Cinnabar; the Seafoam and Fighting Dojo relocation
scenes no longer run in Wayfarer.

Keep all four Lab rooms and their trades, Metronome tutor, and fossil revival.
Helix Fossil, Dome Fossil, and Old Amber each produce their existing species
through a deposit, leave-and-return, and collection flow. Preserve the Center's
healing and Fly/whiteout destination, its second-floor service space, and the
Mart's shop. Services must have a working interaction or a clear unavailable
response; a reachable dead interaction is not an acceptable port.

Replace Cinnabar's eruption dialogue and volcanic encounters with dialogue and
encounters appropriate to the intact FRLG town. Remove the HNS New Bark shortcut
from the replaced exterior. A future eruption is outside this design.

### Seafoam Islands

Replace Wayfarer's two-floor HNS ice cave, relocated Gym, and Gym-linked Secret
Cave with FRLG's 1F through B4F. Preserve both Route 20 entrances, the floor
connections, Strength boulders, currents, and stopped-current layouts. Articuno
is the cave's single local legendary encounter on B4F. The player can explore
the cave before becoming eligible to battle or capture Articuno; an ineligible
visit cannot consume the encounter. Its readiness rule belongs to the Kanto
legendary design.

The HNS Secret Cave contains only a Groudon battle and no other reward or
activity. Do not carry that cave or a second Groudon into FRLG Seafoam. Groudon's
capture belongs to Hoenn's Terra Cave and its separate story rules.

### Routes 19, 20, and 21

Replace Wayfarer's HNS route selection and empty coast-preview events with
the complete FRLG route content. Preserve the FRLG route layouts and their
ordinary Trainers, non-Trainer NPCs, signs, hidden items, Surf and fishing
encounters, and Route 21 land encounters. The two FRLG Route 21 maps form one
continuous Pallet-to-Cinnabar crossing. Trainer battles use Wayfarer's scaling
and defeat-state rules, and route encounter sources follow its combined
FireRed/LeafGreen day/night ecology policy.

Keep both Seafoam doorways on Route 20, with FRLG's reset of unfinished
boulder paths when the player returns to the route. Preserve the open-world
native-Surf crossing without requiring Blaine's badge, a story victory, or an
HM item. The route port does not bring back the HNS Route 19 Kingler blockade,
Seafoam Gym directions, or duplicate HNS route Trainers and encounters.
The old HNS Route 19 Cave is not another Wayfarer Kyogre site; Hoenn's Marine
Cave remains the later Kyogre capture destination under its own story rules.

### Blue and independent stories

Move Blue's one-time Cinnabar introduction to the Viridian City exterior at
the Gym entrance. He invites the player inside. In this standalone port, Blue
remains Wayfarer's Viridian Gym Leader and sole Earth Badge giver; a separate
future Kanto story may assign that role to Giovanni. Meeting Blue requires no
Cinnabar visit, Blaine victory, League
clear, or other region's story. Replace references to the eruption or to a
Cinnabar meeting that did not occur. His later Dojo and League appearances
must follow their own rules rather than depend on visiting Cinnabar.

Blaine's victory does not summon Bill or start a voyage. Bill's Meteorite
delivery and all Sevii travel and stories retain their independently owned
introduction and access rules. Visiting Cinnabar, taking the ferry, and
completing the local Gym can occur in any order allowed by their own local
requirements.

## Boundaries

- This is a Wayfarer selection of existing FRLG maps and gameplay. Standalone
  HNS and FRLG retain their own maps and behavior.
- Pallet and Fuchsia are the external coast interfaces. Keep Wayfarer's
  open-world access and ordinary encounters. Choose connection and shoreline
  changes by Porymap inspection and traversal rather than assuming matching
  dimensions prove a usable crossing.
- The feature does not add an eruption state, another Blaine Gym, a Kanto
  Groudon or Kyogre encounter, a Bill-triggered Sevii unlock, or a new
  campaign order.
- Existing Hoenn Groudon story and capture rules, global badge accounting,
  Trainer Rating, and regional League admission remain owned by their designs.

## Interactions

Mansion and Gym progress is local: the player may reach the island and use its
Center, Mart, and Lab before obtaining the Secret Key. Seafoam exploration is
also independent of the Mansion and Blaine. The selected Route 20 and Route 21
connections must allow travel in both directions with Wayfarer's approved
native Surf rules, without a story victory or HM item requirement.

The port must preserve one Blaine badge award, one Articuno encounter, and
claimable one-time items and fossil rewards after travel, save/reload, a full
Bag or party, and an interrupted battle where applicable. Trainer parties use
Wayfarer's scaling rules; stock FRLG levels alone do not define Wayfarer
difficulty.

## Playtesting

- Reach Cinnabar from Pallet through both Route 21 maps and from Fuchsia
  through Routes 19 and 20. Enter Seafoam from both Route 20 sides, cross
  back, and Surf onto and off each shore without collision or camera breaks.
- Challenge every selected route Trainer, speak to the other route NPCs, claim
  route hidden items, and sample every authored wild method on each route map.
- Complete the Mansion switch and Secret Key loop, all six Gym quizzes, the
  Blaine battle and deferred TM handoff, then return after saving.
- Use every Lab service, heal and whiteout at the Center, Fly back to town, and
  shop at the Mart before and after defeating Blaine.
- Traverse both Seafoam entrances and all five floors; stop each current,
  reach Articuno, and verify an early visit does not consume it.
- Meet Blue at Viridian before visiting Cinnabar, after visiting it, and after
  beating Blaine. Each path gives the same single invitation.

## References

- [Cinnabar implementation specification](../specs/frlg-cinnabar-town-port.md)
- [Unlocked Cinnabar interior preview](../specs/frlg-cinnabar-unlocked-interiors.md)
- [HNS Cinnabar and Seafoam map retirement](../specs/frlg-cinnabar-seafoam-hns-retirement.md)
- [Seafoam implementation specification](../specs/frlg-seafoam-islands-port.md)
- [Coastal route implementation specification](../specs/frlg-kanto-coastal-routes-port.md)
- [Coastal and state integration specification](../specs/frlg-cinnabar-seafoam-integration.md)
- [FRLG Kanto independent story beats](frlg-kanto-independent-story-beats.md)
- [Sevii independent story beats](sevii-independent-story-beats.md)
