# Music

Drop one track here (`.mp3`, `.m4a`, `.wav` or `.ogg`); `pnpm render` mixes the
first file it finds under the voice. A track shorter than the film starts late,
so its own ending lands on the title card; a longer one starts with the film and
fades out over its last three seconds (`--music-at` sets the start instead). It
plays at `--music-volume` (0.3) and dips to `--duck` (0.35) of that while the
narrator speaks. Nothing in this folder but this file is tracked: the game's own
music is Nintendo's, so pick what you have the rights to use, or render without
music.

The launch cut uses "Cozy Town Square" (Suno, the user's, 2:35), made from:

```
instrumental, early 2000s handheld RPG soundtrack, GBA soundfont, cozy town theme, compressed lo-fi sampled instruments, soft square wave lead, warm flute melody, pizzicato strings, marimba and harp arpeggios, gentle bass, light snare and shaker, simple looping melody, 92 bpm, nostalgic, peaceful, homely, gentle build, soft ending
```

Excluded: `guitar, acoustic guitar, vocals, orchestral, cinematic, realistic instruments, heavy drums, EDM, rock, reverb-heavy`.
An earlier, action-leaning track ("The World Opens Up") fought the narrator's calm, and a cozy
acoustic one sounded nothing like a GBA game.
