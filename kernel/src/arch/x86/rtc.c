#include "irq.h"
#include "port.h"
#include "cmos.h"

#include <stdint.h>
#include <util/debug.h>
#include <util/datetime.h>

#define CURRENT_MILLENIA 2000
#define RTC_DEFAULT_FREQ 1024

static uint16_t ticks = 0;
static datetime_t time;

static uint8_t rtc_get_register(uint8_t reg) {
    // disable NMIs while sending registers
    outb(CMOS_ADDR_REG, reg | 0x80);
    return inb(CMOS_DATA_REG);
}

static void populate_datetime(datetime_t *out) {
    out->seconds    = rtc_get_register(0x00);
    out->minutes    = rtc_get_register(0x02);
    out->hours      = rtc_get_register(0x04);
    out->days       = rtc_get_register(0x07);
    out->months     = rtc_get_register(0x08);
    out->years      = rtc_get_register(0x09);
}

static void calculate_time(void) {
    uint8_t reg_b_val;
    datetime_t new, old;

    while (cmos_is_busy());

    populate_datetime(&new);

    do {
        old = new;

        while (cmos_is_busy());
        populate_datetime(&new);
    } while (
        new.seconds != old.seconds  || new.minutes != old.minutes   ||
        new.hours != old.hours      || new.days != old.days         ||
        new.months != old.months    || new.years != old.years);

    // Convert BCD to binary if necessary (bit 2 is clear)
    if (!(reg_b_val & 0x04)) {
        new.seconds = (new.seconds & 0x0F) + ((new.seconds / 16) * 10);
        new.minutes = (new.minutes & 0x0F) + ((new.minutes / 16) * 10);
        new.hours = ((new.hours & 0x0F) + (((new.hours & 0x70) / 16) * 10)) |
        			(new.hours & 0x80);
        new.days = (new.days & 0x0F) + ((new.days / 16) * 10);
        new.months = (new.months & 0x0F) + ((new.months / 16) * 10);
        new.years = (new.years & 0x0F) + ((new.years / 16) * 10);
    }

    if (!(reg_b_val & 0x02) && (new.hours & 0x80)) {
        new.hours = ((new.hours & 0x7F) + 12) % 24;
    }

    new.years += CURRENT_MILLENIA;
    if (new.years < CURRENT_MILLENIA)
        new.years += 100;

    time = new;
}

static void rtc_callback(registers_t *regs) {
    ticks++;

    __asm__ volatile("cli");

    if (ticks % RTC_DEFAULT_FREQ == 0)
        calculate_time();

    // read register C so more IRQ8s can occur
    rtc_get_register(0x0C);

    __asm__ volatile("sti");
}

void rtc_initialize(void) {
    uint8_t prev_reg_b_val = rtc_get_register(0x0B);

    // select register B
    outb(CMOS_ADDR_REG, 0x8B);

    // 0x40 = 0b100000
    // so enable the 6th bit to enable the periodic interrupts
    // at the default rate of ~1024Hz
    outb(CMOS_DATA_REG, prev_reg_b_val | 0x40);

    // read register C to clear any pending IRQ8 interrupts
    rtc_get_register(0x0C);

    irq_register_handler(IRQ8, rtc_callback);

    calculate_time();
}

datetime_t rtc_get_datetime() {
    return time;
}
