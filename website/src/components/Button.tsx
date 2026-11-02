import type { ComponentProps } from "react"

import { cn } from "~/lib/cn"

type ButtonProps = ComponentProps<"button"> & {
  variant?: "solid" | "ghost"
}

export const Button = ({ variant = "solid", className, ...props }: ButtonProps) => (
  <button
    type="button"
    className={cn(
      "inline-flex items-center justify-center rounded-md px-4 py-2 text-sm font-medium transition-colors focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-wayfarer-500",
      variant === "solid" && "bg-wayfarer-500 text-white hover:bg-wayfarer-700",
      variant === "ghost" && "text-wayfarer-50 hover:bg-white/10",
      className,
    )}
    {...props}
  />
)
