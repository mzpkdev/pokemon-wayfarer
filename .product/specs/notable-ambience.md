# Notable ambience

PRD: [Notable ambience](../prds/notable-ambience.md)
Implemented: No

Design status: builds on the on-screen walkers implemented by
[notable world simulation](notable-world-simulation.md) and the
[behaviour templates](notable-spots.md#behaviour-templates) of notable
spots. Every timing, cadence, and threshold below is a placeholder for
playtesting, and the beat pool and tags are a first draft.

## Scope

This spec covers how a notable walker behaves on screen, second to second:

- its walking pace;
- the shared pool of ambient **beats**, and the primitives they are built from;
- the context and qualifiers that make a beat eligible;
- the trainer tags, preferred beats, and relationships;
- deterministic beat selection;
- the ace companion;
- interruption rules, the data format, validation, and costs.

It doesn't cover:

- **Where a trainer goes** (spot choice, routines, travel, capacity). That stays
  in [notable spots](notable-spots.md) and the
  [world simulation](notable-world-simulation.md).
- **Talking to walkers.** That is the friendship work, later.
- **Fame and friendship reactions.** They are listed under [Later](#later).

## Behavior

### Layers

A walker's on-screen behaviour has three layers. Only the first two exist today:

1. **Movement:** walking a path to a goal, as the walker does now.
2. **Template:** what the walker does with the spot (stand, roam an area,
   change shelves, just leaving). Templates keep their movement rules unchanged.
   Their "Emote" column moves into the beat pool: the tall-grass "!", the
   water's-edge "!", and the chat "…" become the beats `grass_rustle`,
   `water_bite`, and `chat_talk`.
3. **Beats:** short expressive moments that run in the gaps of the first two.

A beat never changes the walker's record, spot, destination, or dwell. It may
take a step and must step back afterwards. A beat that would need to leave its
tile (a hop toward the grass) moves at most 1 tile, and only to a free,
standable tile. It returns to the starting tile before it ends.

### Pace

| Movement | Speed |
| --- | --- |
| Default walking, wander moves, browse moves, a beat's steps | **slow**: `MOVEMENT_ACTION_WALK_SLOW_*`, 32 frames a tile |
| Walk-off after a yield or hand-off, a back-off from the player, a Gym visitor just leaving, a strip actor, the step out of or into the map through a side lane | **normal**: `MOVEMENT_ACTION_WALK_NORMAL_*`, 16 frames a tile |

`WALKER_WALKOFF_FRAMES` goes from 600 to 1,200. The walker's other frame
timeouts are standing waits, push counters, and scanline budgets rather than
walking time, so they stay as they are. What the search, spawn, and hand-off
rules decide doesn't change; only when their work runs does: so that beats
add no lag frames, the walkers' search slices, spawn attempts, story checks,
and follower rule wait for a frame with room (each for a bounded time; see
the implementation notes).

### Primitives

Beats are built only from these primitives, implemented once in code:

| Primitive | Effect | Engine |
| --- | --- | --- |
| `face <dir or target>` | Turn to a direction, or toward the player, another walker, the spot's target, or the companion | facing movement action |
| `look_back` | Turn back to the facing the beat started with | facing movement action |
| `wait <ticks>` | Stand still; one tick is 15 frames here | frame counter |
| `step <dir or target>` | One slow step; `step back` undoes the last step | `MOVEMENT_ACTION_WALK_SLOW_*` |
| `jump` | Jump in place | `MOVEMENT_ACTION_JUMP_IN_PLACE_*` |
| `spin` | Spin once | `MOVEMENT_ACTION_SPIN_*` |
| `bow <object>` | Make an adjacent object bow (the Center nurse) | `MOVEMENT_ACTION_NURSE_JOY_BOW_DOWN` |
| `emote <icon>` | Show an icon over the walker | see the next table |
| `effect <kind> <tile>` | Play a field effect on a tile: `ripple`, `splash`, `grass_shake`, `dust`, `sparkle` | `FLDEFF_RIPPLE`, `FLDEFF_SPLASH`, `FLDEFF_SHAKING_GRASS`, `FLDEFF_DUST`, `FLDEFF_SPARKLE` |
| `companion <in or out>` | Bring the ace out beside the walker, or put it away ([companion](#companion)) | an overworld follower sprite object |
| `companion_do <action>` | The companion faces the walker, or jumps | movement actions on the companion object |

Icons:

| Icon | Engine |
| --- | --- |
| `!` | `MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK` |
| `!!` | `MOVEMENT_ACTION_EMOTE_DOUBLE_EXCL_MARK` |
| `?` | `MOVEMENT_ACTION_EMOTE_QUESTION_MARK` |
| `X` | `MOVEMENT_ACTION_EMOTE_X` |
| `heart` | `MOVEMENT_ACTION_EMOTE_HEART` |
| `…`, `happy`, `music`, `love`, `curious`, `pensive`, `sad`, `angry`, `surprise` | `FLDEFF_EMOTE` with the follower emote sheet's frame, as the walker's "…" already does |

Only one `FLDEFF_EMOTE` icon can show at a time. A beat whose icon is busy waits
1 tick, then skips the icon.

### Beats

A beat is one row of the pool:

| Field | Meaning |
| --- | --- |
| `id` | Unique name |
| `class` | `react`, `transition`, or `idle`; this sets its [priority](#selection) |
| `when` | Context conditions; all must hold ([context](#context)) |
| `who` | Qualifiers on the trainer: tags required (`any` of a list), tags excluded, needs a companion; optional |
| `steps` | 1 to 8 primitives |
| `cooldown` | Ticks before the same walker may run it again (default 80, about 20 s) |

### Context

| Condition | Holds when |
| --- | --- |
| `spot: [kinds]` | The walker is dwelling at a spot of one of these kinds; named spots match by their activity (`spot: [named:sightsee]`) |
| `activity: [activities]` | The walker's current activity is one of these |
| `walking` | The walker is following a path, between steps |
| `arriving` | The walker has just reached its spot, before the template starts |
| `leaving` | The walker's stay has ended, before it plans the next goal |
| `blocked` | The next step is blocked by an object that isn't the player |
| `facing: water / grass / npc / counter` | The faced tile or object is of that kind |
| `player_within: n` | The player is within `n` tiles (Manhattan distance) and in view |
| `player_adjacent_ticks: n` | The player has stood next to the walker, facing it, for `n` ticks without pushing |
| `notable_within: n` | Another walker is within `n` tiles; `relation:` narrows it ([relationships](#relationships)) |
| `outdoors` | The map is an outdoor map |
| `companion_room` | A companion could come out now ([companion](#companion)) |

### Selection

The walker evaluates beats at **decision points**:

- each step boundary while walking;
- the arrival and leaving moments;
- each dwell tick (60 frames) while at a spot;
- any frame a `react` condition first becomes true.

At a decision point, with no beat running:

1. **Gate.** Skip unless the walker's **quiet gap** has passed since its last
   beat. The gap is `16 + 4 × (c mod 3)` ticks (4 to 6 s), where `c` is the
   trainer's catalog position. For a `stoic` trainer it is three times that.
   `react` and `transition` beats ignore the gap, so a walker always gets its
   arrival and leaving moments. A freshly spawned walker starts with its gap
   already passed.
2. **Filter.** The pool rows whose `when` and `who` hold, and whose cooldown has
   passed.
3. **Class.** Keep only the highest class present: `react` beats first, then
   `transition`, then `idle`.
4. **Idle odds.** `idle` beats fire only at decision points where
   `(c + t) mod 4 = 0`, where `t` is the walker's decision counter. While
   walking it's every 10th step: `(c + s) mod 10 = 0`, where `s` is the step
   count, and at least 6 steps since the last beat. Otherwise nothing happens.
   That is the common case.
5. **Pick.** List the remaining rows in pool order, with each of the trainer's
   preferred beats listed twice. Take entry `(c + 7k) mod n`, where `k` is the
   walker's beat counter and `n` the list length, with the same prime-stride rule
   as the [template choices](notable-spots.md#behaviour-templates).
6. **Run** the beat's steps in order, then record its cooldown and reset the
   quiet gap.

**Reaction limits:**
- `notice_player` fires once each time the player comes within range.
- A greeting fires once per pair per map visit.
- `player_lingers` fires once per standing-still episode.

All counters live in the walker's RAM and start at 0 when the actor spawns,
except the quiet gap and the steps since the last beat, which start as
already passed. A heap reset (a warp, a menu, a battle) restarts them all the
same way.
Nothing reads the random number generator.

### Interruptions

A running beat stops at once, puts the companion away, and restores the
walker's facing in these cases:

- the player pushes into the walker, or the yield or back-off rules trigger;
- a script, menu, or story scene locks the field controls (walkers already pause
  then);
- a story object appears;
- the walker hands off or walks off;
- the heap resets;
- the map changes.

The template and movement then resume as they do today.

### Relationships

| Relation | Source | Pairs |
| --- | --- | --- |
| `family` | authored | Lance–Clair |
| `friend` | authored | Brock–Misty, Steven–Wallace, Juan–Wallace (mentor) |
| `rival` | derived | any two Champions: Blue, Lance, Wallace, Steven |
| `colleague` | derived | Gym Leaders of the same region; the Elite Four and Champion of the same League |

When a pair matches more than one relation, the authored one wins, so Steven
and Wallace are friends, not rivals.

### Tags

Body-language tags for the 25 walking notables. A trainer has 1 to 3:

| Tag | Reads as | Trainers |
| --- | --- | --- |
| `athletic` | can't sit still, trains anywhere | Lt. Surge, Janine, Chuck |
| `cheerful` | hums, bright icons | Brock, Misty, Blaine, Bugsy, Whitney |
| `curious` | studies the ground and the sky | Brock, Falkner, Bugsy, Steven |
| `dreamy` | dozes, drifts off | Erika |
| `mystic` | meditates, gazes into nothing | Sabrina, Morty, Will |
| `elegant` | flourishes and poses | Erika, Lorelei, Will, Juan, Wallace |
| `water` | drawn to water | Misty, Jasmine, Lorelei, Juan, Wallace |
| `elder` | slow, catches their breath | Blaine, Pryce |
| `nimble` | quick hops | Janine |
| `cocky` | shows off | Blue, Clair |
| `stoic` | mostly still: triple quiet gap, and the stare instead of the glance | Sabrina, Giovanni, Lance, Pryce, Karen, Norman |

Preferred beats (first draft): Misty `admire_water`, Lt. Surge `push_ups`,
Erika `doze`, Whitney `hum`, Morty `meditate`, Chuck `push_ups`, Bugsy
`study_ground`, Wallace `flourish`, Steven `study_ground`, Blue `swagger`.

### The pool

The first draft has 35 beats. Placeholder `wait` lengths are in ticks of
15 frames.

**Walking and transitions**

| id | class | when | who | steps |
| --- | --- | --- | --- | --- |
| `look_around` | idle | walking | not stoic | wait 2, face left, wait 3, face right, wait 3, look_back |
| `hum` | idle | walking | cheerful | emote music, without stopping the walk |
| `stretch` | idle | walking, outdoors | athletic | stop, wait 1, jump, wait 1 |
| `skywatch` | idle | walking, outdoors | curious, mystic, dreamy | face up, wait 6, emote pensive, look_back |
| `blocked_sigh` | react | blocked | not stoic | emote …, wait 4 |
| `arrive_look` | transition | arriving | — | face left, wait 2, face right, wait 2, face target |
| `leave_turn` | transition | leaving | — | face away from target, wait 3 |

**Reacting to the player and each other**

| id | class | when | who | steps |
| --- | --- | --- | --- | --- |
| `notice_player` | react | player_within 3, walking or dwelling | not stoic | face player, wait 4, look_back |
| `stare_down` | react | player_within 3 | stoic | face player, wait 10, look_back |
| `player_lingers` | react | player_adjacent_ticks 12 | — | emote ? |
| `greet_colleague` | react | notable_within 3, relation colleague | — | face other, emote !, wait 4, look_back |
| `greet_friend` | react | notable_within 3, relation friend | — | face other, emote !!, wait 2, emote happy, wait 4, look_back |
| `greet_family` | react | notable_within 3, relation family | — | face other, emote heart, wait 6, look_back |
| `size_up` | react | notable_within 3, relation rival | — | face other, wait 6, emote angry, look_back |

**At spots** (these replace the template emotes)

| id | class | when | who | steps |
| --- | --- | --- | --- | --- |
| `grass_rustle` | idle | spot tall_grass | — | effect grass_shake ahead, wait 2, emote ! |
| `grass_chase` | idle | spot tall_grass, facing grass | not elder | effect grass_shake ahead, step ahead, wait 2, emote X, step back |
| `water_bite` | idle | spot water_edge, facing water | — | effect ripple ahead, wait 8, emote !, effect splash ahead, step back, wait 2, step ahead |
| `water_wait` | idle | spot water_edge, facing water | — | emote …, wait 8 |
| `counter_heal` | idle | spot center_counter | — | bow nurse, emote music, wait 6 |
| `shelf_compare` | idle | spot store | — | emote ?, wait 4, face left, wait 2, face target |
| `slots_result` | idle | spot game_corner | — | wait 4, then emote !! on even beat counters, emote X on odd |
| `chat_talk` | idle | spot npc_chat | — | NPC faces the walker, emote …, wait 4, emote !, wait 2, emote happy |
| `people_watch` | idle | spot square or bench | — | face left, wait 4, face down, wait 4, face right, wait 4, look_back |
| `take_in_view` | idle | spot named:sightsee or named:relax | — | effect sparkle ahead, wait 4, emote happy |

**Signatures** (from tags)

| id | class | when | who | steps |
| --- | --- | --- | --- | --- |
| `push_ups` | idle | spot tall_grass, square, or named:train | athletic | face down, jump, jump, jump, wait 2, emote happy |
| `meditate` | idle | dwelling, not store or game_corner | mystic | face down, wait 4, effect sparkle own tile, wait 8, emote pensive |
| `doze` | idle | dwelling, activity relax or train | dreamy | emote pensive, wait 16, emote ! |
| `admire_water` | idle | facing water | water | wait 4, emote love |
| `study_ground` | idle | dwelling, outdoors | curious | face down, effect dust ahead, wait 4, emote ?, wait 2, emote ! |
| `flourish` | idle | dwelling | elegant | spin, effect sparkle own tile, emote happy |
| `catch_breath` | idle | walking or dwelling | elder | wait 6, emote pensive |
| `quick_hop` | idle | dwelling, outdoors | nimble | step left, step back, step right, step back |
| `swagger` | idle | dwelling or player_within 4 | cocky | spin, emote !! |

**Companion**

| id | class | when | who | steps |
| --- | --- | --- | --- | --- |
| `ace_play` | idle | dwelling, outdoors, companion_room, activity relax, fish, train, or sightsee | — | companion out, face companion, companion_do jump, emote happy, wait 8, companion in, look_back |
| `ace_spar` | idle | dwelling, companion_room, activity train | athletic or cocky | companion out, face companion, jump, companion_do jump, emote !, wait 4, companion in, look_back |

### Companion

- **Species:** the trainer's `companion` from `ambience.json` if set, otherwise
  their first ace in [catalog roster](notable-trainers.md) order. It is shown
  with that species' overworld follower sprite, never shiny.
  - **The optional `companion` field** lets an iconic anime partner stand in
    for the battle ace where it matters. It needn't be in the battle roster.
    First draft: Brock Onix, Misty Psyduck, Erika Gloom, Bugsy Scyther.
  - **Everyone else** shows their first ace, such as Dragonite for Lance and
    Raichu for Lt. Surge.
- **Room** (`companion_room`) requires all of these:
  - the walker is dwelling and not at a store, Game Corner, or Center;
  - after spawning the companion, the map would still keep the walker rule's 3
    free object slots;
  - a sprite palette is free;
  - no other companion is out on the map;
  - a free standable tile is adjacent to the walker and not on the player's tile
    or the tile straight ahead of the player.
- **Placement:** the companion appears on that tile, facing the walker. It
  lasts only for its beat, and is removed when the beat ends or is interrupted.
- **Cost:** bringing it out decompresses its follower sprite sheet, the
  engine's normal cost for any follower sprite. That is one call too long for a
  busy outdoor frame, so the walker waits up to about 2 s for the frame with
  the most room and then accepts one lag frame per companion out. This is the
  one exception to the rule that no beat adds lag frames.
- **Priority:** it is the lowest-priority object. It never hides the player's
  following Pokémon, never blocks a spawn, and is the first object removed when
  slots are short.
- **Large sprites:** species with 64×64 follower sprites (such as Steelix,
  Gyarados, Lapras, and Dragonite) need a free 2×2 area. Without one, the
  trainer's next ace in roster order that fits comes out instead. If none
  fits, the companion beat is skipped. An authored `companion` falls back
  the same way, to the roster's aces.

### Data

The pool, tags, preferred beats, and authored relationships live in
`game/tools/wayfarer_world/ambience.json`. The world generator turns it into ROM
tables, as it does for `routines.json`.

**Validation fails the build** on any of these:
- an unknown primitive, icon, effect, spot kind, activity, tag, or trainer;
- more than 8 steps;
- a step that leaves its tile without returning;
- a preferred beat the trainer can't qualify for;
- a trainer with no tag;
- a relationship naming a trainer twice;
- a companion beat for a trainer with no ace;
- a `companion` species without an overworld follower sprite.

### Saved state and costs

- **Save:** nothing.
- **RAM:** per walker, the running beat, its step index and timer, the beat and
  decision counters, the quiet-gap timer, and a cooldown slot per beat, about
  75 bytes per walker. It lives in one heap block the walker layer allocates
  while walkers are active (dropped on heap resets), not in static EWRAM, per
  the [RAM rules](../../AGENTS.md): the mechanics-test build has almost no
  EWRAM left. Only the few latches that must survive a menu sit in the
  walker's existing EWRAM.
- **ROM:** the pool, tag, trainer, and relation tables, about 2.5 KB.
- **CPU:** filtering about 30 rows at a decision point, well inside the walker's
  per-frame budget. No beat may add lag frames, except the companion's one
  spawn frame ([companion](#companion), "Cost").

### Acceptance

1. Walkers walk at slow speed by default, and at normal speed only for the
   hurried cases in [Pace](#pace).
2. A walker at a water's edge runs `water_bite` or `water_wait` while facing
   water, and never a beat whose context doesn't hold.
3. `notice_player` fires once as the player passes within 3 tiles of a
   non-stoic walker, and `stare_down` fires instead for a stoic one.
4. Two walkers with a relation within 3 tiles greet each other once per map
   visit, with the beat their relation picks.
5. The companion appears only when `companion_room` holds, never hides the
   following Pokémon, and is removed when its beat ends or is interrupted.
6. Pushing a walker, a script, or a story object stops a running beat at once.
7. Two runs with the same inputs give the same beats on the same frames, and the
   random number generator is never read.
8. `ambience.json` validation fails the build on each listed error.
9. No new lag frames in the walker verifier's `perf` and `seam` scenarios, and
   none from beats anywhere else except one per companion out.

## Open questions

- Cadences (the quiet gap, idle odds, and the walking interval of 10 steps)
  stay placeholders until playtesting.

## Later

- **Fame:** a trainer who has [heard of the player](notable-trainers.md) shows
  "!" when the player arrives on the map, using a `player_heard` condition.
- **Friendship:** a close friend waves (`emote happy`, then `heart`), using
  `player_friend: stage`.
- **Time of day and weather** conditions.
- **More beats and tags** after playtesting.

## References

- [Notable spots: behaviour templates](notable-spots.md#behaviour-templates)
- [Notable world simulation](notable-world-simulation.md)
- [Notable trainers: roster and aces](notable-trainers.md)
- Engine: `game/include/constants/event_object_movement.h` (movement actions
  and emotes), `game/include/constants/field_effects.h` (field effects),
  `game/src/wayfarer_walkers.c` (the walker, `ShowEmote`).
