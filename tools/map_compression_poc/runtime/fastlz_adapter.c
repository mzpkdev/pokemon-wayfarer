/* Wrapper is intentionally the production FastLZ77UnCompWram body. */
#include <stdint.h>

typedef uint32_t u32;
extern const u32 LZ77UnCompWRAMOptimized[];
extern const u32 LZ77UnCompWRAMOptimized_end[];

extern void FastUnsafeCopy32(void *, const void *, uint32_t size);
static void copy_func(void *to, const void *from, uint32_t bytes)
{
    FastUnsafeCopy32(to, from, bytes);
}

__attribute__((target("arm"))) static void SwitchToArmCallFastLZ77(const u32 *src, void *dest, void (*funcPtr)(const u32 *src, void *dest))
{
    funcPtr(src, dest);
}

void FastLZ77UnCompWram(const u32 *src, void *dest)
{
    u32 funcBuffer[200];
    copy_func(funcBuffer, LZ77UnCompWRAMOptimized,
              (const uint8_t *)LZ77UnCompWRAMOptimized_end - (const uint8_t *)LZ77UnCompWRAMOptimized);
    SwitchToArmCallFastLZ77(src, dest, (void *)funcBuffer);
}
