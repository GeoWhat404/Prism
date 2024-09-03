#include "elf.h"

int elf_is_hdr_valid(char *hdr) {
    return (hdr[0] == 0x7F && hdr[1] == 'E' &&
            hdr[2] == 'L' && hdr[3] == 'F' &&
            hdr[4] == 2);
}
