#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PIC1_COMMAND_PORT   0x20
#define PIC1_DATA_PORT      0x21
#define PIC2_COMMAND_PORT   0xA0
#define PIC2_DATA_PORT      0xA1

#define ICW1_ICW4           0x01
#define ICW1_SINGLE         0x02
#define ICW1_INTERVAL_4     0x04
#define ICW1_LEVEL          0x08
#define ICW1_INIT           0x10

#define ICW4_8086           0x01
#define ICW4_AUTO           0x02
#define ICW4_BUF_SLAVE      0x08
#define ICW4_BUF_MASTER     0x0C
#define ICW4_SFNM           0x10

#define PIC_REMAP_OFFSET    0x20
#define PIC_EOI             0x20

void i8259_remap(int offset);
void i8259_disable(void);
void i8259_send_eoi(int irq);

void i8259_set_mask(uint16_t new_mask);
uint16_t i8259_get_mask(void);

void i8259_mask(int irq);
void i8259_unmask(int irq);
void i8259_mask_all(void);
void i8259_unmask_all(void);

bool i8259_probe(void);
