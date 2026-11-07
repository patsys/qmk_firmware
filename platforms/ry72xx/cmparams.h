// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
/* Cortex-M0 r0p0 (CPUID 0x410CC200), two implemented priority bits.
 * The RY7203 vector table has 22 external IRQ entries; ChibiOS rounds the
 * table size to a multiple of eight, so entries 22/23 are unused.
 */
#define CORTEX_MODEL 0
#define CORTEX_HAS_FPU 0
#define CORTEX_PRIORITY_BITS 2U
#define CORTEX_NUM_VECTORS 24
#define CORTEX_MPU_REGIONS 0
