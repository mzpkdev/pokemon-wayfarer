#ifndef HYDRA_OUTPUT_CAP_H
#define HYDRA_OUTPUT_CAP_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

// A test that prints in a loop and never reports a result would otherwise grow
// its runner's buffer until the machine runs out of memory.
#define HYDRA_OUTPUT_CAP (1024 * 1024)

#define HYDRA_ROM_DIR_PREFIX "mgba-rom-test-hydra-"

// Whole lines only: a line is kept when it fits under the cap, else dropped.
static inline bool output_line_fits(size_t buffered, size_t line, size_t cap)
{
    return buffered <= cap && line <= cap - buffered;
}

// Parses the owning pid out of "mgba-rom-test-hydra-<pid>-XXXXXX".
static inline bool parse_rom_dir_pid(const char *name, pid_t *pid)
{
    size_t prefix = strlen(HYDRA_ROM_DIR_PREFIX);
    long value = 0;
    const char *c;

    if (strncmp(name, HYDRA_ROM_DIR_PREFIX, prefix) != 0)
        return false;
    c = name + prefix;
    if (*c < '0' || *c > '9')
        return false;
    for (; *c >= '0' && *c <= '9'; c++)
    {
        value = value * 10 + (*c - '0');
        if (value > 0x7FFFFFFF)
            return false;
    }
    if (*c != '-' || c[1] == '\0' || value == 0)
        return false;
    *pid = (pid_t)value;
    return true;
}

#endif
