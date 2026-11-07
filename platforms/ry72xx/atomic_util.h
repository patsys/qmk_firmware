// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include <stdint.h>
static inline uint32_t ry72xx_irq_save_disable(void) {
    uint32_t previous;
    __asm__ volatile("mrs %0, primask\ncpsid i" : "=r"(previous)::"memory");
    return previous;
}
static inline void ry72xx_irq_restore(uint32_t *previous) {
    __asm__ volatile("msr primask, %0" ::"r"(*previous) : "memory");
}
#define ATOMIC_RESTORESTATE 0
#define ATOMIC_FORCEON 1
#define ATOMIC_BLOCK(type) for (uint32_t ry72xx_irq_state __attribute__((cleanup(ry72xx_irq_restore))) = ((type) ? (ry72xx_irq_save_disable(), 0U) : ry72xx_irq_save_disable()), ry72xx_irq_once = 1; ry72xx_irq_once; ry72xx_irq_once = 0)
#define ATOMIC_BLOCK_RESTORESTATE ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#define ATOMIC_BLOCK_FORCEON ATOMIC_BLOCK(ATOMIC_FORCEON)
