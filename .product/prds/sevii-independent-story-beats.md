# Sevii independent story beats

Status: Draft product design for a future Wayfarer story adaptation. Not
implemented. Bounded scene decisions and the shared bird-capture TR threshold
remain open.

## Intent

Let players discover Sevii's local adventures in different orders without following
FRLG's original first-trip and postgame itinerary. Preserve the existing characters,
locations, battles, puzzles, ordinary rewards, and most dialogue.

Keep Lostelle's rescue as a small adventure and Celio's gem investigation as a
connected adventure with two branches. Island transport and core services remain
available regardless of which story the player starts or postpones.

## Design

### Independent local adventures

Introduce adventures through existing local characters. Remove unrelated Gym,
Champion, Pokédex, and distant quest activation requirements. Preserve meaningful
local preparation, such as field moves for optional caves, friendship for a gift,
or an actual item for a delivery.

Keep necessary internal scene order. An early visit must not complete an unseen
event, consume a later scene, or grant its reward. Use a brief introductory or
completion-conditioned dialogue variant when the original assumes an earlier
event. No new NPCs, factions, maps, replacement plots, or quest-selection interface
are needed.

| Adventure | Preserved content | Required independence or local connection |
| --- | --- | --- |
| Lostelle and Three Island trouble | The biker confrontation, investigation and clues, Berry Forest encounter, and reunion with her father. | Discover and complete the adventure without Bill's Meteorite delivery, Blaine, or the original forced first trip. Keep its internal sequence and existing local actors. |
| Bill's Meteorite delivery | Bill gives the Meteorite; Lostelle's father receives it and gives the Moon Stone. | Keep separate from travel entitlement and the rescue's activation. Delivery can follow the reunion immediately or on a later visit when the player has the item. |
| Lorelei and Icefall Cave | Explore the cave, help Lorelei confront the poachers, and complete the rescue. | No Celio, mainland Giovanni, or League-clear prerequisite. Preserve applicable cave and field-move preparation. |
| Selphy and Lost Cave | Find Selphy, complete the existing battle, and accompany her return home. | Make this a local adventure. Her later Pokémon requests remain its direct follow-up. |
| Memorial Pillar | Learn about Tectonix, leave Lemonade, and receive the existing reward. | Preserve the local item requirement and reward giver without unrelated progress checks. |
| Water Labyrinth Egg | The existing giver recognizes care for the player's Pokémon and offers the Egg. | Preserve the friendship and delivery-capacity requirements. Do not add a campaign or TR gate. |
| Tanoby Key and Ruins | Solve the boulder puzzle and enable the existing Unown appearances. | Keep the puzzle-to-encounter connection without Celio or Rocket completion requirements. |
| Smaller services and challenges | Existing tutors, Pokémon-showing requests, and facilities. | Retain their local eligibility. Celio's quest must not become a blanket requirement to use island services. |

### Travel and core services

Preserve the [FRLG traversal design](frlg-open-world-region-traversal.md)'s
independent island travel entitlement. Once that service is unlocked, accepting
or completing an adventure never removes the established island destinations or
the return to Kanto.

Do not require the Ruby for travel to Islands Four through Seven. Celio's story
does not consume, replace, or duplicate an already granted travel pass. Early
arrival initializes only the local state needed for the visit; it does not mark
Lostelle, the bikers, the Meteorite, the gems, or the Warehouse complete.

Remove the original temporary PC-storage shutdown from the story adaptation.
Accepting Bill's errand or Celio's repair request must not disable storage, regional
travel, or another existing core service. Retain the repair story without making
normal play depend on the player finishing it immediately.

Departure from an island does not finish a detour. The player can leave during a
rescue, return later, and continue from the actual next step.

### Lostelle and the Meteorite

Lostelle's father can ask for help without the player carrying Bill's Meteorite.
Existing Three Island events provide the local investigation and route to the
Berry Forest rescue. Preserve the Hypno encounter and reunion without adding a
capture requirement or a late-game reward gate.

The rescue and Bill's delivery have separate completion and reward ownership:

- Rescue Lostelle without ever accepting the Meteorite. Her reunion completes
  normally and is not held open by an unrelated missing delivery.
- If the player already has the Meteorite, retain the existing handoff after the
  reunion and its Moon Stone reward.
- If the player accepts the delivery later, her father accepts the actual item
  on return and grants the same reward once. The rescue does not replay.
- Accepting Bill's delivery after rescuing Lostelle must not make her disappear
  again, re-stage the bikers, or rewind island progress.

The father may remain focused on the rescue before accepting the delivery; this
is a retained local scene connection, not a requirement to start the rescue
through Bill. Neither reunion nor delivery may narrate an item handover when the
item is absent.

