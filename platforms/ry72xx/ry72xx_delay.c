// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>

extern void ry72xx_delay48_loop(uint32_t count);

/* Busy-wait in calibrated units at 48 MHz (not microseconds); zero is a no-op.
 * Requires the 48 MHz clock setup.
 */
void ry72xx_delay_units(uint32_t count) {
    if (count != 0) ry72xx_delay48_loop(count);
}

/* Long delay: one loop of count << 7 units, then count loops of 999 units.
 * count must be <= 0x01FFFFFF so the shift cannot wrap to zero, which would
 * underflow the loop.
 */
int ry72xx_delay_long(uint32_t count) {
    if (count == 0) return 1;
    if (count > UINT32_C(0x01ffffff)) return 0;
    ry72xx_delay48_loop(count << 7);
    do {
        ry72xx_delay48_loop(999);
    } while (--count != 0);
    return 1;
}
