#include "pic.h"
#include "port.h"

void pic_remap(int o1, int o2) {

    // store previous masks
    uint8_t m1 = inb(PIC1_DATA_PORT);
    uint8_t m2 = inb(PIC2_DATA_PORT);

    outb(PIC1_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA_PORT, o1);
    io_wait();
    outb(PIC2_DATA_PORT, o2);
    io_wait();

    outb(PIC1_DATA_PORT, 0x04);
    io_wait();
    outb(PIC1_DATA_PORT, 0x02);
    io_wait();

    outb(PIC1_DATA_PORT, ICW4_8086);
    io_wait();
    outb(PIC2_DATA_PORT, ICW4_8086);
    io_wait();

    outb(PIC1_DATA_PORT, m1);
    outb(PIC2_DATA_PORT, m2);
}

void pic_disable() {
    outb(PIC1_DATA_PORT, 0xFF);
    outb(PIC2_DATA_PORT, 0xFF);
}

void pic_send_eoi(int irq) {
    if (irq >= 8)
        outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void pic_mask(int irq) {
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

void pic_unmask(int irq) {
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

void pic_mask_all() {
    for (int i = 0; i < 15; i++)
        pic_mask(i);
}

void pic_unmask_all() {
    for (int i = 0; i < 15; i++)
        pic_unmask(i);
}
