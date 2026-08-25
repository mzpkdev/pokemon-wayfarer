export type SkyEmuStatus = {
  "rom-loaded"?: boolean
}

export type SkyEmuHealth = {
  ready: boolean
  romLoaded: boolean
}

export type SkyEmuButton =
  | "A"
  | "B"
  | "Up"
  | "Down"
  | "Left"
  | "Right"
  | "L"
  | "R"
  | "Start"
  | "Select"

export type SkyEmuInput = Partial<Record<SkyEmuButton, 0 | 1>>

export class SkyEmuClient {
  readonly baseUrl: string

  constructor(port: number) {
    this.baseUrl = `http://127.0.0.1:${port}`
  }

  async health(): Promise<SkyEmuHealth> {
    const [ping, status] = await Promise.all([fetch(`${this.baseUrl}/ping`), this.status()])
    if (!ping.ok) throw new Error(`SkyEmu health check failed with HTTP ${ping.status}`)
    const romLoaded = status["rom-loaded"] === true
    const ready = (await ping.text()).replaceAll("\0", "") === "pong" && romLoaded

    return { ready, romLoaded }
  }

  async step(frames: number): Promise<string> {
    const response = await fetch(`${this.baseUrl}/step?frames=${frames}`)
    if (!response.ok) throw new Error(`SkyEmu step failed with HTTP ${response.status}`)
    return (await response.text()).replaceAll("\0", "")
  }

  async input(inputs: SkyEmuInput): Promise<string> {
    const parameters = new URLSearchParams()
    for (const [button, value] of Object.entries(inputs)) parameters.set(button, `${value}`)
    const response = await fetch(`${this.baseUrl}/input?${parameters}`)
    if (!response.ok) throw new Error(`SkyEmu input failed with HTTP ${response.status}`)
    return (await response.text()).replaceAll("\0", "")
  }

  async readBytes(address: number, length: number): Promise<Uint8Array> {
    if (!Number.isSafeInteger(address) || !Number.isSafeInteger(length) || length < 1) {
      throw new Error("SkyEmu memory reads require a valid address and positive length")
    }

    const parameters = new URLSearchParams()
    for (let offset = 0; offset < length; offset++) {
      parameters.append("addr", (address + offset).toString(16).padStart(8, "0"))
    }
    const response = await fetch(`${this.baseUrl}/read_byte?${parameters}`)
    if (!response.ok) throw new Error(`SkyEmu memory read failed with HTTP ${response.status}`)
    const hex = (await response.text()).replaceAll("\0", "")
    if (!new RegExp(`^[0-9a-f]{${length * 2}}$`, "i").test(hex)) {
      throw new Error(`SkyEmu returned an invalid memory response: ${hex}`)
    }

    const bytes = new Uint8Array(length)
    for (let index = 0; index < length; index++) {
      bytes[index] = Number.parseInt(hex.slice(index * 2, index * 2 + 2), 16)
    }
    return bytes
  }

  async readUint32LE(address: number): Promise<number> {
    const bytes = await this.readBytes(address, 4)
    return (bytes[0]! | (bytes[1]! << 8) | (bytes[2]! << 16) | (bytes[3]! << 24)) >>> 0
  }

  async status(): Promise<SkyEmuStatus> {
    const response = await fetch(`${this.baseUrl}/status`)
    if (!response.ok) throw new Error(`SkyEmu status failed with HTTP ${response.status}`)
    return response.json() as Promise<SkyEmuStatus>
  }
}
