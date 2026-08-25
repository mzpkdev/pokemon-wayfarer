<script lang="ts">
  import type { CatalogMetatile, MetatileTileset } from "./catalog.js"
  import { metatileAssetUrl, metatileAtlasPosition, metatileScopedLabel } from "./catalog.js"

  type Props = {
    metatile: CatalogMetatile
    size?: number
    tileset: MetatileTileset
  }

  let { metatile, size = 48, tileset }: Props = $props()

  let atlasPosition = $derived(metatileAtlasPosition(metatile, tileset))
  let atlasScale = $derived(size / tileset.atlas.cellPixels)
  let backgroundStyle = $derived(
    [
      `background-image:url(${JSON.stringify(metatileAssetUrl(tileset.atlas.path))})`,
      `background-position:-${atlasPosition.column * size}px -${atlasPosition.row * size}px`,
      `background-size:${tileset.atlas.widthPixels * atlasScale}px ${tileset.atlas.heightPixels * atlasScale}px`,
      `height:${size}px`,
      `width:${size}px`,
    ].join(";"),
  )
</script>

<span
  class="block shrink-0 border border-cartographer-border bg-cartographer-field [image-rendering:pixelated]"
  role="img"
  aria-label={metatileScopedLabel(metatile)}
  style={backgroundStyle}
></span>
