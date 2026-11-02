// Minimal PNG codec for the game's art: reads non-interlaced indexed, gray,
// RGB and RGBA images, and writes 8-bit RGBA. No dependencies.
import * as fs from "node:fs"
import * as zlib from "node:zlib"

const signature = Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a])

const channelsByColorType = { 0: 1, 2: 3, 3: 1, 4: 2, 6: 4 }

/**
 * @typedef {{ width: number, height: number, colorType: number, bitDepth: number,
 *   samples: Uint8Array, palette: number[][] | null }} DecodedPng
 * `samples` holds one entry per channel per pixel. For indexed images that is
 * the palette index, which is what GBA tile data means.
 */

/** @returns {DecodedPng} */
export const readPng = (file) => {
  const data = fs.readFileSync(file)
  if (!data.subarray(0, 8).equals(signature)) throw new Error(`${file} is not a PNG`)

  let offset = 8
  let header
  let palette = null
  let alpha = null
  const idat = []
  while (offset < data.length) {
    const length = data.readUInt32BE(offset)
    const type = data.toString("latin1", offset + 4, offset + 8)
    const body = data.subarray(offset + 8, offset + 8 + length)
    offset += 12 + length
    if (type === "IHDR") {
      header = {
        width: body.readUInt32BE(0),
        height: body.readUInt32BE(4),
        bitDepth: body[8],
        colorType: body[9],
        interlace: body[12],
      }
    } else if (type === "PLTE") {
      palette = []
      for (let i = 0; i < body.length; i += 3)
        palette.push([body[i], body[i + 1], body[i + 2], 255])
    } else if (type === "tRNS") {
      alpha = body
    } else if (type === "IDAT") {
      idat.push(body)
    } else if (type === "IEND") {
      break
    }
  }
  if (!header) throw new Error(`${file} has no IHDR`)
  if (header.interlace) throw new Error(`${file} is interlaced`)
  if (palette && alpha) for (let i = 0; i < alpha.length; i++) palette[i][3] = alpha[i]

  const { width, height, bitDepth, colorType } = header
  const channels = channelsByColorType[colorType]
  if (bitDepth === 16) throw new Error(`${file} uses 16-bit samples`)
  const bitsPerPixel = channels * bitDepth
  const stride = Math.ceil((width * bitsPerPixel) / 8)
  const bytesPerPixel = Math.max(1, bitsPerPixel >> 3)
  const raw = zlib.inflateSync(Buffer.concat(idat))
  const rows = Buffer.alloc(stride * height)

  for (let y = 0; y < height; y++) {
    const filter = raw[y * (stride + 1)]
    const source = raw.subarray(y * (stride + 1) + 1, (y + 1) * (stride + 1))
    const row = rows.subarray(y * stride, (y + 1) * stride)
    const previous = y > 0 ? rows.subarray((y - 1) * stride, y * stride) : null
    for (let x = 0; x < stride; x++) {
      const left = x >= bytesPerPixel ? row[x - bytesPerPixel] : 0
      const up = previous ? previous[x] : 0
      const upLeft = previous && x >= bytesPerPixel ? previous[x - bytesPerPixel] : 0
      let value = source[x]
      if (filter === 1) value += left
      else if (filter === 2) value += up
      else if (filter === 3) value += (left + up) >> 1
      else if (filter === 4) {
        const p = left + up - upLeft
        const pa = Math.abs(p - left)
        const pb = Math.abs(p - up)
        const pc = Math.abs(p - upLeft)
        value += pa <= pb && pa <= pc ? left : pb <= pc ? up : upLeft
      }
      row[x] = value & 0xff
    }
  }

  const samples = new Uint8Array(width * height * channels)
  if (bitDepth === 8) {
    for (let y = 0; y < height; y++)
      samples.set(rows.subarray(y * stride, y * stride + width * channels), y * width * channels)
  } else {
    const perByte = 8 / bitDepth
    const mask = (1 << bitDepth) - 1
    for (let y = 0; y < height; y++)
      for (let x = 0; x < width; x++) {
        const byte = rows[y * stride + Math.floor(x / perByte)]
        const shift = 8 - bitDepth * ((x % perByte) + 1)
        samples[y * width + x] = (byte >> shift) & mask
      }
  }

  return { width, height, colorType, bitDepth, samples, palette }
}

/** Expands any decoded PNG to RGBA, using its own palette when indexed. */
export const toRgba = (png) => {
  const { width, height, colorType, bitDepth, samples, palette } = png
  const rgba = new Uint8Array(width * height * 4)
  const scale = bitDepth < 8 && colorType !== 3 ? 255 / ((1 << bitDepth) - 1) : 1
  for (let i = 0; i < width * height; i++) {
    let pixel
    if (colorType === 3) pixel = palette[samples[i]] ?? [0, 0, 0, 0]
    else if (colorType === 0)
      pixel = [samples[i] * scale, samples[i] * scale, samples[i] * scale, 255]
    else if (colorType === 4)
      pixel = [samples[i * 2], samples[i * 2], samples[i * 2], samples[i * 2 + 1]]
    else if (colorType === 2) pixel = [samples[i * 3], samples[i * 3 + 1], samples[i * 3 + 2], 255]
    else pixel = samples.subarray(i * 4, i * 4 + 4)
    rgba.set(pixel, i * 4)
  }
  return rgba
}

const crcTable = new Int32Array(256).map((_, n) => {
  let c = n
  for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1
  return c
})

const crc32 = (buffer) => {
  let crc = -1
  for (const byte of buffer) crc = crcTable[(crc ^ byte) & 0xff] ^ (crc >>> 8)
  return (crc ^ -1) >>> 0
}

const chunk = (type, body) => {
  const head = Buffer.alloc(8)
  head.writeUInt32BE(body.length, 0)
  head.write(type, 4, "latin1")
  const crc = Buffer.alloc(4)
  crc.writeUInt32BE(crc32(Buffer.concat([head.subarray(4), body])), 0)
  return Buffer.concat([head, body, crc])
}

/** Writes an 8-bit RGBA PNG. */
export const writePng = (file, width, height, rgba) => {
  const header = Buffer.alloc(13)
  header.writeUInt32BE(width, 0)
  header.writeUInt32BE(height, 4)
  header[8] = 8
  header[9] = 6
  const raw = Buffer.alloc((width * 4 + 1) * height)
  for (let y = 0; y < height; y++)
    Buffer.from(rgba.buffer, rgba.byteOffset + y * width * 4, width * 4).copy(
      raw,
      y * (width * 4 + 1) + 1,
    )
  fs.writeFileSync(
    file,
    Buffer.concat([
      signature,
      chunk("IHDR", header),
      chunk("IDAT", zlib.deflateSync(raw, { level: 9 })),
      chunk("IEND", Buffer.alloc(0)),
    ]),
  )
}
