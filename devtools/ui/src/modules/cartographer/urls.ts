/** Build an asset URL that works from both Vite and a relative static deployment. */
export const cartographerUrl = (path: string, baseUrl = import.meta.env.BASE_URL): string => {
  const normalizedBase = baseUrl.endsWith("/") ? baseUrl : `${baseUrl}/`
  return `${normalizedBase}${path.replace(/^\/+/, "")}`
}

export const catalogUrl = (baseUrl?: string): string => {
  return cartographerUrl("catalog.json", baseUrl)
}

export const mapImageUrl = (path: string, baseUrl?: string): string => {
  return cartographerUrl(path, baseUrl)
}

export type CartographerViewState = {
  center: [number, number]
  zoom: number
}

export type CartographerUrlState = {
  region: string | null
  selectedMap: string | null
  view: CartographerViewState | null
  trainerRating: number
  product: string | null
}

export const MIN_TRAINER_RATING = 10
export const MAX_TRAINER_RATING = 80

export const clampTrainerRating = (value: number | null): number => {
  if (value === null) return MIN_TRAINER_RATING
  return Math.min(MAX_TRAINER_RATING, Math.max(MIN_TRAINER_RATING, Math.round(value)))
}

const parameter = (url: URL, name: string): string | null => {
  return url.searchParams.get(name)?.trim() || null
}

const numberParameter = (url: URL, name: string): number | null => {
  const value = url.searchParams.get(name)
  const number = value === null ? Number.NaN : Number(value)
  return Number.isFinite(number) ? number : null
}

const documentOrigin = (): string => {
  return typeof window === "undefined" ? "http://localhost" : window.location.origin
}

export const parseCartographerUrlState = (href: string): CartographerUrlState => {
  const url = new URL(href, documentOrigin())
  const x = numberParameter(url, "x")
  const y = numberParameter(url, "y")
  const zoom = numberParameter(url, "zoom")
  return {
    region: parameter(url, "region"),
    selectedMap: parameter(url, "map"),
    view: x === null || y === null || zoom === null ? null : { center: [x, y], zoom },
    trainerRating: clampTrainerRating(numberParameter(url, "rating")),
    product: parameter(url, "product"),
  }
}

export const cartographerUrlWithState = (href: string, state: CartographerUrlState): string => {
  const url = new URL(href, documentOrigin())
  for (const name of ["region", "map", "x", "y", "zoom", "rating", "product"]) {
    url.searchParams.delete(name)
  }
  if (state.region) url.searchParams.set("region", state.region)
  if (state.selectedMap) url.searchParams.set("map", state.selectedMap)
  url.searchParams.set("rating", String(clampTrainerRating(state.trainerRating)))
  if (state.product) url.searchParams.set("product", state.product)
  if (state.view) {
    url.searchParams.set("x", String(Math.round(state.view.center[0] * 100) / 100))
    url.searchParams.set("y", String(Math.round(state.view.center[1] * 100) / 100))
    url.searchParams.set("zoom", String(Math.round(state.view.zoom * 100) / 100))
  }
  return `${url.pathname}${url.search}${url.hash}`
}