Bill's existing role and reward ownership remain. His local introduction must
become available without requiring Blaine's defeat and the forced Cinnabar trip.
The exact existing interaction used for that introduction is a bounded scene
decision; it must not create a new transport unlock quest.

### Celio's branching gem investigation

Celio introduces a repair that needs both the Ruby and Sapphire. The player can
investigate the two leads in either order:

```text
Celio's two-gem request
    |
    +-> Mt. Ember: Ruby and first Warehouse password --------+
    |                                                      |
    +-> Dotted Hole: Sapphire theft and second password ----+
                                                           |
                                                Rocket Warehouse
                                                           |
                                                Recover Sapphire
                                                           |
                                            Return both gems to Celio
```

Keep the existing Mt. Ember exploration, Rocket encounters, Ruby, and first
password. Keep Dotted Hole's puzzle, the scientist's theft of the Sapphire, and
his second password. These are genuine clues and causes, not unrelated gates.

Either lead may be investigated first after Celio's request. The Dotted Hole
lead ends with the theft and second password; recovering the Sapphire still
requires both passwords and the later Warehouse confrontation. Neither lead
requires Lostelle, the Meteorite delivery, Lorelei, a Champion clear, or an
unrelated Pokédex milestone. Returning the Ruby is not required to start the
Sapphire investigation.

The Warehouse requires both learned passwords. Its confrontation and Sapphire
recovery follow the theft; entering early must not produce a Sapphire that has
not been stolen. The player can discover the locked Warehouse before either
branch, but discovery does not teach its passwords or complete the investigation.

Celio accepts each actual recovered gem independently and acknowledges which
one remains outstanding. Both delivery orders work. The player may learn the
first password, pursue the Sapphire, and return it before delivering the Ruby;
the final repair occurs only after both gems have actually been delivered.

No new gem reward or replacement villain plot is introduced. Preserve the repair's
existing local presentation and completion acknowledgement, with minimal dialogue
changes to explain the parallel request and partial deliveries.

### Lorelei, Dotted Hole, and the Warehouse ending

Icefall Cave is its own rescue. Lorelei's information about the Warehouse can
remain an optional lead, with a variant if that investigation is already complete.
It is not a prerequisite for discovering or entering Dotted Hole.

Remove the dependency where Icefall completion removes Dotted Hole's blocking
scientist. Make that site's existing puzzle and Sapphire scene available through
Celio's investigation instead of an unrelated rescue. Preserve the scientist's
role in the theft without asserting that Lorelei sent the player there.

The Warehouse confrontation must work before or after Giovanni's mainland finale.
If that finale occurred, preserve the administrator's recognition of Giovanni's
badge and the news of Rocket's disbandment. Otherwise, use a short variant in
which the administrator acknowledges defeat and abandons this operation without
claiming Giovanni has quit.

Neither ending completes the mainland Rocket story or requires it to have happened.
The player's statements, references to their badge, and the administrator's later
intentions must all match the same history. Do not invent a new faction to explain
the alternate order.

Icefall and the Warehouse remain mutually recoverable. Finishing the Warehouse
first must not erase Lorelei's rescue; finishing Icefall later must not reopen or
rewind the Warehouse. Apply the same rule to shared island NPC staging.

### Late-game rewards and Celio's completion

