#include <stdlib.h>
#include <hal/panic.h>

void abort() {
    panic("abort()");
}

