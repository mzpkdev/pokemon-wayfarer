#include <stdio.h>
#include "output_cap.h"

struct FitCase
{
    size_t buffered;
    size_t line;
    size_t cap;
    bool expected;
};

struct PidCase
{
    const char *name;
    bool expected;
    pid_t pid;
};

int main(void)
{
    static const struct FitCase fits[] = {
        { 0, 10, 100, true },
        { 90, 10, 100, true },
        { 91, 10, 100, false },
        { 100, 0, 100, true },
        { 100, 1, 100, false },
        { 150, 1, 100, false },
        { 0, 101, 100, false },
        { 0, (size_t)-1, 100, false },
    };
    static const struct PidCase pids[] = {
        { "mgba-rom-test-hydra-1234-AbC123", true, 1234 },
        { "mgba-rom-test-hydra-1-x", true, 1 },
        { "mgba-rom-test-hydra-0jtM6E", false, 0 },
        { "mgba-rom-test-hydra-1234", false, 0 },
        { "mgba-rom-test-hydra-1234-", false, 0 },
        { "mgba-rom-test-hydra--AbC123", false, 0 },
        { "mgba-rom-test-hydra-0-AbC123", false, 0 },
        { "mgba-rom-test-hydra-99999999999-AbC123", false, 0 },
        { "other-1234-AbC123", false, 0 },
    };
    int failed = 0;

    for (size_t i = 0; i < sizeof(fits) / sizeof(fits[0]); i++)
    {
        bool actual = output_line_fits(fits[i].buffered, fits[i].line, fits[i].cap);
        if (actual != fits[i].expected)
        {
            fprintf(stderr, "fit case %zu: expected %d, got %d\n", i, fits[i].expected, actual);
            failed = 1;
        }
    }
    for (size_t i = 0; i < sizeof(pids) / sizeof(pids[0]); i++)
    {
        pid_t pid = 0;
        bool actual = parse_rom_dir_pid(pids[i].name, &pid);
        if (actual != pids[i].expected || (actual && pid != pids[i].pid))
        {
            fprintf(stderr, "pid case %s: expected %d/%d, got %d/%d\n", pids[i].name, pids[i].expected, (int)pids[i].pid, actual, (int)pid);
            failed = 1;
        }
    }
    return failed;
}
