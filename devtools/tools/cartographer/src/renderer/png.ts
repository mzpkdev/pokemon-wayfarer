import * as fs from "node:fs"
import * as path from "node:path"
import * as zlib from "node:zlib"

import type { IndexedPng, Rgb } from "./types"

const pngSignature = "89504e470d0a1a0a"

const readPngData = (filePath: string): { header: Buffer; imageData: Buffer[] } => {
  const data = fs.readFileSync(filePath)
  if (data.subarray(0, 8).toString("hex") !== pngSignature) {
    throw new Error(`not a PNG: ${filePath}`)
  }

  let position = 8
  let header: Buffer | undefined
  const imageData: Buffer[] = []
  while (position < data.length) {
    const size = data.readUInt32BE(position)
    const kind = data.subarray(position + 4, position + 8).toString("ascii")
    const payloadStart = position + 8
    const payloadEnd = payloadStart + size
    const payload = data.subarray(payloadStart, payloadEnd)
    position = payloadEnd + 4
    if (kind === "IHDR") {
      header = payload
    } else if (kind === "IDAT") {
      imageData.push(payload)
    } else if (kind === "IEND") {
      break
    }
  }

  if (!header || header.length !== 13) {
    throw new Error(`missing PNG header: ${filePath}`)
  }
  return { header, imageData }
}

export const readIndexedPng = (filePath: string): IndexedPng => {
  const { header, imageData } = readPngData(filePath)
  const width = header.readUInt32BE(0)
  const height = header.readUInt32BE(4)
  const depth = header[8] ?? 0
  const color = header[9] ?? 0
  const interlace = header[12]
  if (color !== 3 || (depth !== 4 && depth !== 8) || interlace !== 0) {
    throw new Error(`unsupported PNG format in ${filePath}: depth=${depth}, color=${color}`)
  }

  const raw = zlib.inflateSync(Buffer.concat(imageData))
  const packedWidth = Math.ceil((width * depth) / 8)
  const rows: Uint8Array[] = []
  let previous = new Uint8Array(packedWidth)
  let offset = 0
  for (let rowIndex = 0; rowIndex < height; rowIndex += 1) {
    const mode = raw[offset]
    offset += 1
    const scan = new Uint8Array(raw.subarray(offset, offset + packedWidth))
    offset += packedWidth
    for (let x = 0; x < packedWidth; x += 1) {
      const left = x === 0 ? 0 : scan[x - 1]!
      const above = previous[x]!
      const upperLeft = x === 0 ? 0 : previous[x - 1]!
      if (mode === 1) {
        scan[x] = (scan[x]! + left) & 0xff
      } else if (mode === 2) {
        scan[x] = (scan[x]! + above) & 0xff
      } else if (mode === 3) {
        scan[x] = (scan[x]! + Math.floor((left + above) / 2)) & 0xff
      } else if (mode === 4) {
        const estimate = left + above - upperLeft
        const leftDistance = Math.abs(estimate - left)
        const aboveDistance = Math.abs(estimate - above)
        const upperLeftDistance = Math.abs(estimate - upperLeft)
        const predictor =
          leftDistance <= aboveDistance && leftDistance <= upperLeftDistance
            ? left
            : aboveDistance <= upperLeftDistance
              ? above
              : upperLeft
        scan[x] = (scan[x]! + predictor) & 0xff
      } else if (mode !== 0) {
        throw new Error(`unsupported PNG filter ${mode} in ${filePath}`)
      }
    }
    if (depth === 4) {
      const expanded = new Uint8Array(width)
      for (let x = 0; x < width; x += 1) {
        const value = scan[Math.floor(x / 2)]!
        expanded[x] = x % 2 === 0 ? value >> 4 : value & 0x0f
      }
      rows.push(expanded)
    } else {
      rows.push(scan)
    }
    previous = scan
  }
  return { width, height, rows }
}

const crc32 = (bytes: Uint8Array): number => {
  let value = 0xffffffff
  for (const byte of bytes) {
    value ^= byte
    for (let bit = 0; bit < 8; bit += 1) {
      value = (value >>> 1) ^ (value & 1 ? 0xedb88320 : 0)
    }
  }
  return (value ^ 0xffffffff) >>> 0
}

const pngChunk = (kind: string, payload: Uint8Array): Buffer => {
  const kindBytes = Buffer.from(kind, "ascii")
  const chunk = Buffer.alloc(12 + payload.length)
  chunk.writeUInt32BE(payload.length, 0)
  kindBytes.copy(chunk, 4)
  Buffer.from(payload).copy(chunk, 8)
  chunk.writeUInt32BE(crc32(Buffer.concat([kindBytes, Buffer.from(payload)])), 8 + payload.length)
  return chunk
}

