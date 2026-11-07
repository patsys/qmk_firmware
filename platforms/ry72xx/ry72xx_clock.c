// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ry72xx_clock.h"
#include <stddef.h>

/* Override for an offline register model. Registers without a documented
 * meaning stay plain addresses; the access order matches the stock firmware.
 */
#ifndef RY72XX_REG32
#    define RY72XX_REG32(address) (*(volatile uint32_t *)(uintptr_t)(address))
#endif

/* Clock bookkeeping. field_08 and field_0c are written as in the stock
 * firmware; their meaning is unknown.
 */
static volatile struct {
    uint32_t source_hz;
    uint32_t effective_hz;
    uint32_t field_08;
    uint32_t field_0c;
    uint32_t factory_word;
} ry72xx_clock_state = {RY72XX_NOMINAL_CLOCK_HZ, RY72XX_NOMINAL_CLOCK_HZ, 0x2000, 0x2000, 0};

uint32_t ry72xx_clock_source_hz(void) {
    return ry72xx_clock_state.source_hz;
}
uint32_t ry72xx_clock_effective_hz(void) {
    return ry72xx_clock_state.effective_hz;
}
uint32_t ry72xx_clock_factory_word(void) {
    return ry72xx_clock_state.factory_word;
}

uint32_t ry72xx_shared_startup_enable(void (*delay_units)(uint32_t), int (*delay_long)(uint32_t)) {
    if (delay_units == NULL || delay_long == NULL) return 0;
    (void)ry72xx_clock_gate_write(UINT32_C(0x80000), true, delay_units);
    (void)delay_long(10);
    RY72XX_REG32(0x40015000u) = 1;
    uint32_t status         = 0;
    for (unsigned attempt = 0; attempt <= 100u; ++attempt) {
        status = RY72XX_REG32(0x40015038u);
        if (status != 0) break;
    }
    return status;
}

/* Select the 48 MHz RC oscillator from factory trim entry 4 (0x10000234):
 * bits 6:4 frequency select (24 MHz + 4 MHz * n) into 0x4001900C bits 10:8,
 * bits 23:16 fine and 27:24 coarse trim into 0x40019010. Then set the core
 * interrupt priorities. Deliberately not a generic frequency-selection API.
 */
bool ry72xx_clock_calibrate(void (*delay_units)(uint32_t), int (*delay_long)(uint32_t)) {
    if (delay_units == NULL || delay_long == NULL) return false;
    ry72xx_clock_state.field_08 = 0x2000;
    ry72xx_clock_state.field_0c = 0x2000;
    uint32_t value            = RY72XX_REG32(0x4001900cu) & ~UINT32_C(0x700);
    value |= (RY72XX_REG32(0x10000234u) & UINT32_C(0x70)) << 4;
    RY72XX_REG32(0x4001900cu) = value;
    delay_units(10);
    value = (RY72XX_REG32(0x10000234u) << 8) >> 24;
    value |= (RY72XX_REG32(0x10000234u) >> 8) & UINT32_C(0xf0000);
    RY72XX_REG32(0x40019010u) = value;
    if (!delay_long(1)) return false;
    value                         = RY72XX_REG32(0x10000234u);
    ry72xx_clock_state.factory_word = value;
    /* ARM32 (word<<25)>>29 extracts bits4..6, including bit6. */
    ry72xx_clock_state.source_hz    = UINT32_C(24000000) + UINT32_C(4000000) * ((value << 25) >> 29);
    RY72XX_REG32(0x40014018u)       = 0; /* clock divider 0 */
    ry72xx_clock_state.effective_hz = ry72xx_clock_state.source_hz;
    /* SysTick priority 2, external IRQ 2 priority 3, USB IRQ 4 priority 1.
     * The two implemented priority bits are the high bits of each byte.
     */
    RY72XX_REG32(0xe000ed20u) = (RY72XX_REG32(0xe000ed20u) & ~UINT32_C(0xff000000)) | UINT32_C(0x80000000);
    RY72XX_REG32(0xe000e400u) = (RY72XX_REG32(0xe000e400u) & ~UINT32_C(0x00ff0000)) | UINT32_C(0x00c00000);
    RY72XX_REG32(0xe000e404u) = (RY72XX_REG32(0xe000e404u) & ~UINT32_C(0xff)) | UINT32_C(0x40);
    return true;
}

/* System clock configuration: 0x40014050 fields, applied with update bit 24.
 * Keep the order; repeated writes may have side effects. */
void ry72xx_clock_setup(void) {
    RY72XX_REG32(0x40014028u) = 0;
    RY72XX_REG32(0x40014060u) = (RY72XX_REG32(0x40014060u) & ~2u) | 2u;
    RY72XX_REG32(0x40014050u) = RY72XX_REG32(0x40014050u) | 0x01000000u;
    RY72XX_REG32(0x4001404cu) = 0;
    RY72XX_REG32(0x40014050u) = (RY72XX_REG32(0x40014050u) & ~0x1fu) | 0x11u;
    RY72XX_REG32(0x40014050u) = RY72XX_REG32(0x40014050u) | 0x01000000u;
    RY72XX_REG32(0x40014050u) = (RY72XX_REG32(0x40014050u) & ~0xe0u) | 0x40u;
    RY72XX_REG32(0x40014050u) = (RY72XX_REG32(0x40014050u) & ~0x1f00u) | 0x1100u;
    RY72XX_REG32(0x40014050u) = RY72XX_REG32(0x40014050u) | 0x01000000u;
    RY72XX_REG32(0x40014050u) = (RY72XX_REG32(0x40014050u) & ~0xe000u) | 0x4000u;
}

/* Clock source select (0x4001404C) after setting update bit 24 of 0x40014050. */
void ry72xx_clock_domain_select(uint32_t value) {
    RY72XX_REG32(0x40014050u) = RY72XX_REG32(0x40014050u) | 0x01000000u;
    RY72XX_REG32(0x4001404cu) = value;
}

/* Peripheral clock enable (0x4001402C set, 0x40014030 clear) and a short
 * settle delay; the caller supplies the calibrated delay.
 */
bool ry72xx_clock_gate_write(uint32_t mask, bool enable, void (*delay_unit_count)(uint32_t)) {
    if (delay_unit_count == NULL) return false;
    if (enable)
        RY72XX_REG32(0x4001402cu) = mask;
    else
        RY72XX_REG32(0x40014030u) = mask;
    delay_unit_count(10);
    return true;
}
