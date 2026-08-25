export type SkyEmuStatus = {
  "rom-loaded"?: boolean
}

export type SkyEmuHealth = {
  ready: boolean
  romLoaded: boolean
}

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

  async status(): Promise<SkyEmuStatus> {
    const response = await fetch(`${this.baseUrl}/status`)
    if (!response.ok) throw new Error(`SkyEmu status failed with HTTP ${response.status}`)
    return response.json() as Promise<SkyEmuStatus>
  }
}
