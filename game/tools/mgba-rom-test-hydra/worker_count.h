#ifndef HYDRA_WORKER_COUNT_H
#define HYDRA_WORKER_COUNT_H

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define HYDRA_MAX_WORKERS 32
#define HYDRA_DEFAULT_WORKERS 8

static inline bool parse_worker_limit(const char *digits, size_t length, unsigned *limit)
{
    unsigned value = 0;

    if (length == 0)
        return false;
    for (size_t i = 0; i < length; i++)
    {
        if (!isdigit((unsigned char)digits[i]))
            return false;
        if (value < HYDRA_MAX_WORKERS)
        {
            value = value * 10 + (digits[i] - '0');
            if (value > HYDRA_MAX_WORKERS)
                value = HYDRA_MAX_WORKERS;
        }
    }
    *limit = value;
    return true;
}

static inline unsigned select_worker_count(const char *makeflags, long available_cpus)
{
    unsigned available = 1;
    if (available_cpus > 0)
        available = (unsigned long)available_cpus > HYDRA_MAX_WORKERS ? HYDRA_MAX_WORKERS : (unsigned)available_cpus;
    unsigned workers = available < HYDRA_DEFAULT_WORKERS ? available : HYDRA_DEFAULT_WORKERS;

    if (makeflags == NULL)
        return workers;
    for (const char *p = makeflags; *p; )
    {
        while (isspace((unsigned char)*p))
            p++;
        if (*p == '\0')
            break;
        const char *token = p;
        while (*p && !isspace((unsigned char)*p))
            p++;
        size_t length = p - token;
        const char *digits = NULL;
        size_t digit_count = 0;
        unsigned limit;

        if (length >= 2 && token[0] == '-' && token[1] == 'j')
        {
            digits = token + 2;
            digit_count = length - 2;
        }
        else if (length >= 7 && !strncmp(token, "--jobs=", 7))
        {
            digits = token + 7;
            digit_count = length - 7;
        }
        else if (length == 6 && !strncmp(token, "--jobs", 6))
        {
            digits = token + 6;
        }
        if (digits == NULL)
            continue;
        if (digit_count == 0)
        {
            const char *next = p;
            while (isspace((unsigned char)*next))
                next++;
            const char *end = next;
            while (*end && !isspace((unsigned char)*end))
                end++;
            if (parse_worker_limit(next, end - next, &limit))
                workers = limit == 0 ? available : limit;
            else
                workers = available;
        }
        else if (parse_worker_limit(digits, digit_count, &limit))
        {
            workers = limit == 0 ? available : limit;
        }
    }
    return workers;
}

#endif
