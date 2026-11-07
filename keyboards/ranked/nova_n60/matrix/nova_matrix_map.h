// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

/* Evidence-backed pure scan helpers, not a complete QMK hardware driver. */
#pragma once
#include <stdint.h>
#include <stdbool.h>

enum { NOVA_BANKS = 6, NOVA_CHANNELS = 11, NOVA_ABSENT_ID = 1 };
extern const uint8_t   nova_bank_pin_codes[NOVA_BANKS];
extern const uint8_t   nova_channel_pin_codes[NOVA_CHANNELS];
extern const uint8_t   nova_key_ids[NOVA_CHANNELS][NOVA_BANKS];
extern const uintptr_t nova_pin_config_bases[4];

/* Fixed press/release thresholds (above 30 pressed, below 20 released). */
uint16_t  nova_hysteresis(uint16_t bank_bits, unsigned bank, uint16_t sample);
bool      nova_position_present(unsigned bank, unsigned channel);
uintptr_t nova_result_address(unsigned channel);
uintptr_t nova_pin_config_address(uint8_t encoded_pin);
uintptr_t nova_pin_write10_address(uint8_t encoded_pin);
uintptr_t nova_pin_write20_address(uint8_t encoded_pin);
uint32_t  nova_pin_mask(uint8_t encoded_pin);
/* Exact arithmetic at 0xeaec; caller must validate SysTick range/clock source. */
uint32_t nova_systick_reload(uint32_t nominal_clock_hz, uint32_t period_argument);

/* These functions perform no MMIO. A future platform must provide verified
 * clock/reset, acquisition startup, timing, USB, and matrix/event integration.
 * GPIO identifiers are encoded IDs, not chip-package pin numbers. */
