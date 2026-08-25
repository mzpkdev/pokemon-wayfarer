type PaletteToken =
  | "amber"
  | "border"
  | "border-strong"
  | "diagnostic"
  | "diagnostic-ink"
  | "diagnostic-line"
  | "diagnostic-panel"
  | "field"
  | "habitat-land"
  | "habitat-water"
  | "ink"
  | "layout-overlap"
  | "muted"
  | "muted-soft"
  | "object-expression"
  | "object-stateful"
  | "object-unresolved"
  | "object-variable"
  | "panel"
  | "signal"
  | "signal-soft"
  | "signal-strong"

const colorProperty = (token: PaletteToken): string => `--color-cartographer-${token}`

const resolveColorValue = (value: string, styles: CSSStyleDeclaration): string => {
  const reference = value.match(/^var\((--[^)]+)\)$/)?.[1]
  if (!reference) return value
  return resolveColorValue(styles.getPropertyValue(reference).trim(), styles)
}

export const cartographerColor = (token: PaletteToken): string => {
  const styles = getComputedStyle(document.documentElement)
  return resolveColorValue(styles.getPropertyValue(colorProperty(token)).trim(), styles)
}

export const cartographerColorWithAlpha = (token: PaletteToken, alpha: number): string => {
  const color = cartographerColor(token)
  const suffix = Math.round(Math.min(1, Math.max(0, alpha)) * 255)
    .toString(16)
    .padStart(2, "0")
  return `${color}${suffix}`
}
