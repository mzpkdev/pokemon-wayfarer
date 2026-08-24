export type SkyEmuStatus = {
  "rom-loaded"?: boolean
}

export class SkyEmuClient {
  readonly baseUrl: string

  constructor(port: number) {
    this.baseUrl = `http://127.0.0.1:${port}`
  }

  async ping(): Promise<string> {
    const response = await fetch(`${this.baseUrl}/ping`)
    if (!response.ok) throw new Error(`SkyEmu ping failed with HTTP ${response.status}`)
    return response.text()
  }

  async step(frames: number): Promise<string> {
    const response = await fetch(`${this.baseUrl}/step?frames=${frames}`)
    if (!response.ok) throw new Error(`SkyEmu step failed with HTTP ${response.status}`)
    return response.text()
  }

  async status(): Promise<SkyEmuStatus> {
    const response = await fetch(`${this.baseUrl}/status`)
    if (!response.ok) throw new Error(`SkyEmu status failed with HTTP ${response.status}`)
    return response.json() as Promise<SkyEmuStatus>
  }
}
