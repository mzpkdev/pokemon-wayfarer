// When each word of a scene is said: the clock the text box prints by and the
// timeline's moments are told in. With a voice clip, it is the voice's own
// timing (`pnpm voice` keeps ElevenLabs' time for every letter); without one,
// the words are paced as a voice would read them, pausing between sentences
// and at every beat the script asks for. This file imports nothing, so
// `scripts/voice.mjs` can read it under Node.

/** A beat in the script: a held pause before the words that follow, as before a punchline. */
export const BEAT = "|"
/** How long a beat holds, in seconds; `||` holds twice as long. */
export const BEAT_SECONDS = 0.7
/** Letters a second the words are read at when no voice sets the pace: an easy speaking pace. */
const READ_LETTERS_PER_SECOND = 15
/** The pause after a sentence, and at a line break in the script, when no voice sets the pace. */
const SENTENCE_SECONDS = 0.35
const BREAK_SECONDS = 0.6

/**
 * A scene's narration, its beats taken out: the text as the box prints it
 * (line breaks kept, as they turn its page) and how many beats hold before
 * each letter, by its index in the text.
 */
export type Words = { text: string; beats: Map<number, number> }

export const wordsOf = (narration: string): Words => {
  const beats = new Map<number, number>()
  let text = ""
  let last = 0
  for (const match of narration.matchAll(/\s*\|[\s|]*/g)) {
    text += narration.slice(last, match.index)
    last = match.index + match[0].length
    // The beat's own spaces close up: a line break if it had one, else a space between words.
    const atEdge = !text || last === narration.length
    text += match[0].includes("\n") ? "\n" : atEdge ? "" : " "
    const count = match[0].split(BEAT).length - 1
    beats.set(text.length, (beats.get(text.length) ?? 0) + count)
  }
  text += narration.slice(last)
  return { text, beats }
}

/**
 * What the voice is asked to say: the text in one paragraph, each beat a
 * pause tag (eleven_multilingual_v2 honours them; the v3 models don't).
 */
export const voiceText = ({ text, beats }: Words) => {
  let out = ""
  for (let i = 0; i <= text.length; i++) {
    const held = beats.get(i)
    if (held) out += `<break time="${(held * BEAT_SECONDS).toFixed(1)}s" />`
    if (i < text.length) out += text[i] === "\n" ? " " : text[i]
  }
  return out
}

/** A sentence's last letter: . ? or ! before a space, but not after a lone capital, as in S.S. */
const endsSentence = (text: string, i: number) =>
  /[.?!]/.test(text[i]!) && /\s/.test(text[i + 1] ?? " ") && !/\b[A-Z]$/.test(text.slice(0, i))

/**
 * The words paced as if read aloud, for a scene with no voice yet: when each
 * letter of the text is said, in seconds from the first, and when the last is done.
 */
export const paced = ({ text, beats }: Words) => {
  const times: number[] = []
  let t = 0
  for (let i = 0; i < text.length; i++) {
    t += (beats.get(i) ?? 0) * BEAT_SECONDS
    times.push(t)
    t += 1 / READ_LETTERS_PER_SECOND
    if (text[i] === "\n") t += BREAK_SECONDS
    else if (endsSentence(text, i) && text[i + 1] !== "\n") t += SENTENCE_SECONDS
  }
  t += (beats.get(text.length) ?? 0) * BEAT_SECONDS
  return { times, end: t }
}
