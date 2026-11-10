// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

/* QMK CUSTOM_MATRIX=lite integration of the analog acquisition. */
#include "matrix.h"
#include "bootloader.h"
#include "ry72xx_delay.h"
#include "nova_acquisition.h"

static nova_acquisition acquisition;
static void             scan_delay(uint32_t units, void *context) {
    (void)context;
    ry72xx_delay_units(units);
}
static bool scan_settle(uint32_t units, void *context) {
    (void)context;
    return ry72xx_delay_long(units) != 0;
}
void matrix_init_custom(void) {
    const nova_scan_io io = {
        .read32      = nova_scan_mmio_read32,
        .write32     = nova_scan_mmio_write32,
        .delay_units = scan_delay,
        .context     = 0,
    };
    // Without a settled acquisition no key can be read; hand over to the bootloader.
    if (!nova_acquisition_init_settled(&acquisition, &io, scan_settle)) bootloader_jump();
}
#ifdef RANKED_DIAGNOSTICS
/* Last raw analog sample of a matrix position (0 if absent or out of range). */
uint16_t nova_matrix_sample(unsigned bank, unsigned channel) {
    if (!nova_position_present(bank, channel)) return 0;
    return acquisition.samples[bank][channel];
}
#endif
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    if (!nova_acquisition_scan(&acquisition)) return false;
    uint16_t rows[NOVA_BANKS];
    nova_acquisition_rows(&acquisition, rows);
    bool changed = false;
    for (unsigned bank = 0; bank < NOVA_BANKS; ++bank) {
        if (current_matrix[bank] != rows[bank]) changed = true;
        current_matrix[bank] = rows[bank];
    }
    return changed;
}