export const writeRgbPng = (
  filePath: string,
  width: number,
  height: number,
  pixels: Uint8Array,
): void => {
  const stride = width * 3
  const raw = Buffer.alloc(height * (stride + 1))
  for (let y = 0; y < height; y += 1) {
    const outputOffset = y * (stride + 1)
    raw[outputOffset] = 0
    Buffer.from(pixels).copy(raw, outputOffset + 1, y * stride, (y + 1) * stride)
  }
  const header = Buffer.alloc(13)
  header.writeUInt32BE(width, 0)
  header.writeUInt32BE(height, 4)
  header[8] = 8
  header[9] = 2
  fs.mkdirSync(path.dirname(filePath), { recursive: true })
  fs.writeFileSync(
    filePath,
    Buffer.concat([
      Buffer.from(pngSignature, "hex"),
      pngChunk("IHDR", header),
      pngChunk("IDAT", zlib.deflateSync(raw, { level: 9 })),
      pngChunk("IEND", Buffer.alloc(0)),
    ]),
  )
}

type RgbaPng = {
  width: number
  height: number
  pixels: Uint8Array
}

const unfilterPngRows = (
  raw: Buffer,
  stride: number,
  height: number,
  bytesPerPixel: number,
): Uint8Array => {
  const rows = new Uint8Array(height * stride)
  let inputOffset = 0
  for (let y = 0; y < height; y += 1) {
    const filter = raw[inputOffset]!
    inputOffset += 1
    const outputOffset = y * stride
    for (let x = 0; x < stride; x += 1) {
      const source = raw[inputOffset + x]!
      const left = x < bytesPerPixel ? 0 : rows[outputOffset + x - bytesPerPixel]!
      const above = y === 0 ? 0 : rows[outputOffset - stride + x]!
      const upperLeft =
        y === 0 || x < bytesPerPixel ? 0 : rows[outputOffset - stride + x - bytesPerPixel]!
      if (filter === 0) {
        rows[outputOffset + x] = source
      } else if (filter === 1) {
        rows[outputOffset + x] = (source + left) & 0xff
      } else if (filter === 2) {
        rows[outputOffset + x] = (source + above) & 0xff
      } else if (filter === 3) {
        rows[outputOffset + x] = (source + Math.floor((left + above) / 2)) & 0xff
      } else if (filter === 4) {
        const estimate = left + above - upperLeft
        const leftDistance = Math.abs(estimate - left)
        const aboveDistance = Math.abs(estimate - above)
        const upperLeftDistance = Math.abs(estimate - upperLeft)
        const predictor =
          leftDistance <= aboveDistance && leftDistance <= upperLeftDistance
            ? left
            : aboveDistance <= upperLeftDistance
              ? above
              : upperLeft
        rows[outputOffset + x] = (source + predictor) & 0xff
      } else {
        throw new Error(`unsupported PNG filter ${filter}`)
      }
    }
    inputOffset += stride
  }
  return rows
}

const readRgbaPng = (filePath: string): RgbaPng => {
  const data = fs.readFileSync(filePath)
  if (data.subarray(0, 8).toString("hex") !== pngSignature) {
    throw new Error(`not a PNG: ${filePath}`)
  }

  let position = 8
  let header: Buffer | undefined
  let palette: Buffer | undefined
  let transparency: Buffer | undefined
  const imageData: Buffer[] = []
  while (position < data.length) {
    const size = data.readUInt32BE(position)
    const kind = data.subarray(position + 4, position + 8).toString("ascii")
    const payloadStart = position + 8
    const payloadEnd = payloadStart + size
    const payload = data.subarray(payloadStart, payloadEnd)
    position = payloadEnd + 4
    if (kind === "IHDR") {
      header = payload
    } else if (kind === "PLTE") {
      palette = payload
    } else if (kind === "tRNS") {
      transparency = payload
    } else if (kind === "IDAT") {
      imageData.push(payload)
    } else if (kind === "IEND") {
      break
    }
  }
  if (!header || header.length !== 13) {
    throw new Error(`missing PNG header: ${filePath}`)
  }
  const width = header.readUInt32BE(0)
  const height = header.readUInt32BE(4)
  const depth = header[8] ?? 0
  const color = header[9] ?? 0
  if (
    header[12] !== 0 ||
    (color !== 3 && color !== 6) ||
    (color === 3 && depth !== 4 && depth !== 8) ||
    (color === 6 && depth !== 8)
  ) {
    throw new Error(`unsupported sprite PNG format in ${filePath}: depth=${depth}, color=${color}`)
  }
  const bytesPerPixel = color === 3 ? 1 : 4
  const stride = color === 3 ? Math.ceil((width * depth) / 8) : width * bytesPerPixel
  const rows = unfilterPngRows(
    zlib.inflateSync(Buffer.concat(imageData)),
    stride,
    height,
    bytesPerPixel,
  )
  const pixels = new Uint8Array(width * height * 4)
  if (color === 6) {
    pixels.set(rows)
    return { width, height, pixels }
  }
  if (!palette) {
    throw new Error(`indexed sprite PNG is missing a palette: ${filePath}`)
  }
  for (let index = 0; index < width * height; index += 1) {
    const packedIndex =
      Math.floor(((index % width) * depth) / 8) + Math.floor(index / width) * stride
    const packed = rows[packedIndex]!
    const paletteIndex =
      depth === 4 && index % 2 === 0 ? packed >> 4 : depth === 4 ? packed & 0x0f : packed
    const paletteOffset = paletteIndex * 3
    if (paletteOffset + 2 >= palette.length) {
      throw new Error(`sprite PNG references a missing palette entry: ${filePath}`)
    }
    const outputOffset = index * 4
    pixels[outputOffset] = palette[paletteOffset]!
    pixels[outputOffset + 1] = palette[paletteOffset + 1]!
    pixels[outputOffset + 2] = palette[paletteOffset + 2]!
    pixels[outputOffset + 3] = paletteIndex === 0 ? 0 : (transparency?.[paletteIndex] ?? 255)
  }
  return { width, height, pixels }
}

