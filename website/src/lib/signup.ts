/**
 * Where the launch sign-up form posts. Set at build time:
 *
 * - `PUBLIC_SIGNUP_ACTION`: the mailing list service's form URL
 * - `PUBLIC_SIGNUP_FIELD`: the name it expects for the email field (default `email`)
 *
 * Without an action the form is rendered disabled; a script then enables it as a
 * screen to try (the naming screen's input and OK), where a submit goes nowhere
 * and says so, so nobody signs up into a void.
 */
export const SIGNUP_ACTION: string | undefined = import.meta.env.PUBLIC_SIGNUP_ACTION || undefined

export const SIGNUP_FIELD: string = import.meta.env.PUBLIC_SIGNUP_FIELD || "email"
