# The Wayfarer's voice lines

The narrator's takes: ElevenLabs, **Eleven v3** with audio tags, one file per scene, named as
below, all in this folder. The words are `src/script.ts`'s, which the text box prints; only the
tags, punctuation and a few spellings (to get the pronunciation right) differ. Where a take said
a line differently, the script was changed to what the take says.

Lines marked ★ are the jokes: make two or three takes of each and keep the best, since the
delivery makes or breaks them.

## The voice

A mysterious, laconic storyteller with a light Western drawl. A Voice Design voice (guidance 30%
or lower; higher brought noise into the background), from:

```
American English with a light, slow Western drawl. A man in his late 40s. A mysterious, laconic storyteller who already knows how this all ends and isn't telling. Low, smooth, resonant bass voice, quiet and confiding, as if telling one person a secret. Speaks in short, unhurried phrases, weighs every word, and lets the silence after a line do half the work. Wry, with only a hint of a smile; the jokes come out bone-dry. Never loud, cheerful or theatrical. Perfect audio quality, clean close-mic studio recording, silent background.
```

Preview text:

```
[low, unhurried] Well. Look who's back. Last time, you set out from Pallet Town... [wry] Everyone did. [quietly] Remember the S.S. Anne? She sailed out of Vermilion... and you never saw her again. [confiding] Ever wonder where she was going? [dry] Lt. Surge? ... Game Corner... Again. [low] And who's in the Elite Four this year? ... Brock... With a Golem.
```

What didn't work, so a new voice doesn't repeat it: recording words in the prompt ("very close
to the microphone", "smoky grain", "in the dark") came out as a bad mic; a hammy anime-dub
mentor shouted every line.

Use the saved voice for every line: running the prompt again designs a different voice.

## How to read the lines

- **Paste the text as written**, tags included. Every line has the same voice, model and settings.
  Stability "Natural"; "Robust" if he drifts. Never "Creative".
- **Most lines have no tag.** The voice is laconic already; the beats do the work. If a take
  comes out flat, add one small tag before the punchline (`[wry]`, `[quietly]`, `[dry]`), never a
  big one, and no exclamation marks.
- **`...` is a beat**, the script's `|`: a held moment before a punchline. The picture cuts on
  the words after it, so a take that rushes a beat is the wrong take. (v3 ignores `<break>`
  tags; the dots do the job.)
- **Spelled for the ear:** "Lieutenant Surge", "Route one-nineteen", "Route thirty". The text box still prints the real spellings.
- **If a name comes out wrong**, respell it in the take and keep the rest: "Sevii" as _Sevvy_,
  "Kalos" as _Kal-oss_, "Alola" as _Ah-low-la_, "S.S. Anne" as _S. S. Anne_.
- **Bring the takes in** with `pnpm takes <folder>`: it matches each file to its scene by its
  words (name the export after what it says, as ElevenLabs does), trims the silence at its ends,
  copies it here as `<scene>.mp3` and times its letters into `cues.json` from its pauses.
  `--dry-run` shows the matches and where it places each phrase without writing anything.

## Lines

All 20 recorded on 2026-11-05 and brought in with `pnpm takes`. Each entry is the text
suggested for the take, then what the take says where it differs; the length is the trimmed clip.

- [x] ★ `pallet.mp3` (6.9 s)
      `Well, look who's back. Last time, you set out from Pallet Town... [wry] Everyone did.`
- [x] `newbark.mp3` (3.2 s)
      `This time? ... Maybe New Bark Town.`
- [x] `littleroot.mp3` (4.0 s)
      `Or Littleroot... Never did matter much where you start.`
- [x] `anne.mp3` (7.6 s)
      `[quietly] Remember the S.S. Anne? She sailed out of Vermilion, and you never saw her again... Ever wonder where she was going?`
- [x] ★ `alola.mp3` (4.4 s)
      `[quietly] Alola... This time, you don't have to watch her go.`
      Says: "Alola... And this time, you don't have to watch her go."
- [x] `wait.mp3` (5.7 s)
      `Course, she sails when she sails... [wry] Blink, and you're watching her go again.`
- [x] `lands.mp3` (5.9 s)
      `No matter. Kanto was never the whole world... [dry] Seven of 'em, all told.`
- [x] ★ `galar.mp3` (5.8 s)
      `Folk on the Sevii Islands swear they're Galar now... [dry] Ask the Wooloo.`
- [x] `kalos.mp3` (7.2 s)
      `Never been to Kalos myself. Too far... Then again... [wry] Kalos came to the Safari Zone.`
      Says: "Never been to Kalos myself. Too far... But now? Kalos came to the Safari Zone."
- [x] `surf.mp3` (7.4 s)
      `Still on that beach? ... Nobody's handing out Surf anymore. Fish... [quietly] Something down there already knows it.`
      Says: "Still on that beach? ... Nobody's handing out Surf anymore. I bet something down there already knows it."
- [x] `leaders.mp3` (9.1 s)
      `Gym Leaders? ... Hardly ever at the Gym these days. Brock's helping out at the Day-Care in Johto... [wry] Always up for a trade, that one.`
- [x] ★ `surge.mp3` (6.0 s)
      `Lieutenant Surge? ... Game Corner... [sighs] Again.`
      Says: "Lieutenant Surge? ... Oh... Game Corner... Again."
- [x] ★ `wallace.mp3` (4.9 s)
      `Wallace is fishing Route one-nineteen... [amused] Has been for days.`
      Says: "Wallace, still fishing Route one-nineteen... Has been for days."
- [x] `night.mp3` (4.4 s)
      `[low] Stray off the road after dark? ... Pack more Potions than you think.`
- [x] `don.mp3` (2.5 s)
      `[fondly] That Bug Catcher you beat on Route thirty...`
- [x] ★ `donagain.mp3` (3.5 s)
      `He's moved on... [quietly] He hasn't forgotten you.`
- [x] ★ `clair.mp3` (6.5 s)
      `Twenty-four Gyms. You could start with Clair. Go on... [wry] It'll be a fair fight.`
      Says: "Twenty-four Gyms. Want to challenge Clair first? Go on... It'll be a fair fight."
- [x] `fame.mp3` (9.1 s)
      `[dry] Word gets around... [warmly] And soon, the folks you looked up to will know your name... Who's that? ... [surprised] Oh... [wry] it's the League calling.`
- [x] ★ `golem.mp3` (6.6 s)
      `[dry] And who's in the Elite Four this year? ... [raising eyebrow] Brock. With a Golem... [knowingly] Wonder where he got that.`
- [x] `open.mp3` (3.7 s)
      `[with quiet pride] Good to have you back, wayfarer... [softly] See you out there.`
