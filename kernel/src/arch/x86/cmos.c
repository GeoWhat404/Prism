#include "cmos.h"
#include "port.h"

bool cmos_is_busy() {
    // read from status register A and
    // disable NMIs by setting the 0x80 bit
    outb(CMOS_ADDR_REG, 0x8A);

    // if register A's top bit is set
    // CMOS is updating (busy)
    return (inb(CMOS_DATA_REG) & 0x80);
}
