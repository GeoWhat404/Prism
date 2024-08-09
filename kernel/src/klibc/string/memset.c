#include <string.h>
#include <stdint.h>

#define LONG_MASK (sizeof(unsigned long) - 1)
void memset(void *_dst, int val, size_t len) {
  unsigned char *dst = _dst;
  unsigned long *ldst;
  unsigned long  lval =
      (val & 0xFF) *
      (-1ul /
       255);

  if (len >= 16)
  {
    while ((uintptr_t)dst & LONG_MASK) {
      *dst++ = val;
      len--;
    }
    ldst = (void *)dst;
    while (len > sizeof(long)) {
      *ldst++ = lval;
      len -= sizeof(long);
    }
    dst = (void *)ldst;
  }
  while (len--)
    *dst++ = val;
}

