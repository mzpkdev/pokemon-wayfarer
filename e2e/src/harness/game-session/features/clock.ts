import { type SessionRuntime } from "../runtime"

// struct SiiRtcInfo (include/siirtc.h): year, month, day, dayOfWeek, hour, minute, second.
const clockBytes = 7

export type ClockReading = {
  /** Years since 2000, the clock's epoch. */
  year: number
  month: number
  day: number
  dayOfWeek: number
  hour: number
  minute: number
  second: number
  /** Whole in-game seconds since the epoch's midnight, for measuring how far the clock moved. */
  totalSeconds: number
  /** The clock's day number: day 1 is 1 January 2000, the day a new game starts on. */
  dayNumber: number
  /**
   * The day every reader last computed (`gLocalTime.days`, refreshed at least once per in-game
   * minute on the field). It equals `dayNumber` unless something set a local offset.
   */
  localDay: number
}

export type ClockApi = {
  /**
   * The saved in-game clock (.product/specs/in-game-clock.md), the one source of every day and
   * time of day. It runs 16 in-game seconds per 60 frames of play.
   */
  read: () => Promise<ClockReading>
  /**
   * Sets the time of day on the clock's current date, for example to just before midnight so a
   * journey sees a day start. It never changes the date, so the day count can't go backwards.
   */
  setTimeOfDay: (time: { hour: number; minute: number; second: number }) => Promise<void>
}

const daysInMonth = (year: number, month: number): number => {
  const fullYear = 2000 + year
  if (month === 2)
    return fullYear % 4 === 0 && (fullYear % 100 !== 0 || fullYear % 400 === 0) ? 29 : 28
  return [4, 6, 9, 11].includes(month) ? 30 : 31
}

const dayIndex = (year: number, month: number, day: number): number => {
  let days = 0
  for (let y = 0; y < year; y++) days += daysInMonth(y, 2) === 29 ? 366 : 365
  for (let m = 1; m < month; m++) days += daysInMonth(year, m)
  return days + day - 1
}

export const createClockApi = (runtime: SessionRuntime): ClockApi => {
  const clockAddress = async (): Promise<number> => {
    const saveBlock3 = await runtime.readUint32(runtime.address("gSaveBlock3Ptr"))
    return saveBlock3 + (await runtime.readUint16(runtime.address("gE2EInGameClockOffset")))
  }
  return {
    read: async () => {
      const bytes = await runtime.readBytes(await clockAddress(), clockBytes)
      const [year, month, day, dayOfWeek, hour, minute, second] = Array.from(bytes) as [
        number,
        number,
        number,
        number,
        number,
        number,
        number,
      ]
      const index = dayIndex(year, month, day)
      const totalSeconds = ((index * 24 + hour) * 60 + minute) * 60 + second
      const localDays = await runtime.readUint16(runtime.address("gLocalTime"))
      const localDay = localDays >= 0x8000 ? localDays - 0x10000 : localDays
      return {
        year,
        month,
        day,
        dayOfWeek,
        hour,
        minute,
        second,
        totalSeconds,
        dayNumber: index + 1,
        localDay,
      }
    },
    setTimeOfDay: async ({ hour, minute, second }) => {
      if (
        ![hour, minute, second].every(Number.isInteger) ||
        hour < 0 ||
        hour > 23 ||
        minute < 0 ||
        minute > 59 ||
        second < 0 ||
        second > 59
      )
        throw new Error(`Not a time of day: ${hour}:${minute}:${second}`)
      await runtime.writeBytes((await clockAddress()) + 4, new Uint8Array([hour, minute, second]))
    },
  }
}