Moltres remains at Mt. Ember. Exploration of the mountain and the Ruby adventure
do not require Moltres eligibility or capture. Apply the same bird-capture TR
policy as [FRLG Kanto's Articuno and Zapdos](frlg-kanto-independent-story-beats.md)
to the Moltres encounter. The birds do not require one another.

Explain unmet readiness through existing local presentation, preserve the
encounter for a later visit, and cover every activation path. A high encounter
level alone is not a readiness gate. TR establishes eligibility; it is not spent.

Do not apply a late-game TR requirement to Lostelle, the Meteorite reward, the Egg,
the memorial offering, the Tanoby puzzle, or Celio's final repair. Their ordinary
rewards and local prerequisites remain intact.

Celio's completion resolves his repair adventure. It must not implicitly award
Champion status, change League progression, or bypass Mewtwo's separate readiness
policy through the original Cerulean Cave unlock. Removing those unrelated
side effects must not suppress the local repair conclusion.

The future port specification must identify which original communications effects
remain meaningful in Wayfarer. This PRD does not redesign trading or promise a
new network feature. Normal existing services remain available while the quest
is unfinished.

## Boundaries

- Cover FRLG Sevii's ordinary island adventures for a future Wayfarer adaptation.
  No gameplay implementation or standalone FRLG behavior change is claimed here.
- Birth Island and Navel Rock need separate optional-legendary designs. Their
  credentials, puzzles, and rewards are not silently included in this contract.
- HNS integration, regional geography merging, and shared NPC or pass identities
  remain separate port decisions.
- Preserve original NPCs, battles, puzzles, encounter identities, and ordinary
  reward owners. No new cast, factions, quest journal, or replacement campaign.
- Do not redesign ferry routes, facility rules, species eligibility, tutors,
  encounter levels, or battle scaling. Remove only the named unrelated story
  requirements from existing local access.
- Do not change TR calculation or League qualification. The exact shared bird
  threshold is owned by the broader reward-balancing decision.
- The rival's later island scenes retain necessary personal-story order. Early
  visits cannot force premature dialogue or consume a later scene; they must
  not gate local island adventures. Exact chapter handling belongs in the spec.
- Script state, exact revised dialogue, reward failure handling, and map events
  belong in a subsequent implementation specification.

## Interactions

Keep adventure progress distinct from transport, system completion, and each
other's rewards. Revisiting a location recognizes prerequisites satisfied in
either order without requiring another unrelated interaction to activate them.

Save/reload, battle losses, regional travel, full Bag or party capacity, and
postponed handoffs must preserve a recoverable next step. Do not consume an
item or quest state before its required handoff succeeds. Completed puzzles,
rescues, passwords, and one-time rewards must not replay after another quest
changes the location's presentation.

This design revises the original-story isolation behavior described by the
[FRLG traversal specification](../specs/frlg-open-world-region-traversal.md):
early island visits can now discover their locally available adventures rather
than wait for the old Cinnabar detour. Preserve that specification's independent
travel and return guarantees. Replace its retained PC shutdown with the rules
above, and do not restore the original Ruby-to-travel coupling.

## Playtesting

- Discover and complete Lostelle without Bill's introduction or the Meteorite.
  Then accept and deliver the Meteorite without replaying the rescue or bikers.
- Accept the delivery first, rescue Lostelle, and perform the existing reunion
  handoff. Test the Moon Stone and rescue rewards independently when delivery
  initially fails.
- Leave and return during each rescue and delivery. All established ferry
  destinations and PC storage remain available, and departure never completes
  the adventure.
- Begin Celio's request without the original first-trip errands, a League clear,
  or an unrelated Pokédex milestone. Complete the two investigation branches
  in both orders and deliver the gems in both possible orders.
- Approach the Warehouse with neither password, each password alone, and both.
  Only both enable its adventure; early arrival cannot recover an un-stolen gem.
- Complete Celio's repair below the Moltres/Mewtwo readiness thresholds. Its
  conclusion works without changing Champion or League state or bypassing
  Cerulean Cave's separate reward rule.
- Complete Icefall before the gem investigation, during it, and after the
  Warehouse. Lorelei and Dotted Hole remain independently available, and leads
  refer to the correct state of the Warehouse.
- Resolve the Warehouse before and after Giovanni's mainland finale. No version
  claims an unearned badge or an unplayed disbandment event.
- Complete Lost Cave, Memorial Pillar, the Egg gift, and Tanoby independently
  of all major island quests. Keep their local requirements and normal rewards.
- Explore Mt. Ember and obtain the Ruby below bird readiness. Verify Moltres
  eligibility at the chosen threshold, including revisits and save/reload, without
  requiring Articuno, Zapdos, or completion of Celio's repair.
- Visit later rival-scene locations early and return when eligible. Unrelated
  adventures never depend on playing those scenes, and no chapter is consumed
  merely by arrival.

Players should understand each island problem through local introductions.
Players familiar with FRLG should recognize the same rescue, theft, exploration,
and reward scenes without learning a replacement plot.

## Open questions

1. Which existing Bill/Celio interaction introduces the Meteorite locally without
   Blaine's invitation or the forced Cinnabar trip? Keep it separate from Celio's
   two-gem request and the transport introduction.
2. Which small request and partial-delivery dialogue changes explain Celio's two
   parallel investigations while preserving his existing repair conclusion?
3. What exact Warehouse variants cover Giovanni still being active, and which
   Lorelei lines need adjustment after the Warehouse has already been cleared?
4. Which communications effects of Celio's original repair remain applicable to
   the future port, after removing PC disruption and unrelated postgame writes?
5. What is the shared bird-capture TR threshold and the minimal local explanation
   for returning to Moltres later? Birth Island and Navel Rock remain separate.

## References

- [FRLG Kanto independent story beats](frlg-kanto-independent-story-beats.md)
- [Johto story design and late-game reward exception](johto-independent-story-beats.md)
- [FRLG open-world regional traversal](frlg-open-world-region-traversal.md)
- [FRLG traversal specification](../specs/frlg-open-world-region-traversal.md)
