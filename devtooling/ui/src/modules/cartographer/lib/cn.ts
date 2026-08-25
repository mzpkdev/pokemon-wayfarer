import { clsx, type ClassValue } from "clsx"
import { twMerge } from "tailwind-merge"

/** Compose conditional Tailwind classes and resolve conflicting utilities. */
export const cn = (...inputs: ClassValue[]): string => {
  return twMerge(clsx(inputs))
}
