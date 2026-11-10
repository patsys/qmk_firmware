// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* Analog scan: 6 banks (matrix rows) x 11 channels (matrix columns). */
#define NOVA_BANKS 6
#define NOVA_CHANNELS 11

/* Settle time of one analog bank conversion, in ry72xx_delay units. */
#define NOVA_SCAN_CONVERSION_SETTLE_UNITS 20

/* The flash controller programs 32-bit words. */
#define BACKING_STORE_WRITE_SIZE 4
