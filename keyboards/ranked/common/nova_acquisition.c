// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include "nova_acquisition.h"
#ifndef NOVA_SCAN_CONVERSION_SETTLE_UNITS
#    define NOVA_SCAN_CONVERSION_SETTLE_UNITS 5
#endif

uint32_t nova_scan_mmio_read32(uintptr_t address, void *context) {
    (void)context;
    return *(volatile uint32_t *)address;
}
void nova_scan_mmio_write32(uintptr_t address, uint32_t value, void *context) {
    (void)context;
    *(volatile uint32_t *)address = value;
}
static void write(nova_acquisition *s, uintptr_t address, uint32_t value) {
    s->io.write32(address, value, s->io.context);
}
static void delay(nova_acquisition *s, uint32_t units) {
    s->io.delay_units(units, s->io.context);
}
static void configure(nova_acquisition *s, uint8_t pin, uint32_t value) {
    const uintptr_t address = nova_pin_config_address(pin);
    if (address) write(s, address, value);
}
static void pin_write10(nova_acquisition *s, uint8_t pin) {
    write(s, nova_pin_write10_address(pin), nova_pin_mask(pin));
}
static void pin_write20(nova_acquisition *s, uint8_t pin) {
    write(s, nova_pin_write20_address(pin), nova_pin_mask(pin));
}
static void analog_pins(nova_acquisition *s, uint32_t config) {
    for (unsigned channel = 0; channel < NOVA_CHANNELS; ++channel)
        configure(s, nova_channel_pin_codes[channel], config);
}
static bool init(nova_acquisition *s, const nova_scan_io *io, bool (*settle)(uint32_t, void *)) {
    if (!s || !io || !io->read32 || !io->write32 || !io->delay_units) return false;
    s->io          = *io;
    s->initialized = false;
    for (unsigned bank = 0; bank < NOVA_BANKS; ++bank)
        for (unsigned channel = 0; channel < NOVA_CHANNELS; ++channel)
            s->samples[bank][channel] = 0;
    for (unsigned channel = 0; channel < NOVA_CHANNELS; ++channel)
        s->channel_bank_bits[channel] = 0;
    /* Wait after clock calibration, before GPIO setup. */
    if (settle && !settle(100, s->io.context)) return false;

    /* GPIO setup: two auxiliary pins (purpose undocumented), the bank drive
     * pins, then the analog channel pins. */
    configure(s, 0x45, 1);
    configure(s, 0x41, 1);
    pin_write20(s, 0x41);
    pin_write20(s, 0x45);
    for (unsigned bank = 0; bank < NOVA_BANKS; ++bank)
        configure(s, nova_bank_pin_codes[bank], 1);
    for (unsigned bank = 0; bank < NOVA_BANKS; ++bank)
        pin_write20(s, nova_bank_pin_codes[bank]);
    analog_pins(s, 0x32);

    /* Let the external circuitry settle before acquisition setup. */
    if (settle && !settle(300, s->io.context)) return false;

    /* 0x31b4 -> 0xd524(0x800,1), then channel and acquisition setup. */
    write(s, 0x4001402cu, 0x800);
    delay(s, 10);
    for (unsigned channel = 0; channel < NOVA_CHANNELS; ++channel) {
        const uint32_t enabled = s->io.read32(0x4000e00cu, s->io.context);
        write(s, 0x4000e00cu, enabled | (1u << channel));
    }
    write(s, 0x4000e084u, 1);
    write(s, 0x4000e098u, 15);
    write(s, 0x4000e080u, 16);
    write(s, 0x4000e008u, 14);
    s->initialized = true;
    return true;
}
bool nova_acquisition_init(nova_acquisition *s, const nova_scan_io *io) {
    return init(s, io, 0);
}
bool nova_acquisition_init_settled(nova_acquisition *s, const nova_scan_io *io, bool (*delay_long)(uint32_t, void *)) {
    if (!delay_long) return false;
    return init(s, io, delay_long);
}
bool nova_acquisition_scan(nova_acquisition *s) {
    if (!s || !s->initialized) return false;
    analog_pins(s, 0x32);
    write(s, 0x4000e000u, 1);
    for (unsigned bank = 0; bank < NOVA_BANKS; ++bank) {
        const uint8_t pin = nova_bank_pin_codes[bank];
        pin_write10(s, pin);
        delay(s, 10);
        write(s, 0x4000e014u, 1);
        delay(s, NOVA_SCAN_CONVERSION_SETTLE_UNITS);
        pin_write20(s, pin);
        for (unsigned channel = 0; channel < NOVA_CHANNELS; ++channel) {
            if (!nova_position_present(bank, channel)) continue;
            const uint16_t sample         = (uint16_t)s->io.read32(nova_result_address(channel), s->io.context);
            s->samples[bank][channel]     = sample;
            s->channel_bank_bits[channel] = nova_hysteresis(s->channel_bank_bits[channel], bank, sample);
        }
    }
    write(s, 0x4000e000u, 0);
    analog_pins(s, 2);
    return true;
}
void nova_acquisition_rows(const nova_acquisition *s, uint16_t rows[NOVA_BANKS]) {
    for (unsigned bank = 0; bank < NOVA_BANKS; ++bank) {
        uint16_t row = 0;
        for (unsigned channel = 0; channel < NOVA_CHANNELS; ++channel)
            if (nova_position_present(bank, channel) && (s->channel_bank_bits[channel] & (1u << bank))) row |= (uint16_t)(1u << channel);
        rows[bank] = row;
    }
}
