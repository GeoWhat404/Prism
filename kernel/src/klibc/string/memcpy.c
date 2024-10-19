#include <string.h>
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t num) {
    __asm__ ("rep movsb" : : "D" ((char *)dst), "S"((char *)src), "c"(num) : "memory");
    return dst;
}
