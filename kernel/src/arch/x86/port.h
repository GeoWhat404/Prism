#pragma once

#include <stdint.h>

#define UNUSED_PORT 0x80

void outb(uint16_t port, uint8_t byte);
uint8_t inb(uint16_t port);

void io_wait();
