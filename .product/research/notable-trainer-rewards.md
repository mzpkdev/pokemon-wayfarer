# Notable trainer reward pools (review draft)

Starting **reward pools** for the 38 notable trainers in
`devtools/ui/src/modules/trainer-balance/catalog.json`. A friend at a
[haunt](../specs/notable-haunts.md#rewards-and-claims) pays for a finished
quest from their own pool, whatever the quest was: the haunt carries the ask,
and the trainer carries the reward.

Status: review draft. Nothing here is in the catalog yet. Every item is an
`ITEM_*` constant in `game/include/constants/items.h`; gates are
placeholders.

## Rules

- **Pool.** An ordered list of entries per trainer, five or six in v0 (the
  cap is 15, the most a 4-bit counter holds). An entry is an **item** or a
  **lesson**.
- **Lesson.** The trainer teaches the chosen POKéMON the first move in their
  [move pool](../specs/notable-trainers.md#move-pools) that it can learn and
  doesn't know yet. The Why column names the move most learners get first;
  the real move depends on the POKéMON.
- **From world progress.** Each entry opens once world progress (the
  player's TR) reaches its gate. Gates are plain numbers and never decrease
  along the pool. A trainer's own TR plays no part.
- **Order.** Each finished quest pays the next entry the player hasn't
  received, strictly in order. If that entry is still gated, or the pool is
  used up, the trainer pays the **fallback** instead and the counter stays.
- **Fallback.** Prize money: exactly what a win over that trainer would pay
  right now ([prize money](../specs/notable-haunts.md#prize-money)).

### Power bands

The gates follow three bands of world progress, so strong items arrive when
the world is strong too:

| Band | From world progress | What goes there |
| --- | ---: | --- |
| Modest | 0 | Type-boosting items, food and medicine, balls, valuables, contest and charm items. |
| Useful | 40 (lessons) or 50 (items) | Lessons, situational held items (Quick Claw, Scope Lens, Light Clay, weather rocks, White Herb), stones for middling lines. |
| Strong | 80 | Leftovers, Life Orb, Choice items, Assault Vest, Expert Belt, Focus Sash, Weakness Policy, Lucky Egg, and evolution items for strong lines (Dragon Scale, Protector, Razor Claw, Reaper Cloth, Dusk, Dawn, Shiny, and Fire Stone for Arcanine). |

Gates no longer depend on the trainer's own TR, so Lance and Agatha, whose
TR never changes, no longer open their whole pools at once: their entries
open as world progress rises, like everyone else's.

### Sources

Hooks come from the same places as the
[voice bits](notable-trainer-voices.md#source-priority): anime first, then
later games (FRLG, HGSS, ORAS, Emerald, Let's Go), then manga. Where there is
no canon hook, the entry fits the trainer's type, roster, or move pool.
`(verify)` marks a hook whose source needs checking.

---

## Kanto

### Brock

TR 25 → 100 (Steady).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Pewter Crunchies (`ITEM_PEWTER_CRUNCHIES`) | item | 0 | Pewter City's own snack (Let's Go); Brock cooks for everyone (anime). |
| 2 | Hard Stone (`ITEM_HARD_STONE`) | item | 0 | Rock type; his Aerodactyl holds one. |
| 3 | Lesson | lesson | 40 | Pool top: Bind, then Stealth Rock. |
| 4 | Oval Stone (`ITEM_OVAL_STONE`) | item | 50 | The would-be breeder and his Happiny (DP anime). |
| 5 | Leftovers (`ITEM_LEFTOVERS`) | item | 80 | A cook always has leftovers; suits a bulky Steelix. |

### Misty

TR 26 → 110 (Star).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Mystic Water (`ITEM_MYSTIC_WATER`) | item | 0 | Water type; her Starmie holds one. |
| 2 | Pearl (`ITEM_PEARL`) | item | 0 | Sea treasure for a would-be Water POKéMON Master (anime). |
| 3 | Lesson | lesson | 40 | Pool top: Rain Dance, then Surf. |
| 4 | Damp Rock (`ITEM_DAMP_ROCK`) | item | 50 | Rain Dance leads her pool; Politoed on her roster. |
| 5 | Water Stone (`ITEM_WATER_STONE`) | item | 80 | Staryu into Starmie (RBY/FRLG, anime); a strong line. |
| 6 | Dragon Scale (`ITEM_DRAGON_SCALE`) | item | 90 | Her anime Horsea, and Kingdra on her roster. |

### Lt. Surge

TR 30 → 95 (Veteran).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Magnet (`ITEM_MAGNET`) | item | 0 | Electric type; his Raichu holds one. |
| 2 | Lesson | lesson | 40 | Pool top: Thunder Wave, then Thunderbolt. |
| 3 | Thunder Stone (`ITEM_THUNDER_STONE`) | item | 50 | His Raichu against Ash's Pikachu, who refused to evolve (anime). |
| 4 | Electirizer (`ITEM_ELECTIRIZER`) | item | 80 | Electivire, his later ace (HGSS) and a roster ace. |
| 5 | Expert Belt (`ITEM_EXPERT_BELT`) | item | 90 | A soldier's combat belt from "the war" (RBY/FRLG). |

### Erika

TR 28 → 150 (Steady).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Rose Incense (`ITEM_ROSE_INCENSE`) | item | 0 | Her Celadon perfume shop (anime). |
| 2 | Miracle Seed (`ITEM_MIRACLE_SEED`) | item | 0 | Grass type; her Bellossom holds one. |
| 3 | Lesson | lesson | 40 | Pool top: Sleep Powder, then Stun Spore. |
| 4 | Leaf Stone (`ITEM_LEAF_STONE`) | item | 50 | Gloom into Vileplume, her anime buddy's line. |
| 5 | Heat Rock (`ITEM_HEAT_ROCK`) | item | 60 | Sunny Day and Solar Beam in her pool. |
| 6 | Lesson | lesson | 90 | Flower arranging lessons (RBY/FRLG). |

### Janine

TR 27 → 100 (Prodigy).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Poison Barb (`ITEM_POISON_BARB`) | item | 0 | Poison type. |
| 2 | Smoke Ball (`ITEM_SMOKE_BALL`) | item | 0 | A ninja's smoke bomb (HGSS, Masters EX). |
| 3 | Lesson | lesson | 40 | Pool top: Double Team, then Substitute. |
| 4 | Bright Powder (`ITEM_BRIGHT_POWDER`) | item | 50 | Her Venomoth holds one; now-you-see-me ninja tricks. |
| 5 | Leftovers (`ITEM_LEFTOVERS`) | item | 80 | Her Weezing holds them. |

### Sabrina

TR 24 → 180 (Sleeper).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Twisted Spoon (`ITEM_TWISTED_SPOON`) | item | 0 | Spoon bending; her Alakazam holds one. |
| 2 | Poké Doll (`ITEM_POKE_DOLL`) | item | 0 | Her doll house and her habit of turning people into dolls (anime). |
| 3 | Lesson | lesson | 40 | Pool top: Calm Mind, then Psychic. |
| 4 | Light Clay (`ITEM_LIGHT_CLAY`) | item | 50 | Reflect and Light Screen in her pool. |
| 5 | Choice Specs (`ITEM_CHOICE_SPECS`) | item | 100 | Raw psychic power, late. |

### Blaine

TR 37 → 90 (Comeback).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Burn Heal (`ITEM_BURN_HEAL`) | item | 0 | "You better have BURN HEAL!" (RBY/FRLG). |
| 2 | Charcoal (`ITEM_CHARCOAL`) | item | 0 | Fire type. |
| 3 | Lesson | lesson | 40 | His Gym quiz (RBY/FRLG); pool top: Sunny Day, then Flamethrower. |
| 4 | Heat Rock (`ITEM_HEAT_ROCK`) | item | 50 | Sunny Day leads his pool. |
| 5 | Fire Stone (`ITEM_FIRE_STONE`) | item | 80 | Growlithe into Arcanine, his RBY/FRLG ace; a strong line. |
| 6 | Magmarizer (`ITEM_MAGMARIZER`) | item | 90 | Magmar into Magmortar, his signature. |

### Giovanni

TR 24 → 166 (Burst).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Nugget (`ITEM_NUGGET`) | item | 0 | The Rocket recruiter's Nugget on Nugget Bridge (RBY/FRLG). |
| 2 | Soft Sand (`ITEM_SOFT_SAND`) | item | 0 | Ground type. |
| 3 | Amulet Coin (`ITEM_AMULET_COIN`) | item | 50 | Money, and Persian's Pay Day (anime, his pool). |
| 4 | Lesson | lesson | 60 | Pool top: Earthquake, then Stone Edge; gated past the Burst jump at world progress 40. |
| 5 | Protector (`ITEM_PROTECTOR`) | item | 95 | Rhydon into Rhyperior, his signature. |
| 6 | Big Nugget (`ITEM_BIG_NUGGET`) | item | 120 | "Consider it a loan" (his `GIFT`): money to the end. |

### Blue

TR 0 → 170 (Rival).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Silk Scarf (`ITEM_SILK_SCARF`) | item | 0 | Normal type, for the Eevee he starts with. |
| 2 | Black Glasses (`ITEM_BLACK_GLASSES`) | item | 20 | Dark type, for Gary's Umbreon (anime). |
| 3 | Lesson | lesson | 40 | "Gramps says I should share" (his `GIFT`); pool top: Dark Pulse, then Moonlight. |
| 4 | Fire Stone (`ITEM_FIRE_STONE`) | item | 80 | Growlithe into Arcanine, his RBY/FRLG and anime ace. |
| 5 | Metal Coat (`ITEM_METAL_COAT`) | item | 100 | Scyther into Scizor, Gary's Scizor (anime) (verify). |
| 6 | Choice Scarf (`ITEM_CHOICE_SCARF`) | item | 130 | Always one step ahead of the player: the Rival. |

### Lorelei

TR 42 → 92 (Veteran).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Never-Melt Ice (`ITEM_NEVER_MELT_ICE`) | item | 0 | "No one can best me when it comes to icy POKéMON!" (RBY/FRLG). |
| 2 | Lesson | lesson | 45 | Pool top: Shell Smash, then Icicle Spear. |
| 3 | Ice Stone (`ITEM_ICE_STONE`) | item | 50 | Eevee into Glaceon, on her roster. |
| 4 | White Herb (`ITEM_WHITE_HERB`) | item | 60 | Undoes Shell Smash's drops, for her Cloyster. |
| 5 | Focus Sash (`ITEM_FOCUS_SASH`) | item | 85 | Shell Smash's classic partner. |

### Bruno

TR 45 → 94 (Comeback).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Black Belt (`ITEM_BLACK_BELT`) | item | 0 | Fighting type. |
| 2 | Macho Brace (`ITEM_MACHO_BRACE`) | item | 0 | Trains alongside his POKéMON (RBY/FRLG, HGSS). |
| 3 | Lesson | lesson | 45 | Pool top: Dynamic Punch, then Bulk Up. |
| 4 | Muscle Band (`ITEM_MUSCLE_BAND`) | item | 50 | Raw physical power. |
| 5 | Choice Band (`ITEM_CHOICE_BAND`) | item | 85 | "We will grind you down with our superior power!" (RBY/FRLG). |

### Agatha

TR 95 (Legend).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Spell Tag (`ITEM_SPELL_TAG`) | item | 0 | Ghost type. |
| 2 | Lesson | lesson | 50 | Pool top: Mean Look, then Curse. |
| 3 | Dusk Stone (`ITEM_DUSK_STONE`) | item | 80 | Misdreavus into Mismagius, on her roster. |
| 4 | Reaper Cloth (`ITEM_REAPER_CLOTH`) | item | 85 | Dusclops into Dusknoir, her roster ace. |
| 5 | Life Orb (`ITEM_LIFE_ORB`) | item | 95 | "POKéMON are for fighting!" (RBY/FRLG): power at a cost. |

### Lance

TR 200 (Legend).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Dragon Fang (`ITEM_DRAGON_FANG`) | item | 0 | Dragon type; the dragon master (RBY/FRLG, HGSS). |
| 2 | Lesson | lesson | 50 | The mentor who enlists the player (HGSS); pool top: Hyper Beam, then Extreme Speed. |
| 3 | Dragon Scale (`ITEM_DRAGON_SCALE`) | item | 80 | Seadra into Kingdra, on his roster. |
| 4 | Lum Berry (`ITEM_LUM_BERRY`) | item | 90 | A clean Dragon Dance for his Dragonite. |
| 5 | Lesson | lesson | 120 | A second lesson from the Champion. |

## Johto

### Koga

TR 44 → 150 (Steady).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Poison Barb (`ITEM_POISON_BARB`) | item | 0 | Poison type, shared with his daughter Janine. |
| 2 | Lesson | lesson | 45 | "I live in shadows, a ninja!" (HGSS); pool top: Toxic, then Substitute. |
| 3 | King's Rock (`ITEM_KINGS_ROCK`) | item | 50 | His Ariados holds one. |
| 4 | Black Sludge (`ITEM_BLACK_SLUDGE`) | item | 80 | A Poison type's Leftovers, for his Muk and Weezing. |
| 5 | Rocky Helmet (`ITEM_ROCKY_HELMET`) | item | 100 | Spikes and Forretress: make them pay for touching. |

### Falkner

TR 22 → 80 (Prodigy).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Sharp Beak (`ITEM_SHARP_BEAK`) | item | 0 | Flying type. |
| 2 | Pretty Feather (`ITEM_PRETTY_FEATHER`) | item | 0 | A keepsake of his father's bird POKéMON (HGSS). |
| 3 | Lesson | lesson | 40 | Pool top: Tailwind, then Brave Bird. |
| 4 | Dusk Stone (`ITEM_DUSK_STONE`) | item | 60 | Murkrow into Honchkrow, on his roster. |
| 5 | Heavy-Duty Boots (`ITEM_HEAVY_DUTY_BOOTS`) | item | 80 | Wings no one can clip (HGSS): hazards can't touch them. |

### Bugsy

TR 24 → 100 (Star).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Silver Powder (`ITEM_SILVER_POWDER`) | item | 0 | Bug type. |
| 2 | Sport Ball (`ITEM_SPORT_BALL`) | item | 0 | The Bug-Catching Contest ball (GSC/HGSS). |
| 3 | Lesson | lesson | 40 | His research (GSC/HGSS); pool top: Fury Cutter, then Swords Dance. |
| 4 | Scope Lens (`ITEM_SCOPE_LENS`) | item | 50 | Sharp claws and Megahorn crits. |
| 5 | Metal Coat (`ITEM_METAL_COAT`) | item | 80 | Scyther into Scizor, his signature (anime, GSC); a strong line. |

### Whitney

TR 26 → 95 (Star).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Moomoo Milk (`ITEM_MOOMOO_MILK`) | item | 0 | Miltank's milk from Moomoo Farm (GSC/HGSS). |
| 2 | Silk Scarf (`ITEM_SILK_SCARF`) | item | 0 | Normal type. |
| 3 | Lesson | lesson | 40 | Pool top: Rollout, then Milk Drink. |
| 4 | Moon Stone (`ITEM_MOON_STONE`) | item | 50 | Clefairy into Clefable, her GSC Clefairy. |
| 5 | Lucky Egg (`ITEM_LUCKY_EGG`) | item | 80 | What wild Chansey carry; Blissey is her roster ace. |

### Morty

TR 26 → 171 (Sleeper).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Spell Tag (`ITEM_SPELL_TAG`) | item | 0 | Ghost type. |
| 2 | Lesson | lesson | 40 | Pool top: Hypnosis, then Dream Eater. |
| 3 | Wide Lens (`ITEM_WIDE_LENS`) | item | 50 | "It may help you see further" (his `GIFT`); steadies Hypnosis. |
| 4 | Dusk Stone (`ITEM_DUSK_STONE`) | item | 80 | Misdreavus into Mismagius, a roster ace. |
| 5 | Sacred Ash (`ITEM_SACRED_ASH`) | item | 120 | The legendary rainbow POKéMON he trains to meet (GSC/HGSS). |

### Chuck

TR 34 → 85 (Burst).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Black Belt (`ITEM_BLACK_BELT`) | item | 0 | Fighting type; his Poliwrath holds one. |
| 2 | Lesson | lesson | 40 | Dynamic Punch, his GSC/HGSS TM, tops his pool. |
| 3 | Water Stone (`ITEM_WATER_STONE`) | item | 50 | Poliwhirl into Poliwrath, his signature (GSC, anime). |
| 4 | Focus Band (`ITEM_FOCUS_BAND`) | item | 60 | Grit from waterfall training (GSC/HGSS). |
| 5 | Punching Glove (`ITEM_PUNCHING_GLOVE`) | item | 80 | "My POKéMON will crush stones and shatter bones!" (GSC/HGSS). |

### Jasmine

TR 24 → 166 (Steady).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Magnet (`ITEM_MAGNET`) | item | 0 | Electric type, for Amphy the lighthouse Ampharos (GSC/HGSS). |
| 2 | Lesson | lesson | 40 | Iron Tail, her GSC/HGSS TM, tops her pool. |
| 3 | Metal Coat (`ITEM_METAL_COAT`) | item | 50 | Onix into Steelix, her signature (GSC/HGSS, anime). |
| 4 | Leftovers (`ITEM_LEFTOVERS`) | item | 80 | A bulky Steel team. |
| 5 | Lesson | lesson | 110 | A second, gentle lesson. |

### Pryce

TR 40 → 92 (Comeback).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Rage Candy Bar (`ITEM_RAGE_CANDY_BAR`) | item | 0 | Mahogany Town's souvenir sweet (GSC/HGSS). |
| 2 | Never-Melt Ice (`ITEM_NEVER_MELT_ICE`) | item | 0 | Ice type. |
| 3 | Lesson | lesson | 45 | An elder who has "seen and suffered much" (GSC/HGSS); pool top: Hail, then Blizzard. |
| 4 | Icy Rock (`ITEM_ICY_ROCK`) | item | 50 | Hail leads his pool. |
| 5 | Razor Claw (`ITEM_RAZOR_CLAW`) | item | 80 | Sneasel into Weavile, a roster ace. |

### Clair

TR 21 → 185 (Sleeper).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Dragon Fang (`ITEM_DRAGON_FANG`) | item | 0 | "The world's best dragon master" (GSC/HGSS). |
| 2 | Lesson | lesson | 40 | Her HGSS TM is Dragon Pulse; pool top: Dragon Dance, then Dragon Pulse. |
| 3 | Dragon Scale (`ITEM_DRAGON_SCALE`) | item | 80 | Seadra into Kingdra, her signature (GSC/HGSS). |
| 4 | Weakness Policy (`ITEM_WEAKNESS_POLICY`) | item | 100 | She refuses to accept defeat (GSC/HGSS). |
| 5 | Lesson | lesson | 140 | Her cousin Lance's habit of teaching. |

### Will

TR 41 → 110 (Prodigy).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Strange Souvenir (`ITEM_STRANGE_SOUVENIR`) | item | 0 | "I have trained all around the world" (GSC/HGSS). |
| 2 | Twisted Spoon (`ITEM_TWISTED_SPOON`) | item | 0 | Psychic type; his Xatu holds one. |
| 3 | Lesson | lesson | 45 | Pool top: Trick Room, then Calm Mind. |
| 4 | Mental Herb (`ITEM_MENTAL_HERB`) | item | 50 | Keeps Trick Room safe from Taunt. |
| 5 | Room Service (`ITEM_ROOM_SERVICE`) | item | 70 | Made for Trick Room. |

### Karen

TR 47 → 155 (Steady).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Black Glasses (`ITEM_BLACK_GLASSES`) | item | 0 | Dark type; her Houndoom wears them. |
| 2 | Soothe Bell (`ITEM_SOOTHE_BELL`) | item | 0 | "Win with their favorites" (GSC/HGSS); Umbreon evolves by friendship. |
| 3 | Lesson | lesson | 50 | Pool top: Foul Play, then Moonlight. |
| 4 | Dusk Stone (`ITEM_DUSK_STONE`) | item | 80 | Murkrow into Honchkrow, on her roster. |
| 5 | Leftovers (`ITEM_LEFTOVERS`) | item | 100 | Her Vileplume holds them. |

## Hoenn

### Roxanne

TR 24 → 90 (Steady).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Hard Stone (`ITEM_HARD_STONE`) | item | 0 | Rock type. |
| 2 | Lesson | lesson | 40 | The Trainers' School teacher (anime, RSE); Rock Tomb, her RSE TM, tops her pool. |
| 3 | Quick Claw (`ITEM_QUICK_CLAW`) | item | 50 | The Rustboro Trainers' School's gift (RSE). |
| 4 | Thunder Stone (`ITEM_THUNDER_STONE`) | item | 70 | Nosepass into Probopass, her signature (anime, RSE). |
| 5 | Lesson | lesson | 90 | "A reward for such a good student!" (her `GIFT`). |

### Brawly

TR 24 → 90 (Burst).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Black Belt (`ITEM_BLACK_BELT`) | item | 0 | Fighting type. |
| 2 | Wave Incense (`ITEM_WAVE_INCENSE`) | item | 0 | The surfer (RSE/ORAS, anime); Surf is in his pool. |
| 3 | Lesson | lesson | 40 | Bulk Up, his RSE TM, is second in his pool after Fake Out. |
| 4 | Shell Bell (`ITEM_SHELL_BELL`) | item | 50 | A beach find for a surfer (verify: Shoal Cave, RSE). |
| 5 | Assault Vest (`ITEM_ASSAULT_VEST`) | item | 80 | Hariyama, his signature, soaking up hits. |

### Wattson

TR 32 → 75 (Veteran).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Magnet (`ITEM_MAGNET`) | item | 0 | Electric type. |
| 2 | Cell Battery (`ITEM_CELL_BATTERY`) | item | 0 | The tinkerer behind New Mauville's generator (RSE) (verify). |
| 3 | Lesson | lesson | 40 | Pool top: Thunder Wave, then Volt Switch. |
| 4 | Air Balloon (`ITEM_AIR_BALLOON`) | item | 50 | One of his gadgets. |
| 5 | Thunder Stone (`ITEM_THUNDER_STONE`) | item | 75 | Magneton into Magnezone, a roster ace; the last entry of his pool. |

### Flannery

TR 23 → 100 (Star).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Lava Cookie (`ITEM_LAVA_COOKIE`) | item | 0 | Lavaridge Town's specialty (RSE/ORAS). |
| 2 | Charcoal (`ITEM_CHARCOAL`) | item | 0 | Fire type. |
| 3 | Lesson | lesson | 40 | Overheat, her RSE TM, is in her pool (Sunny Day comes first). |
| 4 | White Herb (`ITEM_WHITE_HERB`) | item | 50 | Her Torkoal holds one, for Overheat. |
| 5 | Choice Specs (`ITEM_CHOICE_SPECS`) | item | 80 | Eruption in her pool; she "tried too hard", all in (RSE/ORAS). |

### Norman

TR 26 → 164 (Steady).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Silk Scarf (`ITEM_SILK_SCARF`) | item | 0 | Normal type. |
| 2 | Lesson | lesson | 40 | Facade, his RSE TM, tops his pool. |
| 3 | Flame Orb (`ITEM_FLAME_ORB`) | item | 70 | Facade's classic partner. |
| 4 | Leftovers (`ITEM_LEFTOVERS`) | item | 90 | His Snorlax. |
| 5 | Choice Band (`ITEM_CHOICE_BAND`) | item | 120 | His Slaking (RSE/ORAS, anime) at full power. |

### Winona

TR 18 → 172 (Sleeper).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Sharp Beak (`ITEM_SHARP_BEAK`) | item | 0 | "One with bird POKéMON" (RSE/ORAS). |
| 2 | Lesson | lesson | 40 | Aerial Ace, her RSE TM, tops her pool. |
| 3 | Metal Coat (`ITEM_METAL_COAT`) | item | 60 | Steel type for her Skarmory ace. |
| 4 | Rocky Helmet (`ITEM_ROCKY_HELMET`) | item | 80 | Skarmory's classic item. |
| 5 | Lesson | lesson | 120 | Elegant flying, taught late. |

### Tate & Liza

TR 26 → 170 (Star). **Unused in v0:** the duo is never placed at a haunt
([placement](../specs/notable-haunts.md#candidates)), so this pool waits for
Later. It is authored so the catalog check covers every entry.

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Stardust (`ITEM_STARDUST`) | item | 0 | The Mossdeep Space Center; Solrock and Lunatone came from space (RSE). |
| 2 | Lesson | lesson | 40 | Calm Mind, their RSE TM, tops their pool. |
| 3 | Light Clay (`ITEM_LIGHT_CLAY`) | item | 50 | Reflect and Light Screen in their pool. |
| 4 | Star Piece (`ITEM_STAR_PIECE`) | item | 70 | A twin star for the twins. |
| 5 | Comet Shard (`ITEM_COMET_SHARD`) | item | 100 | Space again, and a big sale. |

### Juan

TR 23 → 185 (Sleeper).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Mystic Water (`ITEM_MYSTIC_WATER`) | item | 0 | Water type. |
| 2 | Lesson | lesson | 40 | "It was I who taught WALLACE everything" (Emerald); Water Pulse, his TM, tops his pool. |
| 3 | Pearl String (`ITEM_PEARL_STRING`) | item | 50 | His flamboyant, artistic style (Emerald). |
| 4 | Dragon Scale (`ITEM_DRAGON_SCALE`) | item | 80 | Seadra into Kingdra, his Emerald ace. |
| 5 | Lesson | lesson | 120 | The teacher again. |

### Sidney

TR 41 → 105 (Prodigy).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Black Glasses (`ITEM_BLACK_GLASSES`) | item | 0 | Dark type. |
| 2 | Lesson | lesson | 45 | Pool top: Sucker Punch, then Swords Dance. |
| 3 | Scope Lens (`ITEM_SCOPE_LENS`) | item | 50 | Absol's crits (Night Slash, Super Luck). |
| 4 | Life Orb (`ITEM_LIFE_ORB`) | item | 90 | "Eh, it was fun, so it doesn't matter" (RSE/ORAS): all offence. |
| 5 | Lesson | lesson | 100 | A second dare. |

### Phoebe

TR 43 → 150 (Steady).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Spell Tag (`ITEM_SPELL_TAG`) | item | 0 | Ghost type. |
| 2 | Cleanse Tag (`ITEM_CLEANSE_TAG`) | item | 0 | "It's not haunted. Probably!" (her `GIFT`); Mt. Pyre training (RSE/ORAS). |
| 3 | Lesson | lesson | 45 | Pool top: Will-O-Wisp, then Pain Split. |
| 4 | Reaper Cloth (`ITEM_REAPER_CLOTH`) | item | 80 | Dusclops into Dusknoir, her signature (anime, RSE). |
| 5 | Lesson | lesson | 110 | Communing with Ghost types (RSE/ORAS). |

### Glacia

TR 44 → 90 (Veteran).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Never-Melt Ice (`ITEM_NEVER_MELT_ICE`) | item | 0 | "To hone my icy skills" (RSE/ORAS). |
| 2 | Lesson | lesson | 45 | Pool top: Hail, then Blizzard. |
| 3 | Icy Rock (`ITEM_ICY_ROCK`) | item | 50 | Hail leads her pool. |
| 4 | Dawn Stone (`ITEM_DAWN_STONE`) | item | 60 | Snorunt into Froslass, on her roster. |
| 5 | Leftovers (`ITEM_LEFTOVERS`) | item | 85 | Her Walrein. |

### Drake

TR 45 → 93 (Veteran).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Dragon Fang (`ITEM_DRAGON_FANG`) | item | 0 | Dragon type. |
| 2 | Big Pearl (`ITEM_BIG_PEARL`) | item | 0 | The old sailor (RSE/ORAS design) (verify). |
| 3 | Lesson | lesson | 45 | Pool top: Dragon Claw, then Dragon Dance. |
| 4 | Lum Berry (`ITEM_LUM_BERRY`) | item | 80 | A clean Dragon Dance for his Salamence. |
| 5 | Expert Belt (`ITEM_EXPERT_BELT`) | item | 90 | "You must have virtue" (RSE/ORAS): skill over brute force. |

### Wallace

TR 48 → 190 (Star).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Blue Scarf (`ITEM_BLUE_SCARF`) | item | 0 | The Beauty contest scarf; POKéMON Contests (Emerald/ORAS, anime). |
| 2 | Mystic Water (`ITEM_MYSTIC_WATER`) | item | 0 | Water type. |
| 3 | Lesson | lesson | 50 | Pool top: Rain Dance, then Scald. |
| 4 | Prism Scale (`ITEM_PRISM_SCALE`) | item | 80 | Feebas into Milotic, his signature (anime, RSE); a strong line. |
| 5 | Leftovers (`ITEM_LEFTOVERS`) | item | 120 | A bulky Milotic. |

### Steven

TR 50 → 195 (Burst).

| # | Entry | Kind | From world progress | Why |
| ---: | --- | --- | ---: | --- |
| 1 | Hard Stone (`ITEM_HARD_STONE`) | item | 0 | A stone from the collector (RSE/ORAS, Masters EX). |
| 2 | Everstone (`ITEM_EVERSTONE`) | item | 0 | Another stone, modest and useful. |
| 3 | Lesson | lesson | 50 | Pool top: Meteor Mash, then Bullet Punch. |
| 4 | Dawn Stone (`ITEM_DAWN_STONE`) | item | 80 | A rare stone for strong lines (Gallade, Froslass). |
| 5 | Shiny Stone (`ITEM_SHINY_STONE`) | item | 100 | The rarest of his stones (Togekiss, Roserade). |
