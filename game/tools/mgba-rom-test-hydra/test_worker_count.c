#include <stdio.h>
#include "worker_count.h"

struct WorkerCase
{
    const char *makeflags;
    long availableCpus;
    unsigned expected;
};

int main(void)
{
    static const struct WorkerCase cases[] = {
        { NULL, 16, 8 },
        { "", 16, 8 },
        { "--jobserver-auth=3,4", 16, 8 },
        { NULL, 4, 4 },
        { NULL, 1, 1 },
        { NULL, 0, 1 },
        { NULL, -1, 1 },
        { "-j1", 16, 1 },
        { "-j6 --jobserver-auth=3,4", 2, 6 },
        { "-j 6", 16, 6 },
        { "--jobs=3", 16, 3 },
        { "--jobs 3", 16, 3 },
        { "-j", 12, 12 },
        { "-j", 64, 32 },
        { "-j", -1, 1 },
        { "-j64", 2, 32 },
        { "-j0", 12, 12 },
        { "-j99999999999999999999", 12, 32 },
        { "-jfoo", 16, 8 },
        { "-j4 -j1", 16, 1 },
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
    {
        unsigned actual = select_worker_count(cases[i].makeflags, cases[i].availableCpus);
        if (actual != cases[i].expected)
        {
            fprintf(stderr, "worker case %zu: expected %u, got %u\n", i, cases[i].expected, actual);
            return 1;
        }
    }
    puts("worker selection: 20 cases passed");
    return 0;
}
