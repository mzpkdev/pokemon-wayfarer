// The narration in the game's text box: names in the game's capitals, the
// words wrapped to the box in its own font, two lines a page, and each letter
// printed as the voice says it (or as the paced words would, without one).
import { BOX_CAPS } from "./script"

/** Width of the box's text, in GBA pixels: 240 across, less the 8-pixel frame and 4-pixel padding a side. */
export const BOX_TEXT_WIDTH = 216
/** GBA pixels across the frame: the video is 480 of them wide, so 4 CSS pixels each at 1080p. */
export const GBA_COLUMNS = 480
/** Gap under the box, and the box's height: its 8-pixel frame top and bottom, 2-pixel padding and two 16-pixel lines. */
export const BOX_BOTTOM = 8
export const BOX_TEXT_HEIGHT = 32
const BOX_HEIGHT = 8 + 2 + BOX_TEXT_HEIGHT + 2 + 8
/** CSS pixels of a frame `width` across that one GBA pixel takes. */
export const gbaPx = (width: number) => Math.max(1, Math.round(width / GBA_COLUMNS))
/** How much of a frame `width` across the text box hides at its foot, in CSS pixels. */
export const boxCover = (width: number) => (BOX_BOTTOM + BOX_HEIGHT) * gbaPx(width)

/** Lines a page holds, as in the game's own text box. */
const LINES_PER_PAGE = 2

/** The game's font at one GBA pixel a pixel, for measuring. */
export const BOX_FONT = '16px "Wayfarer Text"'

/** A page of the box: its lines, and when it comes up and each of its letters prints, in seconds into the scene. */
export type Page = { lines: string[]; text: string; from: number; printAt: number[] }

/** The narration as the game writes it: POKéMON, place names and people in capitals. */
export const gameCase = (text: string) =>
  BOX_CAPS.reduce(
    (out, name) =>
      out.replace(
        new RegExp(`\\b${name.replaceAll(".", "\\.")}\\b`, "g"),
        name.toUpperCase().replace("POKÉMON", "POKéMON"),
      ),
    text,
  )

/** The words in lines no wider than `width`, as `measure` sees them. */
export const wrap = (text: string, width: number, measure: (line: string) => number) => {
  const lines: string[] = []
  let line = ""
  for (const word of text.split(/\s+/).filter(Boolean)) {
    const next = line ? `${line} ${word}` : word
    if (line && measure(next) > width) {
      lines.push(line)
      line = word
    } else line = next
  }
  if (line) lines.push(line)
  return lines
}

/**
 * The text in pages of two lines, turning the page where a sentence ends, as
 * the game's own scripts do: sentences share a page while they fit, and a
 * sentence too long for one page runs on over the next. A line break in the
 * script turns the page wherever it falls.
 */
export const paginate = (text: string, measure: (line: string) => number): string[][] => {
  // A line break in the script always turns the page.
  const paragraphs = text.split(/\s*\n\s*/)
  if (paragraphs.length > 1) return paragraphs.flatMap((paragraph) => paginate(paragraph, measure))
  // A sentence ends at . ? or ! before a space, but not after a lone capital, as in S.S.
  const sentences = text.split(/(?<=(?<!\b[A-Z])[.?!]+)\s+/)
  const pages: string[][] = []
  let open = ""
  for (const sentence of sentences.filter(Boolean)) {
    const joined = open ? `${open} ${sentence}` : sentence
    if (open && wrap(joined, BOX_TEXT_WIDTH, measure).length > LINES_PER_PAGE) {
      pages.push(wrap(open, BOX_TEXT_WIDTH, measure))
      open = sentence
    } else open = joined
    // Full pages of a long sentence go out now; what is left stays open for the next.
    let lines = wrap(open, BOX_TEXT_WIDTH, measure)
    while (lines.length > LINES_PER_PAGE) {
      pages.push(lines.slice(0, LINES_PER_PAGE))
      lines = lines.slice(LINES_PER_PAGE)
    }
    open = lines.join(" ")
  }
  if (open) pages.push(wrap(open, BOX_TEXT_WIDTH, measure))
  return pages
}

/**
 * The words paged, each page up from the moment the voice starts it and each
 * letter printing as it is said, never ahead of the voice, so a punchline
 * isn't read before it's heard. `times` holds when each letter of
 * `words.text` is said, in seconds into the scene.
 */
export const pagesFor = (
  text: string,
  times: number[],
  measure: (line: string) => number,
): Page[] => {
  // The pages hold the text's letters in order, only its spaces redrawn, so the
  // n-th letter on the pages is the n-th letter of the text.
  const letters = text.split("").flatMap((letter, i) => (/\s/.test(letter) ? [] : [times[i]!]))
  let n = 0
  return paginate(gameCase(text), measure).map((lines) => {
    const page = lines.join("\n")
    let at = letters[n] ?? 0
    const printAt = page
      .split("")
      .map((letter) => (/\s/.test(letter) ? at : (at = letters[n++] ?? at)))
    return { lines, text: page, from: printAt[0] ?? 0, printAt }
  })
}

/** The page up at `seconds` into the scene: each stays until the next is started, the last to the end. */
export const pageAt = (pages: Page[], seconds: number) =>
  pages.findLast((page) => page.from <= seconds)

/** How many of a page's letters have printed at `seconds` into the scene. */
export const printedAt = (page: Page, seconds: number) => {
  const next = page.printAt.findIndex((at) => at > seconds)
  return next < 0 ? page.text.length : next
}

/** A measure of text in the box's font, in GBA pixels; the font must have loaded. */
export const boxMeasure = () => {
  const context = document.createElement("canvas").getContext("2d")!
  context.font = BOX_FONT
  return (line: string) => context.measureText(line).width
}