export const cropSpriteFrame = (
  input: string,
  frame: { index: number; width: number; height: number },
): RgbaPng => {
  const sprite = readRgbaPng(input)
  const x = frame.index * frame.width
  if (frame.index < 0 || x + frame.width > sprite.width || frame.height > sprite.height) {
    throw new Error(
      `${input}: frame ${frame.index} (${frame.width}x${frame.height}) is outside ${sprite.width}x${sprite.height}`,
    )
  }
  const pixels = new Uint8Array(frame.width * frame.height * 4)
  for (let y = 0; y < frame.height; y += 1) {
    const sourceStart = (y * sprite.width + x) * 4
    pixels.set(
      sprite.pixels.subarray(sourceStart, sourceStart + frame.width * 4),
      y * frame.width * 4,
    )
  }
  return { width: frame.width, height: frame.height, pixels }
}

export const replaceSpritePalette = (
  sprite: RgbaPng,
  original: readonly Rgb[],
  replacement: readonly Rgb[],
): RgbaPng => {
  if (original.length !== replacement.length) {
    throw new Error("sprite palettes have different color counts")
  }
  const pixels = new Uint8Array(sprite.pixels)
  for (let offset = 0; offset < pixels.length; offset += 4) {
    if (pixels[offset + 3] === 0) continue
    const index = original.findIndex(
      (color) =>
        color[0] === pixels[offset] &&
        color[1] === pixels[offset + 1] &&
        color[2] === pixels[offset + 2],
    )
    const color = replacement[index]
    if (!color) continue
    pixels[offset] = color[0]
    pixels[offset + 1] = color[1]
    pixels[offset + 2] = color[2]
  }
  return { ...sprite, pixels }
}

export const writeRgbaPng = (
  filePath: string,
  width: number,
  height: number,
  pixels: Uint8Array,
): void => {
  const stride = width * 4
  const raw = Buffer.alloc(height * (stride + 1))
  for (let y = 0; y < height; y += 1) {
    const outputOffset = y * (stride + 1)
    raw[outputOffset] = 0
    Buffer.from(pixels).copy(raw, outputOffset + 1, y * stride, (y + 1) * stride)
  }
  const header = Buffer.alloc(13)
  header.writeUInt32BE(width, 0)
  header.writeUInt32BE(height, 4)
  header[8] = 8
  header[9] = 6
  fs.mkdirSync(path.dirname(filePath), { recursive: true })
  fs.writeFileSync(
    filePath,
    Buffer.concat([
      Buffer.from(pngSignature, "hex"),
      pngChunk("IHDR", header),
      pngChunk("IDAT", zlib.deflateSync(raw, { level: 9 })),
      pngChunk("IEND", Buffer.alloc(0)),
    ]),
  )
}

/**
 * Write a deterministic nearest-neighbour preview from a PNG emitted by this
 * renderer. Render output deliberately uses unfiltered 8-bit RGB rows, so the
 * preview can stay dependency-free while retaining exact terrain colours.
 */
export const writeNearestNeighborOverview = (input: string, output: string, scale = 4): void => {
  const { header, imageData } = readPngData(input)
  if (header[8] !== 8 || header[9] !== 2 || header[12] !== 0) {
    throw new Error(`unsupported rendered PNG: ${input}`)
  }
  const width = header.readUInt32BE(0)
  const height = header.readUInt32BE(4)
  if (width % scale !== 0 || height % scale !== 0) {
    throw new Error(`${input}: dimensions must divide evenly by ${scale}`)
  }
  const source = zlib.inflateSync(Buffer.concat(imageData))
  const sourceStride = width * 3 + 1
  if (source.length !== height * sourceStride) {
    throw new Error(`${input}: unexpected RGB data length`)
  }
  const outputWidth = width / scale
  const outputHeight = height / scale
  const pixels = new Uint8Array(outputWidth * outputHeight * 3)
  for (let y = 0; y < outputHeight; y += 1) {
    const sourceRow = y * scale * sourceStride
    if (source[sourceRow] !== 0) {
      throw new Error(`${input}: unsupported PNG filter`)
    }
    for (let x = 0; x < outputWidth; x += 1) {
      const sourceOffset = sourceRow + 1 + x * scale * 3
      pixels.set(source.subarray(sourceOffset, sourceOffset + 3), (y * outputWidth + x) * 3)
    }
  }
  writeRgbPng(output, outputWidth, outputHeight, pixels)
}
