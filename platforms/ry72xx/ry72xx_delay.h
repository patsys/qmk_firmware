// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include <stdint.h>

/* Calibrated busy-wait units of the 48 MHz core (see ry72xx_delay.c). */
void ry72xx_delay_units(uint32_t count);
/* Long delay in units of the clock domain timer; returns 0 if it did not complete. */
int ry72xx_delay_long(uint32_t count);
