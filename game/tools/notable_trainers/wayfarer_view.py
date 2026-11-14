"""Static views of event scripts for the notable story tests."""


def wayfarer_view(text, wayfarer=True):
    """The text as the Wayfarer build sees it (or, with wayfarer=False, as the other builds do): `#if IS_WAYFARER` /
    `#if !IS_WAYFARER` arms resolved, any other conditional left as written."""
    out, stack = [], []  # stack of (known, branch active, parent active)
    for line in text.splitlines(keepends=True):
        stripped = line.strip()
        active = all(entry[1] for entry in stack)
        if stripped in ("#if IS_WAYFARER", "#if !IS_WAYFARER"):
            stack.append((True, (stripped == "#if IS_WAYFARER") == wayfarer))
        elif stripped.startswith("#if"):
            stack.append((False, True))
            if active:
                out.append(line)
        elif stripped == "#else":
            known, branch = stack[-1]
            if known:
                stack[-1] = (True, not branch)
            elif all(entry[1] for entry in stack):
                out.append(line)
        elif stripped == "#endif":
            known, _ = stack.pop()
            if not known and all(entry[1] for entry in stack):
                out.append(line)
        elif active:
            out.append(line)
    return "".join(out)
