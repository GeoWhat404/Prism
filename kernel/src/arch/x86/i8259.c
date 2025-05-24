#include "i8259.h"
#include "port.h"
#include <util/debug.h>

void i8259_set_mask(uint16_t new_mask);

void i8259_send_command(uint16_t data1, uint16_t data2) {
    outb(PIC1_COMMAND_PORT, data1);
    io_wait();
    outb(PIC2_COMMAND_PORT, data2);
    io_wait();
}

void i8259_send_data(uint16_t data1, uint16_t data2) {
    outb(PIC1_DATA_PORT, data1);
    io_wait();
    outb(PIC2_DATA_PORT, data2);
    io_wait();
}

void i8259_remap(int offset) {
    i8259_set_mask(0xFFFF);

    // initialization control word 1
    i8259_send_command(ICW1_ICW4 | ICW1_INIT,
                     ICW1_ICW4 | ICW1_INIT);

    // initialization control word 2 (offsets)
    i8259_send_data(offset, offset + 0x08);


    // initialization control word 3
    // tell PIC1 that it has a slave at IRQ2
    // tell PIC2 its cascade identity
    i8259_send_data(0x04, 0x02);

    // initialization control word 4
    i8259_send_data(ICW4_8086, ICW4_8086);
    i8259_send_data(ICW4_8086, ICW4_8086);

    i8259_set_mask(0xFFFF);
}

void i8259_disable() {
    outb(PIC1_DATA_PORT, 0xFF);
    outb(PIC2_DATA_PORT, 0xFF);
}

void i8259_send_eoi(int irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND_PORT, PIC_EOI);
    outb(PIC1_COMMAND_PORT, PIC_EOI);
}

void i8259_set_mask(uint16_t new_mask) {
    outb(PIC1_DATA_PORT, new_mask & 0xFF);
    outb(PIC2_DATA_PORT, new_mask >> 8);
}

uint16_t i8259_get_mask() {
    return inb(PIC1_DATA_PORT) | (inb(PIC2_DATA_PORT) << 8);
}

void i8259_mask(int irq) {
    uint16_t port;
    uint8_t val;

    if (irq < 8) {
        port = PIC1_DATA_PORT;
    } else {
        port = PIC2_DATA_PORT;
        irq -= 8;
    }
    val = inb(port) | (1 << irq);
    outb(port, val);
}

void i8259_unmask(int irq) {
    uint16_t port;
    uint8_t val;

    if (irq < 8) {
        port = PIC1_DATA_PORT;
    } else {
        port = PIC2_DATA_PORT;
        irq -= 8;
    }
    val = inb(port) & ~(1 << irq);
    outb(port, val);
}

void i8259_mask_all() {
    for (int i = 0; i < 15; i++)
        i8259_mask(i);
}

void i8259_unmask_all() {
    for (int i = 0; i < 15; i++)
        i8259_unmask(i);
}

bool i8259_probe() {
    i8259_disable();
    i8259_set_mask(0x1337);
    return i8259_get_mask() == 0x1337;
}
