# Wayfarer launch video

The launch video, made from the website's world map: the camera flies over
the same map the landing page draws, with its ships, Gym Leaders, weather and
time of day, while a traveler tells of it and the game's text box prints
their words, page by page.

Everything the map needs is the website's own code and data: `~/world-map`
resolves to `../website/src/world-map`, and `/world-map` and `/game` are served
from `../website/public`. Nothing is copied; a change to the site's map shows
up here on the next build. `@wayfarer/website` is a workspace dependency for
that reason alone: a filtered install (`--filter @wayfarer/video...`) then
brings the website's own dependencies, which its source needs to compile.

## Commands

Run these from `video/`, or from the repository root with
`pnpm --filter @wayfarer/video run <script>`.

| Script        | What it does                                                           |
| ------------- | ---------------------------------------------------------------------- |
| `pnpm dev`    | Play the video in the browser, with the voice when it has been made    |
| `pnpm build`  | Build the page the frames are rendered from, into `dist/`              |
| `pnpm voice`  | Speak the script with ElevenLabs into `audio/vo/`                      |
| `pnpm frames` | Render every frame of the built page into `out/frames/`                |
| `pnpm render` | Mux the frames, the voice and the music into `out/wayfarer-launch.mp4` |
| `pnpm check`  | Type check, lint, and check formatting                                 |
| `pnpm format` | Format files with oxfmt                                                |

### Watching it

`pnpm dev` opens the video in a 16:9 stage that fits the window. Space pauses
and plays, ← and → step between scenes, Home starts over, `?t=30` opens at a
time and `?paused` opens paused. The readout at the bottom right names the
scene, the time and the game clock.

### The studio

`pnpm dev` and open `/?studio` (or press **Studio** at the bottom right of
the player). The map is free: drag to pan, wheel to zoom, or fly to a region,
the whole world or the league circuit. A panel beside it has every knob the
video can turn: the game clock as a slider or running at the game's pace, the
site's pace or a time-lapse; each layer (sea, weather, clouds, ships and
tint, night lights, Gym Leaders with markers, wild Pokémon close in); the
badge pins, all bright or earned one by one so they glow and sparkle and a
league's trail lights at eight; the daily roads with the day stepping; the
wild waves popping out of each home region; the ride in on the bike from any
start, with the game's warp before it; and a fade to black. **Copy camera**
gives a `{ kind: "point", x, y, zoom }` target to paste into `timeline.ts`.
Everything the studio shows, a scene can show: the same `Shot` is drawn
either way (`src/shot.ts`).

### Rendering it

```sh
pnpm build
pnpm frames            # 30 fps, 1920x1080; --fps, --width, --height to change
pnpm render            # needs ffmpeg
```

`pnpm frames` serves `dist/` to a headless Chromium (Playwright's own, or the
one `CHROME` names), takes over the page's clocks and turns them by hand: for
every frame it seeks the video, lets one frame's time pass, waits for any map
chunks still drawing, and screenshots. The result is the same on any machine,
however fast it draws. The whole video takes a few minutes; `--scene gyms`
renders one scene and `--from 12 --to 20` a range, to check a change.

`pnpm render` places each scene's voice clip at its cue, mixes in the first
track in `audio/music/` at a quarter volume (see its README; `--music` picks
another, `--music-volume` changes the level) and encodes H.264. `--no-voice`
renders the text-box-and-music version: the text box prints every word the
voice says, so the video stands without a narrator.

### The voice

```sh
export ELEVENLABS_API_KEY=...
pnpm voice --list                         # the account's voices
ELEVENLABS_VOICE_ID=... pnpm voice        # speak what changed
```

One clip per scene into `audio/vo/<scene>.mp3`, and into `audio/vo/cues.json`
its length and ElevenLabs' time for every letter, which the text box prints by
and the timeline's phrases fall on. A beat in the script goes to the voice as a
pause tag (`<break>`; `eleven_multilingual_v2` honours them, the v3 models
don't). Commit them once made, so anyone can render without a key. Until then
the words are paced as a voice would read them, and the render is music and
text box only. `pnpm voice` only speaks the scenes whose
words, voice or model changed since (`--force` speaks them all, `--scene`
one). `ELEVENLABS_MODEL` or `--model` picks the model, `eleven_multilingual_v2`
by default.

## How it is put together

- `src/script.ts` is the storyboard's words: each scene's narration
  (the voice and the text box alike), the least it lasts, and the names the
  box prints in capitals. A `\n` turns the box's page; a `|` is a beat, a
  held pause before the words after it, as before a punchline. It imports
  nothing, so `scripts/voice.mjs` reads it under Node.
- `src/words.ts` is when the words are said: the beats taken out of the
  narration, the text the voice is sent, and the words paced as a voice would
  read them (pausing between sentences and at each beat) for a scene with no
  clip yet.
- `src/timeline.ts` is the storyboard's pictures, keyed by scene. Its moments
  are fractions of the scene or phrases of the narration (`at: "Seven lands"`),
  which fall wherever the voice says them, so the picture keeps to the words
  however long a take runs. Camera
  keyframes (`world`, `regions`, a `map` point, a grid `point` from the
  studio, the league `circuit`, a `notable` wherever their routine has them,
  or a `ship` wherever her loop has her), the game clock (held, or run across the scene for a time-lapse),
  layers, the badge pins, the walkers, the daily roads, the wild waves, the
  rides in and a title card.
- `src/schedule.ts` turns the script into start times and the time of every
  letter: a scene lasts its `seconds`, or its words (the voice clip, or the
  paced words) plus a lead-in and a tail when that is longer, so the picture
  always waits for the words.
- `src/shot.ts` is one picture of the world: camera, game minute, layers,
  pins, daily day, wild waves, the rider and a fade. The timeline makes one
  per frame, the studio makes one from its panel.
- `src/director.ts` loads the website's world and everything drawn over it,
  turns a scene into a shot (camera keyframes, clock, ride) and draws a shot
  with the website's renderer and overlays.
- `src/Studio.tsx` is the studio: the free camera, the panel and its set pieces.
- `src/box.ts` pages the narration for the text box: names in the game's
  capitals, wrapped in the game's font, two lines a page, each page up when
  the voice starts it and each letter printing as it's said, never ahead.
- `src/App.tsx` is the page: the canvas, the text box, the title card, the fades, the keyboard, and
  `window.wayfarerVideo`, which `scripts/frames.mjs` drives.
