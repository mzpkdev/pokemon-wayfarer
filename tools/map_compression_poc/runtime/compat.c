/* Real Thumb BIOS calls, as used by production CpuCopy16/CpuFastFill16. */
#include <stdint.h>
__attribute__((naked)) void CpuSet(const void *src, void *dest, uint32_t control)
{
    __asm__ volatile("swi 0x0b\n bx lr");
}
__attribute__((naked)) void CpuFastSet(const void *src, void *dest, uint32_t control)
{
    __asm__ volatile("swi 0x0c\n bx lr");
}
void *memcpy(void *dst, const void *src, unsigned n)
{
    unsigned char *d=dst; const unsigned char *s=src;
    while(n--) *d++=*s++;
    return dst;
}
void *memset(void *dst, int value, unsigned n)
{
    unsigned char *d=dst;
    while(n--) *d++=value;
    return dst;
}
void AssertfCrashScreen(const void *return0, const char *fmt, ...)
{
    (void)return0; (void)fmt;
    register unsigned code __asm__("r0")=99;
    __asm__ volatile("swi 3"::"r"(code));
    for (;;) {}
}
