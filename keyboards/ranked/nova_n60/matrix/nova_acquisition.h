// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include "nova_matrix_map.h"

typedef struct {
    uint32_t (*read32)(uintptr_t address, void *context);
    void (*write32)(uintptr_t address, uint32_t value, void *context);
    void (*delay_units)(uint32_t units, void *context);
    void *context;
} nova_scan_io;

typedef struct {
    nova_scan_io io;
    uint16_t     samples[NOVA_BANKS][NOVA_CHANNELS];
    uint16_t     channel_bank_bits[NOVA_CHANNELS];
    bool         initialized;
} nova_acquisition;

/* Analog matrix acquisition (no flash controller access). The CPU clock and
 * reset setup must already have completed. */
bool nova_acquisition_init(nova_acquisition *state, const nova_scan_io *io);
/* Cold initialization: long delay 100 before GPIO setup and 300 after it.
 * Callback units are ry72xx_delay_long units, not milliseconds. */
bool     nova_acquisition_init_settled(nova_acquisition *state, const nova_scan_io *io, bool (*delay_long)(uint32_t, void *));
bool     nova_acquisition_scan(nova_acquisition *state);
void     nova_acquisition_rows(const nova_acquisition *state, uint16_t rows[NOVA_BANKS]);
uint32_t nova_scan_mmio_read32(uintptr_t address, void *context);
void     nova_scan_mmio_write32(uintptr_t address, uint32_t value, void *context);
