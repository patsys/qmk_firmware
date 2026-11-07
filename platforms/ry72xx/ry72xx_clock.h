// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include <stdbool.h>
#include <stdint.h>

/* RY7203 clock helpers. The core runs at 48 MHz from the factory-trimmed
 * RC oscillator.
 */
#define RY72XX_NOMINAL_CLOCK_HZ UINT32_C(48000000)

void ry72xx_clock_setup(void);
void ry72xx_clock_domain_select(uint32_t value);
/* Select 48 MHz from the factory trim table; see ry72xx_clock.c. Delay
 * callbacks use ry72xx_delay units; the long delay returns nonzero on success.
 * Callers must reject other effective frequencies: delays assume 48 MHz.
 */
bool     ry72xx_clock_calibrate(void (*delay_units)(uint32_t), int (*delay_long)(uint32_t));
uint32_t ry72xx_clock_source_hz(void);
uint32_t ry72xx_clock_effective_hz(void);
uint32_t ry72xx_clock_factory_word(void);
/* Start the block at 0x40015000 (peripheral clock bit 19) and return its
 * ready flag (+0x38) after at most 101 polls. Its role is not documented.
 */
uint32_t ry72xx_shared_startup_enable(void (*delay_units)(uint32_t), int (*delay_long)(uint32_t));
/* The delay callback uses ry72xx_delay units. */
bool ry72xx_clock_gate_write(uint32_t mask, bool enable, void (*delay_unit_count)(uint32_t));
