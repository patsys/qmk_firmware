// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define RY72XX_FLASH_BACKING_BYTES 4096u
#define RY72XX_FLASH_WRITE_BYTES 4u

static inline bool ry72xx_flash_address_valid(uint32_t address) {
    return address <= RY72XX_FLASH_BACKING_BYTES - RY72XX_FLASH_WRITE_BYTES && (address % RY72XX_FLASH_WRITE_BYTES) == 0u;
}
