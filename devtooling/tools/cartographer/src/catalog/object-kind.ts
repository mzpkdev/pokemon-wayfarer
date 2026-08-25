import type { ObjectEvent } from "./types"

import { speciesGraphicsFor } from "./species-graphics"

export type ObjectKind = {
  id:
    | "trainer"
    | "item"
    | "pokemon"
    | "npc"
    | "sign"
    | "light"
    | "berry-tree"
    | "obstacle"
    | "scripted"
    | "scenery"
  label: string
  evidence: "trainer-type" | "graphics" | "script" | "fallback"
  action: string | null
}

const titleFor = (value: string, prefix: string): string => {
  return value
    .replace(prefix, "")
    .split("_")
    .filter((part) => part && part !== "HNS")
    .map((part) =>
      part === "HP" || part === "PP" || /^TM\d*$/.test(part)
        ? part
        : `${part[0]}${part.slice(1).toLowerCase()}`,
    )
    .join(" ")
}

const actionForScript = (script: string | undefined): string | null => {
  if (!script) return null
  const item = script.match(/\b(?:finditem|giveitem)\s+(ITEM_\w+)/)
  if (item?.[1]) return `Gives ${titleFor(item[1], "ITEM_")}`
  const trainer = script.match(/\btrainerbattle(?:_\w+)?\s+(TRAINER_\w+)/)
  if (trainer?.[1]) return `Battles ${titleFor(trainer[1], "TRAINER_")}`
  if (/\bmsgbox\b[^\n]*\bMSGBOX_SIGN\b/.test(script)) return "Reads a sign"
  return null
}

export const classifyObject = (
  event: ObjectEvent,
  spriteSource: string | null,
  script: string | undefined,
): ObjectKind => {
  const action = actionForScript(script)
  if (event.trainer_type !== "TRAINER_TYPE_NONE") {
    return { id: "trainer", label: "Trainer", evidence: "trainer-type", action }
  }
  if (event.graphics_id === "OBJ_EVENT_GFX_ITEM_BALL") {
    return { id: "item", label: "Item", evidence: "graphics", action }
  }
  if (speciesGraphicsFor(event.graphics_id)) {
    return { id: "pokemon", label: "Pokémon", evidence: "graphics", action }
  }
  if (event.graphics_id === "OBJ_EVENT_GFX_LIGHT_SPRITE") {
    return { id: "light", label: "Light source", evidence: "graphics", action }
  }
  if (event.graphics_id === "OBJ_EVENT_GFX_BERRY_TREE") {
    return { id: "berry-tree", label: "Berry tree", evidence: "graphics", action }
  }
  if (event.graphics_id.includes("BREAKABLE_ROCK")) {
    return { id: "obstacle", label: "Obstacle", evidence: "graphics", action }
  }
  if (event.graphics_id.includes("SIGN") || action === "Reads a sign") {
    return {
      id: "sign",
      label: "Sign",
      evidence: event.graphics_id.includes("SIGN") ? "graphics" : "script",
      action,
    }
  }
  if (spriteSource?.includes("/people/")) {
    return { id: "npc", label: "NPC", evidence: "graphics", action }
  }
  if (action) return { id: "scripted", label: "Scripted object", evidence: "script", action }
  return { id: "scenery", label: "Scenery", evidence: "fallback", action: null }
}
