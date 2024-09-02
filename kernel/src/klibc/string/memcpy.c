#include <string.h>
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t num) {
    uint8_t *u8_dst = (uint8_t *) dst;
    const uint8_t  *u8_src = (const uint8_t *) src;

    for (uint16_t i = 0; i < num; i++) {
        u8_dst[i] = u8_src[i];
    }

    return dst;
}
