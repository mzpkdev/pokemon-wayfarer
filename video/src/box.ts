// The narration in the game's text box: names in the game's capitals, the
// words wrapped to the box in its own font, two lines a page, and each page
// given its share of the scene to print and be read.
import { BOX_CAPS } from "./script"

/** Letters the box prints a second: the game's MID text speed, a letter every two frames. */
export const LETTERS_PER_SECOND = 30
/** Width of the box's text, in GBA pixels: 240 across, less the 8-pixel frame and 4-pixel padding a side. */
export const BOX_TEXT_WIDTH = 216
/** Lines a page holds, as in the game's own text box. */
const LINES_PER_PAGE = 2
/** Weight of a page beyond its letters, so a short page still stays up long enough to read. */
const PAGE_LETTERS = 24

/** The game's font at one GBA pixel a pixel, for measuring. */
export const BOX_FONT = '16px "Wayfarer Text"'

export type Page = { lines: string[]; text: string; from: number; to: number }

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
 * sentence too long for one page runs on over the next.
 */
export const paginate = (text: string, measure: (line: string) => number) => {
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
 * The narration paged and timed between `from` and `to` seconds into its
 * scene: each page's share follows its length, and it prints at the game's
 * speed from the start of its share.
 */
export const pagesFor = (
  narration: string,
  from: number,
  to: number,
  measure: (line: string) => number,
): Page[] => {
  const groups = paginate(gameCase(narration), measure)
  const weights = groups.map((group) => group.join(" ").length + PAGE_LETTERS)
  const total = weights.reduce((sum, weight) => sum + weight, 0)
  let at = from
  return groups.map((group, index) => {
    const length = ((to - from) * weights[index]!) / total
    const page = { lines: group, text: group.join("\n"), from: at, to: at + length }
    at += length
    return page
  })
}

/** The page showing at `seconds` into the scene: the last one stays up to the scene's end. */
export const pageAt = (pages: Page[], seconds: number) =>
  pages.find((page) => seconds < page.to) ?? pages[pages.length - 1]

/** A measure of text in the box's font, in GBA pixels; the font must have loaded. */
export const boxMeasure = () => {
  const context = document.createElement("canvas").getContext("2d")!
  context.font = BOX_FONT
  return (line: string) => context.measureText(line).width
}
