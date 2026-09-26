# Viridian Gym and Giovanni finale

PRD: [FRLG Kanto story on HNS maps](../prds/frlg-kanto-story-on-hns-maps.md)

Supporting requirements: [independent story beats](../prds/frlg-kanto-independent-story-beats.md) and [shared Indigo circuit](wayfarer-interregional-league-circuit.md).

Implemented: Yes. The Wayfarer Viridian Gym/Giovanni finale and first-committed-Indigo Dojo unlock are implemented and [validated](../research/viridian-finale-implementation.md) on the shared FRLG/Blue Indigo circuit.

## Gym and access

Keep the HNS Viridian City exterior and existing Gym doorway. Select the complete FRLG `ViridianCity_Gym_Frlg` 20×24 layout and arrow maze, its eight ordinary Trainers, guide, Giovanni, and both doorway directions for Wayfarer. Reuse the existing layout binary; event, warp, script, Trainer, asset, and map selection must not edit `map.bin`. Audit selected-map inclusion and rendered collision before choosing arrival positions. The Gym can be entered and explored before Giovanni is available, and remains traversable after he departs. Trainers use ordinary Wayfarer scaling and one-time defeat state; importing FRLG script names does not make their raw Trainer IDs safe to reuse.

Giovanni's challenge becomes available once the Celadon Rocket Hideout is resolved **and** Silph Co. is liberated, in either order. The Hideout's Giovanni battle and Silph Scope reward, and Silph's Giovanni battle and liberation, are the two local prerequisites. Pokémon Tower/Fuji, either Snorlax, any specific badge count, Trainer Rating, an Indigo clear, and unrelated regional progress are not prerequisites. Entering early gives truthful local guidance without granting a victory or closing the Gym. A loss leaves Giovanni available to retry and does not award a badge, TM, or finale state.

Use the FRLG final Gym team in source order: Rhyhorn 45, Dugtrio 42, Nidoqueen 44, Nidoking 45, Rhyhorn 50. Keep both Rhyhorn. These are source levels for Wayfarer's established Gym scaling, not fixed runtime levels. Preserve the Gym Leader battle identity and local dialogue where true; adapt claims about prior meetings to the order the player actually played.

## Victory, reward, and departure

Giovanni awards the single Kanto Earth Badge through existing Wayfarer badge accounting: one first-award badge and Trainer Rating contribution, with no duplicate from reload, repeated interaction, or another Gym owner. He also offers the TM that teaches **Earthquake**, selected by move semantics rather than assuming FRLG's TM26 number matches Wayfarer's item numbering. Prior ownership does not count as this reward's delivery; give one copy on the first successful handoff. A full Bag leaves that TM claimable at Giovanni. He departs only after delivery succeeds, and his departure is permanent. The badge is not granted a second time during a deferred TM claim.

Giovanni's defeat must not run the source script's broad `FLAG_HIDE_MISC_KANTO_ROCKETS` cleanup, mark optional Rocket incidents complete, erase unfinished rewards, or make the Tower or Silph story state regress. Keep the Gym, guide, arrows, and entrance usable after departure. No Blue succession, Giovanni rematch, other Viridian Leader, or repeat Earth Badge/TM path follows. Existing FRLG Gym Trainer post-victory handling must be mapped to stable Wayfarer state; it must not create repeatable ordinary battles.

Blue is removed from Viridian's exterior introduction and Gym scripts, objects, guide/statue text, and dependent dialogue. No Cinnabar-to-Viridian Gym invitation remains. His Indigo Champion role and future Kanto-origin rivalry remain separate from this Gym. The old Blue Gym Trick Room TM reward is dropped, with no replacement acquisition in this feature.

## Blue at the Saffron Dojo

The proposed [Seeded Trainer Circuit](../prds/seeded-trainer-circuit.md) changes
this unlock to the first committed scheduled clear and removes guaranteed Blue
participation at Indigo. That proposal is pending adoption; the current approved
Indigo-specific contract follows below.

Keep Blue's existing repeatable Saffron Fighting Dojo battle, authored Wayfarer party, and current Battle Point reward rules. Unlock his Dojo appearance when the **first Indigo Champion victory is committed** by the shared circuit. Starting or losing the Champion battle, entering the Hall of Fame room without a committed clear, defeating Giovanni, receiving the Earth Badge, or choosing an origin does not unlock him. Indigo can be cleared before the Viridian finale; in that order, Blue is available at the Dojo while Giovanni still leads the Gym. A later Indigo replay does not duplicate an unlock or add Battle Points. The Dojo attendant may mention Blue, but no separate invitation or quest is required. Remove Dojo dialogue that presumes a Viridian battle the player never had.

## Acceptance

- Enter and leave the full arrow maze before either investigation, after each alone, after both in either order, and after Giovanni leaves. Check all Gym warps both ways, guide, eight Trainers, collision, save/reload, and loss/retry.
- Confirm the exact five-species source team and Wayfarer Gym scaling. Win once; verify one Earth Badge/TR contribution, one Earthquake TM, a full-Bag retry, prior TM ownership, and permanent departure only after delivery. No repeated battle or reward follows.
- Finish Indigo before Giovanni and Giovanni before Indigo. Shared Indigo admission still requires at least eight global badges, with no Viridian badge prerequisite. Blue's Dojo appearance follows only a committed first Indigo win in both orders; declines, losses, replays, save/reload, party choice, and BP delivery keep existing Dojo rules without bonus reward.
- Verify no Blue object or invitation remains in Viridian or Cinnabar, while Blue still appears as Indigo Champion. Leave Tower/Fuji, Snorlax, optional Rocket scenes, and unrelated League/region state untouched by the Gym finale.
- Generate selected maps, warps, Trainer and item dependencies, and a release ROM; inspect the rendered FRLG maze and HNS city doorway. Source assets and screenshots are not runtime proof.

## Source anchors

- [FRLG Gym map](../../game/data/maps/ViridianCity_Gym_Frlg/map.json), [scripts](../../game/data/maps/ViridianCity_Gym_Frlg/scripts.inc), and [Giovanni party](../../game/src/data/trainers_frlg.party)
- [HNS city](../../game/data/maps/ViridianCity_hns/map.json), [current Gym](../../game/data/maps/ViridianCity_Gym_hns/scripts.inc), and [Saffron Dojo](../../game/data/maps/SaffronCity_FightingDojoVIP_hns/scripts.inc)
