import { type ControlsApi } from "./controls"
import { decodeFieldMessageText, type StateApi } from "./state"
import { type StoryApi } from "./story"
import { type SessionRuntime } from "../runtime"

// League ids (include/league_selection.h) and LEAGUE_PHONE_DEBUG_ROTATION (include/constants/league_phone.h).
const leagueIds = { indigo: 1, hoenn: 3 } as const
const debugRotation = 0xff

export type CallingLeague = keyof typeof leagueIds

export type LeaguesApi = {
  /**
   * Gives the player the phone and places a league's invitation call now, as if the seven-day
   * countdown had run out (.product/specs/leagues.md#invitations): from `league` when it knows the
   * player, else by the usual rotation. The field rings at its next frame with control.
   */
  forceInvitation: (league?: CallingLeague) => Promise<void>
  /**
   * Waits for a league call to ring, presses through it, answers the yes/no prompt when an
   * invitation asks one (a decline also confirms it at its confirmation), and presses through the
   * reply until the field is free. Returns each distinct
   * text seen, in order: the field message box and the phone's call box (`gStringVar4`). When no
   * call rings within about ten seconds of play, it returns an empty list.
   */
  answerCall: (answer?: "accept" | "decline") => Promise<string[]>
}

export const createLeaguesApi = (
  runtime: SessionRuntime,
  state: StateApi,
  story: StoryApi,
  controls: ControlsApi,
): LeaguesApi => ({
  forceInvitation: async (league) => {
    await story.setFlag("hasMatchCall", true)
    await runtime.writeBytes(
      runtime.address("gLeaguePhoneDebugInvitation"),
      new Uint8Array([league ? leagueIds[league] : debugRotation]),
    )
  },
  answerCall: async (answer = "accept") => {
    // Calls and replies advance with B, which never talks to whoever the player is facing once the
    // call is over. Each yes/no prompt is answered on purpose: the invitation with the answer, and the
    // decline's confirmation ("Decline? ...") with YES to decline or NO to go back and accept.
    const seen: string[] = []
    let rang = false
    // The prompt last answered: its text can stay in the field box while the phone reply plays.
    let answered = ""
    const note = (text: string): void => {
      if (text && seen.at(-1) !== text) seen.push(text)
    }
    for (let attempt = 0; attempt < 900; attempt++) {
      const current = await state.read()
      const busy = current.scriptActive || current.dialogueOpen || current.controlsLocked
      rang ||= busy
      if (busy) {
        note(current.dialogue.fullText)
        note(
          decodeFieldMessageText(
            Array.from(await runtime.readBytes(runtime.address("gStringVar4"), 400)),
          ),
        )
      }
      if (rang && current.ready && !busy && !current.battle.active) return seen
      if (!rang && attempt >= 60) return seen
      if (current.choice.kind === "yes-no") {
        const confirming = current.dialogue.fullText.startsWith("Decline?")
        const yes = confirming ? answer === "decline" : answer === "accept"
        const wanted = yes ? 0 : 1
        if (current.choice.cursor !== null && current.choice.cursor !== wanted)
          await controls.press(wanted === 0 ? "up" : "down")
        else {
          await controls.press(yes ? "a" : "b")
          answered = current.dialogue.fullText
        }
        await runtime.advance(10)
      } else if (
        /^(Accept the |Decline\?)/.test(current.dialogue.fullText) &&
        current.dialogue.fullText !== answered
      ) {
        // A prompt's yes/no box opens by itself after its text: pressing now would answer it blind.
        await runtime.advance(4)
      } else if (busy || current.battle.active) {
        // B never talks to anyone on the field, but a battle (such as a lost match before its blackout
        // and result call) only moves on with A.
        await runtime.advance(20)
        await controls.press(current.battle.active ? "a" : "b")
      } else await runtime.advance(10)
    }
    throw new Error(`The league call did not finish: ${JSON.stringify(await state.read())}`)
  },
})
